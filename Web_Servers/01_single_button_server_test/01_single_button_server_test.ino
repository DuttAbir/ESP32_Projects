#include "WiFi.h"
#include "C:\Users\dutta\Documents\ESP32\Web_Servers\info.h"

const char *SSID = ssid();
const char *PASSWORD = password();

//set web server port number to 80
WiFiServer server(80);        

//store HTTP request
String header;                

//store output state
String ledState = "off";

//assign GPIO_26 as output pin
const int ledPin = 26;

//current time
unsigned long currentTime = millis();
//previous time
unsigned long previousTime = 0;
//timeout period in ms
const long timeoutTime = 2000;


void setup() {
  // set pinmode and set output to LOW
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);



  Serial.begin(115200);
  delay(1000);
  //Connect to WiFi network
  WiFi.begin(SSID, PASSWORD);
  Serial.print("\nConnecting to ");
  Serial.println(SSID);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local IP address : ");
  //print local IP and start web server
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop() {
  //Listen for incoming clients
  WiFiClient client = server.available();
  //if client connects
  if (client) {
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");
    //store incoming data
    String currentLine = "";
    while (client.connected() && currentTime-previousTime <= timeoutTime) {
      currentTime = millis();
      //if there is bytes to read
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        header+=c;
        //if byte is a newline character
        if (c == '\n') {
          //if current line is blank, means two newline character in a row and it's the end of the client HTTP request
          if (currentLine.length() == 0) {
            //HTTP headers always start with a response code (eg; HTTP/1.1 200 OK)
            client.println("HTTP/1.1 200 OK");
            //so the client knows what content is coming 
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println();
            //turn led on and off
            if (header.indexOf("GET /26/on") >= 0) {
              Serial.println("LED ON");
              ledState = "on";
              digitalWrite(ledPin, HIGH);
            }
            else if (header.indexOf("GET /26/off") >=0) {
              Serial.println("LED OFF");
              ledState = "off";
              digitalWrite(ledPin, LOW);
            }
            //display HTML webpage
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\"></head>");
            //CSS for style
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            //Page heading
            client.println("<body><h1>ESP32 Web Server</h1>");
            //display current state of led
            client.println("<p>GPIO 26 - State " + ledState + "</p>");
            if (ledState == "off") {
              client.println("<p><a href=\"/26/on\"><button class=\"button\">ON</button></a></p>");
            }
            else{
              client.println("<p><a href=\"/26/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            client.println("</body></html>");
            //HTTP response ends with blank line and breaks out of the while loop
            client.println();
            break;
          }
          else{
            //clear current line if there is a new line
            currentLine = "";
          }
        }
        //if there is anything but a carriage return , add it to current line
        else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    //clear header
    header = "";
    //close connection 
    client.stop();
    Serial.println("Client Disconnected");
    Serial.println();
  }
}
