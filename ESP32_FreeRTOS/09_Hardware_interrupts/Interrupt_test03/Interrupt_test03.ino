#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

static const uint64_t timer_freq = 1000000;
static const uint64_t timer_max_count = 1000000;

static const int adc_pin = 36;

static hw_timer_t *timer = NULL;
static volatile uint16_t val;
static SemaphoreHandle_t bin_sem = NULL;

void IRAM_ATTR onTimer(){
  BaseType_t task_woken = pdFALSE;

  val = analogRead(adc_pin);

  xSemaphoreGiveFromISR(bin_sem, &task_woken);

  if (task_woken){
    portYIELD_FROM_ISR();
  }
}

void printValues(void *param){
  while (1) {
    xSemaphoreTake(bin_sem, portMAX_DELAY);
    Serial.println(val);
  }
}

void setup() {
  Serial.begin(115200);
  vTaskDelay(1000/portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("-----FreeRTOS ISR Buffer Demmo-----");

  bin_sem = xSemaphoreCreateBinary();

  if (bin_sem == NULL){
    Serial.println("Couldn't create semaphore");
    ESP.restart();
  }

  xTaskCreatePinnedToCore(printValues, "Print Values", 1024, NULL, 2, NULL, app_cpu);

  timer = timerBegin(timer_freq);
  timerAttachInterrupt(timer, &onTimer);
  timerAlarm(timer, timer_max_count, true, 0);
}

void loop() {}
