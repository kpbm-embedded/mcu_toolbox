#include "board.h"
#include "hardware/i2c.h"
#include <stdio.h>
#define SDA 4
#define SCL 5
int main(void) {
    stdio_init_all();
    i2c_init(i2c0, 100 * 1000);
    gpio_set_function(SDA, GPIO_FUNC_I2C); gpio_set_function(SCL, GPIO_FUNC_I2C);
    gpio_pull_up(SDA); gpio_pull_up(SCL);
    while (true) {
        printf("scanning I2C0 (SDA=GP%d SCL=GP%d)\n", SDA, SCL);
        int found = 0;
        for (int a = 0x08; a < 0x78; a++) {
            uint8_t b;
            if (i2c_read_blocking_until(i2c0, a, &b, 1, false, make_timeout_time_ms(5)) >= 0) {
                printf("  found 0x%02x\n", a); found++;
            }
        }
        printf("%d device(s)\n", found);
        sleep_ms(3000);
    }
}
