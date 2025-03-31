#include <Arduino.h>
#include <Wire.h>

TwoWire Wire1(PB9,PB8);
HardwareSerial uart2(PA3, PA2);

void setup() {
  uart2.begin(115200);
  uart2.println("\nI2C Scanner");
  Wire1.begin();
  
  

  
}

void loop() {
  int nDevices = 0;
  uart2.println("Scanning I2C1");
  for (byte address = 1; address < 127; ++address) {
    Wire1.beginTransmission(address);
    byte error = Wire1.endTransmission();

    if (error == 0) {
      uart2.print("I2C device found at address 0x");
      if (address < 16) {
        uart2.print("0");
      }
      uart2.println(address, HEX);

      ++nDevices;
    } else if (error == 4) {
      uart2.print("Unknown error at address 0x");
      if (address < 16) {
        uart2.print("0");
      }
      uart2.println(address, HEX);
    }
  }
  if (nDevices == 0) {
    uart2.println("No I2C devices found\n");
  } else {
    uart2.println("done\n");
  }


  



  
  delay(5000); // Wait 5 seconds for next scan
}