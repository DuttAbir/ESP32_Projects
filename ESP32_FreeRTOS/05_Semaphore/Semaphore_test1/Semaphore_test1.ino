#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

static const int led_pin = 2;

static SemaphoreHandle_t bin_sem;

void blinkLed(void *parameter){
  int num = *(int *)parameter;
  xSemaphoreGive(bin_sem);

  Serial.print("Received : ");
  Serial.println(num);

  pinMode(led_pin, OUTPUT);

  while (1) {
    digitalWrite(led_pin, HIGH);
    vTaskDelay(num/portTICK_PERIOD_MS);
    digitalWrite(led_pin, LOW);
    vTaskDelay(num/portTICK_PERIOD_MS);
  }
}

void setup() {
  long int delay_arg;

  Serial.begin(115200);
  vTaskDelay(1000/portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("__FreeRTOS Semaphore Demo__");
  Serial.println("Enter a number for delay in MS");

  while (Serial.available()<=0); 
    
  delay_arg = Serial.parseInt();
  Serial.print("Sending : ");
  Serial.println(delay_arg);

  bin_sem = xSemaphoreCreateBinary();

  xTaskCreatePinnedToCore(blinkLed, "Blink Led", 1024, (void *)&delay_arg, 1, NULL, app_cpu);

  xSemaphoreTake(bin_sem, portMAX_DELAY);

  Serial.println("Done!");
  
}

void loop() {
  vTaskDelay(1000/portTICK_PERIOD_MS);
}
