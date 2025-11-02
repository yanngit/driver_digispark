#include <libusb-1.0/libusb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define VENDOR_ID   0x16c0
#define PRODUCT_ID  0x05df
#define USB_LED_COMMAND  0x01  // must match firmware command

int send_led_command(libusb_device_handle *dev, int value) {
    int r = libusb_control_transfer(
        dev,
        LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT,
        USB_LED_COMMAND,   // bRequest
        value, 0,          // wValue (LED ON/OFF)
        NULL, 0,           // no data
        1000               // timeout ms
    );

    if (r < 0) {
        fprintf(stderr, "USB transfer error: %s\n", libusb_error_name(r));
        return -1;
    }
    return 0;
}

int main(void) {
    libusb_device_handle *dev;
    libusb_context *ctx = NULL;

    if (libusb_init(&ctx) < 0) {
        fprintf(stderr, "libusb init failed\n");
        return 1;
    }

    dev = libusb_open_device_with_vid_pid(ctx, VENDOR_ID, PRODUCT_ID);
    if (!dev) {
        fprintf(stderr, "Device not found (VID=0x%04x PID=0x%04x)\n", VENDOR_ID, PRODUCT_ID);
        libusb_exit(ctx);
        return 1;
    }

    printf("Connected to device.\n");

    while (1) {
        int ping_status = system("ping -c1 -W1 8.8.8.8 > /dev/null 2>&1");
        int led_value = (ping_status == 0) ? 1 : 0;

        printf("Ping %s, sending LED=%d\n", 
               (ping_status == 0 ? "OK" : "FAILED"), led_value);

        send_led_command(dev, led_value);

        sleep(2);
    }

    libusb_close(dev);
    libusb_exit(ctx);
    return 0;
}
