#include <Arduino.h>
#include <Sunrise.h>
#include <Math.h>
#include <TimeLib32.h>

Sunrise::Sunrise(float latitude, float longitude, float timezone){
	
	rd=57.295779513082322; 	// Constante para conversión de grados a radianes.
	lat=latitude/rd;		// Pasamos la latitud a radianes.
	lon=-longitude/rd;		// Pasamos la longitud a radianes.

	tz=timezone;

  	//zenith=1.579522973054868; // default to actual times (90,50º) ???
	zenith=(90.0 + 50.0/60.0) / rd; // zenith for 90º 50m -> 90.833333º
	theHour=255;
	theMinute=0;
}

Sunrise::Sunrise(float latitude, float longitude){
	Sunrise(latitude, longitude,0);
}

Sunrise::Sunrise(long latitude, long longitude){
	Sunrise((float)(latitude/10000000L), (float)(longitude/10000000L), 0);
}

void Sunrise::SetTZ(float timezone){
	tz=timezone;
}

void Sunrise::Actual(){
	zenith=(90.0 + 50.0/60.0) / rd; // 90.83333333333333/rd;
}

void Sunrise::Civil(){
  zenith=(90.0 + 6.0) / rd; // 1.675516081914556;
}

void Sunrise::Nautical(){
  zenith=(90.0 + 12.0) / rd; //1.780235837034216;
}

void Sunrise::Astronomical(){
  zenith=(90.0 + 18.0) / rd; //1.884955592153876;
}

int Sunrise::Rise(unsigned char  month, unsigned char  day){
  return Sunrise::Compute(month, day, 1);
}

int Sunrise::Set(unsigned char  month, unsigned char  day){
  return Sunrise::Compute(month, day, 0);
}

int Sunrise::Noon(unsigned char  month, unsigned char  day){
	return Sunrise::Compute(month, day, 2);
}

int Sunrise::SolarTime(time32_t mytime){
  return Sunrise::Compute(mytime);
}

int Sunrise::Compute(unsigned char  month, unsigned char  day, int rs) {
  float y, decl, eqt, ha;
  unsigned char a;
  int doy, minutes;
  int dom[] = {0,31,59,90,120,151,181,212,243,273,304,334};
  
  // approximate hour;
  a=6;
  if(rs) a=18;
  
  // approximate day of year (365d 6h 9m 9.7632s) Año sidéreo = 365.256363 days.
  //doy=(month-1)*30.43803025+day-1;
  doy=dom[month-1]+day-1;
  
  // compute fractional year
  y=1.721420632104e-02*(doy+a/24);
  
  // compute equation of time
  eqt=229.18 * (0.000075+0.001868*cos(y)  -0.032077*sin(y) -0.014615*cos(y*2) -0.040849*sin(y* 2) );
  
  // compute solar declination
  decl=0.006918-0.399912*cos(y)+0.070257*sin(y)-0.006758*cos(y*2)+0.000907*sin(y*2)-0.002697*cos(y*3)+0.00148*sin(y*3);
  
  //compute hour angle
  ha=(  cos(zenith) / (cos(lat)*cos(decl)) -tan(lat) * tan(decl)  );
  if(fabs(ha)>1){// we're in the (ant)arctic and there is no rise(or set) today!
  	theHour=255;
  	return -1; 
  }
  ha=acos(ha); 
  if(rs==0) ha=-ha;
	
  if(rs<2){
	  // computes minutes into the day of the event
	  minutes=720+4*(lon-ha)*rd-eqt;
  
	  // convert from UTC back to our timezone
	  minutes+= (tz*60);
	  if(minutes<0) minutes+=1440;
	  minutes%=1440;
  
	  // stuff hour into month, minute into day
	  theHour=minutes/60;
	  theMinute=minutes-theHour*60;

    }else if(rs==2){
	  // computes minutes into the day of the event
	  minutes=720+4*lon*rd-eqt;
	  
	  // convert from UTC back to our timezone
	  minutes+= (tz*60);
	  if(minutes<0) minutes+=1440;
	  minutes%=1440;
	  
	  // stuff hour into month, minute into day
	  theHour=minutes/60;
	  theMinute=minutes-theHour*60;
	  
  }
	  return minutes;
}

int Sunrise::Compute(time32_t mytime) {
	double JC, GMLS, GMAS, EEO, SEC, STL, SAL, MOE, OC, SD, Y, eqt, HA;
	time32_t days, SolarNoon, SunriseTime, SunsetTime;


	// Podemos calcular la fecha Juliana con la función siguiente:
	// JC = CalcJC(mytime);		// Pierde precisión conforme aumenta la fecha.

	// Mejor obtener la fecha Juliana como números de dias que han pasado desde 1970.
	days = mytime / SECS_PER_DAY;
	// Después se obtiene la fecha Juliana con la siguiente operación:
    JC = ((float)days)/36525.0 - 0.3;	// Esta operación es el resultado de simplificar el calculo para el tiempo juliano.

	// Calculado el tiempo juliano se aplican las ecuaciones astronómicas de Jean Meeus.
	// Estas ecuaciones son una adaptación de: https://gml.noaa.gov/grad/solcalc/NOAA_Solar_Calculations_year.xls
	GMLS = fmod(280.46646+JC*(36000.76983 + JC*3.032e-4),360);
	GMAS = (357.52911+JC*(35999.05029 - 1.537e-4*JC));
	EEO = 1.6708634e-2-JC*(4.2037e-5+1.267e-7*JC);
	SEC = sin(GMAS/rd)*(1.914602-JC*(4.817e-3+1.4e-5*JC))+sin(2*GMAS/rd)*(1.9993e-2-1.01e-4*JC)+sin(3*GMAS/rd)*2.89e-4;
	STL = GMLS + SEC;
	SAL = STL-5.69e-3-4.78e-3*sin((125.04-1934.136*JC)/rd);
	MOE = 23.0+(26.0+((21.448-JC*(46.815+JC*(0.00059-JC*0.001813))))/60.0)/60.0;
	OC = MOE+0.00256*cos((125.04-1934.136*JC)/rd);
	SD = asin(sin(OC/rd)*sin(SAL/rd));
	Y = tan(OC/(2*rd))*tan(OC/(2*rd));

	eqt = 4.0*rd*(Y*sin(2.0*GMLS/rd)-2.0*EEO*sin(GMAS/rd)+4.0*EEO*Y*sin(GMAS/rd)*cos(2.0*GMLS/rd)-0.5*Y*Y*sin(4*GMLS/rd)-1.25*EEO*EEO*sin(2*GMAS/rd));
	HA = cos(zenith)/(cos(lat)*cos(SD))-tan(lat)*tan(SD);

	if(fabs(HA)>1){// we're in the (ant)arctic and there is no rise(or set) today!
		return -1; 
	}
	HA=acos(HA)*rd;

	// Calculate seconds for noon, rise and set from last midnight.
	SolarNoon = (time32_t)((float)SECS_PER_MIN * (720.0-(4.0*(-lon)*rd)-eqt));
	SunriseTime = SolarNoon - (time32_t)((float)SECS_PER_MIN * (HA*4.0));
	SunsetTime = SolarNoon + (time32_t)((float)SECS_PER_MIN * (HA*4.0));

	SetTime_t=days*SECS_PER_DAY+SunsetTime;
	RiseTime_t=days*SECS_PER_DAY+SunriseTime;
	NoonTime_t=days*SECS_PER_DAY+SolarNoon;

	//SetTime_t -= leap_seconds(SetTime_t);
	//RiseTime_t -= leap_seconds(RiseTime_t);
	//NoonTime_t -= leap_seconds(NoonTime_t);

	return (8*HA);
}


unsigned char Sunrise::Hour(){
	return theHour;
}

unsigned char Sunrise::Minute(){
	return theMinute;
}

time32_t Sunrise::NoonTime(){
	return NoonTime_t;
}

time32_t Sunrise::RiseTime(){
	return RiseTime_t;
}

time32_t Sunrise::SetTime(){
	return SetTime_t;
}

// This function for stimate Julian Day and Century is not more exactly.
double Sunrise::CalcJC(time32_t mytime){
	int JDyear, JDmonth, JDday;
	JDday = day(mytime);
	JDmonth = month(mytime);
	JDyear = year(mytime);

		if (JDmonth <= 2) {
			JDyear -= 1;
			JDmonth += 12;
		}
		int A = (int)(JDyear/100);
		int B = 2 - A + (int)(A/4);

		long JD = (long)(365.25*(JDyear + 4716)) + (long)(30.6001*(JDmonth+1)) + JDday + B - 1524.5;
		double JC = (JD - 2451545.0)/36525.0 - 1.36892539356e-5;
		return JC;
}