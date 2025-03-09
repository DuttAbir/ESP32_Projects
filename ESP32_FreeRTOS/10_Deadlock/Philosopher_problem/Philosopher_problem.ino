#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

enum {NUM_TASK = 5};
enum {TASK_STACK_SIZE = 2048};

static SemaphoreHandle_t bin_sem;
static SemaphoreHandle_t done_sem;
static SemaphoreHandle_t chopstick[NUM_TASK];

void eat(void *param){
  int num;
  char buf[50];

  num = *(int *)param;
  xSemaphoreGive(bin_sem);

  xSemaphoreTake(chopstick[num], portMAX_DELAY);
  sprintf(buf, "Philosopher %i took chpostick %i", num, num);
  Serial.println(buf);

  vTaskDelay(100/portTICK_PERIOD_MS);  

  xSemaphoreTake(chopstick[(num+1)%NUM_TASK], portMAX_DELAY);
  sprintf(buf, "Philosopher %i took chopstick %i", num, (num+1)%NUM_TASK);
  Serial.println(buf);

  sprintf(buf, "Philosopher %i is eating", num);
  Serial.println(buf);
  vTaskDelay(100/portTICK_PERIOD_MS);

  xSemaphoreGive(chopstick[(num+1)%NUM_TASK]);
  sprintf(buf, "Philosopher %i returned chopstick %i", num, (num+1)%NUM_TASK);
  Serial.println(buf);

  xSemaphoreGive(chopstick[num]);
  sprintf(buf, "Philosopher %i  has returned chopstick %i", num, num);
  Serial.println(buf);

  xSemaphoreGive(done_sem);
  vTaskDelete(NULL);
}

void setup() {
  char taskName[20];
  Serial.begin(115200);
  vTaskDelay(1000/portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("----FreeRTOS Philosophers Deadlock problem----");
  
  bin_sem = xSemaphoreCreateBinary();
  done_sem = xSemaphoreCreateCounting(NUM_TASK, 0);

  for (int  i = 0; i < NUM_TASK; i++) {
    chopstick[i] = xSemaphoreCreateMutex();
  }

  for (int i  = 0; i < NUM_TASK; i++) {
    sprintf(taskName, "Philosopher %i", i);
    xTaskCreatePinnedToCore(eat, taskName, TASK_STACK_SIZE, (void*)&i, 1, NULL, app_cpu);
    xSemaphoreTake(bin_sem, portMAX_DELAY);
  }
  for (int i = 0; i < NUM_TASK; i++) {
    xSemaphoreTake(done_sem, portMAX_DELAY);
  }

  Serial.println("Done... NO deadlock");
}

void loop() {}
