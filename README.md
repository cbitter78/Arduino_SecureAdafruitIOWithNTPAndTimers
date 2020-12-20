# Secure Adafruit.io with NTP time sync and Alarms

This is a small arduino example of how to:

- Connect to wifi
- Sync the clock (No hardware required) with NTP
- Securely Connect to Adafruit.io MQTT
- Subscribe to a feeds
- Process feeds
- Run tasks at a Specified interval such as every 30 seconds

## Prerequisites

- Adafruit.io account and key.  See [Adafruit IO Setup](https://learn.adafruit.com/adafruit-io-basics-esp8266-arduino/adafruit-io-setup).
  Add the user and key to the arduino_secrets.h file. You will need to rename the [arduino_secrets_example.h](WiFi101/SecureAdafruitIOWithNTPAndTimers/arduino_secrets_example.h) to arduino_secrets.h

## Required libraries

[EasyNTPClient](https://github.com/aharshac/EasyNTPClient)
[Time](https://playground.arduino.cc/Code/Time/)
[TimeAlarms](https://playground.arduino.cc/code/time/)
[Adafruit_MQTT](https://github.com/adafruit/Adafruit_MQTT_Library)

## Tested Boards

- [Adafruit Feather M0 WiFi - ATSAMD21 + ATWINC1500](https://www.adafruit.com/product/3010) Uses [WiFi101](WiFi101/README.md)
