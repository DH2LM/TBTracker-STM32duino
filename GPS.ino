#include <TinyGPS++.h>


// The TinyGPS++ object
TinyGPSPlus gps;

/*********************************************************************************************************************************/
void CheckGPS()
{    
  processGPSData();
  printGPSData();
}


/*********************************************************************************************************************************/
// This custom version of delay() ensures that the gps object is being "fed".
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (SerialGPS.available())
      gps.encode(SerialGPS.read());
  } while (millis() - start < ms);
}

/*********************************************************************************************************************************/
static void processGPSData()
{
  
  // Number of Satellitese
  if (gps.satellites.isValid())
    UGPS.Satellites = gps.satellites.value();
  else
    UGPS.Satellites = 0;

 // Time
 if (gps.time.isValid())
 {
    UGPS.Hours = gps.time.hour();
    UGPS.Minutes = gps.time.minute();
    UGPS.Seconds = gps.time.second();
    UGPS.Day = gps.date.day();
 }
 else
 {
    UGPS.Hours = 0;
    UGPS.Minutes = 0;
    UGPS.Seconds = 0;
    UGPS.Day = 0;
 }

 // Position
 if (gps.location.isValid())
 {
    UGPS.Longitude = gps.location.lng();
    UGPS.Latitude = gps.location.lat();
    UGPS.Heading = gps.course.deg();
 }
 else
 {
   UGPS.Longitude = 0;
   UGPS.Latitude = 0;
   UGPS.Heading = 0;
 }

 // Altitude
 if (gps.altitude.isValid())
    UGPS.Altitude = gps.altitude.meters();
 else
    UGPS.Altitude = 0;    

 if (UGPS.Altitude < 0)
   UGPS.Altitude = 0;    
   
}

/*********************************************************************************************************************************/
void printGPSData()
{
  // String str;
  // SerialDebug.print(F("Time: ")); // SerialDebug.print(UGPS.Hours); // SerialDebug.print(":"); // SerialDebug.print(UGPS.Minutes); // SerialDebug.print(":"); // SerialDebug.println(UGPS.Seconds);
  // SerialDebug.print(F(" Lat: ")); // SerialDebug.print(UGPS.Latitude, 6);
  // str = getAPRSlat(UGPS.Latitude); // SerialDebug.print("  ("); // SerialDebug.print(str); // SerialDebug.println(")");
  // SerialDebug.print(F(" Lon: ")); // SerialDebug.print(UGPS.Longitude, 6);
  // str = getAPRSlon(UGPS.Longitude); // SerialDebug.print("  ("); // SerialDebug.print(str); // SerialDebug.println(")");
  // SerialDebug.print(F(" Alt: ")); // SerialDebug.print(UGPS.Altitude);
  // str = getAPRSAlt(UGPS.Altitude); // SerialDebug.print("  ("); // SerialDebug.print(str); // SerialDebug.println(")");
  // SerialDebug.print(F("Sats: ")); // SerialDebug.println(UGPS.Satellites);
  // SerialDebug.println("------"); 
}


/*********************************************************************************************************************************/
String getAPRSlon(float lon_dd)
{
  float lon_d1;
  int lon_d;
  float lon_m1;
  int lon_m;
  float lon_s;
  String lon_d_str;
  String lon_m_str;
  char lon_s_str[6];
  String lon;


  String SouthN;
  // West or East
  if (lon_dd < 0)
    SouthN = "W";
  else
    SouthN = "E";
  
  // Calculate the string for the degrees
  lon_d1 = abs(lon_dd);
  lon_d = lon_d1;
  lon_d_str = String(lon_d);
  if (abs(lon_d) < 10)
  {
    lon_d_str = "00" + lon_d_str;
  }
  else if (abs(lon_d) < 100)
       {
         lon_d_str = "0" + lon_d_str;
       }

  // Calculate the string for the minutes
  lon_m1 = (lon_d1 - lon_d) * 60;
  lon_m = lon_m1;
  lon_m_str = String(lon_m);
  if ( abs(lon_m) < 10 )
  {
     lon_m_str = "0" + lon_m_str;
  }
  
  // Calculate the string for the seconds
  lon_s = (lon_m1 - lon_m);
  dtostrf(lon_s,4,2,lon_s_str) ;

  lon = lon_d_str + lon_m_str + &lon_s_str[1] + SouthN;
  
  return lon;
}


/*********************************************************************************************************************************/
String getAPRStimestamp()
{
  char buf[10];

  sprintf(buf,"%02d%02d%02dh",UGPS.Hours,UGPS.Minutes,UGPS.Seconds);
  return buf;
}

/*********************************************************************************************************************************/
String getAPRSlat(float lat_dd)
{
  float lat_d1;
  int lat_d;
  float lat_m1;
  int lat_m;
  float lat_s;
  String lat_d_str;
  String lat_m_str;
  char lat_s_str[6];
  String lat;


  String lat_SouthN;
  // South or North
  if (lat_dd < 0)
    lat_SouthN = "S";
  else
    lat_SouthN = "N";
  
  // Calculate the string for the degrees
  lat_d1 = abs(lat_dd);
  lat_d = lat_d1;
  lat_d_str = String(lat_d);
  if (abs(lat_d) < 10)
  {
    lat_d_str = "0" + lat_d_str;
  }

  // Calculate the string for the minutes
  lat_m1 = (lat_d1 - lat_d) * 60;
  lat_m = lat_m1;
  lat_m_str = String(lat_m);
  if ( abs(lat_m) < 10 )
  {
     lat_m_str = "0" + lat_m_str;
  }
  
  // Calculate the string for the seconds
  lat_s = (lat_m1 - lat_m);
  dtostrf(lat_s,4,2,lat_s_str) ;

  lat = lat_d_str + lat_m_str + &lat_s_str[1] + lat_SouthN;
  
  return lat;
}

/*********************************************************************************************************************************/
String getAPRSAlt(long Alt)
{
   long Feet;
   char alt_str[15];
   String res = "";

   // altitudein feet 
   Feet = Alt*3.28;
   ltoa(Feet,alt_str,10);
   res = alt_str;
   
   for(int i=(6-res.length()); i>0; i--)  
   {
      res = "0" + res;
   }
   res = "/A=" + res;
     
   return res;
}