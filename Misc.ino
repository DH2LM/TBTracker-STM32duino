/***********************************************************************************
* The function CreateTXLine generates two payload sentences to transmit. One
* for RTTY and one for LoRa. These sentences are compatible with the habhub tracker.
* 
* Run the software and decode your payload sentences. Then go to 
* http://habitat.habhub.org/genpayload/ and register your payload there by
* generating a payload configuration document. You will need the generated sentences
* to create a payload configuration document. When you do that, other people will be
* able to see your tracker on the map at https://tracker.habhub.org
*
* Sample of a RTTY payload sentence this software will generate:
* 
* $$RTTY-ID,5,12:11:10,53.16307,6.16444,12,3,12.0,4.61,3.71*109F 
* 
* This sentence contains the following fields, separated by comma's:
*     $$        = prefix
*     RTTY-ID   = callsign
*     5         = sequence number
*     12:11:10  = Time (UTC)
*     53.163067 = Latitude
*     6.16444   = Longitude
*     12        = Altitude
*     3         = Number of satellites
*     12.0      = Internal (chip) temperature in Celsius
*     4.61      = VCC voltage
*     3.71      = External voltage on analog pin A1
*     *         = seprator
*     109F      = Checksum
*     
* The LoRa payload looks the same, except for the callsign (if you changed that).     
************************************************************************************/
//===============================================================================
void CreateTXLine(const char *PayloadID, unsigned long aCounter, const char *aPrefix)
{
   char Sen[5];
   int Count, i, j;
   unsigned char c;
   unsigned int uiCRC, xPolynomial;
   char LatitudeString[16], LongitudeString[16], CRCString[8];
   char InternalTemp[10];
   char BattVoltage[10];
   char ExtVoltage[10];

   // Get the internal chip temperature
   dtostrf(ReadTemp(), 3, 1, InternalTemp);

   // Get the battery voltage
   dtostrf(ReadVCC(), 4, 2,BattVoltage);
         
   dtostrf(UGPS.Latitude, 7, 5, LatitudeString);
   dtostrf(UGPS.Longitude, 7, 5, LongitudeString);   
   
   sprintf(Sentence,
#if defined(USE_FIELDSTR)               
            "%s%s,%ld,%02d:%02d:%02d,%s,%s,%ld,%u,%u,%s,%s,%s"
#else
            "%s%s,%ld,%02d:%02d:%02d,%s,%s,%ld,%u,%u,%s,%s"
#endif
            ,
            aPrefix,
            PayloadID,
            aCounter,
            UGPS.Hours, UGPS.Minutes, UGPS.Seconds,   
            LatitudeString,
            LongitudeString,
            UGPS.Altitude,
            UGPS.Satellites,
            UGPS.Heading,
            InternalTemp,
            BattVoltage
#if defined(USE_FIELDSTR)            
            ,
            FIELDSTR
#endif            
            );

   Count = strlen(Sentence);

   // Calc CRC
   uiCRC = 0xffff;           // Seed
   xPolynomial = 0x1021;
   
   for (i = strlen(aPrefix); i < Count; i++)
   {   // For speed, repeat calculation instead of looping for each bit
      uiCRC ^= (((unsigned int)Sentence[i]) << 8);
      for (j=0; j<8; j++)
      {
          if (uiCRC & 0x8000)
              uiCRC = (uiCRC << 1) ^ 0x1021;
          else
              uiCRC <<= 1;
      }
   }

   Sentence[Count++] = '*';
   Sentence[Count++] = Hex((uiCRC >> 12) & 15);
   Sentence[Count++] = Hex((uiCRC >> 8) & 15);
   Sentence[Count++] = Hex((uiCRC >> 4) & 15);
   Sentence[Count++] = Hex(uiCRC & 15);
   Sentence[Count++] = '\n';  
   Sentence[Count++] = '\0';
}

//===============================================================================
char Hex(char Character)
{
  char HexTable[] = "0123456789ABCDEF";
  
  return HexTable[Character];
}



