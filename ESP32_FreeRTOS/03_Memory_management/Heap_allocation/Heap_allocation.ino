#if CONFIG_FREERTOS_UNICORE
  static const BaseType_t app_cpu = 0;
#else
  static const BaseType_t app_cpu = 1;
#endif

static const uint8_t buf_len = 255;   //maximum length of the serial input buffer.

static char *msg_ptr = NULL;    //Pointer to store the dynamically allocated message.
static volatile uint8_t msg_flag = 0;   //Flag to indicate if a new message is available.

//Task to read serial input and store it in a dynamically allocated buffer.
void readSerial(void *parameter){
  char c;
  char buf[buf_len];    //buffer to store incoming characters.
  uint8_t idx = 0;    //Index for the buffer.
  memset(buf, 0, buf_len);    //Initialize the buffer with zeros.

  while (1) {
    if (Serial.available()>0) {
      c = Serial.read();    //Read a character from the serial port.
      if (idx<buf_len-1) {    //if there is space in the buffer.
        buf[idx] = c;   //Store the character in the buffer.
        idx ++;   //Increment the index.
      }
      if (c == '\n' || c == '\r') {   //Check for newline or carriage return.
        buf[idx-1] = '\0';    //Null-terminate the string.
        if (msg_flag=0) {   // if a message is already being processed.
          msg_ptr = (char*)pvPortMalloc(idx * sizeof(char));    //Allocate memory for the message.
          configASSERT(msg_ptr);    //Assert that memory allocation was successful.
          memcpy(msg_ptr, buf, idx);    //Copy the message to the allocated memory.
          msg_flag=1;   //indicate a new message is available.
        }
        memset(buf, 0, buf_len);    // Reset the buffer.
        idx = 0;    //Reset the index.
      }
    }
  }
}

//Task to print the message and free the allocated memory.
void printMessage(void *parameter){
  while (1) {
    if (msg_flag == 1) {    // if a new message is available.
      Serial.println(msg_ptr);    //Print the message
      Serial.print("Free Heap bytes : ");
      Serial.println(xPortGetFreeHeapSize());   //Print the free heap size.

      vPortFree(msg_ptr);   // Free the allocated memory.
      msg_ptr = NULL;   //Reset the pointer.
      msg_flag = 0;   //Reset the flag.
    }
  }
}


void setup() {
  Serial.begin(115200);
  vTaskDelay(1000/portTICK_PERIOD_MS);
  Serial.println();
  Serial.println("___________FreeRTOS Heap Demo___________");
  Serial.println("Enter a string ");

  xTaskCreatePinnedToCore(readSerial, "Read Serial", 1024, NULL, 1, NULL, app_cpu);   //Create the readSerial task
  xTaskCreatePinnedToCore(printMessage, "Print message", 1024, NULL, 1, NULL, app_cpu);   //Create the printMessage task

  vTaskDelete(NULL);    //delete the setup task itself.
}

void loop() {}
