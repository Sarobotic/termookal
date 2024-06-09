#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
// WiFi
const char *ssid = "Ehrlich"; // Enter your WiFi name
const char *password = "@SaraSurena13871395";  // Enter WiFi password

// MQTT Broker
const char *mqtt_broker = "broker.emqx.io";
const char *topic = "esp32/test";
const char *mqtt_username = "emqx";
const char *mqtt_password = "public";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;
// GPIO where the DS18B20 is connected to
const int oneWireBus = 4;
 
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor
DallasTemperature sensors(&oneWire);
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows); 



void setup() {
// Start the Serial Monitor
Serial.begin(115200);
 while (!Serial)
    continue;
// Start the DS18B20 sensor
sensors.begin();
  lcd.init();
  // turn on LCD backlight                      
  lcd.backlight();
   WiFi.begin(ssid, password);
 while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.println("Connecting to WiFi..");
 }
 Serial.println("Connected to the WiFi network");
 //connecting to a mqtt broker
 client.setServer(mqtt_broker, mqtt_port);
 client.setCallback(callback);
 while (!client.connected()) {
     String client_id = "esp32-client-";
     client_id += String(WiFi.macAddress());
     Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
     if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
         Serial.println("Public emqx mqtt broker connected");
     } else {
         Serial.print("failed with state ");
         Serial.print(client.state());
         delay(2000);
     }
      }
}

void callback(char *topic, byte *payload, unsigned int length) {
 Serial.print("Message arrived in topic: ");
 Serial.println(topic);
 Serial.print("Message:");
 for (int i = 0; i < length; i++) {
     Serial.print((char) payload[i]);
 }
 Serial.println();
 Serial.println("-----------------------");
}
 
void loop() {
   client.loop();
  // Allocate the JSON document
  JsonDocument doc;
sensors.requestTemperatures();
char payload[60];
float temperatureC = sensors.getTempCByIndex(0);
float temperatureF = sensors.getTempFByIndex(0);
Serial.print(temperatureC);
Serial.println("C");
Serial.print(temperatureF);
Serial.println("F");
lcd.setCursor(0, 0);
  // print message
lcd.print(temperatureC);
lcd.print("C");
lcd.setCursor(0,1);
lcd.print(temperatureF);
lcd.println("F");
//sprintf(payload, "temprature: %f", temperatureC);

  // Add values in the document
 doc["temprature"] = temperatureC ;
 doc["time"] = 1351824120;

  // Generate the minified JSON and send it to the Serial port
 serializeJson(doc, payload);
 serializeJsonPretty(doc, Serial);
 Serial.println();
 client.publish(topic,payload );
 client.print(temperatureC);
 client.subscribe(topic);
 delay(1000);
}
