// Functions for APRS
#ifdef LORA_APRS_TELEM_ENABLED
#if LORA_APRS_TELEM_ENABLED == true
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

String generateLoraAprsTelemetry()
{
  static uint16_t lnr = 0;
  char telemBuf[31];
  String Outstring;

  Outstring = "";
  Outstring += "<\xff\x01";
  // Add Source
  Outstring += LORA_APRS_PAYLOAD_ID;
  // Add SSID
  Outstring += LORA_APRS_SSID;
  //Add Destination (do not use digipeating)
  Outstring += ">";
  Outstring += "APETBT";   // destination callsign_APRS_DEST;
  // start of "real" data (Telemetry, duh)
  Outstring += ":T#";

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
  uint8_t ui8val5=0;
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

String generateWXPayload()
{
    // char buf[46];
    String Outstring = String("");

    MeasureBME(false);

    //Correct pressure
    float temp_K = 273.15;
    temp_K += bme_temp;
    // float pres = bme_pres * powf((temp_K / (temp_K + (0.0065 * (float)UGPS.Altitude))), -5.255);
    float pres = bme_pres * 10.0;

    float tempf=(bme_temp*1.8)+32; // celsius to fahrenheit
    float humi = bme_humi;
    //float wind=0;     // Kein Wind, das ist noch W.I.P.!
    //float wind2 = wind*0.621;  // wind2 is in mph
    Outstring = "";
    Outstring += "<\xff\x01";
    // Add Source
    Outstring += LORA_APRS_PAYLOAD_ID;
    // Add SSID
    Outstring += LORA_APRS_SSID;
    //Add Destination (do not use digipeating)
    Outstring += ">";
    Outstring += "APETBT";   // destination callsign_APRS_DEST;
    // start of "real" data (Telemetry, duh)
    Outstring += ":_";

    if(UGPS.Month<10) { Outstring += "0"; }
    Outstring += String(UGPS.Month);
    if(UGPS.Day<10) { Outstring += "0"; }
    Outstring += String(UGPS.Day);
    if(UGPS.Hours<10) { Outstring += "0"; }
    Outstring += String(UGPS.Hours);
    if(UGPS.Minutes<10) { Outstring += "0"; }
    Outstring += String(UGPS.Minutes);
    Outstring += ("c...s...");
    Outstring += ("g...");
    Outstring += ("t");
    if(tempf<100) { Outstring += "0"; }
    if(tempf<10) { Outstring += "0"; }
    Outstring += String(tempf,0);
    Outstring += ("r...P...p...h");
    Outstring += String(humi,0);
    Outstring += ("b");
       if(pres<1000) { Outstring += "0"; }
       Outstring += String(pres,0);
    // Outstring += String("Actual Data: Temp ") + String(bme_temp, 1) + String(", Humi: ") + String(humi, 1) + String("  ");

    return Outstring;
}

#endif
#endif