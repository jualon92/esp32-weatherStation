#include "DHTesp.h" // Click here to get the library: http://librarymanager/All#DHTesp
#include <Ticker.h>

#ifndef ESP32
#pragma message(THIS EXAMPLE IS FOR ESP32 ONLY!)
#error Select ESP32 board.
#endif

//http requests
#include <WiFi.h>
#include <HTTPClient.h>
#include "Arduino.h"



#include "WiFi.h"
#include "driver/adc.h"
#include <esp_wifi.h>
#include <esp_bt.h>
// How many minutes the ESP should sleep
#define DEEP_SLEEP_TIME 5

//
const char* ssid = "TeleCentro-8bd5";
const char* password = "*********";
String serverName = "http://home-sensordata.herokuapp.com/temperaturas";


DHTesp dht;

void tempTask(void *pvParameters);
String getTemperature();
void triggerGetTemp();

/** Task handle for the light value read task */
TaskHandle_t tempTaskHandle = NULL;
/** Ticker for temperature reading */
Ticker tempTicker;
/** Comfort profile */
ComfortState cf;
/** Flag if task should run */
bool tasksEnabled = false;
/** Pin number for DHT11 data pin */
int dhtPin = 14;
//time in between readings
int cooldown =  60; //10 minutes


void initTemp() {
  byte resultValue = 0;
  // Initialize temperature sensor
  dht.setup(dhtPin, DHTesp::DHT11);
  Serial.println("DHT initiated");

}




String getTemperature() {
  // Reading temperature for humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
  TempAndHumidity newValues = dht.getTempAndHumidity();

  return  String(newValues.temperature);


}

void changeCPUClock(int freq) {
  while (!Serial) { //wait to initialize
    delay(500);
  }

  Serial.print("CPU Freq: ");
  Serial.println(getCpuFrequencyMhz());

  setCpuFrequencyMhz(freq);

  Serial.print("CPU Freq: "); //show new clock speed
  Serial.println(getCpuFrequencyMhz());
}

void initWIFI() {
  //wifi
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void dispatchTemperature(String temperatura) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;

    // Your Domain name with URL path or IP address with path
    http.begin(client, serverName);

    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(temperatura);




    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    // Free resources
    http.end();
  }
}


void goToSleep() {
  Serial.println("Going to sleep...");
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  btStop();

  adc_power_off();
  esp_wifi_stop();
  esp_bt_controller_disable();

  // Configure the timer to wake us up!
  esp_sleep_enable_timer_wakeup(DEEP_SLEEP_TIME * 60L * 1000000L);

  // Go to sleep! Zzzz
  esp_deep_sleep_start();
}

void setup() {
  Serial.begin(115200);

 // changeCPUClock(80);

  Serial.print("CPU Freq: ");
  Serial.println(getCpuFrequencyMhz());
  
  initWIFI();

  //sensor temp
  Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
  Serial.println("DHT ESP32 example with tasks");
  initTemp();

  // Signal end of setup() to tasks
  tasksEnabled = true;
}

void loop() {

  delay(2000);

  Serial.println("woke up");
  String temperaturaMedicion = getTemperature();
  Serial.println("temp: " + temperaturaMedicion );
  dispatchTemperature(temperaturaMedicion);
  goToSleep();

 
  yield();

}

