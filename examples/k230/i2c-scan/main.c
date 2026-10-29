/* Scan a hardware I2C bus at 100 kHz: probe 0x08..0x77 with a 1-byte read (the DesignWare controller cannot send an address-only write) and list the addresses that ACK (drv_i2c). */
#include <stdio.h>
#include "drv_fpioa.h"
#include "drv_i2c.h"
#include "board.h"

int main(void)
{
    drv_i2c_inst_t *bus = NULL;
    drv_fpioa_set_pin_func(I2C_SCL_GPIO, IIC0_SCL + 2 * I2C_ID);   /* IICn_SCL / IICn_SDA are consecutive per bus */
    drv_fpioa_set_pin_func(I2C_SDA_GPIO, IIC0_SDA + 2 * I2C_ID);
    if (drv_i2c_inst_create(I2C_ID, 100000, 20, 0xff, 0xff, &bus) < 0) { printf("i2c%d open failed\n", I2C_ID); return 1; }

    printf("i2c-scan (%s): I2C%d SCL IO%d SDA IO%d\n", BOARD_NAME, I2C_ID, I2C_SCL_GPIO, I2C_SDA_GPIO);
    int found = 0;
    for (uint16_t a = 0x08; a <= 0x77; a++) {
        uint8_t dummy = 0;
        i2c_msg_t msg = { .addr = a, .flags = DRV_I2C_RD, .len = 1, .buf = &dummy };
        if (drv_i2c_transfer(bus, &msg, 1) == 0)   /* ioctl returns 0 (RT_EOK) when the address ACKed */ { printf("  found 0x%02x\n", a); found++; }
    }
    printf("%d device(s)\n", found);
    drv_i2c_inst_destroy(&bus);
    return 0;
}
