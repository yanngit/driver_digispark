#include <DigiVendorUSB.h>  // your modified DigiUSB

void setup() {
    pinMode(0, OUTPUT);  // LED pin
    DigiUSB.begin();     // Initialize USB
}

void loop() {
    DigiUSB.refresh();   // Keep USB alive
    // You don’t have to do anything here — all logic is in usbFunctionSetup()
}