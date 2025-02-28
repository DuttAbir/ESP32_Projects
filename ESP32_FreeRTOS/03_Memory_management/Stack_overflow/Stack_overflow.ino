// Determine the CPU core to run the task on based on the FreeRTOS configuration.
#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;    //If single core, use core 0.
#else 
  static const BaseType_t app_cpu = 1;    //If dual core, use core 1.
#endif

//Function to be executed as a FreeRTOS task
void testTask(void *parameter){
  while (1) {
    // Local variables, small stack usage.
    int a = 1;
    int b[100];
    //Simple loop to fill the array 'b' with values. So it's not optimized out by the compiler
    for (int i = 0; i<100; i++) {
      b[i] = a+1;
    }
    //the minimum amount of free stack space the task has had since it started.
    Serial.print("High water mark(words): ");
    Serial.println(uxTaskGetStackHighWaterMark(NULL));    //NULL indicates current task.

    //amount of free heap memory before allocating memory.
    Serial.print("Heap before malloc(bytes) : ");
    Serial.println(xPortGetFreeHeapSize());

    //Allocate 1024 integers (4096 bytes on a 32-bit system) from the heap.
    int *ptr = (int*)pvPortMalloc(1024 * sizeof(int));
    //If allocation failed
    if (ptr == NULL) {    
      Serial.println("Not enough Heap");
      vPortFree(NULL);
    }
    //If allocation was successful
    else{   
      //fill the allocated memory with a value. So it's not optimized out by the compiler
      for (int i = 0; i<1024; i++) {
        ptr[i] = 3;
      }
    }
    //amount of free heap memory after allocation
    Serial.print("Heap after malloc(bytes) : ");
    Serial.println(xPortGetFreeHeapSize());

    //vPortFree(ptr);   //Uncommenting this line would free the allocated memory.

    vTaskDelay(100/portTICK_PERIOD_MS);   //Delay the task
  }
}

void setup() {
  Serial.begin(115200);
  vTaskDelay(1000/portTICK_PERIOD_MS);      //allow serial monitor to connect.
  Serial.println("FreeRTOS memory demo");

  xTaskCreatePinnedToCore(testTask, "Test task", 1500, NULL, 1, NULL, app_cpu);

  vTaskDelete(NULL);    //Delete the setup task itself
}

void loop() {}    //not used
