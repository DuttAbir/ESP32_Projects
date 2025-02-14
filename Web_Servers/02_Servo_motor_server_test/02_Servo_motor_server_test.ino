#include<WiFi.h>
#include<ESP32Servo.h>
#include "C:\Users\dutta\Documents\ESP32\Web_Servers\info.h"

Servo myServo;                          //create servo object

static const int servoPin = 13;         //define servo pin

const char *SSID = ssid();
const char *PASSWORD = password();

WiFiServer server(80);                  //create web server object to listen on port 80

String header;                          //to store HTTP request header

String valueString = String(5);         //to store servo angle from webpage
int pos1 = 0;                           //to parse the servo value from the HTTP request
int pos2 = 0;

unsigned long currentTime = 0;          //timeout of 2sec to prevent the server from hanging if a client disconnects unexpectedly
unsigned long previousTime = 0;
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  myServo.attach(servoPin);             //initialize servo motor on specific pin

  Serial.print("Connecting to ");
  //Serial.println(ssid);
  WiFi.begin(SSID, PASSWORD);           //initialize wifi connectivity

  while (WiFi.status()!= WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.println("IP Address : ");
  Serial.println(WiFi.localIP());     //print local IP address of ESP32 where the server will be hosted
  server.begin();                     //start the web server
}

void loop() {
  //check for new client conecting to the server
  WiFiClient client = server.available();
  //if client is connected
  if (client) {
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client");
    String currentLine = "";
    while (client.connected() && currentTime - previousTime <= timeoutTime) {
      currentTime = millis();
      if (client.available()) {
        //read a character from the client's request
        char c = client.read();
        Serial.write(c);
        header+=c;
        //if request line is ends
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println();

            //HTML webpage
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            //CSS style 
            client.println("<style>body { text-align: center; font-family: \"Trebuchet MS\", Arial; margin-left:auto; margin-right:auto;}");
            client.println(".slider { width: 300px; }</style>");
            client.println("<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js\"></script>");

            client.println("</head><body><h1>ESP32 with Servo</h1>");
            client.println("<p>Position: <span id=\"servoPos\"></span></p>");
            //HTMl slider          
            client.println("<input type=\"range\" min=\"0\" max=\"180\" class=\"slider\" id=\"servoSlider\" onchange=\"servo(this.value)\" value=\""+valueString+"\"/>");
            //JavaScript code uses AJAX to communicate with the ESP32.that makes the slider control the servo without requiring the entire webpage to reload every time you move the slider
            client.println("<script>var slider = document.getElementById(\"servoSlider\");");
            client.println("var servoP = document.getElementById(\"servoPos\"); servoP.innerHTML = slider.value;");
            client.println("slider.oninput = function() { slider.value = this.value; servoP.innerHTML = this.value; }");
            client.println("$.ajaxSetup({timeout:1000}); function servo(pos) { ");
            client.println("$.get(\"/?value=\" + pos + \"&\"); {Connection: close};}</script>");

            client.println("</body></html>");  
            //If the request contains the "GET /?value=" parameter
            if (header.indexOf("GET /?value=")>=0) {
              pos1 = header.indexOf('=');
              pos2 = header.indexOf('&');
              //extract the value 
              valueString = header.substring(pos1+1, pos2);
              //converts it to an integer, and sets the servo position
              myServo.write(valueString.toInt());
              Serial.println(valueString);
            }
            client.println();
            break;
          }
          else{
            currentLine="";
          }
        }
        else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    header = "";
    //close connection
    client.stop();
    Serial.println("Client Disconnected");
    Serial.println("");
  }
}
