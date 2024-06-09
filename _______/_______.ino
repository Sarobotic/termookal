#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
const char* ssid = "Ehrlich";
const char* password = "@SaraSurena13871395";
const char* serverIP = "192.168.1.100"; // آدرس IP سرور خود را جایگزین کنید
const int serverPort = 5000; // پورت سرور Flask (مثلا 5000)
LiquidCrystal_I2C lcd(0x27, 16, 2);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);

const int oneWireBus = 4;
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);
void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.clear();
  lcd.backlight(); 
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to Wi-Fi");

  timeClient.begin();
  sensors.begin();
}
void loop() {
  timeClient.update();

  String formattedTime = timeClient.getFormattedTime(); 
  int offsetHours = 3; // Offset for Iran time zone
  int offsetMinutes = 30; // Offset for Iran time zone (considering daylight saving time)

// Extract current hour, minute, and second
  int currentHour = timeClient.getHours(); 
  int currentMinute = timeClient.getMinutes();
  int currentSecond = timeClient.getSeconds();

// Calculate Iran local time
  int hour = (currentHour + 4) % 24;
  int minute = (currentMinute + offsetMinutes) % 60;
  int second = currentSecond;

// Handle edge case where time exceeds 24 hours
  hour = (hour < 0) ? hour + 24 : hour;

// Format Iran local time with seconds
  String iranTime =  String(hour) + ":" + String(minute) + ":" + String(second);
  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0);
  DynamicJsonDocument jsonDoc(1024);
  jsonDoc["temperature"] = temperatureC;
  jsonDoc["time"] = iranTime;
  String jsonString;
  serializeJson(jsonDoc, jsonString);
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;

    if (client.connect(serverIP, serverPort)) {
      Serial.println("Connected to server");

      String data = "message=Hello, ESP32!";
      String httpRequest;
      httpRequest.concat("POST /postHTTP/1.1\r\n");
      httpRequest.concat("Host: " + String(serverIP) + ":" + String(serverPort) + "\r\n");
      httpRequest.concat("Content-Type: application/x-www-form-urlencoded\r\n");
      httpRequest.concat("Content-Length: " + String(data.length()) + "\r\n");
      httpRequest.concat("Connection: close\r\n");
      httpRequest.concat("\r\n");
      httpRequest.concat(data);

      client.print(httpRequest);

      unsigned long timeout = millis();
      while (client.connected() && millis() - timeout < 10000) {
        while (client.available()) {
          String line = client.readStringUntil('\r');
          Serial.print(line);
          timeout = millis();
        }
      }

      client.stop();
      Serial.println("Disconnected from server");
    } else {
      Serial.println("Connection to server failed");
    }
  } else {
    Serial.println("WiFi Disconnected");
  }

  delay(10000); // تاخیر 10 ثانیه‌ای بین درخواست‌ها
  lcd.setCursor(0, 0);
  lcd.print("TEMP: ");
  lcd.print(temperatureC);
  lcd.print(" C");
  lcd.setCursor(0, 1);
  lcd.print(iranTime);
  delay(1000);   
}
