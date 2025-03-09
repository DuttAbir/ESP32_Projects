#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

enum {NUM_TASK = 5};                                                // Define the number of philosopher tasks.
enum {TASK_STACK_SIZE = 2048};                                      // Define the stack size for each task

static SemaphoreHandle_t bin_sem;                                   // Semaphore for ensuring all tasks are created before proceeding.
static SemaphoreHandle_t done_sem;                                  // Semaphore to signal when all tasks have finished.
static SemaphoreHandle_t chopstick[NUM_TASK];                       // Array of mutexes representing chopsticks.

// Task function representing a philosopher's actions.
void eat(void *param){
  int num;
  char buf[50];
  int idx_1;
  int idx_2;

  num = *(int *)param;                                              // Get the philosopher's number from the parameter.
  xSemaphoreGive(bin_sem);                                          // Signal that this task has started.


  // Determine the order in which to pick up chopsticks to avoid deadlock.
  // This is the core logic that prevents deadlock by ordering the access to chopsticks.
  // The philosopher with the higher number will pick up the right chopstick first.
  
  if (num < (num + 1)% NUM_TASK) {                                  // If the philosopher's number is less than the next philosopher's number
    idx_1 = num;                                                    // pick up the left chopstick (their own number) first.
    idx_2 = (num+1)%NUM_TASK;                                       // then pick up the right chopstick (the next philosopher's number).
  }
  else{                                                             // If the philosopher's number is greater than or equal to the next philosopher's number
    idx_1 = (num+1)%NUM_TASK;                                       // pick up the right chopstick (the next philosopher's number) first.
    idx_2 = num;                                                    // then pick up the left chopstick (their own number).
  }
  // This is the crucial part that breaks the circular dependency.
  // By having the last philosopher pick up the "next" chopstick first,
  // they don't wait for the previous philosopher to release their chopstick.


  xSemaphoreTake(chopstick[idx_1], portMAX_DELAY);                  // Take the first chopstick.
  sprintf(buf, "Philosopher %i took chpostick %i", num, num);
  Serial.println(buf);

  vTaskDelay(100/portTICK_PERIOD_MS);                               // Simulate thinking time.

  xSemaphoreTake(chopstick[idx_2], portMAX_DELAY);                  // Take the second chopstick
  sprintf(buf, "Philosopher %i took chopstick %i", num, (num+1)%NUM_TASK);
  Serial.println(buf);

  sprintf(buf, "Philosopher %i is eating", num);                    // Simulate eating.
  Serial.println(buf);
  vTaskDelay(100/portTICK_PERIOD_MS);

  xSemaphoreGive(chopstick[idx_2]);                                  // Return the second chopstick.
  sprintf(buf, "Philosopher %i returned chopstick %i", num, (num+1)%NUM_TASK);
  Serial.println(buf);

  xSemaphoreGive(chopstick[idx_1]);                                  // Return the first chopstick.
  sprintf(buf, "Philosopher %i  has returned chopstick %i", num, num);
  Serial.println(buf);

  xSemaphoreGive(done_sem);                                          // Signal that this task has finished.
  vTaskDelete(NULL);                                                 // Delete the task.
}

void setup() {
  char taskName[20];
  Serial.begin(115200);
  vTaskDelay(1000/portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("----FreeRTOS Philosophers Deadlock Arbitrator Solution----");
  
  bin_sem = xSemaphoreCreateBinary();                                 // Create binary semaphore for task synchronization.
  done_sem = xSemaphoreCreateCounting(NUM_TASK, 0);                   // Create counting semaphore to signal task completion.

  // Create mutexes for each chopstick.
  for (int  i = 0; i < NUM_TASK; i++) {
    chopstick[i] = xSemaphoreCreateMutex();
  }

  // Create philosopher tasks.
  for (int i  = 0; i < NUM_TASK; i++) {
    sprintf(taskName, "Philosopher %i", i);
    xTaskCreatePinnedToCore(eat, taskName, TASK_STACK_SIZE, (void*)&i, 1, NULL, app_cpu);
    xSemaphoreTake(bin_sem, portMAX_DELAY);                           // Wait for the task to start
  }

  // Wait for all tasks to finish.
  for (int i = 0; i < NUM_TASK; i++) {
    xSemaphoreTake(done_sem, portMAX_DELAY);
  }

  Serial.println("Done... NO deadlock");
}

void loop() {}

