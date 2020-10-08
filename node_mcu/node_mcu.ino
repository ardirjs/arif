#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <SoftwareSerial.h>

#define SSID "rjs"
#define PASS "mboh opo"
#define HOST "kontroler-mcu.firebaseio.com"
#define AUTH "JLpe1vjuPmuihvhNmldtBemCMDfdX5NjQNQuNwuV"

SoftwareSerial usart(12, 14);

void setup() {
  usart.begin(9600);
  Serial.begin(9600);
  WiFi.begin(SSID, PASS);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Connected: ");
  Serial.println(WiFi.localIP());
  Firebase.begin(HOST, AUTH);
  delay(5000);
  Serial.println("Start");
  usart.print("Start");
}

void loop() {
  if (usart.available()) {
    String json = usart.readString();
    Serial.print("Recv:\t");
    Serial.println(json);

    StaticJsonBuffer <200> doc;
    JsonObject &root = doc.parseObject(json);
    if (!root.success()) {
      return;
    }

    Firebase.setFloat("temperature", root["data"][0]);
    Firebase.setFloat("humidity", root["data"][1]);
    Firebase.setInt("fanStatus", root["data"][2]);
    Firebase.setInt("lampStatus", root["data"][3]);
    Firebase.setInt("sprayerStatus", root["data"][4]);

    JsonObject &roots = doc.createObject();
    JsonArray &data = roots.createNestedArray("data");
    data.add(Firebase.getFloat("setTemperature"));
    data.add(Firebase.getFloat("setHumidity"));
    data.add(Firebase.getFloat("setInterval"));

    String dataJson;
    roots.printTo(dataJson);
    Serial.print("Send:\t");
    Serial.println(dataJson);
    usart.print(dataJson);
  }
}
