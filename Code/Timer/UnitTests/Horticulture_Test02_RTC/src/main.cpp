//Lib: https://github.com/stm32duino/STM32RTC
//
#include <Arduino.h>
#include <STM32RTC.h>
HardwareSerial uart2(PA3, PA2);

STM32RTC& rtc = STM32RTC::getInstance();

const byte seconds = 0;
const byte minutes = 57;
const byte hours = 9;
const byte weekDay = 3;
const byte day = 17;
const byte month = 7;
const byte year = 24;

void setup()
{
  uart2.begin(115200);

  rtc.setClockSource(STM32RTC::LSE_CLOCK); //LSI_CLOCK, LSE_CLOCK or HSE_CLOCK.

  rtc.begin(); 

  rtc.setTime(hours, minutes, seconds);
  rtc.setDate(weekDay, day, month, year);
}

void loop()
{
  uart2.printf("%02d/%02d/%02d ", rtc.getDay(), rtc.getMonth(), rtc.getYear());
  uart2.printf("%02d:%02d:%02d.%03d\n", rtc.getHours(), rtc.getMinutes(), rtc.getSeconds(), rtc.getSubSeconds());

  delay(1000);
}