#include <DHT.h>
#include <ArduinoJson.h>
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>

#define FAN 7
#define LAMP 8
#define POMPA A2
#define UNKNOWNS A3

DHT dht(9, DHT22);
SoftwareSerial usart(6, 5);
LiquidCrystal lcd(A1, A0, 13, 12, 11, 10);

static float h = 0, t = 0;
static int setTemperature = 0, setHumidity = 0, setInterval = 0;
static byte lampStatus = 0, fanStatus = 0, pompaStatus = 0;

void setup() {
  dht.begin();
  lcd.begin(16, 2);
  usart.begin(9600);
  Serial.begin(9600);
  int num = 4, bits[num] = {FAN, LAMP, POMPA, UNKNOWNS};
  for (int i = 0; i < num; i ++) {
    pinMode(bits[i], true);
    digitalWrite(bits[i], true);
  }
  lcd.clear();
  char *name = "ARIF SETIAWAN 16";
  lcd.setCursor(0, 0);
  for (int i = 0; i < 16; i ++) {
    lcd.print(name[i]);
    delay(0);
    if (name[i] != ' ') {
      delay(150);
    }
  }
  delay(1500);
  lcd.clear();
  delay(1000);
  lcd.setCursor(0, 1); lcd.print("Connecting...!!!");
}

void loop() {
  if (usart.available()) {
    String json = usart.readString();
    Serial.print("Recv:\t");
    Serial.println(json);

    h = dht.readHumidity();
    t = dht.readTemperature();
    if (isnan(h) || isnan(t)) {
      return;
    }

    StaticJsonBuffer <200> doc;

    JsonObject &root = doc.createObject();
    JsonArray &data = root.createNestedArray("data");
    data.add(t);
    data.add(h);
    data.add(fanStatus);
    data.add(lampStatus);
    data.add(pompaStatus);

    String dataJson;
    root.printTo(dataJson);
    Serial.print("Send:\t");
    Serial.println(dataJson);
    usart.print(dataJson);

    JsonObject &roots = doc.parseObject(json);
    if (!roots.success()) {
      return;
    }

    setTemperature = roots["data"][0];
    setHumidity = roots["data"][1];
    setInterval = roots["data"][2];
    /*
        static boolean clear = true;
        if (clear) {
          lcd.clear();
          clear = false;
        }
    */
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("T:     C");
    lcd.setCursor(6, 0); lcd.print((char)223);
    lcd.setCursor(9, 0); lcd.print("H:    %");
    lcd.setCursor(2, 0); lcd.print(t, 1);
    lcd.setCursor(11, 0); lcd.print(h, 1);

    lcd.setCursor(0, 1); lcd.print("ST:");
    lcd.setCursor(3, 1); lcd.print(setTemperature);
    lcd.setCursor(6, 1); lcd.print("SH:");
    lcd.setCursor(9, 1); lcd.print(setHumidity);
    lcd.setCursor(12, 1); lcd.print("IN:");
    lcd.setCursor(15, 1); lcd.print(setInterval);
  }

  if (t && h) {
    if ((float)t > (float)(setTemperature + setInterval)) {
      digitalWrite(FAN, false);
      digitalWrite(LAMP, true);
      fanStatus = 1;
      lampStatus = 0;
    }
    else if ((float)t < (float)(setTemperature - setInterval)) {
      digitalWrite(FAN, true);
      digitalWrite(LAMP, false);
      fanStatus = 0;
      lampStatus = 1;
    }
    else {
      digitalWrite(FAN, true);
      digitalWrite(LAMP, true);
      fanStatus = 0;
      lampStatus = 0;
    }
    if ((float)h < (float)setHumidity) {
      digitalWrite(POMPA, false);
      pompaStatus = 1;
    }
    else {
      digitalWrite(POMPA, true);
      pompaStatus = 0;
    }
  }
}
