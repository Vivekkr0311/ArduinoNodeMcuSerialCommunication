#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <SoftwareSerial.h>
//SoftwareSerial s(D6,D5);
#include <ArduinoJson.h>

#ifndef STASSID
#define STASSID "CIMFR"
#define STAPSK  "CIMFR_2019"
#endif
String data="Configuring...";
String page="";
String text="";
const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);

const int led = 13;
//Static IP address configuration
IPAddress staticIP(192, 168, 43, 90); //ESP static ip
IPAddress gateway(192, 168, 43, 1);   //IP Address of your WiFi Router (Gateway)
IPAddress subnet(255, 255, 255, 0);  //Subnet mask
IPAddress dns(8, 8, 8, 8);  //DNS
void handleRoot() {
  digitalWrite(led, 1);
  server.send(200, "text/plain", "hello from esp8266!");
  digitalWrite(led, 0);
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}
const char* deviceName = "GasSensor";
void setup(void) {
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(9600);
  //s.begin(9600);
  while (!Serial) continue;
  WiFi.disconnect();  //Prevent connecting to wifi based on previous configuration
  
  WiFi.hostname(deviceName);      // DHCP Hostname (useful for finding device for static lease)
  WiFi.config(staticIP, subnet, gateway, dns);
  //WiFi.begin(ssid, password);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/data.txt", [](){
    text = data;
    server.send(200, "text/html", text);
});
server.on("/inline", [](){
   page = "<h1>Sensor to Node MCU Web Server</h1><h1>Data:</h1><table style='width:100%'>";
   page+="<tr>";
   page+=   "<th>LPG</th>";
   page+=   "<th>SMOKE</th>";
   page+=   "<th>CO</th>";
   page+=   "<th>CH4</th>";
   page+=   "<th>ALCOHOL</th>";
   page+=   "<th>H2</th>";
   page+=   "<th>Propane</th>";
   page+="</tr>";
   page+=   "<th id='lpg'></th>";
   page+=   "<th id='smoke'></th>";
   page+=   "<th id='co'></th>";
    page+=   "<th id='ch4'></th>";
   page+=   "<th id='alcohol'></th>";
   page+=   "<th id='h2'></th>";
   page+=   "<th id='propane'></th>";
   page+="<tr>";
   page+="<h1 id=\"data\">""</h1>";
   page+="</tr>";
   page+="  </table>";
   page += "<script>\r\n";
   page += "var x = setInterval(function() {loadData(\"data.txt\",updateData)}, 1000);\r\n";
   page += "function loadData(url, callback){\r\n";
   page += "var xhttp = new XMLHttpRequest();\r\n";
   page += "xhttp.onreadystatechange = function(){\r\n";
   page += " if(this.readyState == 4 && this.status == 200){\r\n";
   page += " callback.apply(xhttp);\r\n";
   page += " }\r\n";
   page += "};\r\n";
   page += "xhttp.open(\"GET\", url, true);\r\n";
   page += "xhttp.send();\r\n";
   page += "}\r\n";
   page += "function updateData(){\r\n";
   page+=     "var tempData=this.responseText;";
   page+=     "tempData=tempData.split(',');";
   page +=     "document.getElementById('lpg').innerHTML =tempData[0];";
   page +=     "document.getElementById('smoke').innerHTML =tempData[1];";
   page +=     "document.getElementById('co').innerHTML =tempData[2];";
   page +=     "document.getElementById('ch4').innerHTML =tempData[3];";
   page +=     "document.getElementById('alcohol').innerHTML =tempData[4];";
   page +=     "document.getElementById('h2').innerHTML =tempData[5];";
   page +=     "document.getElementById('propane').innerHTML =tempData[6];";
   page += "}\r\n";
   page += "</script>\r\n";
   server.send(200, "text/html", page);
 });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  
  DynamicJsonDocument root(1000);
 DeserializationError error = deserializeJson(root, Serial); //9600
  if (error)
    return;
  Serial.println("JSON received and parsed");
  serializeJson(root, Serial);
  Serial.print("Data 1 ");
  Serial.println("");
  data=String((int)root["LPG"])+",";
  data+=String((int)root["Smoke"])+",";
  data+=String((int)root["CO"])+",";
  data+=String((int)root["CH4"])+",";
  data+=String((int)root["Alcohol"])+",";
  data+=String((int)root["H2"])+",";
  data+=String((int)root["Propane"]);
  Serial.print(data);
  server.handleClient();
  MDNS.update();
  delay(1000);
 
}
