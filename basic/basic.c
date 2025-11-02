#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    const char *tty = "/dev/ttyACM0";  // Digispark CDC device
    const char *ip = "192.168.1.22";   // Server to ping
    int fd;

    // Open the serial device
    fd = open(tty, O_WRONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    for(;;) {
        // Ping the server once, suppress output
        int ret = system("ping -c 1 -W 1 192.168.1.22 > /dev/null 2>&1");

        // Send '1' if ping successful, '0' otherwise
        char cmd = (ret == 0) ? '1' : '0';
        if (write(fd, &cmd, 1) < 0) {
            perror("write");
        }
        sleep(5);
    }



    close(fd);
    return 0;
}
