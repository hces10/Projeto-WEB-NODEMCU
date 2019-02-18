#include <Ethernet.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiType.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiServer.h>
#include <WiFiServerSecure.h>
#include <WiFiUdp.h>

const char* ssid = "hces10"; //VARIÁVEL QUE ARMAZENA O NOME DA REDE SEM FIO EM QUE VAI CONECTAR
const char* password = "henrique1";
// MAC address from Ethernet shield sticker under board
byte mac[] = { 0x84, 0xF3, 0xEB, 0x89, 0x56, 0xF6 };
IPAddress ip(192, 168, 0, 20); // IP address, may need to change depending on network
WiFiServer server(8082);  // create a server at port 80

String HTTP_req;            // stores the HTTP request

void setup()
{
    WiFi.begin(ssid, password); //PASSA OS PARÂMETROS PARA A FUNÇÃO QUE VAI FAZER A CONEXÃO COM A REDE SEM FIO

    Ethernet.begin(mac, WiFi.localIP());  // initialize Ethernet device
    server.begin();           // start to listen for clients
    Serial.begin(9600);       // for diagnostics
    pinMode(4, INPUT);        // switch is attached to Arduino pin 4
    pinMode(4, OUTPUT);        // switch is attached to Arduino pin 4
    Serial.println(WiFi.localIP()); //ESCREVE NA SERIAL O IP RECEBIDO DENTRO DA REDE SEM FIO (O IP NESSA PRÁTICA É RECEBIDO DE FORMA AUTOMÁTICA)
}

// send the state of the switch to the web browser
void GetSwitchState(WiFiClient cl)
{
    if (digitalRead(4)) {
        cl.println("Switch state: ON");
    }
    else {
        cl.println("Switch state: OFF");
    }
}

void loop()
{
  
    WiFiClient client = server.available();  // try to get client

    if (client) {  // got client?
        boolean currentLineIsBlank = true;
        String currentLine = "";
        while (client.connected()) {
            if (client.available()) {   // client data available to read
                char c = client.read(); // read 1 byte (character) from client
                HTTP_req += c;  // save the HTTP request 1 char at a time
                // last line of client request is blank and ends with \n
                // respond to client only after last line received
                if (c == '\n' && currentLineIsBlank) {
                    // send a standard http response header
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-Type: text/html");
                    client.println("Connection: keep-alive");
                    client.println();
                    // AJAX request for switch state
                    if (HTTP_req.indexOf("ajax_switch") > -1) {
                        // read switch state and send appropriate paragraph text
                        GetSwitchState(client);
                    }
                    else {  // HTTP request for web page
                       
                        // send web page - contains JavaScript with AJAX calls
                        client.println("<!DOCTYPE html>");
                        client.println("<html>");
                        client.println("<head>");
                        client.println("<title>Arduino Web Page</title>");
                        client.println("<script>");
                        client.println("function GetSwitchState() {");
                        client.println("nocache = \"&nocache=\"\+ Math.random() * 1000000;");
                        client.println("var request = new XMLHttpRequest();");
                        client.println("request.onreadystatechange = function() {");
                        client.println("if (this.readyState == 4) {");
                        client.println("if (this.status == 200) {");
                        client.println("if (this.responseText != null) {");
                        client.println("document.getElementById(\"switch_txt\")\
.innerHTML = this.responseText;");
                        client.println("}}}}");
                        client.println(
                        "request.open(\"GET\", \"ajax_switch\" + nocache, true);");
                      //client.println("request.open(\"GET\", \"ajax_switch\", true);");
                        client.println("request.send(null);");
                        client.println("}");
                        client.println("</script>");
                        client.println("</head>");
                        client.println("<body>");
                        client.println("<h1>Arduino AJAX Switch Status</h1>");
                        client.println(
                        "<h1 id=\"switch_txt\">Switch state: Not requested...</h1>");
                        client.println("<button style=\"width\:160;height\:50\" type=\"button\"\onclick=\"GetSwitchState()\">Get Switch State</button><br>");
                        
                        client.print("<h1>Click <a href=\"/H\">here</a> turn the LED on pin 4 on</h1>");
                        client.print("<h1>Click <a href=\"/L\">here</a> turn the LED on pin 4 off</h1>");
                        client.println("</body>");
                        client.println("</html>");
                    }
                    // display received HTTP request on serial port
                    Serial.print(HTTP_req);
                    HTTP_req = "";            // finished with request, empty string
                    break;
                }
                // every line of text received from the client ends with \r\n
                if (c == '\n') {
                    // last character on line of received text
                    // starting new line with next character read
                    currentLineIsBlank = true;
                } 
                else if (c != '\r') {
                    // a text character was received from client
                    currentLineIsBlank = false;
                    currentLine += c;      // add it to the end of the currentLine
                }
                if (currentLine.endsWith("GET /H")) {
              digitalWrite(4, HIGH);               // GET /H turns the LED on
            }
            if (currentLine.endsWith("GET /L")) {
              digitalWrite(4, LOW);                // GET /L turns the LED off
            }
            } // end if (client.available())
        } // end while (client.connected())
        delay(1);      // give the web browser time to receive the data

  

    }
}
