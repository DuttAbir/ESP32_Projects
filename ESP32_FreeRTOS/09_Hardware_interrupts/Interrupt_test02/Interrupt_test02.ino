#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

static const uint64_t timer_freq = 10000000;                           // timer frequency (10 MHz).
static const uint64_t timer_max_count = 1000000;                       // timer alarm count, this combined with the frequency,determines the interrupt rate.
static const TickType_t task_delay = 2000/portTICK_PERIOD_MS;          // task delay in milliseconds and convert it to ticks.

static hw_timer_t *timer = NULL;                                       // pointer to the hardware timer.
static volatile int isr_counter;                                       // to count interrupts. Volatile ensures that compiler doesn't optimize away.
static portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;           // prevents race conditions when accessing shared resources (isr_counter).

//ISR that called when the timer alarm triggers.
void IRAM_ATTR onTimer(){
  portENTER_CRITICAL_ISR(&spinlock);                    // Enter a critical section to prevent other interrupts or tasks from modifying isr_counter
  isr_counter ++;                                       // Increment the interrupt counter.
  portEXIT_CRITICAL_ISR(&spinlock);                     // Exit the critical section.
}

// Task that prints the interrupt counter values.
void printValues(void *param){
  while (1) {
    // Check if there are any pending interrupt counts.
    while (isr_counter > 0) {
      Serial.println(isr_counter);
      portENTER_CRITICAL_ISR(&spinlock);                // Enter a critical section before decrementing the counter.
      isr_counter --;                                   // Decrement the interrupt counter.
      portEXIT_CRITICAL_ISR(&spinlock);                 // Exit the critical section.
    }
    vTaskDelay(task_delay);                             // Delay the task for the specified amount of time.
  }
}

void setup() {
  Serial.begin(115200);
  vTaskDelay(1000/portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("_________FreeRTOS ISR Critical section Demo_________");

  // Create a task that prints the interrupt counter values.
  xTaskCreatePinnedToCore(printValues, "Print values", 1024, NULL, 1, NULL, app_cpu);

  timer = timerBegin(timer_freq);                       // initializes the timer with the specified frequency.
  timerAttachInterrupt(timer, &onTimer);                // attach the interrupt handler.
  timerAlarm(timer, timer_max_count, true, 0);          // configure alarm value and autoreload of the timer. 

  vTaskDelete(NULL);                                    // delete the setup task
}

void loop() {}
