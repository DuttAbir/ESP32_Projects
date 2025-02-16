#define LED_PIN 2

TaskHandle_t ledTaskHandle = NULL;

void ledTask(void *pvParameters){
  int core = xPortGetCoreID();
  for(;;){
    digitalWrite(LED_PIN, HIGH);
    Serial.printf("LED ON running on core %d\n", core);
    vTaskDelay(1000/portTICK_PERIOD_MS);

    digitalWrite(LED_PIN, LOW);
    Serial.printf("LED OFF running on core %d\n", core);
    vTaskDelay(1000/portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  xTaskCreatePinnedToCore(ledTask,"LED TASK", 2048, NULL, 1, &ledTaskHandle, 1);
}

void loop() {}
