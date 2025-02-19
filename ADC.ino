// You will need to create your own code here.


//===============================================================================
// Read the VCC voltage.
// Create your own code here
// Useful for checking battery voltage.
float ReadVCC() 
{
  uint32_t batval = 0xFFFFFFFF;
  batval = analogRead(BAT_ADC);

  float batVol = (float)batval * 0.006764706;
  // SerialDebug.print("Value of Battery ADC: ");
  // SerialDebug.println(batval);
  return (batVol);
}


//===============================================================================
// Read the internal chip temperature//
// Create your own code here
float ReadTemp(void)
{

  // The returned temperature is in degrees Celcius.
  return (0.0);
}
