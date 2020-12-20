#include <SPI.h>
#include <WiFi101.h>
#include <WiFiUdp.h>
#include <EasyNTPClient.h>
#include <TimeLib.h>
#include <TimeAlarms.h>
#include "arduino_secrets.h"

#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>   

#define WINC_CS   8
#define WINC_IRQ  7
#define WINC_RST  4
#define WINC_EN   2 
#define UDP_NTP_PORT 2390

WiFiSSLClient client;
WiFiUDP Udp;
EasyNTPClient ntpClient(Udp, "time.nist.gov"); 

Adafruit_MQTT_Client    mqtt(&client, "io.adafruit.com", 8883, AIO_USERNAME, AIO_KEY);  
Adafruit_MQTT_Subscribe aio_errors     = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/errors");
Adafruit_MQTT_Subscribe aio_throttle   = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/throttle");
Adafruit_MQTT_Subscribe aio_command    = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/cmd"); 

void setup()
{
  /*  Open serial communications and wait for port to open we only wait for this
   *  kind of demo.  You would remove this wait for any project.
   */
  Serial.begin(115200);
  while (!Serial) {
    delay(1000);
  }

  WiFi.setPins(WINC_CS, WINC_IRQ, WINC_RST, WINC_EN);
  wifi_connect();

  // Set up NTP
  delay(2000);             /* Give the UDP port time to set up */
  time_t t = getTime();
  while (t == 0){
    delay(1000);
    Serial.print(F("Waiting for NTP Sync: "));
    Serial.println((unsigned int)now());
    t = getTime();
  }
  setTime(t);
  setSyncInterval(1800);     /* NTP sync every 30 Minutes */
  setSyncProvider(getTime);
  
  Alarm.alarmRepeat(5,0,0, every_day_at_five_am); 
  Alarm.timerRepeat(15,    every_fifteen_seconds);  
  Alarm.timerRepeat(30,    every_thirty_seconds);

  mqtt.subscribe(&aio_command);
  mqtt.subscribe(&aio_errors);
  mqtt.subscribe(&aio_throttle); 
  MQTT_connect();
}

void loop(){
    MQTTProcessMessages(500);   /* Hand the thread to Adafruit to process incoming messages */
    Alarm.delay(250);           /* Hand the thread to the Alarm Class.  It will call Time.now() to ensure NTP sync */
    printTime();
    flash_built_in_led();       /* This includes a 400ms delay */
}


void MQTTProcessMessages(int timeout){
  MQTT_connect();
  Adafruit_MQTT_Subscribe *sub;
    while ((sub = mqtt.readSubscription(timeout))) {  /* You need a if block for each feed you subscribe to */
      if (sub == &aio_command) {
        handle_aio_command((char *)aio_command.lastread);
        
      } else if(sub == &aio_errors) {
        String mqtt_error = String((char *)aio_errors.lastread);
        Serial.print(F("MQTT ERROR: "));
        Serial.println(mqtt_error);

      } else if(sub == &aio_throttle){
        String mqtt_error = String((char *)aio_errors.lastread);
        Serial.print(F("MQTT THROTTLE ERROR: "));
        Serial.println(mqtt_error);
      }  
    }   
}

void handle_aio_command(char *data){
  /* You can handle events from the cmd feed here */
  
  Serial.print(F("handle_aio_command: recived data -> "));
  Serial.print(data);
}

void wifi_connect(){
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(F("Attempting to connect to SSID: "));
    Serial.println(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
  
    // wait 10 seconds for connection:
    uint8_t timeout = 10;
    while (timeout && (WiFi.status() != WL_CONNECTED)) {
      timeout--;
      for (int i = 0; i < 4; i++){
        Serial.print(F("."));
        delay(250);
      }
    }
    
    Serial.println(F("Wifi Connected"));
    Udp.begin(UDP_NTP_PORT);  /* Set up UDP port to listen and get the NTP responce */
    printWiFiStatus();
  }
}

void MQTT_connect() {

  wifi_connect();
  
  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.println(F("Connecting to MQTT... "));
  int8_t ret;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
     mqtt.disconnect();

     Serial.print(F("MQTT Error: "));
     Serial.println(mqtt.connectErrorString(ret));
     Serial.println(F("Retrying MQTT connection in 5 seconds..."));
     delay(5000);
  }
  Serial.println(F("MQTT Connected!"));
}


void every_fifteen_seconds(){
  Serial.println(F("Every 15 Seconds"));
}

void every_thirty_seconds(){
  Serial.println(F("Every 30 Seconds"));
  mqtt.ping();   /* This keeps the MQTT session alive */
}

void every_day_at_five_am(){
  Serial.println(F("Its 5 AM"));
}

time_t getTime(){
  wifi_connect();
  Serial.print(F("Syncing Time from NTP: "));
  time_t t = ntpClient.getUnixTime();
  Serial.println((unsigned long)t);
  return t;
}

void flash_built_in_led(){
  digitalWrite(LED_BUILTIN, HIGH); 
  delay(200);                     
  digitalWrite(LED_BUILTIN, LOW);  
  delay(200);                       
}


/* Print methods */

void printTime(){
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(F(" "));
  Serial.print(day());
  Serial.print(F(" "));
  Serial.print(month());
  Serial.print(F(" "));
  Serial.print(year()); 
  Serial.println(); 
}

void printDigits(int digits){
  // utility for digital clock display: prints preceding colon and leading 0
  Serial.print(F(":"));
  if(digits < 10)
    Serial.print(F("0"));
  Serial.print(digits);
}

void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print(F("SSID: "));
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  Serial.print(F("IP Address: "));
  Serial.println(WiFi.localIP());

  // print the received signal strength:
  Serial.print(F("Signal strength (RSSI):"));
  Serial.print(WiFi.RSSI());
  Serial.println(F(" dBm"));

  Serial.print(F("WiFi status: "));
  Serial.println(wifi_status_string(WiFi.status()));
}


String wifi_status_string(uint8_t wifi_status){

  switch(wifi_status) {
     case 255 :
        return F("NO SHIELD");
     case 0 :
        return F("IDLE STATUS");
     case 1 :
        return F("NO SSID AVAIL");
     case 2 :
        return F("SCAN COMPLETED");
     case 3 :
        return F("CONNECTED");
     case 4 :
        return F("CONNECT FAILED");
     case 5 :
        return F("CONNECTION LOST");
     case 6 :
        return F("AP LISTENING");
     case 7 :
        return F("AP CONNECTED");
     case 8 :
        return F("AP FAILED");
     case 9 :
        return F("PROVISIONING");
     case 10 :
        return F("PROVISIONING FAILED");
     default:
        return F("UNKNOWN STATUS");
    }
}
