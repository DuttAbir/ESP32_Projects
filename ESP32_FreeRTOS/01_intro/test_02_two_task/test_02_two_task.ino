#if CONFIG_FREERTOS_UNICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

#define LED_PIN 2

static const int rate_1 = 1000;
static const int rate_2 = 2000;

void toggleLED_1(void *parameters){
  while (1) {
    digitalWrite(LED_PIN, HIGH);
    vTaskDelay(rate_1/portTICK_PERIOD_MS);
    digitalWrite(LED_PIN, LOW);
    vTaskDelay(rate_1/portTICK_PERIOD_MS);
  }
}

void toggleLED_2(void *parameters){
  while (1) {
    digitalWrite(LED_PIN, HIGH);
    vTaskDelay(rate_2/portTICK_PERIOD_MS);
    digitalWrite(LED_PIN, LOW);
    vTaskDelay(rate_2/portTICK_PERIOD_MS);
  }
}

void setup() {
  pinMode(LED_PIN, OUTPUT);

  xTaskCreatePinnedToCore(toggleLED_1, "TOGGLE 1", 1024, NULL, 1, NULL, app_cpu);
  xTaskCreatePinnedToCore(toggleLED_2, "TOGGLE 2", 1024, NULL, 1, NULL, app_cpu);

}

void loop() {}
