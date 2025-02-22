#ifdef USE_BME280
#include <Wire.h>

#include <forcedBMX280.h>

ForcedBME280Float bme = ForcedBME280Float(); 

float temperature, humidity, pressure;
#endif

#define CALX_TEMP 30

#define VTEMP      760
#define AVG_SLOPE 2500
#define VREFINT   1212

/* Analog read resolution */
#define LL_ADC_RESOLUTION LL_ADC_RESOLUTION_12B
#define ADC_RANGE 4096

// You will need to create your own code here.

bool BMEIsValid = false;

//Taken from official STM32duino doc
static int32_t readVref()
{
#ifdef STM32U0xx
  /* On some devices Internal voltage reference calibration value not programmed
     during production and return 0xFFFF. See errata sheet. */
  if ((uint32_t)(*VREFINT_CAL_ADDR) == 0xFFFF) {
    return 3300U;
  }
#endif
#ifdef __LL_ADC_CALC_VREFANALOG_VOLTAGE
#ifdef STM32U5xx
  return (__LL_ADC_CALC_VREFANALOG_VOLTAGE(ADC1, analogRead(AVREF), LL_ADC_RESOLUTION));
#else
  return (__LL_ADC_CALC_VREFANALOG_VOLTAGE(analogRead(AVREF), LL_ADC_RESOLUTION));
#endif
#else
  return (VREFINT * ADC_RANGE / analogRead(AVREF)); // ADC sample to mV
#endif
}

//Taken from official STM32duino doc
static int32_t readTempSensor(int32_t VRef)
{
#ifdef __LL_ADC_CALC_TEMPERATURE
#ifdef STM32U5xx
  return (__LL_ADC_CALC_TEMPERATURE(ADC1, VRef, analogRead(ATEMP), LL_ADC_RESOLUTION));
#else
  return (__LL_ADC_CALC_TEMPERATURE(VRef, analogRead(ATEMP), LL_ADC_RESOLUTION));
#endif
#elif defined(__LL_ADC_CALC_TEMPERATURE_TYP_PARAMS)
  return (__LL_ADC_CALC_TEMPERATURE_TYP_PARAMS(AVG_SLOPE, VTEMP, CALX_TEMP, VRef, analogRead(ATEMP), LL_ADC_RESOLUTION));
#else
  return 0;
#endif
}

static int32_t readVoltage(int32_t VRef, uint32_t pin)
{
#ifdef STM32U5xx
  return (__LL_ADC_CALC_DATA_TO_VOLTAGE(ADC1, VRef, analogRead(pin), LL_ADC_RESOLUTION));
#else
  return (__LL_ADC_CALC_DATA_TO_VOLTAGE(VRef, analogRead(pin), LL_ADC_RESOLUTION));
#endif
}

void SetupBME280()
{
  #ifdef USE_BME280
  uint8_t timeout = 0;
  Wire.begin(SDA, SCL);

  while(bme.begin() && timeout < 20) {
    BMEIsValid = false;
    timeout++;
    digitalWrite(LED_RED, !digitalRead(LED_RED));
    delay(500);
  }

  // bme.setSettings(Bme280Settings::weatherMonitoring());
  // bme.sleep();
  if(timeout < 20) BMEIsValid = true;

  temperature = ReadTemp();
  pressure = ReadPres();
  humidity = ReadHumi();
  #endif
}

//===============================================================================
// Read the VCC voltage.
// Create your own code here
// Useful for checking battery voltage.
float ReadVCC() 
{
  //We know that the STM32 ADC is 10 bits
  //That means at a maximum voltage its value will be 1024.
  //We assume that maximum voltage is either 3.3V or 3.6V.
  //Assuming that the ADC works in a linear fashion the battery voltage can be easily calculated.
  //We also have to take the voltage divider before the ADC in account. Since both resistors in the divider
  //are equal, we have a factor of 2.

  //First we read our value from the ADC
  uint32_t adcPinVal = 0xFFFFFFFF;
  adcPinVal = analogRead(BAT_ADC);

  //Then we calculate the voltage on the pin
  //So, our voltage on the Pin is v(P) = adcPinVal * (3.6V/4096);
  float adcPinVoltage = (float)adcPinVal * (3.3 / 4096.0);

  //And since the voltage is half of our battery voltage due to the divider we simply double it.
  float batVoltage = adcPinVoltage * 2.0;
  // SerialDebug.print("Value of Battery ADC: ");
  // SerialDebug.println(batval);
  return (batVoltage);
}


//===============================================================================
// Read the internal chip temperature//
// Create your own code here

//This will read the temperature off the BME280, if one is connected.
//Else it will return 0.0
float ReadTemp(void)
{
  if(!BMEIsValid) return 0.0;
  float temp = 0.0;

  #ifdef USE_BME280
  bme.takeForcedMeasurement();
  temp = bme.getTemperatureCelsiusAsFloat(true);
  #endif

  // The returned temperature is in degrees Celcius.
  return (temp);
}

//This will read the humidity off the BME280, if one is connected.
//Else it will return 0.0
float ReadHumi(void)
{
  if(!BMEIsValid) return 0.0;
  float humi = 0.0;

  #ifdef USE_BME280
  bme.takeForcedMeasurement();
  humi = bme.getRelativeHumidityAsFloat();
  #endif

  // The returned humidity is in Percent.
  return(humi);
}

//This will read the pressure off the BME280, if one is connected.
//Else it will return 0.0
float ReadPres(void)
{
  if(!BMEIsValid) return 0.0;
  float pres = 0.0;

  #ifdef USE_BME280
  bme.takeForcedMeasurement();
  pres = bme.getPressureAsFloat();
  #endif

  // The returned pressure is in Hectopascal (hPa).
  return(pres);
}
