# TBTracker-STM32duino
TBTracker originally is a Plug and Play High Altitude Balloon tracker for ESP32. Supports LoRa, LoRa-APRS, RTTY, Horus V1 4FSK, Horus V2 4FSK.

In this fork the code is ported to an STM32G031F8P6 to replicate the same functionality on a small DIY balloon board.

TBTracker-ESP32 is an Arduino sketch for a tracker for high altitude weather or scientific balloons. 
It supports sending telemetry data in mulitple formats:
- RTTY
- LoRa
- LoRa-APRS
- Horus 4FSK V1
- Horus 4FSK V2

You can select multiple modes or just a single mode.

It is designed to send telemetry data in the correct format for https://amateur.sondehub.org.

The code is written in the Arduino IDE for ESP32 with a GPS module and a LoRa module like the sx1278 or sx1276 or Hope RFM9x.

# Library
You will need to install two extra Arduino library from the librairy manager:
- Radiolib
- TinyGPS++
- STM32 Arduino Core

# Hardware
This fork is currently only running on an experimental balloon board based around the STM32G031F8P6, which will be made available after some testing.
Since the STM32 is not as flexible as the ESP32 it currently has some limitations:
- No Debug Serial, due to low pin count
- Serial is currently implemented in SoftwareSerial

To add more functionality to the tracker, some extra features are already on the board which still have to be added or modified in code:
- Battery voltage is read via STM32 ADC -> conversion code from ADC value to voltage
- An I2C Header is fitted on the board for external sensors like BME280
- LEDs for status like Errors, TX and GPS are also fitted onto the board

# Horusbinary_radiolib
The radio related basecode for this sketch was taken from the horusbinary_radiolib project.
See: https://github.com/projecthorus/horusbinary_radiolib

# Installation
Change the values in the settings.h file.See the comments in the settings.h file. 
Prepare compilation in the Arduino IDE:
- Set the correct board: Tools -> Board -> STM32 MCU Based Boards -> Generic STM32G0 Series
- Select the correct model: Tools -> Board Part Number -> Generic G031F8Px
- Disable Serial, as it causes problems for now: Tools -> U(S)ART Support -> Disabled (No Serial Support)
- Select the correct programmer (I prefer SWD): Tools -> Upload Method -> STM32CubeProgrammer (SWD)

Now hook up the board to the ST-Link, Compile in the Arduino IDE and upload to your board.

# Versions

V0.1.0:
- Initial commit
- 
V0.1.1:  
- Added a temporary APRS device-ID (APZTBT)
- Added a timestamp to to the APRS packets for better compatibility with Sondehub

# TO DO:
- [X] Get the code running on STM32
- [X] Get Radio running
- [X] Get GPS running
- [ ] Add documentation on how to get the STM32 Programmer running
- [ ] Implement code for BME280 sensor
- [ ] Implement code for reading battery voltage from ADC and convert the ADC value into the voltage

