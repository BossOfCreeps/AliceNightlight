#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <SPI.h>
#include <SD.h>

ESP8266WebServer server(80);

const char Get_WiFi_HTML[] = "<!DOCTYPE html>"
                             "<html><head><meta name='viewport' content='width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no'><style>"
                             "form { text-align: center; margin: auto; } input { width: 400px; height: 24px; margin: 1px; padding: 0px 0px 0px 3px; }"
                             ".but{ height: 30px; width: 407px; } h1{ font-size: 24px; text-align: center; }"
                             "@media (max-width: 767px) and (min-width: 360px) { input{ width: 90%; } .but{ width: 92%' } }"
                             "</style></head><body><form action='/' method='post'><h1>Enter login and password for WiFi</h1>"
                             "<input placeholder='login' type='text' name='login'> <br><input placeholder='password' type='password' name='password'> <br>"
                             "<input class= 'but' type='submit' value='Send'> <br></form></body></html>";

void SD_card(String login = "NULL", String password = "NULL") {
  File myFile = SD.open("wifi.data", FILE_WRITE);
  if (login == "NULL" and password == "NULL") {
    login = myFile.read();
    password = myFile.read();
  } else {
    Serial.print("dsd");
    myFile.println(login);
    myFile.println(password);
  }
  myFile.close();
}

void handleRoot() {
  String login, password;
  if (server.hasArg("login")) login = server.arg("login");
  if (server.hasArg("password")) password = server.arg("password");
  Serial.println(login + " " + password);
  SD_card(login, password);
  server.send(200, "text/html", Get_WiFi_HTML);
}

void handleNotFound() {
  server.send(404, "text/plain", "Go to 192.168,4,1");
}

void setup(void) {
  Serial.begin(115200);

  String wifi_state = "Ok";

  if (!SD.begin(4)) {
    wifi_state = "SD error";
  }
  if (!SD.exists("wifi.data")) {
    wifi_state = "SD empty";
    const char* wifi_ssid = "AliceNightLight";  // Enter SSID here
    const char* wifi_password = "12345678";  //Enter Password here
    WiFi.begin(wifi_ssid, wifi_password);
    server.on("/", handleRoot);
    server.onNotFound(handleNotFound);
    server.begin();
  }
  if (wifi_state == "Ok") {
    Serial.println("Conect to Wifi");
  }
  Serial.println(wifi_state);
}

void loop(void) {
  server.handleClient();
}
