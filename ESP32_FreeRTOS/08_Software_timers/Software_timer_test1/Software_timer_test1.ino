#if CONFIG_FREERTOS_CONFIG
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

static TimerHandle_t one_shot_timer = NULL;
static TimerHandle_t auto_reload_timer = NULL;

void myTimerCallback(TimerHandle_t xTimer){
  if ((uint32_t)pvTimerGetTimerID(xTimer) == 0) {
    Serial.println("One-shot-Timer Expired");
  }

  
  if ((uint32_t)pvTimerGetTimerID(xTimer) == 1) {
    Serial.println("Auto-reload-Timer Expired");
  }
}

void setup() {
  Serial.begin(115200);
  vTaskDelay(1000/portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("__________FreeRTOS Timer Demo__________");

  one_shot_timer = xTimerCreate("one-shot-timer", 2000/portTICK_PERIOD_MS, pdFALSE, (void *)0, myTimerCallback);
  auto_reload_timer = xTimerCreate("auto-reload-timer", 1000/portTICK_PERIOD_MS, pdTRUE, (void *)1, myTimerCallback);

  if (one_shot_timer == NULL || auto_reload_timer == NULL) {
    Serial.println("Couldn't create on eof the timers");
  }
  else{
    vTaskDelay(1000/portTICK_PERIOD_MS);
    Serial.println("Starting timers...");

    xTimerStart(one_shot_timer, portMAX_DELAY);
    xTimerStart(auto_reload_timer, portMAX_DELAY);
  }
  vTaskDelete(NULL);
}

void loop() {}
