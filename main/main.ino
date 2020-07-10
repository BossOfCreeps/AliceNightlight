/*
   MOSI ---> D7
   MISO ---> D6
   SCK ----> D5
   CS -----> D8
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <SPI.h>
#include <SD.h>

ESP8266WebServer ap_server(80);
WiFiServer wifi_server(80);

const char Get_WiFi_HTML[] = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no'>"
                             "<style>form { text-align: center; margin: auto; } input { width: 400px; height: 24px; margin: 1px; padding: 0px 0px 0px 3px; }"
                             ".but{ height: 30px; width: 407px; } h1{ font-size: 24px; text-align: center; }"
                             "@media (max-width: 767px) and (min-width: 360px) { input{ width: 90%; } .but{ width: 92%; } h1{ font-size: 20px; } }"
                             "</style></head><body><form action='/' method='post'><h1>Enter ssid and password for WiFi and MQTT information</h1>"
                             "<input required placeholder='WiFi SSID' type='text' name='wifi_ssid'><br>"
                             "<input required placeholder='WiFi Password' type='password' name='wifi_pass'><br><br>"
                             "<input required placeholder='MQTT Server' type='text' name='mqtt_serv'><br>"
                             "<input required placeholder='MQTT Port' type='text' name='mqtt_port'><br>"
                             "<input required placeholder='MQTT User' type='text' name='mqtt_user'><br>"
                             "<input required placeholder='MQTT Password' type='password' name='mqtt_pass'><br><br>"
                             "<input class= 'but' type='submit' value='Send'><br>"
                             "</form></body></html>";

String wifi_state = "Ok";

void(* resetFunc) (void) = 0; // объявляем функцию reset

int ledPin = LED_BUILTIN;

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void SD_card_write(String wifi_ssid, String wifi_pass, String mqtt_serv, String mqtt_port, String mqtt_user, String mqtt_pass) {
  File myFile = SD.open("data", FILE_WRITE);
  myFile.seek(0);
  myFile.println(wifi_ssid);
  myFile.println(wifi_pass);
  myFile.println(mqtt_serv);
  myFile.println(mqtt_port);
  myFile.println(mqtt_user);
  myFile.println(mqtt_pass);
  myFile.close();
  resetFunc();
}

void SD_card_read(String & wifi_ssid, String & wifi_pass, String & mqtt_serv, String & mqtt_port, String & mqtt_user, String & mqtt_pass ) {
  Serial.println("read");
  File myFile = SD.open("data");
  //myFile.seek(0);
  String data = "";
  while (myFile.available()) {
    data += String(char(myFile.read()));
  }
  wifi_ssid = getValue(data, '\\', 0);
  wifi_pass = getValue(data, '\\', 1);
  mqtt_serv = getValue(data, '\\', 2);
  mqtt_port = getValue(data, '\\', 3);
  mqtt_user = getValue(data, '\\', 4);
  mqtt_pass = getValue(data, '\\', 5);

  Serial.println(wifi_ssid);
  Serial.println(wifi_pass);
  Serial.println(mqtt_serv);
  Serial.println(mqtt_port);
  Serial.println(mqtt_user);
  Serial.println(mqtt_pass);

  //  ssid = ssid_password.substring(0, ssid_password.indexOf("\n") - 1);
  //password = ssid_password.substring(ssid_password.indexOf("\n") + 1, ssid_password.length() - 2);
  myFile.close();
}

void handleRoot() {
  if (ap_server.hasArg("wifi_ssid")) {
    SD_card_write(ap_server.arg("wifi_ssid"), ap_server.arg("wifi_pass"), ap_server.arg("mqtt_serv"), ap_server.arg("mqtt_port"), ap_server.arg("mqtt_user"), ap_server.arg("mqtt_pass"));
  }
  ap_server.send(200, "text/html", Get_WiFi_HTML);
}

void handleNotFound() {
  ap_server.send(404, "text/plain", "Go to 192.168,4,1");
}

void setup(void) {
  Serial.begin(9600);


  if (!SD.begin(4)) {
    wifi_state = "SD error";
    Serial.println(wifi_state);
  } else {
    if (!SD.exists("data")) {
      wifi_state = "SD empty";
      Serial.println(wifi_state);
      const char* wifi_ssid = "AliceNightLight";  // Enter SSID here
      const char* wifi_password = "12345678";  //Enter Password here
      WiFi.begin(wifi_ssid, wifi_password);
      ap_server.on("/", handleRoot);
      ap_server.onNotFound(handleNotFound);
      ap_server.begin();
    } else {
      String wifi_ssid, wifi_pass, mqtt_serv, mqtt_port, mqtt_user, mqtt_pass;
      SD_card_read(wifi_ssid, wifi_pass, mqtt_serv, mqtt_port, mqtt_user, mqtt_pass);
      Serial.println("Connect to Wifi");

      delay(10);

      pinMode(ledPin, OUTPUT);
      digitalWrite(ledPin, LOW);

      // Connect to WiFi network
      Serial.println();
      Serial.println();
      Serial.print("Connecting to ");
      Serial.print(wifi_ssid);
      Serial.print(" password ");
      Serial.println(wifi_pass);

      WiFi.begin(wifi_ssid, wifi_pass);

      int counter = 0;
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        counter++;
        if (counter > 60) {
          Serial.print("remove");
          SD.remove("data");
          resetFunc();
        }
      }

      Serial.println("");
      Serial.println("WiFi connected");

      // Start the server
      wifi_server.begin();
      Serial.println("Server started");

      // Print the IP address
      Serial.print("Use this URL to connect: ");
      Serial.print("http://");
      Serial.print(WiFi.localIP());
      Serial.println("/");
    }
  }
}

void loop(void) {

  if (wifi_state != "Ok") {
    ap_server.handleClient();
  } else {

    // Check if a client has connected
    WiFiClient client = wifi_server.available();
    if (!client) {
      return;
    }

    // Wait until the client sends some data
    Serial.println("new client");
    while (!client.available()) {
      delay(1);
    }

    // Read the first line of the request
    String request = client.readStringUntil('\r');
    Serial.println(request);
    client.flush();

    // Match the request

    int value = LOW;
    if (request.indexOf("/LED=ON") != -1)  {
      digitalWrite(ledPin, HIGH);
      value = HIGH;
    }
    if (request.indexOf("/LED=OFF") != -1)  {
      digitalWrite(ledPin, LOW);
      value = LOW;
    }

    client.println("<html><br><br><a href=\"/LED=ON\"\"><button>Turn On </button></a>client.println(\" < a href = \"/LED=OFF\"\"><button>Turn Off </button></a><br /></html>");

    delay(1);
    Serial.println("Client disonnected");
    Serial.println("");
  }
}
