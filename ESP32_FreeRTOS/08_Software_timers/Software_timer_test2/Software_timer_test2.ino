#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

// Define the delay for the auto-dimming timer in ticks.
static const TickType_t dim_delay = 5000/portTICK_PERIOD_MS;

// Define the LED pin.
static const int led_pin = 2;

// Declare a handle for the one-shot ti
static TimerHandle_t one_shot_timer = NULL;

// Callback function for the one-shot timer.
void autoDimmerCallback(TimerHandle_t xTimer){
  digitalWrite(led_pin, LOW);
}

// Task function for handling serial CLI input.
void doCLI(void *param){
  char c;
  pinMode(led_pin, OUTPUT);
  while (1) {
     // Check if there is data available on the serial port.
    if (Serial.available() > 0) {
      c = Serial.read();
      Serial.print(c);
      // Turn on the LED when a character is received.
      digitalWrite(led_pin, HIGH);
      // Start the one-shot timer to dim the LED after the specified delay.
      xTimerStart(one_shot_timer, portMAX_DELAY);
    }
  }
}

void setup() {
  Serial.begin(115200);
  vTaskDelay(1000/portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("____FreeRTOS timer led___");

   // Create a one-shot timer with the specified delay and callback function.
  one_shot_timer = xTimerCreate("One-shot-timer", dim_delay, pdFALSE, (void *)0, autoDimmerCallback);

  // Create a task for handling the CLI input and pin it to the specified CPU core.
  xTaskCreatePinnedToCore(doCLI, "Do CLI", 1024, NULL, 1, NULL, app_cpu);

  // Delete the setup task itself
  vTaskDelete(NULL);
}

void loop() {}