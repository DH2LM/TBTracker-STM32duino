/************************************************************************************
* TBTracker-esp32 - roel@kroes.com
* 
*  FIRST THING YOU NEED TO DO IS ADJUST THE SETTINGS IN Settings.h
*  
*  Have FUN!
***********************************************************************************/

#include "Settings.h"
#include <SPI.h>
// #include "esp32-hal-cpu.h"
#include <RadioLib.h>
#include "horus_l2.h"
#include <SoftwareSerial.h>
#ifdef USE_BME280
#include <Wire.h>
#endif

#define TBTRACKER_VERSION v0.1.1

/***********************************************************************************
* DATA STRUCTS
*  
* Normally no change necessary
************************************************************************************/
// Struct to hold GPS data
struct TGPS
{
  int Hours, Minutes, Seconds, Day, Month, Year;
  float Longitude, Latitude;
  long Altitude;
  uint32_t RawTime;
  float Climb;
  float Speed;
  unsigned int Satellites;
  byte FlightMode;
  unsigned int Heading;
} UGPS;

// Struct to hold LoRA settings
struct TLoRaSettings
{
  float Frequency = LORA_FREQUENCY;
  float Bandwidth = LORA_BANDWIDTH;
  uint8_t SpreadFactor = LORA_SPREADFACTOR;
  uint8_t CodeRate = LORA_CODERATE;
  uint8_t SyncWord = LORA_SYNCWORD;
  uint8_t Power = LORA_POWER;
  uint8_t CurrentLimit = LORA_CURRENTLIMIT;
  uint16_t PreambleLength =  LORA_PREAMBLELENGTH;
  uint8_t Gain = LORA_GAIN;
} LoRaSettings;

// Struct to hold FSK settings
struct TFSKSettings
{
  float Frequency = FSK_FREQUENCY;
  float BitRate = FSK_BITRATE; 
  float FreqDev = FSK_FREQDEV;
  float RXBandwidth = FSK_RXBANDWIDTH;
  int8_t  Power = FSK_POWER;                  // in dbM range 2 - 17
  uint16_t  PreambleLength = FSK_PREAMBLELENGTH;
  bool  EnableOOK = FSK_ENABLEOOK;
  float dataShaping = FSK_DATASHAPING;
} FSKSettings;

// Struct to hold RTTY settings
struct TRTTYSettings
{
  float Frequency = RTTY_FREQUENCY;   // Base frequency
  uint32_t Shift = RTTY_SHIFT;        // RTTY shift
  uint16_t Baud = RTTY_BAUD;          // Baud rate
  uint8_t Encoding = RTTY_ASCII;   // Encoding (ASCII = 7 bits)
  uint8_t StopBits = RTTY_STOPBITS;   // Number of stopbits 
} RTTYSettings;

// Horus Binary Packet Structure - Version 1
struct HorusBinaryPacketV1
{
    uint8_t     PayloadID;    // Refer to  https://github.com/projecthorus/horusdemodlib/blob/master/payload_id_list.txt
    uint16_t    Counter;      // Packet counter 
    uint8_t     Hours;        // Hours Zulu
    uint8_t     Minutes;      // Minutes
    uint8_t     Seconds;      // Seconds
    float       Latitude;     // Latitude in format xx.yyyy
    float       Longitude;    // Longitude in format xx.yyyy
    uint16_t    Altitude;     // Altitude in meters
    uint8_t     Speed;        // Speed in kmh
    uint8_t     Sats;         // Number of satellites visible
    int8_t      Temp;         // Twos Complement Temp value.
    uint8_t     BattVoltage;  // 0 = 0.5v, 255 = 5.0V, linear steps in-between.
    uint16_t    Checksum;     // CRC16-CCITT Checksum.
}  __attribute__ ((packed));

// Horus v2 Mode 1 (32-byte) Binary Packet
struct HorusBinaryPacketV2
{
    uint16_t    PayloadID;    // Refer to  https://github.com/projecthorus/horusdemodlib/blob/master/payload_id_list.txt 
    uint16_t    Counter;      // Packet counter 
    uint8_t     Hours;        // Hours Zulu
    uint8_t     Minutes;      // Minutes
    uint8_t     Seconds;      // Seconds
    float       Latitude;     // Latitude in format xx.yyyy
    float       Longitude;    // Longitude in format xx.yyyy
    uint16_t    Altitude;     // Altitude in meters
    uint8_t     Speed;        // Speed in kmh
    uint8_t     Sats;         // Number of satellites visible 
    int8_t      Temp;         // Twos Complement Temp value.
    uint8_t     BattVoltage;  // 0 = 0.5v, 255 = 2.0V, linear steps in-between.
    // The following 9 bytes (up to the CRC) are user-customizable. The following just
    // provides an example of how they could be used.
    // Refer here for details: https://github.com/projecthorus/horusdemodlib/wiki/5-Customising-a-Horus-Binary-v2-Packet
    int16_t     dummy1;       // Interpreted as Ascent rate divided by 100 for the Payload ID: 4FSKTEST-V2 
    int16_t     dummy2;       // External temperature divided by 10 for the Payload ID: 4FSKTEST-V2 
    uint8_t     dummy3;       // External humidity for the Payload ID: 4FSKTEST-V2 
    uint16_t    dummy4;       // External pressure divided by 10 for the Payload ID:  4FSKTEST-V2  
    uint16_t    unused;       // 2 bytes which are not interpreted
    uint16_t    Checksum;     // CRC16-CCITT Checksum.

}  __attribute__ ((packed));

uint8_t rawbuffer [256];   // Buffer to temporarily store a raw binary packet.
uint8_t codedbuffer [256]; // Buffer to store an encoded binary packet
char debugbuffer[256]; // Buffer to store debug strings

/***********************************************************************************
* GLOBALS
*  
* Normally no change necessary
************************************************************************************/
// HardwareSerial SerialGPS(Rx, Tx);
SoftwareSerial SerialGPS(Rx, Tx);
// SoftwareSerial SerialDebug(PA15, PA13);
// SPIClass radioSPI(MOSI, MISO, SCK, CS);
char Sentence[SENTENCE_LENGTH];
long RTTYCounter=1;
long LoRaCounter=1;
long horusCounterV1=1;
long horusCounterV2=1;
unsigned long previousTX_LoRa = 0;
unsigned long previousTX_RTTY = 0;
unsigned long previousTX_HorusV1 = 0;
unsigned long previousTX_HorusV2 = 0;
unsigned long previousTX_LoRa_APRS = 0;
bool disableLEDs;
uint32_t prevTime = 1;
uint32_t prevHeight = 0;

//*********************************************************************************************************************
// Generate a Horus Binary v1 packet, and populate it with data.
//*********************************************************************************************************************
int build_horus_binary_packet_v1(uint8_t *buffer)
{
  struct HorusBinaryPacketV1 BinaryPacket;

  BinaryPacket.PayloadID   = PAYLOAD_ID_V1; 
  BinaryPacket.Counter     = horusCounterV1++;
  BinaryPacket.Hours       = UGPS.Hours;
  BinaryPacket.Minutes     = UGPS.Minutes;
  BinaryPacket.Seconds     = UGPS.Seconds;
  BinaryPacket.Latitude    = UGPS.Latitude;
  BinaryPacket.Longitude   = UGPS.Longitude;
  BinaryPacket.Altitude    = UGPS.Altitude;
  BinaryPacket.Speed       = 0;
  BinaryPacket.BattVoltage = 0;
  BinaryPacket.Sats        = UGPS.Satellites;
  BinaryPacket.Temp        = 0;
  BinaryPacket.Checksum    = (uint16_t)crc16((unsigned char*)&BinaryPacket,sizeof(BinaryPacket)-2);

  memcpy(buffer, &BinaryPacket, sizeof(BinaryPacket));
  return sizeof(struct HorusBinaryPacketV1);
}

//*********************************************************************************************************************
// Generate a Horus Binary v2 packet, and populate it with data.
//*********************************************************************************************************************
int build_horus_binary_packet_v2(uint8_t *buffer)
{
  struct HorusBinaryPacketV2 BinaryPacketV2;

  //calculate voltage
  float fVoltage = ReadVCC();

  uint8_t ui8Voltage = round( fVoltage * (255.0 / 5.0) );

  //calculate ascent rate
  int16_t iAscRate = round(UGPS.Climb * 100.0);

  //calculate speed
  float speedKph = UGPS.Speed;
  uint8_t ui8speed = 0;
  if(speedKph < 0) ui8speed = 0;
  else if(speedKph > 255) ui8speed = 255;
  else ui8speed = round(speedKph);

  //calculate voltage
  int32_t i32vrefVal = readVref();
  int32_t i32tempVal = readTempSensor(i32vrefVal);
  int8_t i8tempVal;
  if(i32tempVal < -128) i8tempVal = -128;
  else if(i32tempVal > 127) i8tempVal = 127;
  else i8tempVal = i32tempVal;

  BinaryPacketV2.PayloadID   = PAYLOAD_ID_V2; 
  BinaryPacketV2.Counter     = horusCounterV2++;
  BinaryPacketV2.Hours       = UGPS.Hours;
  BinaryPacketV2.Minutes     = UGPS.Minutes;
  BinaryPacketV2.Seconds     = UGPS.Seconds;
  BinaryPacketV2.Latitude    = UGPS.Latitude;
  BinaryPacketV2.Longitude   = UGPS.Longitude;
  BinaryPacketV2.Altitude    = UGPS.Altitude;
  BinaryPacketV2.Speed       = ui8speed;
  BinaryPacketV2.BattVoltage = ui8Voltage;
  BinaryPacketV2.Sats        = UGPS.Satellites;
  BinaryPacketV2.Temp        = i8tempVal;
  // Custom section. This is an example only, and the 9 bytes in this section can be used in other
  // ways. Refer here for details: https://github.com/projecthorus/horusdemodlib/wiki/5-Customising-a-Horus-Binary-v2-Packet
  // BinaryPacketV2.dummy1      = 200;  // int16_t Interpreted as Ascent rate divided by 100 for 4FSKTEST-V2. This value would display as 2.0 on HabHub 
  // BinaryPacketV2.dummy2      = -20;  // int16_t External temperature divided by 10 for 4FSKTEST-V2. This value would display as -2.0 on HabHub   
  // BinaryPacketV2.dummy3      = 51;   // uint8_t External humidity for 4FSKTEST-V2. This value would display as 51 on HabHub  
  // BinaryPacketV2.dummy4      = 21;   // uint16_t External pressure divided by 10 for 4FSKTEST-V2. This value would display as 2.1 on HabHub  
  // BinaryPacketV2.unused      = 0;    // Two unused filler bytes
  
  BinaryPacketV2.dummy1      = iAscRate;  // int16_t Interpreted as Ascent rate divided by 100 for 4FSKTEST-V2. This value would display as 2.0 on HabHub 
  BinaryPacketV2.dummy2      = 0;  // int16_t External temperature divided by 10 for 4FSKTEST-V2. This value would display as -2.0 on HabHub   
  BinaryPacketV2.dummy3      = 0;   // uint8_t External humidity for 4FSKTEST-V2. This value would display as 51 on HabHub  
  BinaryPacketV2.dummy4      = 0;   // uint16_t External pressure divided by 10 for 4FSKTEST-V2. This value would display as 2.1 on HabHub  
  BinaryPacketV2.unused      = 0;    // Two unused filler bytes
  BinaryPacketV2.Checksum    = (uint16_t)crc16((unsigned char*)&BinaryPacketV2,sizeof(BinaryPacketV2)-2);

  memcpy(buffer, &BinaryPacketV2, sizeof(BinaryPacketV2));
  return sizeof(struct HorusBinaryPacketV2);
}


//============================================================================
void setup()
{
    disableLEDs = false;
  pinMode(LED_GRN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GPS, OUTPUT);

  digitalWrite(LED_GRN, HIGH);
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GPS, HIGH);

  delay(100);
  // SEt CPU speed to 80MHz to spare energy
  // setCpuFrequencyMhz(80);
    // Setup Serial for debugging
  SerialGPS.begin(GPSBaud);
  // SerialDebug.begin(9600);
    // Setup the GPS
  // SerialGPS.begin(GPSBaud, SERIAL_8N1);  //TX, RX
  delay(100);

  digitalWrite(LED_RED, LOW);
  delay(250);

    // SPI.begin(SCK,MISO,MOSI,CS);
  SPI.setMISO(MISO);
  SPI.setMOSI(MOSI);
  // SPI.setSSEL(CS);
  SPI.setSCLK(SCK);
  SPI.begin();

  digitalWrite(LED_GRN, LOW);

    // Setup the Radio
  ResetRadio();
  SetupRadio();  
  SetupBME280();

  digitalWrite(LED_GRN, HIGH);

  analogReadResolution(12);
  // float ftemp = getInternalTemp();
  // delay(500);
  digitalWrite(LED_GRN, LOW);
}


//============================================================================
void loop()
{
     unsigned long currentMillis = millis();

     // Get data from the GPS
     smartDelay(1000);   
     CheckGPS();
     
     if(UGPS.Altitude > ALT_DISABLE_LEDs)
     {
        digitalWrite(LED_GPS, LOW);
        digitalWrite(LED_GRN, LOW);
        digitalWrite(LED_RED, LOW);
        disableLEDs = true;
     }
     else disableLEDs = false;

       // Send RTTY
       if ((RTTY_ENABLED) && (currentMillis - previousTX_RTTY >= ((unsigned long)RTTY_LOOPTIME*(unsigned long)1000)))
       { 
          if(!disableLEDs) digitalWrite(LED_GRN, HIGH);
          for (int i=1; i <= RTTY_REPEATS; i++)
          {
            CreateTXLine(RTTY_PAYLOAD_ID, RTTYCounter++, RTTY_PREFIX);
            sendRTTY(Sentence); 
          }
          previousTX_RTTY = currentMillis;
          digitalWrite(LED_GRN, LOW);
       }
     
       // Send LoRa 
       if ((LORA_ENABLED) && (currentMillis - previousTX_LoRa >= ((unsigned long)LORA_LOOPTIME*(unsigned long)1000)))
       { 
          if(!disableLEDs) digitalWrite(LED_GRN, HIGH);
          delay(1000);
          for (int i=1; i <= LORA_REPEATS; i++)
          {
            CreateTXLine(LORA_PAYLOAD_ID, LoRaCounter++, LORA_PREFIX);
            sendLoRa(Sentence,LORA_MODE); 
          }
          previousTX_LoRa = currentMillis;
          digitalWrite(LED_GRN, LOW);
       }

       // Send HORUS V1
       if ((HORUS_V1_ENABLED) && (currentMillis - previousTX_HorusV1 >= ((unsigned long)HORUS_LOOPTIME*(unsigned long)1000)))
       {
          if(!disableLEDs) digitalWrite(LED_GRN, HIGH);
          delay(1000);
          sendHorusV1();
          previousTX_HorusV1 = currentMillis;
          digitalWrite(LED_GRN, LOW);
       }

       // Send HORUS V2
       if ((HORUS_V2_ENABLED) && (currentMillis - previousTX_HorusV2 >= ((unsigned long)HORUS_LOOPTIME*(unsigned long)1000)))
       {
          if(!disableLEDs) digitalWrite(LED_GRN, HIGH);
          delay(1000);  
          sendHorusV2();
          previousTX_HorusV2 = currentMillis;
          digitalWrite(LED_GRN, LOW);
       }

       // Send LORA-APRS
       if ((LORA_APRS_ENABLED) && (currentMillis - previousTX_LoRa_APRS >= ((unsigned long)LORA_APRS_LOOPTIME*(unsigned long)1000)))
       {
          if(!disableLEDs) digitalWrite(LED_GRN, HIGH);
         delay(1000);
         sendLoRaAprs();
         previousTX_LoRa_APRS = currentMillis;
         digitalWrite(LED_GRN, LOW);
       }
}
