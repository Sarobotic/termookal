#include <WiFi.h>
#include <ESP32Ping.h>
 
const char* ssid = "yourNetworkName";
const char* password =  "yourNetworkPass";
 
IPAddress staticIP(192, 168, 1, 150);
IPAddress gateway(192, 168, 1, 254);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(192, 168, 1, 254);
    
void setup(){
  Serial.begin(115200);
 
  if (WiFi.config(staticIP, gateway, subnet, dns, dns) == false) {
    Serial.println("Configuration failed.");
  }
    
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("Connecting...\n\n");
  }
 
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("Subnet Mask: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("Gateway IP: ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("DNS 1: ");
  Serial.println(WiFi.dnsIP(0));
  Serial.print("DNS 2: ");
  Serial.println(WiFi.dnsIP(1));
   
  bool success = Ping.ping("www.google.com", 3);
  
  if(!success){
    Serial.println("\nPing failed");
    return;
  }
  
  Serial.println("\nPing successful.");
}
    
void loop(){}
