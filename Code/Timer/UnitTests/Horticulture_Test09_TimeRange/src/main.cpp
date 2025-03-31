#include <Arduino.h>
HardwareSerial uart2(PA3, PA2);
#define ONHOURS 23
#define ONMINUTES 0
#define OFFHOURS 1
#define OFFMINUTES 0
#define CLOCKHOURS 1
#define CLOCKMINUTES 0 


bool timeWithinInterval(uint16_t start, uint16_t end, uint16_t now);

void setup()
{
  uart2.begin(115200);
  uint16_t onTime=(ONHOURS*60)+ONMINUTES;
  uint16_t offTime=(OFFHOURS*60)+OFFMINUTES;
  uint16_t clock=(CLOCKHOURS*60)+CLOCKMINUTES;
  if(timeWithinInterval(onTime, offTime, clock))
  {
    uart2.println("Within interval");
  }
  else
  {
    uart2.println("Outside interval");
  }
  
}




void loop()
{
  
}



bool timeWithinInterval(uint16_t start, uint16_t end, uint16_t now) 
{
  uint16_t duration=(end-start+1440)%1440;
  uint16_t time_on=(now-start+1440)%1440;
  return (time_on<duration);
}