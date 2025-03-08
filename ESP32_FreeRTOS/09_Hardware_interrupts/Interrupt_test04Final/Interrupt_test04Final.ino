#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

static const char command[] = "avg";                                  // Command for displaying the average
static const uint64_t timer_freq = 10000000;                          // Timer frequency (10 MHz)
static const uint64_t timer_max_count = 1000000;                      // Timer maximum count (100 ms period)
static const uint32_t cli_delay = 20;                                 // CLI task delay (20 ms)
enum {BUF_LEN = 10};                                                  // Buffer length for ADC samples
enum {MSG_LEN = 100};                                                 // Message length for CLI
enum {MSG_QUEUE_LEN = 5};                                             // Message queue length
enum {CMD_BUF_LEN = 255};                                             // Command buffer length

static const int adc_pin = 36;                                        // ADC pin to read from

// Message structure for CLI
typedef struct Message {
  char body[MSG_LEN];
} Message;


static hw_timer_t *timer = NULL;                                      // Timer handle
static TaskHandle_t processing_task = NULL;                           // Task handle for average calculation
static SemaphoreHandle_t sem_done_reading = NULL;                     // Semaphore to signal buffer ready for processing
static portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;          // Spinlock for protecting shared variables
static QueueHandle_t msg_queue;                                       // Message queue for CLI
static volatile uint16_t buf_0[BUF_LEN];                              // Buffers for ADC samples (double buffering)
static volatile uint16_t buf_1[BUF_LEN];
static volatile uint16_t *write_to = buf_0;                           // Pointers to the current write and read buffers
static volatile uint16_t *read_from = buf_1;
static volatile uint8_t buf_overrun = 0;                              // Flag for buffer overrun
static float adc_avg;                                                 // Average ADC value

// Function to swap the write and read buffers
void IRAM_ATTR swap(){
  volatile uint16_t *temp_ptr = write_to;
  write_to = read_from;
  read_from = temp_ptr;
}

// Timer interrupt service routine (ISR)
void IRAM_ATTR onTimer(){
  static uint16_t idx = 0;                                             // Index for writing to the current buffer
  BaseType_t task_woken = pdFALSE;                                     // Flag to indicate if a higher priority task was woken
  // Read ADC value and store in the current buffer if not full and no overrun
  if ((idx<BUF_LEN) && (buf_overrun == 0)) {
    write_to[idx] = analogRead(adc_pin);
    idx ++;
  }
  // If the current buffer is full
  if (idx >= BUF_LEN) {
    // If semaphore is not available, buffer overrun occurred
    if (xSemaphoreTakeFromISR(sem_done_reading, &task_woken) == pdFALSE) {
      buf_overrun = 1;                                                      
    }
    // If no buffer overrun occurred
    if (buf_overrun == 0) {
      idx = 0;                                                          // Reset buffer index
      swap();                                                           // Swap buffers
      vTaskNotifyGiveFromISR(processing_task, &task_woken);             // Notify the processing task that a buffer is ready
    }
  }
  // Yield to a higher priority task if one was woken
  if (task_woken) {
    portYIELD_FROM_ISR();
  }
}

// Task for handling command-line interface (CLI)
void doCLI(void *param){
  Message rcv_msg;                                                  // Message received from the processing task
  char c;                                                           // Character read from the serial port
  char cmd_buf[CMD_BUF_LEN];                                        // Buffer for storing the command string
  uint8_t idx = 0;                                                  // Index for writing to the command buffer
  uint8_t cmd_len = strlen(command);                                // Length of the command string

  memset(cmd_buf, 0, CMD_BUF_LEN);                                  // Initialize command buffer

  while (1) {
    // Check for messages from the processing task
    if (xQueueReceive(msg_queue, (void*)&rcv_msg, 0) == pdTRUE) {
      Serial.println(rcv_msg.body);                                 // Print the received message
    }
    // Check for serial input
    if (Serial.available() > 0) {
      c = Serial.read();                                            // Read a character from the serial port
      if (idx < CMD_BUF_LEN - 1) {
        cmd_buf[idx] = c;                                           // Store the character in the command buffer
        idx ++;
      }
      // If a newline or carriage return is received, process the command
      if (c == '\n' || c == '\r') {
        Serial.println();
        cmd_buf[idx - 1] = '\0';                                    // Null-terminate the command string
        // If the command matches the "avg" command
        if (strcmp(cmd_buf, command) == 0) {
          Serial.print("Average : ");
          Serial.println(adc_avg);                                  // Print the calculated average
        }
        memset(cmd_buf, 0, CMD_BUF_LEN);                            // Reset the command buffer
        idx = 0;                                                    // Reset the command buffer index
      }
      //print the character to the serial monitor.
      else {
        Serial.print(c);
      }
    }
    vTaskDelay(cli_delay/portTICK_PERIOD_MS);                       // Delay to prevent serial buffer overload
  }
}

// Task for calculating the average ADC value
void calcAverage(void *param){
  Message msg;                                                      // Message to send to the CLI task
  float avg;                                                        // Variable to store the calculated average
  while (1) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);                        // Wait for notification from the ISR that a buffer is ready
    avg = 0.0;                                                      // Reset the average
    // Calculate the average of the samples in the read buffer
    for (int i = 0; i < BUF_LEN; i++) {
      avg += (float)read_from[i];
    }
    avg /= BUF_LEN;

    // Protect shared variable with spinlock
    portENTER_CRITICAL(&spinlock);
    adc_avg = avg;
    portEXIT_CRITICAL(&spinlock);

    // If a buffer overrun occurred, send an error message to the CLI
    if (buf_overrun == 1) {
      strcpy(msg.body, "ERROR : Buffer overrun. Samples has been dropped");
      xQueueSend(msg_queue, (void *)&msg, 10);
    }

    // Reset the buffer overrun flag and release the semaphore
    portENTER_CRITICAL(&spinlock);
    buf_overrun = 0;
    xSemaphoreGive(sem_done_reading);
    portEXIT_CRITICAL(&spinlock);
  }
}

void setup() {
  Serial.begin(115200);
  vTaskDelay(1000/portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("____FreeRTOS Sample and Process using ISR____");

  sem_done_reading = xSemaphoreCreateBinary();                          // Create binary semaphore for buffer synchronization
  if (sem_done_reading == NULL) {
    Serial.println("Couldn't create one or more semaphore");
    ESP.restart();
  }
  xSemaphoreGive(sem_done_reading);                                     // Give initial semaphore

  msg_queue = xQueueCreate(MSG_QUEUE_LEN, sizeof(Message));             // Create message queue for CLI communication

  // Create tasks for CLI and average calculation
  xTaskCreatePinnedToCore(doCLI, "Do CLI", 2048, NULL, 2, NULL, app_cpu);
  xTaskCreatePinnedToCore(calcAverage, "Calculate Average", 1024, NULL, 1, &processing_task, app_cpu);

  // Configure and start timer
  timer = timerBegin(timer_freq);
  timerAttachInterrupt(timer, &onTimer);
  timerAlarm(timer, timer_max_count, true, 0);

  // Delete setup task
  vTaskDelete(NULL);
}

void loop() {}