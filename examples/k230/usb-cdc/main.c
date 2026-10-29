/* USB CDC-ACM device echo. The RT-Smart kernel's CherryUSB gadget (canmv_usb/usbd_cdc.c, enabled in both board defconfigs as
 * "CDC + MTP") registers the serial device "ttyGS0"; user space opens it as /dev/ttyGS0 like any tty. Lines typed in the host's
 * terminal (/dev/ttyACM0 on Linux) come back upper-cased. */
#include <ctype.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "board.h"

#ifndef CDC_DEV
#define CDC_DEV "/dev/ttyGS0"
#endif

int main(void)
{
    int fd = open(CDC_DEV, O_RDWR | O_NONBLOCK);
    if (fd < 0) { perror("open " CDC_DEV); return 1; }
    printf("usb-cdc (%s): echoing on %s for 60 s\n", BOARD_NAME, CDC_DEV);
    const char hello[] = "k230 usb-cdc echo: type something\r\n";
    (void)write(fd, hello, sizeof hello - 1);

    char buf[128];
    for (int t = 0; t < 60; t++) {
        struct pollfd p = { .fd = fd, .events = POLLIN };
        if (poll(&p, 1, 1000) <= 0 || !(p.revents & POLLIN)) continue;
        ssize_t n = read(fd, buf, sizeof buf);
        if (n <= 0) continue;
        for (ssize_t i = 0; i < n; i++) buf[i] = (char)toupper((unsigned char)buf[i]);
        (void)write(fd, buf, (size_t)n);
        printf("echoed %d byte(s)\n", (int)n);
    }
    close(fd);
    return 0;
}
