#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

TickType_t cs_wait = 250;
TickType_t med_wait = 5000;

static SemaphoreHandle_t lock;

void lowerTask(void *param){
  TickType_t timestamp;
  
  while (1) {
    Serial.println("Lower task trying to taske lock");
    timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS;
    xSemaphoreTake(lock, portMAX_DELAY);

    Serial.print("Lower task got lock. spent ");
    Serial.print((xTaskGetTickCount() * portTICK_PERIOD_MS) - timestamp);
    Serial.println(" ms waiting for lock. Doing some work.");

    timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS;
    while ((xTaskGetTickCount() * portTICK_PERIOD_MS) - timestamp < cs_wait);

    Serial.println("lower task releasing lock");
    xSemaphoreGive(lock);

    vTaskDelay(500/portTICK_PERIOD_MS);
  }
}

void mediumTask(void *param){
  TickType_t timestamp;

  while (1) {
    Serial.println("medium task doing some work");
    timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS;
    while ((xTaskGetTickCount() * portTICK_PERIOD_MS) - timestamp < med_wait);

    Serial.println("medium task done");
    vTaskDelay(500/portTICK_PERIOD_MS);
  }
}

void higherTask(void *param){
  TickType_t timestamp;

  while (1) {
    Serial.println("Higer task trying to taske lock");
    timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS;
    xSemaphoreTake(lock, portMAX_DELAY);

    Serial.print("Higher task got lock. spent ");
    Serial.print((xTaskGetTickCount() * portTICK_PERIOD_MS) - timestamp);
    Serial.println(" ms waiting for lock. Doing some work.");

    timestamp = xTaskGetTickCount() * portTICK_PERIOD_MS;
    while ((xTaskGetTickCount() * portTICK_PERIOD_MS) - timestamp < cs_wait);

    Serial.println("Higher task releasing lock");
    xSemaphoreGive(lock);

    vTaskDelay(500/portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  vTaskDelay(1000/portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("_____FreeRTOS Priority Inversion Demo______");

  lock = xSemaphoreCreateBinary();
  xSemaphoreGive(lock);

  xTaskCreatePinnedToCore(lowerTask, "Task L", 1024, NULL, 1, NULL, app_cpu);
  vTaskDelay(100/portTICK_PERIOD_MS);
  xTaskCreatePinnedToCore(higherTask, "Task H", 1024, NULL, 3, NULL, app_cpu);
  xTaskCreatePinnedToCore(mediumTask, "Task M", 1024, NULL, 2, NULL, app_cpu);

  vTaskDelete(NULL);
}

void loop() {}
