#include<ESP8266WiFi.h>
#include<ESP8266HTTPClient.h>
#include<WifiClient.h>
#include <ESP8266WebServer.h>

#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

#include <OneWire.h>
#include <DallasTemperature.h>

#include "DHT.h"

#include <MQ135.h>

#define REPORTING_PERIOD_MS     1000
#define DHTPIN 0     
#define DHTTYPE DHT11   
#define DS18B20 D5
#define PIN_MQ135 A0

float temperature, humidity, BPM, SpO2, bodytemperature, AirQuality;

DHT dht(DHTPIN, DHTTYPE);
OneWire oneWire(DS18B20);
DallasTemperature sensors(&oneWire);
MQ135 mq135_sensor(PIN_MQ135);
PulseOximeter pox;
uint32_t tsLastReport = 0;

ESP8266WebServer server(80);

const char* ssid = "KoneksiJelek";  // Enter SSID here
const char* password = "bilangdulu";  //Enter Password here

void onBeatDetected()
{
    Serial.println("Heartbeat detected!");
}

void setup() {
  Serial.begin(115200);
  Serial.print("Initializing pulse oximeter..");

  dht.begin();

  Serial.begin(115200);
  pinMode(16, OUTPUT);

  delay(2000);   

  Serial.println("Connecting to ");
  Serial.println(ssid);
 
  //connect to your local wi-fi network
  WiFi.begin(ssid, password);
 
  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
  delay(5000);
  Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());
 
  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);
 
  server.begin();
  Serial.println("HTTP server started");
 
  if (!pox.begin()) {
    Serial.println("FAILED");
    for (;;);
  } else {
    Serial.println("SUCCESS");
    Serial.println(" ");

    pox.setOnBeatDetectedCallback(onBeatDetected);
  }
    pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
}


void loop() {

  delay(2000);
  server.handleClient();
  sensors.requestTemperatures();

  MQ135 gasSensor = MQ135(A0);
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  float bodytemperature = sensors.getTempCByIndex(0);
  float BPM = pox.getHeartRate();
  float Spo2 = pox.getSpO2();
  float air_quality = gasSensor.getPPM();
  uint32_t tsLastReport = 0;
  pox.update();

   if (millis() - tsLastReport > REPORTING_PERIOD_MS)
  {
  Serial.print("Beat Perminute: ");
  Serial.println(BPM); // detak permenit

  Serial.print("Spo2: ");
  Serial.print(Spo2);
  Serial.println("%"); // oksigen tubuh

  // Check if any reads failed and exit early (to try again).
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  Serial.print(F("Humidity: "));
  Serial.print(humidity);
  Serial.print(F("%  Temperature: "));
  Serial.print(temperature);
  Serial.println(F("°C "));

  if (isnan(bodytemperature)){
    Serial.println("Pembacaan suhu tubuh Gagal");
    return;
  }
  Serial.print("Suhu Tubuh: ");
  Serial.print(bodytemperature);
  Serial.println("°C"); //suhu ruangan  


  Serial.print("Air Quality: ");  
  Serial.print(air_quality);
  Serial.println("  PPM");  
  Serial.println(" ");

    tsLastReport = millis();
  }

}
