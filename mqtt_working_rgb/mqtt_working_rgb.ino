
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char *ssid =  "";  // Имя вайфай точки доступа
const char *pass =  ""; // Пароль от точки доступа

const char *mqtt_server = "alicenightlight.ddns.net"; // Имя сервера MQTT
const int mqtt_port = 1883; // Порт для подключения к серверу MQTT
const char *mqtt_user = ""; // Логи от сервер
const char *mqtt_pass = ""; // Пароль от сервера

const int R = D7;
const int G = D6;
const int B = D5;

int bright = 100;
int stled = 1;
int r = 255;
int g = 255;
int b = 255;

#define BUFFER_SIZE 100

void set_rgb() {
  analogWrite(R, bright * r * stled / 100);
  analogWrite(G, bright * g * stled / 100);
  analogWrite(B, bright * b * stled / 100);
}

void callback(const MQTT::Publish& pub)
{
  String payload = pub.payload_string();

  if (String(pub.topic()) == "test/rgb")
  {
    int dataSt = String(payload).toInt();
    stled = 1;

    switch (dataSt) {
      case 2700 : r = 251; g = 165; b = 30; break;
      case 3400 : r = 255; g = 234; b = 0; break;
      case 4500 : r = 255; g = 255; b = 255;  break;
      case 5600 : r = 179; g = 255; b = 253;  break;
      case 6500 : r = 141; g = 232; b = 253;  break;
      case 7500 : r = 122; g = 215; b = 249;  break;
      default: r = dataSt / 256 / 256; g = dataSt / 256 % 256; b = dataSt % 256; break;
    }
  }

  if (String(pub.topic()) == "test/led") // проверяем из нужного ли нам топика пришли данные
  {
    stled = payload.toInt(); // преобразуем полученные данные в тип integer
    if (stled == 1) {
      bright = 100;
      r = 255;
      g = 255;
      b = 255;
    }
  }

  if (String(pub.topic()) == "test/bright") // проверяем из нужного ли нам топика пришли данные
  {
    bright = payload.toInt();
  }

  set_rgb();
}

WiFiClient wclient;
PubSubClient client(wclient, mqtt_server, mqtt_port);

void setup()
{
  pinMode (R, OUTPUT);
  pinMode (G, OUTPUT);
  pinMode (B, OUTPUT);
  pinMode (D6, OUTPUT);
  Serial.begin(115200);
  set_rgb();
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    WiFi.begin(ssid, pass);
    if (WiFi.waitForConnectResult() != WL_CONNECTED)
      return;
  }
  if (WiFi.status() == WL_CONNECTED)
  {
    if (!client.connected())
    {
      if (client.connect(MQTT::Connect("arduinoClient2").set_auth(mqtt_user, mqtt_pass)))
      {
        client.set_callback(callback);
        // подписывааемся по топик с данными для светодиода
        client.subscribe("test/rgb");
        client.subscribe("test/led");
        client.subscribe("test/bright");
      }
    }
    if (client.connected()) {
      client.loop();
    }
  }
}
