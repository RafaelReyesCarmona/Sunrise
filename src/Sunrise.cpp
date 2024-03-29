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


unsigned char Sunrise::Hour(){
	return theHour;
}

unsigned char Sunrise::Minute(){
	return theMinute;
}
