#include <Arduino.h>
#include <FlashStorage_STM32.h> 

/******************************************************************************************************************************************
  FlashStoreAndRetrieve.ino
  For STM32 using Flash emulated-EEPROM

  The FlashStorage_STM32 library aims to provide a convenient way to store and retrieve user's data using the non-volatile flash memory
  of STM32F/L/H/G/WB/MP1. It's using the buffered read and write to minimize the access to Flash.
  It now supports writing and reading the whole object, not just byte-and-byte.

  Inspired by Cristian Maglie's FlashStorage (https://github.com/cmaglie/FlashStorage)

  Built by Khoi Hoang https://github.com/khoih-prog/FlashStorage_STM32
  Licensed under MIT license
 ******************************************************************************************************************************************/

// To be included only in main(), .ino with setup() to avoid `Multiple Definitions` Linker Error

HardwareSerial uart2(PA3, PA2);

void setup()
{
  uart2.begin(115200);
  while (!uart2);

  delay(200);

  uart2.print(F("\nStart FlashStoreAndRetrieve on ")); uart2.println(BOARD_NAME);
  uart2.println(FLASH_STORAGE_STM32_VERSION);

  uart2.print("EEPROM length: ");
  uart2.println(EEPROM.length());

  uint16_t address = 0;
  int number;

  // Read the content of emulated-EEPROM
  EEPROM.get(address, number);

  // Print the current number on the uart2 monitor
  uart2.print("Number = 0x"); uart2.println(number, HEX);

  // Save into emulated-EEPROM the number increased by 1 for the next run of the sketch
  EEPROM.put(address, (int) (number + 1));
  EEPROM.commit();

  uart2.println("Done writing to emulated EEPROM. You can reset now");
}

void loop()
{
  // Do nothing...
}