// Functions for APRS
#if defined(LORA_APRS_TELEM_ENABLED) || defined(FM_APRS_TELEM_ENABLED)
#if LORA_APRS_TELEM_ENABLED == true || FM_APRS_TELEM_ENABLED == true
//Calculates a telemetry value according to formula in APRS101:
//TelemVal = a*v² + b*v + c
uint8_t calculateTelemValue(float vi, float a, float b, float c)
{
  uint8_t v = 0;
  float rawVal = (a*(vi*vi)) + (b*vi) + c;

  if(rawVal > 255) v = 255;
  else if(rawVal < 0) v = 0;
  else v = round(rawVal);

  return v;
}

String generateAprsTelemetry(bool bIsFM = false)
{
  static uint16_t lnr = 0;
  char telemBuf[31];
  String Outstring;

  String aprsSSID = "";
  String payloadID = "";
  if(bIsFM)
  {
    payloadID = LORA_APRS_PAYLOAD_ID;
    aprsSSID = LORA_APRS_SSID;
  }
  else
  {
    payloadID = FM_APRS_PAYLOAD_ID;
    aprsSSID = FM_APRS_SSID;
  } 

  Outstring = "";

  if(!bIsFM)
  {
    Outstring += "<\xff\x01";
    // Add Source
    Outstring += payloadID;
    // Add SSID
    Outstring += aprsSSID;
    //Add Destination (do not use digipeating)
    Outstring += ">";
    Outstring += "APETBT:";   // destination callsign_APRS_DEST;
    // start of "real" data (Telemetry, duh)
  }
  
  Outstring += "T#";

  //Add Telemetry payload
  //Equations
  //Voltage = (value) * 0.02; --> Range: 0 .. ~5.1V
  //Temperature = (value) * 0.5 - 70; --> Range: -70 ..  57.5 °C
  //Speed = (value) * 1 --> 0 .. 255 km/h
  //Climb = (value) * 0.2 - 50; --> Range: -50 .. 21 m/s
  
  float fVoltage = ReadVCC();
  uint8_t ui8val1=calculateTelemValue(fVoltage, 0, 50, 0);

  int32_t i32vrefVal = readVref();
  int32_t i32tempVal = readTempSensor(i32vrefVal);
  uint8_t ui8val2= calculateTelemValue((i32tempVal+70), 0, 2, 0);
  uint8_t ui8val3= calculateTelemValue(UGPS.Speed, 0, 1, 0);
  uint8_t ui8val4= calculateTelemValue((UGPS.Climb + 40.0), 0, 3.2, 0);
  uint8_t ui8val5= calculateTelemValue(UGPS.Altitude, 0, 0.0073, 0);

  sprintf(telemBuf, "%03d,%03d,%03d,%03d,%03d,%03d,00000000",
    lnr,
    ui8val1,
    ui8val2,
    ui8val3,
    ui8val4,
    ui8val5
  );

  Outstring += telemBuf;

  lnr++;
  if(lnr > 999) lnr = 0;

  return Outstring;
}

String generateWXPayload(bool bIsFM = false)
{
    // char buf[46];
    String Outstring = String("");

    String aprsSSID = "";
    String payloadID = "";
    if(bIsFM)
    {
      payloadID = LORA_APRS_PAYLOAD_ID;
      aprsSSID = LORA_APRS_SSID;
    }
    else
    {
      payloadID = FM_APRS_PAYLOAD_ID;
      aprsSSID = FM_APRS_SSID;
    } 

    #ifndef TESTMODE
    MeasureBME(false);

    //Correct pressure
    float temp_K = 273.15;
    temp_K += bme_temp;
    // float pres = bme_pres * powf((temp_K / (temp_K + (0.0065 * (float)UGPS.Altitude))), -5.255);
    float pres = bme_pres * 10.0;

    float tempf=(bme_temp*1.8)+32; // celsius to fahrenheit
    #ifdef USE_BME280
    float humi = bme_humi;
    #else
    float humi = 0.0;
    #endif
    //float wind=0;     // Kesendcwin Wind, das ist noch W.I.P.!
    //float wind2 = wind*0.621;  // wind2 is in mph
    #else
    float humi = 05.6;
    float pres = 123.45;
    float tempf = -50.4;
    #endif
    Outstring = "";

    if(!bIsFM)
    {
      Outstring += "<\xff\x01";
      // Add Source
      Outstring += payloadID;
      // Add SSID
      Outstring += aprsSSID;
      //Add Destination (do not use digipeating)
      Outstring += ">";
      Outstring += "APETBT";   // destination callsign_APRS_DEST;
      // start of "real" data (Telemetry, duh)
    }

    Outstring += ":_";

    char wxstr[46];

    #ifdef USE_BME280
    snprintf(wxstr, 46, "%02d%02d%02d%02dc...s...g...t%03dr...P...p...h%02db%05d",
      (int)UGPS.Month,
      (int)UGPS.Day,
      (int)UGPS.Hours,
      (int)UGPS.Minutes,
      (int)roundf(tempf),
      (int)roundf(humi),
      (int)roundf(pres));
    #else
    snprintf(wxstr, 46, "%02d%02d%02d%02dc...s...g...t%03dr...P...p...h..b%05d",
      (int)UGPS.Month,
      (int)UGPS.Day,
      (int)UGPS.Hours,
      (int)UGPS.Minutes,
      (int)roundf(tempf),
      (int)roundf(pres));
    #endif

    // if(UGPS.Month<10) { Outstring += "0"; }
    // Outstring += String(UGPS.Month);
    // if(UGPS.Day<10) { Outstring += "0"; }
    // Outstring += String(UGPS.Day);
    // if(UGPS.Hours<10) { Outstring += "0"; }
    // Outstring += String(UGPS.Hours);
    // if(UGPS.Minutes<10) { Outstring += "0"; }
    // Outstring += String(UGPS.Minutes);
    // Outstring += ("c...s...");
    // Outstring += ("g...");
    // Outstring += ("t");
    // if(tempf<100) { Outstring += "0"; }
    // if(tempf<10) { Outstring += "0"; }
    // Outstring += String(tempf,0);
    // Outstring += ("r...P...p...h");
    // Outstring += String(humi,0);
    // Outstring += ("b");
    //    if(pres<1000) { Outstring += "0"; }
    //    Outstring += String(pres,0);
    // Outstring += String("Actual Data: Temp ") + String(bme_temp, 1) + String(", Humi: ") + String(humi, 1) + String("  ");

    Outstring+=String(wxstr);

    return Outstring;
}

String generateWXInlinePayload()
{
  static uint16_t lnrt = 0;
  char wxp[34];

  float ftemp_c = 0.0;
  float fpres_hpa = 0.0;
  float fvoltage_v = 0.0;

  #if defined(USE_BME280) || defined(USE_BMP280)
  MeasureBME(false);

  //Correct pressure
  float temp_K = 273.15;
  temp_K += bme_temp;
  // float pres = bme_pres * powf((temp_K / (temp_K + (0.0065 * (float)UGPS.Altitude))), -5.255);
  fpres_hpa = bme_pres;

  ftemp_c = bme_temp;
  #endif

  #ifdef TESTMODE
  ftemp_c = -45.2;
  fpres_hpa = 123.45;
  fvoltage_v = 3.333;
  #else
  fvoltage_v = ReadVCC();
  #endif

  #ifndef TinyTracker
  snprintf(wxp, 34, "%03dTxC %05.2fC %07.2fhPa %1.2fV %02dS",
  lnrt,
  ftemp_c,
  fpres_hpa,
  fvoltage_v,
  UGPS.Satellites);
  #else
  //We need to "simulate" printf float for the TinyTracker, since the STM32G031F8P6 does not offer enough flash for that
  snprintf(wxp, 34, "%03dTxC %03d.%02dC %04d.%02dhPa %02d.%02dV %02dS",
  (int)lnrt,
  (int)truncf(ftemp_c),
  (int)((float)(ftemp_c - truncf(ftemp_c)) * (ftemp_c < 0 ? -1.0 : 1.0) * 100.0),
  (int)truncf(fpres_hpa),
  (int)((float)(fpres_hpa - truncf(fpres_hpa)) * (fpres_hpa < 0 ? -1.0 : 1.0) * 100.0),
  (int)truncf(fvoltage_v),
  (int)((float)(fvoltage_v - truncf(fvoltage_v)) * (fvoltage_v < 0 ? -1.0 : 1.0) * 100.0),
  (int)UGPS.Satellites);
  #endif

  lnrt++;
  if(lnrt > 999) lnrt = 0;

  return String(wxp);
}

#endif
#endif
