/***********************************************************************************
* Important: CHANGE THE SETTINGS BELOW so it matches your configuration
************************************************************************************/

/***********************************************************************************
* PIN NUMBERS for SX127x
*  
* Change if needed
* the default pins for my ESP32-S2 are: SCK=36, MISO=37, MOSI=35, NSS=34 
* But you can redefine the SPI pins by creating defines: 
* #define SCK 36
* #define MISO 37
* #define MOSI 35
* #define CS 34
************************************************************************************/
//Model defines, use only one
// #define TinyTracker //TinyTracker (V2), based on STM32G031F8P6 + Ra-01
#define AttoAPRSv1 //AttoAPRS V1, based on STM32WLE5CCU6
// #define AttoAPRSv2 //AttoAPRS V2, based on STM32L432KBU6 + Si4432

#ifdef TinyTracker
  #ifdef AttoAPRSv1
    #error "Only one model can be defined at a time!"
  #endif
  #ifdef AttoAPRSv2
    #error "Only one model can be defined at a time!"
  #endif
#endif

#ifdef AttoAPRSv1
  #ifdef AttoAPRSv2
    #error "Only one model can be defined at a time!"
  #endif
#endif

// #define USE_BME280
// #define TESTMODE
#define USE_BMP280

#ifdef USE_BMP280
  #ifdef USE_BME280
    #error "Only one external sensor model can be defined at a time!"
  #endif
#endif

#define ALT_DISABLE_LEDs 1500

#ifdef TinyTracker
//Original variant with STM32G031F8P6 + Ra-01
#define LED_GPS PC15
#define LED_RED PB6
#define LED_GRN PA11
#define ALT_DISABLE_LEDs 1500

#define SCL PB8
#define SDA PB9

#define SCK PA5
#define MISO PA6
#define MOSI PA2
#define CS PA4

#define BAT_ADC PA12

#define PIN_NSS   PA4
#define PIN_DIO0  PA7
#define PIN_BUSY  -1  // Not used in this sketch for sx1278
#define PIN_RESET  PA3
#define PIN_DIO1  -1  // Not used in this sketch for sx1278
#define PIN_DIO2  -1  // Not used in this sketch for sx1278
#endif

#ifdef AttoAPRSv1
//Original variant of AttoAPRS with STM32WLE5CCU6
#define LED_GPS PA7
#define LED_RED PA6
#define LED_GRN PA5

#define SCL PA12
#define SDA PA11

#define SCK -1
#define MISO -1
#define MOSI -1
#define CS -1

#define BAT_ADC PB2

//Radio pin definition not needed for STM32WLx
#define PIN_NSS   -1
#define PIN_DIO0  -1
#define PIN_BUSY  -1 
#define PIN_RESET  -1
#define PIN_DIO1  -1 
#define PIN_DIO2  -1 
#endif

#ifdef AttoAPRSv2
//New variant of AttoAPRS with STM32L432KBU6 + Si4432. Note that the Si4432 is not supported by all features of this sketch, as it is a more basic radio than the SX127x series.
//For example, LoRa is NOT supported by Si4432, but the rest of the modes are.
#define LED_GPS -1
#define LED_GRN PB3
#define LED_RED PB4

#define SCL PB6
#define SDA PB7

#define SCK PA1
#define MISO PA6
#define MOSI PA7
#define CS -1

#define BAT_ADC PB1

#define PIN_NSS   PA4
#define PIN_IRQ   PA5
#define PIN_BUSY  -1  // Not used in this sketch for Si4432
#define PIN_SDN   PA8
#define PIN_DIO1  -1  // Not used in this sketch for Si4432
#define PIN_DIO2  PA0 //AKA GPIO2
#endif
/***********************************************************************************
* Enable the different protocols for transmission.
* You can choose one, more or all
* See below for the settings for the different protocols
************************************************************************************/
#define RTTY_ENABLED        false      // Set to true if you want RTTY transmissions
#define LORA_ENABLED        true       // Set to true if you want LoRa transmissions
#define RECEIVING_ENABLED   true  // Set to true if you want the Tracker to listen on the LoRa frequency for incoming packets
#define HORUS_V1_ENABLED    false  // Set to true if you want HorusBinary V1 transmissions
#define HORUS_V2_ENABLED    true  // Set to true if you want HorusBinary V2 transmissions
#define LORA_APRS_ENABLED   true  // Set to true if you want LORA-APRS transmissions (experimental)
#define FM_APRS_ENABLED   false  // Set to true if you want FM-APRS transmissions (experimental)
#define LORA_APRS_TELEM_ENABLED true //Set to true if you want LORA-APRS transmit telemetry data
#define FM_APRS_TELEM_ENABLED false //Set to true if you want FM-APRS transmit telemetry data
#define CW_ENABLED   true  // Set to true if you want FM-APRS transmissions (experimental)


/***********************************************************************************
* RTTY SETTINGS
*  
* Personalize when you have RTTY_ENABLED set to true and you want RTTY transmissions
* Default RTTY setting is: 7,N,2 at 100 Baud.
************************************************************************************/
#define RTTY_PAYLOAD_ID  "N0CALL" // This will show on Sondehub. Payload ID for RTTY protocol. CHANGE THIS!
#define RTTY_FREQUENCY  434.4    // in MHz
#define RTTY_SHIFT 610             // in increments of 61. 610 is usually a good value.
#define RTTY_BAUD 100              // Baud rate. You should set this to 50 or 100 usually
#define RTTY_STOPBITS 2            // Usually leave this at 2 
#define RTTY_PREFIX "$$$$$$"       // As RTTY with the sx127x chip is challenging, prefix with at least 4x$   
#define RTTY_REPEATS 1             // number of RTTY transmits during a cycle, usually set to 1 
#define RTTY_LOOPTIME 45           // Transmit RTTY every xx seconds
#define RTTY_IDLE_TIME 4000        // Idle carrier in ms before sending actual RTTY string. 
                                   // Set to a low value (i.e. 1000 or lower) if you have a very frequency stable signal
                                   // Set to a high value (i.e. 5000 or even higher) if you have a hard time to tune the signal

/***********************************************************************************
* LORA SETTINGS
*  
* Personalize when you have LORA_ENABLED set to true and you want LORA transmissions
* About HAB modes:
* 0 = (normal for telemetry,     60 baud)  Explicit mode, Error coding 4:8, Bandwidth 20.8kHz, SF 11,Low data rate optimize on  - SUPPORTED
* 1 = (normal for SSDV,        1400 baud)  Implicit mode, Error coding 4:5, Bandwidth 20.8kHz, SF 6, Low data rate optimize off - SUPPORTED
* 2 = (normal for repeater,    2000 baud)  Explicit mode, Error coding 4:8, Bandwidth 62.5kHz, SF 8, Low data rate optimize off - SUPPORTED
* 3 = (normal for fast SSDV,   8000 baud)  Explicit mode, Error coding 4:6, Bandwidth 250kHz,  SF 7, Low data rate optimize off - SUPPORTED
* 4 = (Turbo SSDV,            16828 baud)  Explicit mode, Error coding 4:5, Bandwidth 250kHz,  SF 6, Low data rate optimize off - SUPPORTED
* 5 = (normal for calling mode, 200 baud)  Explicit mode, Error coding 4:8, Bandwidth 41.7kHz, SF 11,Low data rate optimize off - SUPPORTED
* 6 = (uplink 868,             2800 baud)  Explicit mode, Error coding 4:5, Bandwidth 41.7kHz, SF 6, Low data rate optimize off - SUPPORTED
* 7 = (Telnet comms 434,       2800 baud)  Explicit mode, Error coding 4:5, Bandwidth 20.8kHz, SF 7, Low data rate optimize off - SUPPORTED
* 8 = (SSDV repeater,          4500 baud)  Explicit mode, Error coding 4:5, Bandwidth 62.5kHz, SF 6, Low data rate optimize off - SUPPORTED
* 99 = (World wide LoRa-APRS mode at 433.775MHz, Explicit mode, Error coding 4:5, Bandwidth 125kHz, SF 12,Low data rate optimize off - SUPPORTED  )
************************************************************************************/
#define LORA_PAYLOAD_ID  "N0CALL-L"  // This will show on Sondehub. Payload ID for LoRa protocol. CHANGE THIS!
#define LORA_FREQUENCY  434.662     // in MHz
#define LORA_MODE 2                 // Mode 2 is usually used for simple telemetry data
#define LORA_REPEATS 1              // number of LoRa transmits during a cycle
#define LORA_LOOPTIME 120            // Transmit LoRa every xx seconds

/***********************************************************************************
* LORA-APRS SETTINGS
*  
* Personalize when you have LORA_APRS_ENABLED set to true and you want LORA-APRS transmissions
************************************************************************************/
#define LORA_APRS_PAYLOAD_ID  "N0CALL"  // CHANGE THIS. This will show on Sondehub. For LORA-APRS this should be a HAM call without SSID.
#define LORA_APRS_SSID "-11"            // 11 is the symbol for balloon
#define LORA_APRS_MODE 99                     
#define LORA_APRS_FREQUENCY  433.775  // LORA-APRS is worldwide on 433.775 MHz, do not change
#define LORA_APRS_LOOPTIME 60        // Set this rather high (>120s), so you won't be flagged for misusing the APRS network
#define LORA_APRS_TELEMETRY_EVERY 2        // Set this rather high (>300s), so you won't be flagged for misusing the APRS network


/***********************************************************************************
* FM-APRS SETTINGS
*  
* Personalize when you have FM_APRS_ENABLED set to true and you want FM-APRS transmissions
************************************************************************************/
#define FM_APRS_PAYLOAD_ID  "N0CALL"   // CHANGE THIS. This will show on Sondehub. For LORA-APRS this should be a HAM call without SSID.
#define FM_APRS_SSID "-11"            // 11 is the symbol for balloon
#define FM_APRS_SSIDNUM 11
#define FM_APRS_MODE 99                     
#define FM_APRS_POWER 10
#define FM_APRS_FREQUENCY  432.500    // FM-APRS is worldwide on 433.775 MHz, do not change
#define FM_APRS_LOOPTIME 30          // Set this rather high (>120s), so you won't be flagged for misusing the APRS network
#define FM_APRS_TELEMETRY_EVERY 2     // Set this rather high (>300s), so you won't be flagged for misusing the APRS network

/***********************************************************************************
* CW SETTINGS
*  
* Personalize when you have FM_APRS_ENABLED set to true and you want FM-APRS transmissions
************************************************************************************/
#define CW_CALLSIGN  "N0CALL"   // CHANGE THIS. This will show on Sondehub. For LORA-APRS this should be a HAM call without SSID.
#define CW_TRANSMIT_COORDS false // Set to true if you want the CW beacon to include the full coordinates. This may be less efficient than using a locator.
#define CW_TRANSMIT_LOC true // Set to true if you want the CW beacon to include a QTH locator
#define CW_QTH_LOC_SIZE 8 // Set size of QTH locator. Example: 6 will result in a locator like "JO60LN"
#define CW_TRANSMIT_ALT true // Set to true if you want to transmit the altitude like "A 12345 M"
#define CW_TRANSMIT_VOLTAGE true
#define CW_TRANSMIT_TEMP false // Set to true if you want to transmit the INTERNAL temperature
#define CW_TRANSMIT_SENSORDATA false // Set to true if you want to transmit external sensor data (if available)
#define CW_CUSTOM_MSG "Still alive 73"
#define CW_POWER 10
#define CW_SPEED 25 // CW speed in WPM
#define CW_FREQUENCY  434.400    // FM-APRS is worldwide on 433.775 MHz, do not change
#define CW_LOOPTIME 300          // Set this rather high (>120s), so you won't be flagged for misusing the APRS network
// #define FM_APRS_TELEMETRY_EVERY 2     // Set this rather high (>300s), so you won't be flagged for misusing the APRS network

/***********************************************************************************
* HORUSBINARY SETTINGS
*  
* Personalize when you have HORUS_V*_ENABLED set to true and you want HORUS transmissions
************************************************************************************/
//**********************************************************************************
// Set your Payload IDs
// Please refer to: https://github.com/projecthorus/horusdemodlib/blob/master/payload_id_list.txt
//
// If you do not have a payload ID, you can use 0 (=4FSKTEST) for V1 and 256 (=4FSKTEST-V2) for V2:
//#define PAYLOAD_ID_V1  0
//#define PAYLOAD_ID_V2   256
//**********************************************************************************
#define PAYLOAD_ID_V1  0
#define PAYLOAD_ID_V2   256
#define HORUS_FREQUENCY 434.7144
#define HORUS_POWER        10   // In dBm. Valid values +2 to +17 dBm. 10dBm = 10mW, 13dBm=20mW
#define HORUS_BAUD         100  // recommended 50 (8MHz processor) or 100 baud (16MHz or better processor)
#define HORUS_SPACING      270  // NOTE: This results in a shift of 244 Hz due to the PLL Resolution of the SX127x which is 61Hz
#define HORUS_LOOPTIME     60   // Transmit Horus every xx seconds

/***********************************************************************************
* GPS SETTINGS
*  
* Change if needed
************************************************************************************/
// GPS Serial device
static const int Rx = PA3, Tx = PA2;  // This will probably be different for your board
static const uint32_t GPSBaud = 9600;

/***********************************************************************************
* TRANSMISSIONS SETTINGS
*   
* Change if needed
************************************************************************************/
#define SENTENCE_LENGTH 100     // Maximum length of telemetry line to send

/***********************************************************************************
* SONDEHUB EXTRA FIELDS SETTINGS
*  
* For displaying extra fields at sondehub, we need to define which fields are
* in the telemetry after the lat, lon, alt fields
* This can be done by adding a specific string after the last telemetry field
* This is supported by the various receivers made by Dave Akerman,
* See: https://www.daveakerman.com/?page_id=2410
* 
* 0  PayloadID
* 1 Counter
* 2 Time
* 3 Latitude
* 4 Longitude
* 5 Altitude
* 6 Satellites
* 7 Speed
* 8 Heading
* 9 Battery Voltage
* A InternalTemperature
* B ExternalTemperature
* C PredictedLatitude
* D PredictedLongitude
* E CutdownStatus
* F LastPacketSNR
* G LastPacketRSSI
* H ReceivedCommandCount
* I-N ExtraFields
* O MaximumAltitude
* P Battery Current
* Q External Temperature 2
* R Pressure
* S Humidity
* T CDA
* U Predicted Landing Speed
* V Time Till Landing
* W Last Command Received
* 
* Our string would be: "01234568A9"
* You can disable FIELDSTR by undefining it, if you want.
************************************************************************************/
#define USE_FIELDSTR
#define FIELDSTR "01234568A9"

/***********************************************************************************
* DEFAULT RTTY SETTINGS
*  
* Normally needs no change
************************************************************************************/  
#define RTTY_ASCII 0               // 7 data bits 
#define RTTY_ASCII_EXTENDED 1      // 8 data bits
#define RTTY_ITA2  2               // Baudot 

/***********************************************************************************
* DEFAULT LORA SETTINGS
*  
* Normally needs no change
************************************************************************************/
#define DEFAULT_POWER 10

#define LORA_BANDWIDTH 125.0         // Do not change, change LORA_MODE instead
#define LORA_SPREADFACTOR 9          // Do not change, change LORA_MODE instead
#define LORA_CODERATE 7              // Do not change, change LORA_MODE instead
#define LORA_PREFIX "$$"             // Prefix for "Telemetry". Some older LoRa software does not accept a prefix of more than 2x "$"
#define LORA_SYNCWORD 0x12           // Default syncword
#define LORA_POWER 10               // in dBm between 2 and 17. 10 = 10mW (recommended)
#define LORA_CURRENTLIMIT 100
#define LORA_PREAMBLELENGTH 8
#define LORA_GAIN 0

/***********************************************************************************
* DEFAULT FSK SETTINGS
*  
* Normally needs no change
************************************************************************************/
#define FSK_FREQUENCY 432.662
#define FSK_BITRATE 100.0
#define FSK_FREQDEV 50.0
#define FSK_RXBANDWIDTH 156.2
#define FSK_POWER 10   // in dBm between 2 and 17. 10 = 10mW (recommended). Sets also RTTY power
#define FSK_PREAMBLELENGTH 16
#define FSK_ENABLEOOK false
#define FSK_DATASHAPING 0.5
