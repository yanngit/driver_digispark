# Playing with digispark to learn driver coding

Need digispark with LED to play along. By default a LED is already connected to the PIN 0 so 
you can play along with the board only. At least for basic scenario.

You can see digispark with `lsusb command` it shows up with 
```
Bus 001 Device 033: ID 16d0:0753 MCS Digistump DigiSpark
```

The Digispark uses V-USB, a software implementation of USB on ATtiny85.
The Digispark bootloader only keeps the USB interface active for a few seconds after reset for uploading new firmware.
After that, the firmware takes over, and if the firmware doesn’t keep the USB connection alive, the device may disappear from lsusb.

It's your firmware that should handle the USB connection up if you want to communicate with it after the 5 seconds boot (by using `DigiUSB.delay(10);` for example)

By doing so it will appear as 
```
Bus 001 Device 034: ID 16c0:05df Van Ooijen Technische Informatica HID device except mice, keyboards, and joysticks
```

## Base setup

Install Arduino IDE and add the board manager URL : 
```
https://raw.githubusercontent.com/ArminJo/DigistumpArduino/master/package_digistump_index.json
```

Install `DigiStump AVR Board` and select digispark board and micronucleus as programmer.

Open the sketch in arduino IDE and upload it to the board.

:warning: Could have issue to upload to the board, do this if not done previously: 
```
wget https://raw.githubusercontent.com/micronucleus/micronucleus/master/commandline/49-micronucleus.rules
sudo mv 49-micronucleus.rules /etc/udev/rules.d/
sudo chmod 644 /etc/udev/rules.d/49-micronucleus.rules
sudo udevadm control --reload-rules
sudo udevadm trigger
```
This will allow any user to open the /dev/hidraw0 device.

For C development install 
```
sudo apt update
sudo apt install build-essential libusb-1.0-0-dev
```

## Basic scenario

This basic sketch will uplaod in the board a programm responding to command. Command 1 is powering ON the LED 
and the command 0 is powering it OFF. The LED that will be powered ON is PIN 0.

Since digispark does not have builtin OUT endpoint if used with DigiUSB (for HID USB emulation) we will add in 
the firmware a serial emulator to be able to communicate in both ways via DigiCDC. The device will appear as : 
```
Bus 001 Device 045: ID 16d0:087e MCS Digispark Serial
```

We can now toggle the LED with basic tty manipulation: 
```
echo "1" | sudo tee /dev/ttyACM0
echo "0" | sudo tee /dev/ttyACM0
```

### Compile and run it 

```
cc -o basic basic.c -lusb-1.0
sudo ./digispark_led
```

