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

## Vendor specific USB device

Before we used a special device descriptor that is known by the system (serial communication). The kernel already knows how to deal with it and is creating tty to talk with the device. 

For vendor specific we will need to write our own driver to specify how device and host discuss together.

### Step 1 firmware

The fist step is to be able to make our device identified as a vendor specific. We will modify DigiUSB (built on top of V-USB) to do this. 


Update the `/home/yann/.arduino15/packages/digistump/hardware/avr/1.7.5/libraries/DigisparkUSB/usbconfig.h` to declare the device as vendor specific and not HID anymore : 
```
#define USB_CFG_DEVICE_CLASS        0xFF
#define USB_CFG_INTERFACE_CLASS     0xFF
#define USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH  0
```

Now in `DigiUSB.cpp` look for `usbFunctionSetup`. This is called for every control request sent by the host (on endpoint 0).

Actual function look like this: 
```
usbMsgLen_t usbFunctionSetup(uchar data[8])
{
  usbRequest_t    *rq = (usbRequest_t*)((void *)data);

    if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS){    /* HID class request */
        if(rq->bRequest == USBRQ_HID_GET_REPORT){  /* wValue: ReportType (highbyte), ReportID (lowbyte) */
            /* since we have only one report type, we can ignore the report-ID */
	    static uchar dataBuffer[1];  /* buffer must stay valid when usbFunctionSetup returns */
	    if (tx_available()) {
	      dataBuffer[0] = tx_read();
	      usbMsgPtr = dataBuffer; /* tell the driver which data to return */
	      return 1; /* tell the driver to send 1 byte */
	    } else {
	      // Drop through to return 0 (which will stall the request?)
	    }
        }else if(rq->bRequest == USBRQ_HID_SET_REPORT){
            /* since we have only one report type, we can ignore the report-ID */

	  // TODO: Check race issues?
	  store_char(rq->wIndex.bytes[0], &rx_buffer);

        }
    }else{
        /* ignore vendor type requests, we don't use any */
    }
    return 0;
}
```

We can intercept the ignored else and implement our protocol as is: 
```
    } else if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_VENDOR) {
        if (rq->bRequest == USB_LED_COMMAND) {
            if (rq->wValue.word == 1) {
                digitalWrite(0, HIGH);
            } else {
                digitalWrite(0, LOW);
            }
        }
    } else {
        /* ignore other type of requests */
    }
```

And add `#define USB_LED_COMMAND  0x01` in the header of this cpp file.

### Step 2 user space driver

Then we will talk to the device using libusb in user space to avoid crashing the system.

