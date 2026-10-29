#include <stdio.h>
#include <stdint.h>
#include "fpioa.h"
#include "i2c.h"
#include "sleep.h"
#include "board.h"

int main(void)
{
    fpioa_set_function(PIN_I2C_SCL, FUNC_I2C0_SCLK);
    fpioa_set_function(PIN_I2C_SDA, FUNC_I2C0_SDA);
    printf("i2c-scan on I2C0 (SCL IO%d, SDA IO%d)\n", PIN_I2C_SCL, PIN_I2C_SDA);
    for (;;) {
        int found = 0;
        for (uint32_t addr = 0x08; addr <= 0x77; addr++) {
            i2c_init(I2C_DEVICE_0, addr, 7, 100000);
            uint8_t zero = 0;
            if (i2c_send_data(I2C_DEVICE_0, &zero, 1) == 0) {   /* returns 1 on NACK/abort */
                printf("  found 0x%02x\n", addr);
                found++;
            }
        }
        printf("scan done, %d device(s)\n", found);
        msleep(3000);
    }
}
