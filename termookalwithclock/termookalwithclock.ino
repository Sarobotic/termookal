#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>

const char* ssid = "Ehrlich";
const char* password = "@SaraSurena13871395";

const char *mqtt_broker = "broker.emqx.io";
const char *topic = "esp32/test";
const char *mqtt_username = "emqx";
const char *mqtt_password = "public";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsgTime = 0;
unsigned long interval = 5000;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);

const int oneWireBus = 4;
OneWire oneWire(oneWireBus);
DallasTemperature sensors(&oneWire);
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  lcd.init();
  lcd.clear();
  lcd.backlight(); 

  Serial.begin(115200);
  sensors.begin();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to Wi-Fi");

  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  
  while (!client.connected()) {
    String client_id = "mqttx_" + String(WiFi.macAddress());
    Serial.printf("Connecting to MQTT broker as %s\n", client_id.c_str());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Connected to MQTT broker");
    } else {
      Serial.print("Failed to connect, state: ");
      Serial.print(client.state());
      delay(2000);
    }
  }
  timeClient.begin();
  client.subscribe(topic); 
}

void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message received in topic: ");
  Serial.println(topic);

  String received_message = "";
  for (int i = 0; i < length; i++) {
    received_message += (char)payload[i];
  }
  Serial.print("Message: ");
  Serial.println(received_message);
  Serial.println("-----------------------");
}

void loop() {

  client.loop();
  timeClient.update();
// Get current time from TimeClient
String formattedTime = timeClient.getFormattedTime(); 
int offsetHours = 3; // Offset for Iran time zone
int offsetMinutes = 30; // Offset for Iran time zone (considering daylight saving time)

// Extract current hour, minute, and second
int currentHour = timeClient.getHours(); 
int currentMinute = timeClient.getMinutes();
int currentSecond = timeClient.getSeconds();

// Calculate Iran local time
int hour = (currentHour + offsetHours) % 24;
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
  client.publish(topic, jsonString.c_str());
  Serial.println("Published JSON message: " + jsonString);
  lcd.setCursor(0, 0);
  lcd.print("TEMP: ");
  lcd.print(temperatureC);
  lcd.print(" C");
  lcd.setCursor(0, 1);
  lcd.print(iranTime);
  delay(1000);
}
