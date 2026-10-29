/* Scan I2C0 (7-bit addresses 0x08..0x77) by trying a 1-byte read; a device that ACKs is printed.
 * Pins from the SDK board files (board_i2c0_gpio_init): bl616 SDA=GPIO11 SCL=GPIO14 (bsp/board/bl616dk/board_gpio.c),
 * bl702 SDA=GPIO11 SCL=GPIO16 (bsp/board/bl702dk/board.c), bl808 SDA=GPIO11 SCL=GPIO16 (legacy bsp/board/bl808dk/board.c). Use 4.7k pull-ups if the module has none. */
#include "bflb_i2c.h"
#include "bflb_mtimer.h"
#include "board.h"

int main(void)
{
    board_init();
    board_i2c0_gpio_init();

    struct bflb_device_s *i2c0 = bflb_device_get_by_name("i2c0");
    bflb_i2c_init(i2c0, 100000);

    while (1) {
        int found = 0;
        printf("i2c scan:\r\n");
        for (uint16_t addr = 0x08; addr <= 0x77; addr++) {
            uint8_t b = 0;
            struct bflb_i2c_msg_s msg = { .addr = addr, .flags = I2C_M_READ, .buffer = &b, .length = 1 };
            if (bflb_i2c_transfer(i2c0, &msg, 1) == 0) {
                printf("  found 0x%02x\r\n", addr);
                found++;
            }
        }
        printf("%d device(s)\r\n", found);
        bflb_mtimer_delay_ms(3000);
    }
}
