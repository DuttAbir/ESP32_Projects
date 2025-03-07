#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

enum {BUF_SIZE = 5};        // Size of the shared buffer

static const int num_prod_task = 5;       // Number of producer tasks
static const int num_cons_task = 2;       // Number of consumer tasks
static const int num_writes = 3;          // Number of items each producer writes

static int buf [BUF_SIZE];          // Shared buffer
static int head = 0;                // Head index for producers
static int tail = 0;                // Tail index for consumers

static SemaphoreHandle_t bin_sem;            // Binary semaphore for consumer start synchronization
static SemaphoreHandle_t mutex;              // Mutex for protecting shared buffer access
static SemaphoreHandle_t sem_empty;          // Counting semaphore for empty buffer slots
static SemaphoreHandle_t sem_filled;         // Counting semaphore for filled buffer slots

void producer(void *parameters){
  int num = *(int *)parameters;                     // Get producer number

  for (int i = 0; i < num_writes; i++) {
    xSemaphoreTake(sem_empty, portMAX_DELAY);       // Wait for an empty slot

    xSemaphoreTake(mutex, portMAX_DELAY);           // Acquire mutex to protect buffer
    buf[head] = num;                                // Write data to buffer
    head = (head + 1) % BUF_SIZE;                   // Update head pointer (circular buffer)
    xSemaphoreGive(mutex);                          // Release mutex

    xSemaphoreGive(sem_filled);                     // Signal a filled slot
  }
  vTaskDelete(NULL);                                // Delete producer task after finishing
}

void consumer(void *parameters){
  xSemaphoreTake(bin_sem, portMAX_DELAY);           // Wait for start signal from setup()
  int val;
  while (1) {
    xSemaphoreTake(sem_filled, portMAX_DELAY);      // Wait for a filled slot

    xSemaphoreTake(mutex, portMAX_DELAY);           // Acquire mutex to protect buffer
    val = buf[tail];                                // Read data from buffer
    tail = (tail + 1) % BUF_SIZE;                   // Update tail pointer (circular buffer)
    Serial.println(val);
    xSemaphoreGive(mutex);                          // Release mutex

    xSemaphoreGive(sem_empty);                      // Signal an empty slot
  }
}

void setup() {
  char taskName[12];
  Serial.begin(115200);
  vTaskDelay(1000/portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("_______FreeRTOS Semaphore Producer-Consumer Demo______");

  bin_sem = xSemaphoreCreateBinary();                                  // Create binary semaphore
  mutex = xSemaphoreCreateMutex();                                     // Create mutex
  sem_empty = xSemaphoreCreateCounting(BUF_SIZE, BUF_SIZE);            // Create counting semaphore for empty slots
  sem_filled = xSemaphoreCreateCounting(BUF_SIZE, 0);                  // Create counting semaphore for filled slots

  for (int  i = 0; i < num_prod_task; i++) {
    sprintf(taskName, "Producer %i", i);
    xTaskCreatePinnedToCore(producer, taskName, 1024, (void *)&i, 1, NULL, app_cpu);      // Create producer tasks
  } 

  xSemaphoreTake(mutex, portMAX_DELAY);                                // Acquire mutex before printing
  Serial.println("All tasks created");
  xSemaphoreGive(mutex);                                               // Release mutex
  xSemaphoreGive(bin_sem);                                             // Signal consumers to start

  for (int i = 0; i < num_cons_task; i++) {
    sprintf(taskName, "Consumer %i", i);
    xTaskCreatePinnedToCore(consumer, taskName, 1024, NULL, 1, NULL, app_cpu);            // Create consumer tasks
  }
}
void loop() {
  vTaskDelay(1000/portTICK_PERIOD_MS);        //Allow low-priority task to yeild
}
