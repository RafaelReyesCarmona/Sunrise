#ifndef Sunrise_h
#define Sunrise_h

#include <TimeLib32.h>

class Sunrise{
  public:
  Sunrise(float, float, float);
  Sunrise(float, float);
  Sunrise(long, long);
  void SetTZ(float);
  void Actual();
  void Civil();
  void Nautical();
  void Astronomical();
  int Rise(unsigned char ,unsigned char );
  int Set(unsigned char ,unsigned char );
  int Noon(unsigned char ,unsigned char );
  int SolarTime(time32_t);
  unsigned char Hour();
  unsigned char Minute();
  time32_t RiseTime();
  time32_t SetTime();
  time32_t NoonTime();
  
  private:
  int Compute(unsigned char ,unsigned char, int);
  int Compute(time32_t);
  double CalcJC(time32_t mytime);
  float lat,lon, zenith, rd, tz;
  unsigned char  theHour,theMinute;
  time32_t RiseTime_t, SetTime_t, NoonTime_t; 
};

#endif

//http://souptonuts.sourceforge.net/code/sunrise.c.html
//https://gml.noaa.gov/grad/solcalc/sunrise.html
//https://gml.noaa.gov/grad/solcalc/
//https://www.suncalc.org/