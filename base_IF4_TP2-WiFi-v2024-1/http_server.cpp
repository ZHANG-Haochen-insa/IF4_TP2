
#include "http_server.hpp"
#include <WiFi.h>


// Variables liées au serveur HTTP / Wifi
WiFiServer wifiServer(80);

WiFiClient client;

//- Vars globales ----------------------------
// the order received by wifi
unsigned short reponse = ORDER_ROBOT_STOP;

// Variable to know whether a client is connected
bool clientConnected = false;

// Variable to store the HTTP request
String header;

void wifi_start(const char * ssid, const char * password) {
  
  Serial.println("\n[INFO] Configuring access point");
  WiFi.mode(WIFI_AP);  
  WiFi.softAP(ssid, password);
 
  Serial.print("[INFO] Started access point at IP ");
  Serial.println(WiFi.softAPIP());

  // Start wifi
  wifiServer.begin();
}


int communicate_with_phone() {

  int reponse = 0;
  
  // put your main code here, to run repeatedly:
  WiFiClient client = wifiServer.available();   // Listen for incoming clients

  if (client) {         // If a new client connects,
    reponse = 1;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // analyse la requête pour déterminer l'ordre
            if (header.indexOf("GET /26/on") >= 0) {
              Serial.println("Received Robot Forward");
              reponse = ORDER_ROBOT_FORWARD;              
            } 

            if (header.indexOf("GET /27/on") >= 0) {
              Serial.println("Received Robot Left");
              reponse = ORDER_ROBOT_LEFT;
            } 


            if (header.indexOf("GET /28/on") >= 0) {
              Serial.println("Received Robot Right");
              reponse = ORDER_ROBOT_RIGHT;
            } 

            if (header.indexOf("GET /29/on") >= 0) {
              Serial.println("Received Robot Backward");
              reponse = ORDER_ROBOT_BACKWARD;              
            } 

            if (header.indexOf("GET /26/off") >= 0 || header.indexOf("GET /27/off") >= 0 || header.indexOf("GET /28/off") >= 0 || header.indexOf("GET /29/off") >= 0) {
              Serial.println("Received Robot Stop");
              reponse = ORDER_ROBOT_STOP;       
            }
            

            // Send the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}");
            client.println(".marge {margin-left: 10em;}");
            client.println(".marge2 {margin-left: 2em;}");
            client.println("</style></head>");

            // Web Page Heading
            client.println("<body><h1><p>ESP32Robot</p> <p>DC motor drive over Wi-Fi</p></h1>");

            // Display current mode
            client.println("<p>Forward</p>");
            
            // If the output26State is off, it displays the ON button
            if (reponse != ORDER_ROBOT_FORWARD) {
              client.println("<p><a href=\"/26/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/26/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            client.println("<p>Left <span class=\"marge\">Right</span></p>");
           if  (reponse != ORDER_ROBOT_LEFT) {
              client.println("<p><a href=\"/27/on\"><button class=\"button\">ON</button></a><span class=\"marge2\">");
            } else {
              client.println("<p><a href=\"/27/off\"><button class=\"button button2\">OFF</button></a><span class=\"marge2\">");
            }
           if  (reponse != ORDER_ROBOT_RIGHT) {
              client.println("<a href=\"/28/on\"><button class=\"button\">ON</button></a></span></p></p>");
            } else {
              client.println("<a href=\"/28/off\"><button class=\"button button2\">OFF</button></a></span></p></p>");
            }
            
            client.println("<p>Backward</p>");
            // If the output29State is off, it displays the ON button
            if  (reponse != ORDER_ROBOT_BACKWARD) {
              client.println("<p><a href=\"/29/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/29/off\"><button class=\"button button2\">OFF</button></a></p>");
            }

            client.println("</body></html>");

            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
  return reponse;
}
