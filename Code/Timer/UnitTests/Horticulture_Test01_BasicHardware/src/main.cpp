#include <Arduino.h>
#define PIN_OUTPUT_EN PA4

void setup() 
{
  pinMode(PIN_OUTPUT_EN, OUTPUT);
  digitalWrite(PIN_OUTPUT_EN, HIGH);
}

void loop() 
{
  

}

