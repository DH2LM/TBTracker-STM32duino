// include the library
#include <RadioLib.h>
#include "cwled.h"

#define PACKETLEN 255

// Change 'SX1278' in the line below to 'SX1276' if you have a SX1276 module.
#ifdef TinyTracker
SX1278 radio = new Module(PIN_NSS, PIN_DIO0, PIN_DIO1);
#endif

#ifdef AttoAPRSv2
// Si4432 has following connections:
// nSEL: PA4
// nIRQ: PA5
// SDN:  PA8
Si4432 radio = new Module(PIN_SEL, PIN_IRQ, PIN_SDN);
#endif

#ifdef AttoAPRSv1
STM32WLx radio = new STM32WLx_Module();

// set RF switch configuration for Nucleo WL55JC1
// NOTE: other boards may be different!
//       Some boards may not have either LP or HP.
//       For those, do not set the LP/HP entry in the table.
static const uint32_t rfswitch_pins[] =
                         {PB8, PC13, PC13, PC13, PC13};

static const Module::RfSwitchMode_t rfswitch_table[] = {
  {STM32WLx::MODE_IDLE,  {LOW, LOW}},
  {STM32WLx::MODE_RX,    {HIGH, LOW}},
  // {STM32WLx::MODE_TX_LP, {HIGH}},
  {STM32WLx::MODE_TX_HP, {LOW, HIGH}},
  END_OF_MODE_TABLE,
};
#endif

// create RTTY client instance using the radio module
RTTYClient rtty(&radio);

// create FM APRS client instance using the radio module
AFSKClient FMaudio(&radio, PIN_DIO2);
AX25Client FMax25(&FMaudio);
APRSClient aprs(&FMax25);

// create Morse Client for CW using the radio module
MorseClient cw(&radio);

int telemCounter = 0;


/************************************************************************************
* this function is called when a complete packet is received by the radio module
* IMPORTANT: this function MUST be 'void' type and MUST NOT have any arguments!
************************************************************************************/
void setFlag(void) 
{
  // we got a packet, set the flag
  receivedFlag = true;
}

void unsetFlag(void) 
{
  // radio.clearDio0Action();
  radio.clearDio1Action();
}

//===============================================================================
void SetupRTTY()
{

  // First setup FSK
  SetupFSK();

  // SerialDebug.print(F("RTTY init.."));

  int16_t state = rtty.begin(RTTYSettings.Frequency,
                     RTTYSettings.Shift,
                     RTTYSettings.Baud,
                     RTTYSettings.Encoding,
                     RTTYSettings.StopBits  );
                     
  if(state == RADIOLIB_ERR_NONE) // Change this to (state == ERR_NONE) if you use an older radiolib library
  {
    digitalWrite(LED_GRN, HIGH);
    // SerialDebug.println(F("done"));
  } else 
  {
    digitalWrite(LED_RED, HIGH);
    // SerialDebug.print(F("failed, code "));
    // SerialDebug.println(state);
    while(true)
    {
      giveS();
      delay(250);
      giveR();
      delay(700);
    }
  }
}

//===============================================================================
void SetupHorus()
{
  // SerialDebug.println();
  // SerialDebug.print("Setting up radio for Horus...");
  // Initialize the radio in FSK mode
  #ifndef SI4432
  int16_t state = radio.beginFSK();
  #else
  int16_t state = radio.begin();
  #endif

  // Only continue if there is no error state
  if(state == RADIOLIB_ERR_NONE) 
  {
    digitalWrite(LED_GRN, HIGH);
    // SerialDebug.println(F("success!"));
  } else 
  {
    digitalWrite(LED_RED, HIGH);
    // SerialDebug.print(F("failed, code "));
    // SerialDebug.println(state);
    while(true)
    {
      giveH();
      delay(700);
    }
  }

  state = radio.setOutputPower(HORUS_POWER);

  // SerialDebug.print(F("[FSK4] Initializing ... ")); 

  // initialize FSK4 transmitter
  // NOTE: FSK4 frequency shift will be rounded
  //       to the nearest multiple of frequency step size.
  //       The exact value depends on the module:
  //         SX127x/RFM9x - 61 Hz
  //         RF69 - 61 Hz
  //         CC1101 - 397 Hz
  //         SX126x - 1 Hz
  //         nRF24 - 1000000 Hz
  //         Si443x/RFM2x - 156 Hz
  //         SX128x - 198 Hz
  state = fsk4_setup(&radio, HORUS_FREQUENCY, HORUS_SPACING, HORUS_BAUD);

  if(state == RADIOLIB_ERR_NONE) 
  {
    digitalWrite(LED_GRN, HIGH);
    // SerialDebug.println(F("success!"));
  } else 
  {
    digitalWrite(LED_RED, HIGH);
    // SerialDebug.print(F("failed, code "));
    // SerialDebug.println(state);
    while(true)
    {
      give4();
      delay(250);
      giveF();
      delay(700);
    }
  }
}

//===============================================================================
void SetupFSK()
{
  // Initialize the SX1278
  // SerialDebug.print(F("[SX1278] init.."));

 // int16_t state = radio.beginFSK();
 
  #ifndef SI4432
  int16_t state = radio.beginFSK(FSKSettings.Frequency,
                               FSKSettings.BitRate,
                               FSKSettings.FreqDev,
                               FSKSettings.RXBandwidth,
                               FSKSettings.Power,
                               FSKSettings.PreambleLength);
  #else
  int16_t state = radio.begin(FSKSettings.Frequency,
                               FSKSettings.BitRate,
                               FSKSettings.FreqDev,
                               FSKSettings.RXBandwidth,
                               FSKSettings.Power,
                               FSKSettings.PreambleLength);
  #endif


  if(state == RADIOLIB_ERR_NONE) // Change this to (state == ERR_NONE) if you use an older radiolib library
  {
    digitalWrite(LED_GRN, HIGH);
    // SerialDebug.println(F("done"));
  } 
  else 
  {
    digitalWrite(LED_RED, HIGH);
    // SerialDebug.print(F("failed, code "));
    // SerialDebug.println(state);
    while(true)
    {
      giveF();
      delay(700);
    }
  }
}


//===============================================================================
void SetupLoRa(int aMode)
{
  #ifndef SI4432
  // Initialize the SX1278
  // SerialDebug.print(F("[LoRA] Initializing ... "));

  ResetRadio(); 

  switch (aMode)
  {
    case 0: 
      LoRaSettings.CodeRate = 8;
      LoRaSettings.Bandwidth = 20.8;
      LoRaSettings.SpreadFactor = 11;
      LoRaSettings.Frequency = LORA_FREQUENCY;
      break;   

    case 1:
      LoRaSettings.CodeRate = 5;
      LoRaSettings.Bandwidth = 20.8;      
      LoRaSettings.SpreadFactor = 6; 
      LoRaSettings.implicitHeader = 255;     
      LoRaSettings.Frequency = LORA_FREQUENCY;
    break;   
    
    case 2:
      LoRaSettings.CodeRate = 8;
      LoRaSettings.Bandwidth = 62.5;      
      LoRaSettings.SpreadFactor = 8;
      LoRaSettings.Frequency = LORA_FREQUENCY;      
      break;   

    case 3:
      LoRaSettings.CodeRate = 6;
      LoRaSettings.Bandwidth = 250;      
      LoRaSettings.SpreadFactor = 7;            
      LoRaSettings.Frequency = LORA_FREQUENCY;
      break;   
    
    case 4:
      LoRaSettings.CodeRate = 5;
      LoRaSettings.Bandwidth = 250;      
      LoRaSettings.SpreadFactor = 6;            
      LoRaSettings.Frequency = LORA_FREQUENCY;
      break;   

    case 5:
      LoRaSettings.CodeRate = 8;
      LoRaSettings.Bandwidth = 41.7;      
      LoRaSettings.SpreadFactor = 11;            
      LoRaSettings.Frequency = LORA_FREQUENCY;
    break;

    case 6:
      LoRaSettings.CodeRate = 5;
      LoRaSettings.Bandwidth = 41.7;      
      LoRaSettings.SpreadFactor = 6;            
      LoRaSettings.Frequency = LORA_FREQUENCY;
    break;

    case 7:
      LoRaSettings.CodeRate = 5;
      LoRaSettings.Bandwidth = 20.8;      
      LoRaSettings.SpreadFactor = 7;            
      LoRaSettings.Frequency = LORA_FREQUENCY;
    break;

    case 8:
      LoRaSettings.CodeRate = 5;
      LoRaSettings.Bandwidth = 62.5;      
      LoRaSettings.SpreadFactor = 6;            
      LoRaSettings.Frequency = LORA_FREQUENCY;
    break;


    case 99:  // LORA-APRS
      LoRaSettings.CodeRate = 5;
      LoRaSettings.Bandwidth = 125;      
      LoRaSettings.SpreadFactor = 12;              
      LoRaSettings.Frequency = LORA_APRS_FREQUENCY;
    break;
  }

  
  #ifdef AttoAPRSv1
  radio.setRfSwitchTable(rfswitch_pins, rfswitch_table);
  radio.XTAL = false;
  #endif

  int16_t state = radio.begin
  (
    LoRaSettings.Frequency,
    LoRaSettings.Bandwidth,
    LoRaSettings.SpreadFactor,
    LoRaSettings.CodeRate,
    LoRaSettings.SyncWord,
    LoRaSettings.Power,
    LoRaSettings.PreambleLength 
  );
  
  switch(LORA_MODE) 
  {
    case 0:
      radio.forceLDRO(true);
      radio.setCRC(true);  
    break;
    case 1:
      radio.implicitHeader(PACKETLEN);
      //radio.forceLDRO(true);
      radio.setCRC(true);
    break;
    default:
      radio.explicitHeader();
      //radio.autoLDRO();
      radio.setCRC(true);
    break;
  }  
  
  if(state == RADIOLIB_ERR_NONE) // Change this to (state == ERR_NONE) if you use an older radiolib library
  {
    digitalWrite(LED_GRN, HIGH);
    // SerialDebug.println(F("done"));
  } 
  else 
  {
    digitalWrite(LED_RED, HIGH);
    // SerialDebug.print(F("failed, code "));
    // SerialDebug.println(state);
    while(true)
    {
      giveL();
      delay(700);
    }
  }
    #else
  #pragma message "LoRa is not supported by the SI4432."
  #endif
}


void SetupCW()
{
  // SerialDebug.println();
  // SerialDebug.print("Setting up radio for CW...");
  
  // Initialize the radio in FSK mode
  #ifndef SI4432
  int16_t state = radio.beginFSK();
  #else
  int16_t state = radio.begin();
  #endif

  digitalWrite(PIN_DIO2, LOW);

  // Only continue if there is no error state
  if(state == RADIOLIB_ERR_NONE) 
  {
    digitalWrite(LED_GRN, HIGH);
    // SerialDebug.println(F("success!"));
  } else 
  {
    digitalWrite(LED_RED, HIGH);
    // SerialDebug.print(F("failed, code "));
    // SerialDebug.println(state);
    while(true)
    {
      giveC();
      delay(100);
      giveF();
      delay(700);
    }
  }

  state = radio.setOutputPower(CW_POWER);

  state = radio.setFrequency(CW_FREQUENCY);

  //Init CW
  state = cw.begin(CW_FREQUENCY, CW_SPEED);

  if(state == RADIOLIB_ERR_NONE) 
  {
    digitalWrite(LED_GRN, HIGH);
    // SerialDebug.println(F("success!"));
  } else 
  {
    digitalWrite(LED_RED, HIGH);
    // SerialDebug.print(F("failed, code "));
    // SerialDebug.println(state);
    while(true)
    {
      giveS();
      delay(100);
      giveC();
      delay(700);
    }
  }
}

void SetupFM()
{
  // SerialDebug.println();
  // SerialDebug.print("Setting up radio for FM AFSK...");
  
  // Initialize the radio in FSK mode
  #ifndef SI4432
  int16_t state = radio.beginFSK();
  #else
  int16_t state = radio.begin();
  #endif

  // Only continue if there is no error state
  if(state == RADIOLIB_ERR_NONE) 
  {
    digitalWrite(LED_GRN, HIGH);
    // SerialDebug.println(F("success!"));
  } else 
  {
    digitalWrite(LED_RED, HIGH);
    // SerialDebug.print(F("failed, code "));
    // SerialDebug.println(state);
    while(true)
    {
      giveS();
      delay(100);
      giveF();
      delay(700);
    }
  }

  state = radio.setOutputPower(FM_APRS_POWER);

  state = radio.setFrequency(FM_APRS_FREQUENCY);

  //Init AX25
  state = FMax25.begin(FM_APRS_PAYLOAD_ID, FM_APRS_SSIDNUM, 64);

  if(state == RADIOLIB_ERR_NONE) 
  {
    digitalWrite(LED_GRN, HIGH);
    // SerialDebug.println(F("success!"));
  } else 
  {
    digitalWrite(LED_RED, HIGH);
    // SerialDebug.print(F("failed, code "));
    // SerialDebug.println(state);
    while(true)
    {
      giveS();
      delay(100);
      giveA();
      delay(100);
      giveX();
      delay(700);
    }
  }

  //Init APRS
  state = aprs.begin('O', FM_APRS_PAYLOAD_ID, FM_APRS_SSIDNUM, false);

  if(state == RADIOLIB_ERR_NONE) 
  {
    digitalWrite(LED_GRN, HIGH);
    // SerialDebug.println(F("success!"));
  } else 
  {
    digitalWrite(LED_RED, HIGH);
    // SerialDebug.print(F("failed, code "));
    // SerialDebug.println(state);
    while(true)
    {
      giveS();
      delay(100);
      giveA();
      delay(700);
    }
  }
}

//===============================================================================
void SetupRadio()
{
  // Setting up the radio
  #ifdef AttoAPRSv1
  radio.setRfSwitchTable(rfswitch_pins, rfswitch_table);
  radio.XTAL = false;
  #endif

  if (RTTY_ENABLED) {SetupRTTY();}
  if (LORA_ENABLED) {SetupLoRa(LORA_MODE);}

  // int16_t state = radio.setOutputPower(DEFAULT_POWER);
  // if(state != 0)
  // {
  //   while(true)
  //   {
  //     giveF();
  //     giveL();
  //     delay(700);
  //   }
  // }
}

//===============================================================================
void sendRTTY(String TxLine)
{
   // Disable the GPS on the softwareserial temporarily 
   // SerialGPS.end();
   
   SetupRTTY();
   
   // Send only idle carrier to let people get their tuning right
   rtty.idle();     
   delay(RTTY_IDLE_TIME);

   // Send the string 
   // SerialDebug.print(F("Send RTTY: "));
   // SerialDebug.println(TxLine);

   int state = rtty.println(TxLine); 
   rtty.standby();   
   // Enable the GPS again.  
   //SerialGPS.begin(GPSBaud);
}

//===============================================================================
void ResetRadio()
{
  // Use for ESP based boards
  #ifndef SI4432
  pinMode(PIN_RESET,OUTPUT);
  digitalWrite(PIN_RESET, LOW);
  delay(100);
  digitalWrite(PIN_RESET,HIGH);
  delay(100);
  #else
  #pragma message "SI4432 has no reset pin."
  #endif
}

//===============================================================================
void sendLoRa(String TxLine, int aMode)
{
  #ifndef SI4432
   int state;

   SetupLoRa(aMode);
   // SerialDebug.println(TxLine);

  switch (LORA_MODE)
  {
    case 1:
      int i;
      int j; 
      // Send the string 
      char buf[PACKETLEN];
      for (j=0; j<PACKETLEN; j++) { buf[j] = '\0';}
      for (i=0; i<TxLine.length(); i++) {buf[i] = TxLine[i];}
      state = radio.transmit((uint8_t*)buf,PACKETLEN);       
    break;
    default:
      // Send the string 
      state = radio.transmit(TxLine); 
    break;      
  }   

  if(state == RADIOLIB_ERR_NONE) // Change this to (state == ERR_NONE) if you use an older radiolib library
  {
    digitalWrite(LED_GRN, HIGH);
    // SerialDebug.println(F("done"));
  } else 
  {
    digitalWrite(LED_RED, HIGH);
    // SerialDebug.print(F("failed, code "));
    // SerialDebug.println(state);
    while(true)
    {
      giveS();
      delay(250);
      giveL();
      delay(700);
    }
  } 
  #else
  #pragma message "LoRa is not supported by the SI4432."
  #endif
}

void sendFM(String TxLine, bool bIsPos = false)
{
  //  SetupLoRa(aMode);
  SetupFM();
   // SerialDebug.println(TxLine);

  int state = 0;

  if(bIsPos)
  {
    aprs.sendPosition(
      "APETBT",
      0,
      const_cast<char*>(getAPRSlat(UGPS.Latitude).c_str()),
      const_cast<char*>(getAPRSlon(UGPS.Longitude).c_str()),
      const_cast<char*>(TxLine.c_str()),
      const_cast<char*>(getAPRStimestamp().c_str())
    );
  }
  else
  {
    char* frame = new char(TxLine.length());

    strncpy(frame, TxLine.c_str(), TxLine.length());

    state = aprs.sendFrame("APETBT", 0, frame);

    delete[] frame;
  }
  FMaudio.noTone();

  if(state == RADIOLIB_ERR_NONE) // Change this to (state == ERR_NONE) if you use an older radiolib library
  {
    digitalWrite(LED_GRN, HIGH);
    // SerialDebug.println(F("done"));
  } else 
  {
    digitalWrite(LED_RED, HIGH);
    // SerialDebug.print(F("failed, code "));
    // SerialDebug.println(state);
    while(true)
    {
      giveA();
      delay(700);
    }
  } 
}

String coordStringFromCoords(float lat = 0.0, float lon = 0.0, int precision = 4)
{
  float fLat = lat;
  float fLon = lon; 

  String strPos = "";
  
  if(fLat >= 0)
  {
    strPos += String("N") + String(fLat, 5);
  }
  else
  {
    strPos += String("S") + String(-fLat, 5);
  }

  if(fLon >= 0)
  {
    strPos += String("E") + String(fLon, 5);
  }
  else
  {
    strPos += String("W") + String(-fLon, 5);
  }

  return strPos;
}

String qthLocFromCoords(float lat = 0.0, float lon = 0.0, int precision = 4)
{
  int iPrecision = precision;
  if(iPrecision < 4) iPrecision = 4;
  if(iPrecision > 10) iPrecision = 10;
  if(iPrecision % 2 == 1) iPrecision--;

  //Check boundaries and add offsets
  float fLat = lat;
  if(fLat < -90.0) fLat = -90.0;
  if(fLat > 90.0) fLat = 90.0;
  fLat += 90.0;

  float fLon = lon;
  if(fLon < -180.0) fLon = -180.0;
  if(fLon > 180.0) fLon = 180.0;
  fLon+=180.0;

  //Some little helpers
  String strLocator = "";
  bool bIsLetter = true;
  char cLoc = 0;
  int iRes = 0;
  int iPow = 1;

  for(int i = 0; i<4; i++)
  {
    iPow = powf(10, i/2);

    if(i%4 == 2 || i%4 == 3) bIsLetter = false;
    else bIsLetter = true;
    
    if(i%2==0)
    {
      //Odd space --> encodes longtitude
      iRes = floor( fLon / (20.0 / (float)iPow));
      fLon -= ((20.0 / (float)iPow) * iRes);
    }
    else
    {
      //Even space --> encodes latitude
      iRes = floor( fLat / (10.0 / (float)iPow));
      fLat -= ((10.0 / (float)iPow) * iRes);
    }

    //"calculate" the character
    cLoc = (bIsLetter ? 0x41 : 0x30) + iRes;

    //Add character to result string
    strLocator += cLoc;
  }

  if(iPrecision == 4) return strLocator;

  //Now we should have only the decimals
  float fLatMins = fLat * 60.0;
  float fLonMins = fLon * 60.0;

  iRes = fLonMins / 5.0;
  fLonMins -= (5.0 * (float)iRes);
  cLoc = 0x41 + iRes;
  strLocator += cLoc;

  iRes = fLatMins / 2.5;
  fLatMins -= (2.5 * (float)iRes);
  cLoc = 0x41 + iRes;
  strLocator += cLoc;

  if(iPrecision == 6) return strLocator;

  float fLonSecs = fLonMins * 60.0;
  float fLatSecs = fLatMins * 60.0;

  iRes = fLonSecs / 30.0;
  fLonSecs -= (30.0 * (float)iRes);
  cLoc = 0x30 + iRes;
  strLocator += cLoc;

  iRes = fLatSecs / 15.0;
  fLatSecs -= (15.0 * (float)iRes);
  cLoc = 0x30 + iRes;
  strLocator += cLoc;

  if(iPrecision == 8) return strLocator;

  iRes = fLonSecs / 1.25;
  fLonSecs -= (1.25 * (float)iRes);
  cLoc = 0x41 + iRes;
  strLocator += cLoc;

  iRes = fLatSecs / 0.625;
  fLatSecs -= (0.625 * (float)iRes);
  cLoc = 0x41 + iRes;
  strLocator += cLoc;

  return strLocator;
}

void sendCW()
{
   int state;

  SetupCW();
   // SerialDebug.println(TxLine);

  #if CW_TRANSMIT_LOC == true
  uint8_t qthsize = CW_QTH_LOC_SIZE;
  if(qthsize < 4) qthsize = 4;
  if(qthsize > 12) qthsize = 12;
  if(qthsize % 2 == 1) qthsize++;
  #ifndef TESTMODE
  String qthLoc = qthLocFromCoords(UGPS.Latitude, UGPS.Longitude, qthsize);
  #else
  String qthLoc = qthLocFromCoords(50.5571079754216, 12.9573322739812, 12);
  #endif
  #else
  String qthLoc = "";
  #endif

  #if CW_TRANSMIT_COORDS == true
  #ifndef TESTMODE
  String qthCoords = coordStringFromCoords(UGPS.Latitude, UGPS.Longitude, qthsize);
  #else
  String qthCoords = coordStringFromCoords(50.5571079754216, 12.9573322739812, 12);
  #endif
  #else
  String qthCoords = "";
  #endif

  String cwMsg = CW_CALLSIGN;
  cwMsg += String(" ") + String(CW_CALLSIGN) + String(" HAB BCN ");
  cw.print(cwMsg);

  #if CW_TRANSMIT_LOC == true
  // cwMsg = "QTH " + qthLoc + String(" ") + qthLoc + String(" ");
  cwMsg = "QTH " + qthLoc + String(" ");
  cw.print(cwMsg);
  #endif

  #if CW_TRANSMIT_COORDS == true
  // cwMsg = "QTH " + qthLoc + String(" ") + qthLoc + String(" ");
  cwMsg = "COORD " + qthCoords + String(" ");
  cw.print(cwMsg);
  #endif

  #if CW_TRANSMIT_ALT == true
  #ifndef TESTMODE
  cwMsg = "A " + String(UGPS.Altitude) + "M ";
  #else
  cwMsg = "A " + String(562) + "M ";
  #endif
  cw.print(cwMsg);
  #endif

  #if CW_TRANSMIT_TEMP == true
  #ifndef TESTMODE
  int iIntTemp = ReadTemp();
  cwMsg = String("IT ") + String(iIntTemp) + String("C ");
  #else
  cwMsg = String("IT ") + String(18) + String("C ");
  #endif
  cw.print(cwMsg);
  #endif

  #if CW_TRANSMIT_VOLTAGE == true
    #ifndef TESTMODE
    float fIntVolt = ReadVCC();
    cwMsg = String("U ") + String(fIntVolt, 1) + String("V ");
    #else
    cwMsg = String("U ") + String(1.5, 1) + String("V ");
    #endif
    cw.print(cwMsg);
  #endif

  #if CW_TRANSMIT_SENSORDATA == true
    #ifndef TESTMODE
      #if defined(USE_BME280) || defined(USE_BMP280)
        //Take measurement before TXing
        int iExtTemp = 0, iExtHumi = 0, iExtPres = 0;

        MeasureBME(false);

        //Put values in variables
        iExtTemp = round(bme_temp);
        // float pres = bme_pres * powf((temp_K / (temp_K + (0.0065 * (float)UGPS.Altitude))), -5.255);
        iExtPres = round(bme_pres);

        #ifdef USE_BME280
          iExtHumi = round(bme_humi);
        #endif

        cwMsg = String("ST ") + String(iExtTemp) + String("C ");

        #ifdef USE_BME280
        cwMsg += String("SH ") + String(iExtHumi) + String("PCNT ");
        #endif

        cwMsg += String("SP ") + String(iExtPres) + String("HPA ");
        cw.print(cwMsg);
      #endif
    #else
        cwMsg = String("ST ") + String(-2) + String("C ");

        cwMsg += String("SH ") + String(57) + String("PCNT ");

        cwMsg += String("SP ") + String(950) + String("HPA ");
        cw.print(cwMsg);
    #endif
  #endif

  cwMsg = String(CW_CUSTOM_MSG);

  // cw.startSignal();
  cw.print(cwMsg);

  if(state == RADIOLIB_ERR_NONE) // Change this to (state == ERR_NONE) if you use an older radiolib library
  {
    digitalWrite(LED_GRN, HIGH);
    // SerialDebug.println(F("done"));
  } else 
  {
    digitalWrite(LED_RED, HIGH);
    // SerialDebug.print(F("failed, code "));
    // SerialDebug.println(state);
    while(true)
    {
      giveC();
      delay(250);
      giveW();
      delay(700);
    }
  } 
}


//===============================================================================
void sendHorusV1()
{
  int pkt_len;
  int coded_len;

  // Setup the radio for Horus communication 
  SetupHorus();

  // Start Horus Binary V1
  // SerialDebug.println(F("Generating Horus Binary v1 Packet"));

 // Generate packet for V1
  pkt_len = build_horus_binary_packet_v1(rawbuffer);
  PrintHex(rawbuffer, pkt_len, debugbuffer);
  // SerialDebug.print(F("Uncoded Length (bytes): "));
  // SerialDebug.println(pkt_len);
  // SerialDebug.print(F("Uncoded: ")); 
  // SerialDebug.println(debugbuffer);

 // Apply Encoding
  coded_len = horus_l2_encode_tx_packet((unsigned char*)codedbuffer,(unsigned char*)rawbuffer,pkt_len);
  PrintHex(codedbuffer, coded_len, debugbuffer);
  // SerialDebug.print(F("Encoded Length (bytes): "));
  // SerialDebug.println(coded_len);
  // SerialDebug.print(F("Coded: "));
  // SerialDebug.println(debugbuffer);

  // Transmit!
  // SerialDebug.println(F("Transmitting Horus Binary v1 Packet"));
  // send out idle condition for 1000 ms
  fsk4_idle(&radio);
  delay(1000);
  fsk4_preamble(&radio, 8);
  fsk4_write(&radio, codedbuffer, coded_len);
  // SerialDebug.println();
}

//===============================================================================
void sendHorusV2()
{
  int pkt_len;
  int coded_len;

  // Setup the radio for Horus communication 
  SetupHorus();

     // Start Horus Binary V1
  // SerialDebug.println(F("Generating Horus Binary v2 Packet"));

  // Generate packet for V1
  pkt_len = build_horus_binary_packet_v2(rawbuffer);
  PrintHex(rawbuffer, pkt_len, debugbuffer);
  // SerialDebug.print(F("Uncoded Length (bytes): "));
  // SerialDebug.println(pkt_len);
  // SerialDebug.print(F("Uncoded: ")); 
  // SerialDebug.println(debugbuffer);

  // Apply Encoding
  coded_len = horus_l2_encode_tx_packet((unsigned char*)codedbuffer,(unsigned char*)rawbuffer,pkt_len);
  PrintHex(codedbuffer, coded_len, debugbuffer);
  // SerialDebug.print(F("Encoded Length (bytes): "));
  // SerialDebug.println(coded_len);
  // SerialDebug.print("Coded: ");
  // SerialDebug.println(debugbuffer);
  
  // Transmit!
  // SerialDebug.println(F("Transmitting Horus Binary v2 Packet"));

  // send out idle condition for 1000 ms
  fsk4_idle(&radio);
  delay(1000);
  fsk4_preamble(&radio, 8);
  fsk4_write(&radio, codedbuffer, coded_len);
}

//===============================================================================
void sendLoRaAprs()
{
  #ifndef SI4432
   String aprs_packet;
   String lat="";
   String lon="";
   int deg;
   int min;
   float minute_remainder;
   float second_remainder;
   static bool infoSent = false;


   aprs_packet = "";
   aprs_packet += "<\xff\x01";
   // Add Source
   aprs_packet += LORA_APRS_PAYLOAD_ID;
   // Add SSID
   aprs_packet += LORA_APRS_SSID;
   //Add Destination (do not use digipeating)
   aprs_packet += ">";
   aprs_packet += "APETBT";   // destination callsign_APRS_DEST;
   // start of "real" data (Coordinates with timestamp)
   aprs_packet += ":@";
   // get the APRS timestamp
   aprs_packet += getAPRStimestamp();
   // get the APRS latitude
   aprs_packet += getAPRSlat(UGPS.Latitude);
   // Add the symbol for the primary symbol table
   aprs_packet += "/";
   // Add the longitude
   aprs_packet += getAPRSlon(UGPS.Longitude);
   // Add the symbol for balloon
   aprs_packet += "O";
   // Add the altitude
   aprs_packet += getAPRSAlt(UGPS.Altitude);

   #ifdef LORA_APRS_TELEM_ENABLED
   #if LORA_APRS_TELEM_ENABLED
   //Add an inline WX payload for Sondehub, if telemetry is activated
   aprs_packet += generateWXInlinePayload();
   #endif
   #endif

   // Add a meesage
   #ifndef TESTMODE
  //  aprs_packet += " HAB LoRa APRS https://github.com/dh2lm/tbtracker-stm32duino U=";
   aprs_packet += " Testing Payload";

   // Add voltage
  //  float fVoltage = ReadVCC();
  //  aprs_packet += String(fVoltage, 2);
  //  aprs_packet += "V";
   #else
   aprs_packet += " HAB LoRA APRS TESTMODE";
   #endif


   // SerialDebug.println("Sending LoRa APRS packet...");
   sendLoRa(aprs_packet,LORA_APRS_MODE);

#ifdef LORA_APRS_TELEM_ENABLED
#if LORA_APRS_TELEM_ENABLED == true
    // if(!infoSent)
    // {
    //     delay(2500);
    //     infoSent = true;
    //     String header = "";
    //     header += "<\xff\x01";   
    //     header += LORA_APRS_PAYLOAD_ID;
    //     // Add SSID
    //     header += LORA_APRS_SSID;
    //     //Add Destination (do not use digipeating)
    //     header += ">";
    //     header += "APETBT::";   // destination callsign_APRS_DEST;
    //     String strCall = String(LORA_APRS_PAYLOAD_ID) + String(LORA_APRS_SSID);
    //     for(int i=0; i<10-strCall.length(); i++) strCall += " ";
    //     header += strCall;
    //     header += ":";

    //     //Parameter info
    //     aprs_packet = header + "PARM.Voltage,Temp,Speed,Climb,CoarseHeight";
    //     sendLoRa(aprs_packet, LORA_APRS_MODE);

    //     delay(2500);
    //     //Units
    //     aprs_packet = header + "UNIT.VDC,DegC,kph,mps,m";
    //     sendLoRa(aprs_packet, LORA_APRS_MODE);

    //     delay(2500);
    //     //Equations
    //     //(value) = a * target² + b * target + c
    //     //Voltage = (value) * 0.02; --> Range: 0 .. ~5.1V
    //     //Temperature = (value) * 0.5 - 70; --> Range: -70 ..  57.5 °C
    //     //Speed = (value) * 1 --> 0 .. 255 km/h
    //     aprs_packet = header + "EQNS.0,0.02,0,0,0.5,-70,0,1,0,0,0.3125,-40,0,137,0";
    //     sendLoRa(aprs_packet, LORA_APRS_MODE);
    // }

    if (telemCounter == 0)
    {
      delay(2000);

      aprs_packet = generateAprsTelemetry();
      sendLoRa(aprs_packet, LORA_APRS_MODE);

      #if defined(USE_BME280) || defined(USE_BMP280)
      delay(2000);
      aprs_packet = generateWXPayload();
      sendLoRa(aprs_packet, LORA_APRS_MODE);
      #endif
    }

    telemCounter++;
    if(telemCounter >= LORA_APRS_TELEMETRY_EVERY) telemCounter = 0;

#endif
#endif
  #else
  #pragma message "LoRa is not supported by the SI4432."
  #endif
}

//===============================================================================
void sendFMAprs()
{
   String aprs_packet;
   String lat="";
   String lon="";
   int deg;
   int min;
   float minute_remainder;
   float second_remainder;
   static bool infoSent = false;


   aprs_packet = "";
  //  aprs_packet += "<\xff\x01";
   // Add Source
  //  aprs_packet += FM_APRS_PAYLOAD_ID;
  //  // Add SSID
  //  aprs_packet += FM_APRS_SSID;
  //  //Add Destination (do not use digipeating)
  //  aprs_packet += ">";
  //  aprs_packet += "APETBT";   // destination callsign_APRS_DEST;
  //  // start of "real" data (Coordinates with timestamp)
  //  aprs_packet += ":@";
  //  // get the APRS timestamp
  //  aprs_packet += getAPRStimestamp();
  //  // get the APRS latitude
  //  aprs_packet += getAPRSlat(UGPS.Latitude);
  //  // Add the symbol for the primary symbol table
  //  aprs_packet += "/";
  //  // Add the longitude
  //  aprs_packet += getAPRSlon(UGPS.Longitude);
  //  // Add the symbol for balloon
  //  aprs_packet += "O";
   // Add the altitude
   aprs_packet += getAPRSAlt(UGPS.Altitude);
   // Add a meesage
   aprs_packet += " HAB FM APRS https://github.com/dh2lm/tbtracker-stm32duino U=";

   // Add voltage
   float fVoltage = ReadVCC();
   aprs_packet += String(fVoltage, 2);
   aprs_packet += "V";

   // SerialDebug.println("Sending FM APRS packet...");
   sendFM(aprs_packet, true);

#ifdef FM_APRS_TELEM_ENABLED
#if FM_APRS_TELEM_ENABLED == true
    if (telemCounter == 0)
    {
      delay(2000);

      aprs_packet = generateAprsTelemetry(true) + "\0";
      sendFM(aprs_packet);

      delay(2000);
      aprs_packet = generateWXPayload(true) + "\0";
      sendFM(aprs_packet);
    }

    telemCounter++;
    if(telemCounter >= FM_APRS_TELEMETRY_EVERY) telemCounter = 0;

    // if(!infoSent)
    // {
    //     delay(2500);
    //     infoSent = true;
    //     String header = "";
    //     // header += "<\xff\x01";   
    //     // header += FM_APRS_PAYLOAD_ID;
    //     // Add SSID
    //     // header += FM_APRS_SSID;
    //     //Add Destination (do not use digipeating)
    //     // header += ">";
    //     // header += "APETBT::";   // destination callsign_APRS_DEST;
    //     // String strCall = String(FM_APRS_PAYLOAD_ID) + String(FM_APRS_SSID);
    //     // for(int i=0; i<10-strCall.length(); i++) strCall += " ";
    //     // header += strCall;
    //     // header += ":";

    //     //Parameter info
    //     aprs_packet = header + "PARM.Voltage,Temp,Speed,Climb";
    //     sendFM(aprs_packet, false);

    //     delay(2500);
    //     //Units
    //     aprs_packet = header + "UNIT.VDC,DegC,kph,mps";
    //     sendFM(aprs_packet, false);

    //     delay(2500);
    //     //Equations
    //     //(value) = a * target² + b * target + c
    //     //Voltage = (value) * 0.02; --> Range: 0 .. ~5.1V
    //     //Temperature = (value) * 0.5 - 70; --> Range: -70 ..  57.5 °C
    //     //Speed = (value) * 1 --> 0 .. 255 km/h
    //     aprs_packet = header + "EQNS.0,0.02,0,0,0.5,-70,0,1,0,0,0.3125,-40";
    //     sendFM(aprs_packet, false);
    // }
#endif
#endif
}


//===============================================================================
// Put the radio in RX mode
//===============================================================================
void StartReceiveLoRaPacket()
{
  #ifndef SI4432
   int16_t state;

   SetupLoRa(LORA_MODE);  
   radio.setPacketReceivedAction(setFlag);  // As of RadioLib 6.0.0 all methods to attach interrupts no longer have a default level change direction

  if (LORA_MODE == 1) 
  {
    state  = radio.startReceive(LoRaSettings.implicitHeader);
  } 
  else 
  {
    state = radio.startReceive();
  }

   if (state == RADIOLIB_ERR_NONE) 
   {
     // SerialDebug.println(F("success!"));
     // SerialDebug.print(F("[LoRa] Waiting for packets on: ")); // SerialDebug.print(LoRaSettings.Frequency,3); // SerialDebug.println(F(" MHz"));
     // SerialDebug.println(F("----------------------------"));
   } 
   else 
   {
     // SerialDebug.print(F("failed, code "));
     // SerialDebug.println(state);
     // while (true);
   }
     #else
  #pragma message "LoRa is not supported by the SI4432."
  #endif
}
