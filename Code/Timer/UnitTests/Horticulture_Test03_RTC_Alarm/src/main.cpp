#include <Arduino.h>
#include <STM32RTC.h>
HardwareSerial uart2(PA3, PA2);

/* Get the rtc object */
STM32RTC& rtc = STM32RTC::getInstance();

/* Change these values to set the current initial time */
const byte seconds = 0;
const byte minutes = 0;
const byte hours = 16;

/* Change these values to set the current initial date */
const byte day = 25;
const byte month = 9;
const byte year = 15;

void alarmMatch(void *data);

void setup()
{
  uart2.begin(115200);
  rtc.setClockSource(STM32RTC::LSE_CLOCK); //LSI_CLOCK, LSE_CLOCK or HSE_CLOCK.

  rtc.begin(); // initialize RTC 24H format

  rtc.setTime(hours, minutes, seconds);
  rtc.setDate(day, month, year);

  rtc.attachInterrupt(alarmMatch);
  rtc.setAlarmDay(day);
  rtc.setAlarmTime(16, 0, 10, 123); //Hours, Minutes, Seconds
  rtc.enableAlarm(rtc.MATCH_DHHMMSS);
}

void loop()
{

}

void alarmMatch(void *data)
{
  UNUSED(data);
  uart2.println("Alarm Match!");
}