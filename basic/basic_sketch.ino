#include <DigiCDC.h>

void setup() {
  pinMode(0, OUTPUT); // LED1 
  SerialUSB.begin(); 
}

// the loop routine runs over and over again forever:
void loop() {
  
  if (SerialUSB.available()) {
    char cmd;
    cmd = SerialUSB.read();
    if (cmd == '1') digitalWrite(0, HIGH); 
    if (cmd == '0') digitalWrite(0, LOW);
  }
  
   SerialUSB.delay(10);
   /*
   if you don't call a SerialUSB function (write, print, read, available, etc) 
   every 10ms or less then you must throw in some SerialUSB.refresh(); 
   for the USB to keep alive - also replace your delays - ie. delay(100); 
   with SerialUSB.delays ie. SerialUSB.delay(100);
   */
}