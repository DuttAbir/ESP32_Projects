#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

static const uint8_t buf_len = 255;   // Buffer size for serial input.
static const char command[] = "delay ";    // Command to change LED delay.
static const int delay_queue_len = 5;   // Length of the delay queue.
static const int msg_queue_len = 5;   // Length of the message queue.
static const uint8_t blink_max = 100;   // Maximum blink count before sending a message.

static const int led_pin = 2;   // Define the LED pin.

//structure for messages to be sent through the message queue.
typedef struct Message{
  char body[20];    //Message body.
  int count;    //Message count.
} Message;

//queue handles for delay and message queues.
static QueueHandle_t delay_queue;
static QueueHandle_t msg_queue;

//Task to handle command line input and send delay values to the delay queue.
void doCLI(void *parameter){
  Message rcv_msg;    //to receive from the message queue.
  char c;   //to read from serial.
  char buf[buf_len];    //to store serial input.
  uint8_t idx = 0;      //Index for the buffer.
  uint8_t cmd_len = strlen(command);
  int led_delay;    //LED delay value.

  memset(buf, 0, buf_len);    //Initialize the buffer.
  while (1) {
    //if there is a message in the message queue.
    if (xQueueReceive(msg_queue, (void*)&rcv_msg, 0) == pdTRUE) {
      Serial.print(rcv_msg.body);
      Serial.print(" : ");
      Serial.println(rcv_msg.count);
      Serial.
    }
    //if there is serial input available.
    if (Serial.available() > 0) {
      c = Serial.read();    //Read a character from serial.
      if (idx<buf_len-1) {
        buf[idx] = c;   //Store the character in the buffer.
        idx ++;   //Increment the buffer index.
      }
      //if the character is a newline or carriage return.
      if (c == '\n' || c == '\r') {
        Serial.print('\r\n');
        // if the input starts with the "delay " command.
        if (memcmp(buf, command, cmd_len) == 0) {
          char *tail = buf + cmd_len;   //Pointer to the part of the buffer after the command.
          led_delay = atoi(tail);   // Convert the tail to an integer (delay value).
          led_delay = abs(led_delay);   //Take the absolute value of the delay.

          // Send the delay value to the delay queue.
          if (xQueueSend(delay_queue, (void*)&led_delay, 10) != pdTRUE) {
            Serial.println("ERROR : couldn't put item on the delay queue");
          }
        }
        memset(buf, 0, buf_len);    //Clear the buffer.
        idx=0;    //reset the index
      }
    }
  }
}

//Task to blink the LED and receive delay values from the delay queue.
void blinkLED(void *parameter){
  Message msg;    //to send to the message queue.
  int led_delay = 500;    //Default LED delay.
  uint8_t counter = 0;    //Blink counter.
  pinMode(led_pin, OUTPUT);
  while (1) {
    //if there is a delay value in the delay queue.
    if (xQueueReceive(delay_queue, (void*)&led_delay, 0) == pdTRUE) {
      //to the message queue indicating a delay change.
      strcpy(msg.body, "Message Recived");
      msg.count = 1;
      xQueueSend(msg_queue, (void*)&msg, 10);
    }
    //Blink the LED.
    digitalWrite(led_pin, HIGH);
    vTaskDelay(led_delay/portTICK_PERIOD_MS);
    digitalWrite(led_pin, LOW);
    vTaskDelay(led_delay/portTICK_PERIOD_MS);

    counter ++;   /// Increment the blink counter.
    //if the blink counter has reached the maximum.
    if (counter >= blink_max) {
      //to the message queue indicating the blink count.
      strcpy(msg.body, "Blinked");
      msg.count = counter;
      xQueueSend(msg_queue, (void*)&msg, 10);
      counter = 0;    //Reset the blink counter.
    }
  }
}

void setup() {
  Serial.begin(115200);
  vTaskDelay(1000/portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("Enter the command 'delay xxx' where xxx is your desired LED blink delay time in miliSec");

  //Create the delay and message queues.
  delay_queue = xQueueCreate(delay_queue_len, sizeof(int));
  msg_queue = xQueueCreate(msg_queue_len, sizeof(Message));

  //Create the CLI and blink LED tasks.
  xTaskCreatePinnedToCore(doCLI, "CLI", 4096, NULL, 1, NULL, app_cpu);
  xTaskCreatePinnedToCore(blinkLED, "Blink LED", 2048, NULL, 1, NULL, app_cpu);

  vTaskDelete(NULL);    // Delete the setup task.
}

void loop() {}
