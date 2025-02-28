#include <stdlib.h>
#include <string.h>

//Check if using a single-core FreeRTOS configuration
#if CONFIG_FREERTOS_UNICORE   
  static const BaseType_t app_cpu = 0;     //If single-core, use core 0
#else
  static const BaseType_t app_cpu = 1;    //If dual-core, use core 1
#endif

static const uint8_t buf_len = 20;    //buffer length for serial input
static const int led_pin = 2;   //GPIO pin for the LED

static int led_delay = 500;   //Initialize the LED delay

// Task to toggle the LED
void ToggleLED(void *parameters){
  while (1) {
    digitalWrite(led_pin, HIGH);
    vTaskDelay(led_delay/portTICK_PERIOD_MS);
    digitalWrite(led_pin, LOW);
    vTaskDelay(led_delay/portTICK_PERIOD_MS);
  }
}

// Task to read serial input and update the LED delay
void readSerial(void *parameters){
  char c;   //to store the received character
  char buf[buf_len];    //to store the serial input
  uint8_t idx = 0;    //Index to track the position in the buffer

  memset(buf, 0, buf_len);    //Initialize the buffer with zeros
  while (1) {
    if (Serial.available()>0) {   //if serial data is available
      c = Serial.read();
      if (c == '\n' || c == '\r') {   //Check for newline or carriage return
        buf[idx] = '\0';    //Null-terminate the string in the buffer
        long new_delay = strtol(buf, NULL, 10);   // Convert the string to a long integer
        if (new_delay >= 0) {   //Validate that the input is a positive number
          led_delay = (int)new_delay;
          Serial.print("Updated LED delay to ");
          Serial.println(led_delay);
        } else {
          Serial.println("Invalid input. Please enter a positive number.");
        }
        memset(buf, 0, buf_len);    //Clear the buffer
        idx = 0;    //Reset the buffer index
      }
      else if (isDigit(c) && idx < buf_len - 1) {   //if the character is a digit and the buffer is not full
          buf[idx] = c;   //Store the digit in the buffer
          idx++;    //Increment the buffer index
        }
      else if(idx>= buf_len -1) {   //if the buffer is full
          Serial.println("Input too long. Please enter a shorter number.");
          memset(buf, 0, buf_len);
          idx = 0;
      }
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);    // delay to prevent busy-waiting
  }
}

void setup() {
  pinMode(led_pin, OUTPUT);
  Serial.begin(115200);
  vTaskDelay(1000/portTICK_PERIOD_MS);    //allow serial to initialize
  Serial.println("Multitasking Demo");
  Serial.println("Enter a number to change the LED delay (in miliSec) : ");

  xTaskCreatePinnedToCore(ToggleLED, "Toggle LED", 1024, NULL, 1, NULL, app_cpu);
  xTaskCreatePinnedToCore(readSerial, "Read Serial", 1024, NULL, 1, NULL, app_cpu);

  vTaskDelete(NULL);    //Delete the setup task (self-delete)

}

void loop() {}
