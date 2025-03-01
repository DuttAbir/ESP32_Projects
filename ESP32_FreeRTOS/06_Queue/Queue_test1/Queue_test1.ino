#if CONFID_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

static const uint8_t msg_queue_len = 5;

static QueueHandle_t msg_queue;


void printMessage(void *parameter){
  int item;
  while (1) {
    if (xQueueReceive(msg_queue, (void*)&item, 0) == pdTRUE) {
      //Serial.println(item);
    }
    Serial.println(item);
    vTaskDelay(500/portTICK_PERIOD_MS);
  }
}

void setup(){
  Serial.begin(115200);
  vTaskDelay(1000/portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("___________FreeRTOS Queue Demo____________");

  msg_queue = xQueueCreate(msg_queue_len, sizeof(int));

  xTaskCreatePinnedToCore(printMessage, "Print Message", 1024, NULL, 1, NULL, app_cpu);
}

void loop(){
  static int num = 0;
  if (xQueueSend(msg_queue, (void*)&num, 10) != pdTRUE) {
    Serial.println("Queue Full");
  }
  num ++;
  vTaskDelay(1000/portTICK_PERIOD_MS);
}