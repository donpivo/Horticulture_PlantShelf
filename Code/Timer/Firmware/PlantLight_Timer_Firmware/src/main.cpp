#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <STM32RTC.h> //https://github.com/stm32duino/STM32RTC
#include <FlashStorage_STM32.h> //https://github.com/khoih-prog/FlashStorage_STM32

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define DISPLAY_RESET -1
#define DISPLAY_ADDRESS 0x3C
#define PIN_ENCA PA0
#define PIN_ENCB PA1
#define PIN_ENCSW PA15
#define PIN_OUTPUT_EN PA4
#define DEBOUNCE_TIME 500
#define FLASH_SAVE_DELAY 5000



TwoWire Wire1(PB9,PB8);
HardwareSerial uart2(PA3,PA2);
Adafruit_SSD1306 display(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire1, DISPLAY_RESET);
STM32RTC& rtc = STM32RTC::getInstance();

volatile bool aFlag=false;
volatile bool bFlag=false;
volatile int16_t encPos, oldEncPos;
volatile uint16_t reading=0;
uint8_t clockMinutes, clockHours;
uint8_t onMinutes, onHours;
uint8_t offMinutes, offHours;
uint8_t itemToUpdate=0;
volatile uint32_t lastBtnPress=millis();
uint32_t lastTimeChange=millis();
bool saveToFlash=false;
bool updateDisplayOnTime=true;
bool updateDisplayOffTime=true;
bool updateDisplayClock=true;
bool lightIsOn=false;
volatile bool btnPressed;
enum timeItem{ONTIME, OFFTIME, CLOCK};

void isrENCA();
void isrENCB();
void isrENCSW();
void printTime(timeItem showItem);
void flashSave();
void lightSwitch();
bool timeWithinInterval(uint16_t start, uint16_t end, uint16_t now);


void setup()
{
  uart2.begin(115200);
  uart2.println("Retronics Horticulture Timer");
  pinMode(PIN_ENCA, INPUT_PULLUP);
  pinMode(PIN_ENCB, INPUT_PULLUP);
  pinMode(PIN_ENCSW, INPUT_PULLUP);
  pinMode(PIN_OUTPUT_EN, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(PIN_ENCA), isrENCA, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_ENCB), isrENCB, RISING);
  attachInterrupt(digitalPinToInterrupt(PIN_ENCSW), isrENCSW, FALLING);
  rtc.setClockSource(STM32RTC::LSE_CLOCK);
  rtc.begin();  
  delay(100);
  display.begin(SSD1306_SWITCHCAPVCC, DISPLAY_ADDRESS);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(1,1);
  display.print("On");
  display.setCursor(88,1);
  display.print(":");
  display.setCursor(1,21);
  display.print("Off");
  display.setCursor(88,21);
  display.print(":");
  display.drawLine(0,40,127,40, SSD1306_WHITE);
  display.setCursor(1,45);
  display.print("Time");
  display.setCursor(88,45);
  display.print(":");
  display.display();
  uint8_t eepromData;
  EEPROM.get(0,eepromData);
  onHours=eepromData<0?0:eepromData>23?23:eepromData;
  EEPROM.get(1,eepromData);
  onMinutes=eepromData<0?0:eepromData>59?59:eepromData;
  EEPROM.get(2,eepromData);
  offHours=eepromData<0?0:eepromData>23?23:eepromData;
  EEPROM.get(3,eepromData);
  offMinutes=eepromData<0?0:eepromData>59?59:eepromData;
}

void loop()
{
  if(btnPressed)
  {
    btnPressed=false;
    itemToUpdate=itemToUpdate<6?itemToUpdate+1:0;
    switch (itemToUpdate)
    {
    case 1:
      encPos=onHours;
      updateDisplayOnTime=true;
      break;
    case 2:
      encPos=onMinutes;
      updateDisplayOnTime=true;
      break;
    case 3:
      encPos=offHours;
      updateDisplayOnTime=true;
      updateDisplayOffTime=true;
      break;
    case 4:
      encPos=offMinutes;
      updateDisplayOffTime=true;
      break;
    case 5:
      encPos=clockHours;
      updateDisplayOffTime=true;
      updateDisplayClock=true;
      break;
    case 6:
      encPos=clockMinutes;
      updateDisplayClock=true;
      break;
    default:
      updateDisplayClock=true;
      break;
    }
    oldEncPos=encPos;
  }
  if(oldEncPos!=encPos) 
  {
    switch (itemToUpdate)
    {
    case 1:
      encPos=(encPos>23)?23:encPos;
      onHours=encPos;
      saveToFlash=true;
      lastTimeChange=millis();
      updateDisplayOnTime=true;
      break;
    case 2:
      encPos=(encPos>59)?59:encPos;
      onMinutes=encPos;
      saveToFlash=true;
      lastTimeChange=millis();
      updateDisplayOnTime=true;
      break;
    case 3:
      encPos=(encPos>23)?23:encPos;
      offHours=encPos;
      saveToFlash=true;
      lastTimeChange=millis();
      updateDisplayOffTime=true;
      break;
    case 4:
      encPos=(encPos>59)?59:encPos;
      offMinutes=encPos;
      saveToFlash=true;
      lastTimeChange=millis();
      updateDisplayOffTime=true;
      break;
    case 5:
      encPos=(encPos>23)?23:encPos;
      rtc.setHours(encPos);
      updateDisplayClock=true;
      break;
    case 6:
      encPos=(encPos>59)?59:encPos;
      rtc.setMinutes(encPos);
      updateDisplayClock=true;
      break;
    default:
      break;
    }
    oldEncPos = encPos;
  }
  if(updateDisplayClock||(clockMinutes!=rtc.getMinutes())) printTime(CLOCK);
  if(updateDisplayOnTime) printTime(ONTIME);
  if(updateDisplayOffTime) printTime(OFFTIME);
  if(saveToFlash&(millis()-lastTimeChange>FLASH_SAVE_DELAY))
  {
    flashSave();
  }
  lightSwitch();
}

void isrENCA()
{
  noInterrupts();
  reading = GPIOA->IDR & 0x3;
  if(reading == 0b0000000000000011 && aFlag) 
  {
    encPos=(encPos<1)?0:encPos-1;
    bFlag = 0;
    aFlag = 0;
  }
  else if (reading == 0b0000000000000001) bFlag = 1;
  interrupts();
}

void isrENCB()
{
  noInterrupts();
  reading = GPIOA->IDR & 0x3;
  if (reading == 0b0000000000000011 && bFlag) 
  {
    encPos++;
    bFlag = 0;
    aFlag = 0;
  }
  else if (reading == 0b0000000000000010) aFlag = 1;
  interrupts();
}

void isrENCSW()
{
  if(millis()-lastBtnPress>DEBOUNCE_TIME)
  {
    noInterrupts();
    btnPressed=true;
    lastBtnPress=millis();
    interrupts();
  }
  
}

void printTime(timeItem showItem)
{
  uint8_t yPixels;
  String strHours, strMinutes;
  bool invertHours, invertMinutes;
  switch (showItem)
  {
    case ONTIME:
      yPixels=1;
      invertHours=(itemToUpdate==1);
      invertMinutes=(itemToUpdate==2);
      if(onHours<10) strHours="0";
      strHours+=String(onHours);
      if(onMinutes<10) strMinutes="0";
      strMinutes+=String(onMinutes);
      updateDisplayOnTime=false;
      uart2.println("Print ON time");
      break;
    case OFFTIME:
      yPixels=21;
      invertHours=(itemToUpdate==3);
      invertMinutes=(itemToUpdate==4);
      if(offHours<10) strHours="0";
      strHours+=String(offHours);
      if(offMinutes<10) strMinutes="0";
      strMinutes+=String(offMinutes);
      updateDisplayOffTime=false;
      uart2.println("Print OFF time");
      break;
    case CLOCK:
      clockHours=rtc.getHours();
      clockMinutes=rtc.getMinutes();
      yPixels=45;
      invertHours=(itemToUpdate==5);
      invertMinutes=(itemToUpdate==6);
      if(clockHours<10) strHours="0";
      strHours+=String(clockHours);
      if(clockMinutes<10) strMinutes="0";
      strMinutes+=String(clockMinutes);
      updateDisplayClock=false;
      uart2.println("Print CLOCK");
      break;
    default:
      break;
  }
  display.fillRect(64,yPixels-2,25,19,invertHours);
  display.setCursor(65,yPixels);
  display.setTextColor(!invertHours);
  display.print(strHours);
  display.fillRect(97,yPixels-2,25,19,invertMinutes);
  display.setCursor(99,yPixels);
  display.setTextColor(!invertMinutes);
  display.print(strMinutes);
  display.display();
}

void flashSave()
{
  uart2.println("Save to flash");
  EEPROM.put(0,onHours);
  EEPROM.put(1,onMinutes);
  EEPROM.put(2,offHours);
  EEPROM.put(3,offMinutes);
  EEPROM.commit();
  saveToFlash=false;
}

void lightSwitch()
{
  uint16_t onTimeMinutes=onMinutes+(60*onHours);
  uint16_t offTimeMinutes=offMinutes+(60*offHours);
  uint16_t nowMinutes=rtc.getMinutes()+(60*rtc.getHours());
  if(timeWithinInterval(onTimeMinutes, offTimeMinutes, nowMinutes)&&!lightIsOn)
  {
    uart2.println("Turning on lights.");
    digitalWrite(PIN_OUTPUT_EN, HIGH);
    display.setCursor(36,1);
    display.setTextColor(SSD1306_WHITE);
    display.print("*");
    display.display();
    lightIsOn=true;
  }
  else if(!timeWithinInterval(onTimeMinutes, offTimeMinutes, nowMinutes)&&lightIsOn)
  {
    uart2.println("Turning off lights.");
    digitalWrite(PIN_OUTPUT_EN, LOW);
    display.fillRect(35,0,12,16,SSD1306_BLACK);
    display.display();
    lightIsOn=false;
  }

  
}

bool timeWithinInterval(uint16_t start, uint16_t end, uint16_t now) 
{
  uint16_t duration=(end-start+1440)%1440;
  uint16_t time_on=(now-start+1440)%1440;
  return (time_on<duration);
}