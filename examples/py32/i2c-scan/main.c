#include "board.h"
#include "py32f0xx_ll_i2c.h"

/* Probe every 7-bit address: START, address+W, then ADDR (ACK) or AF (NACK), STOP. PY32F0 I2C is the "F1-style"
 * peripheral (SR1/SR2 flags), so this follows LL/I2C/AT24C32 of the Puya examples. */
#define TIMEOUT 20000u

static int wait_flag(uint32_t (*is)(I2C_TypeDef *))
{
    for (uint32_t n = 0; n < TIMEOUT; n++) if (is(I2C1)) return 1;
    return 0;
}

static void i2c_init(void)
{
    LL_IOP_GRP1_EnableClock(I2C_CLK);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);
    LL_GPIO_InitTypeDef gi = {0};
    gi.Pin = I2C_SDA | I2C_SCL; gi.Mode = LL_GPIO_MODE_ALTERNATE; gi.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    gi.OutputType = LL_GPIO_OUTPUT_OPENDRAIN; gi.Pull = LL_GPIO_PULL_UP; gi.Alternate = I2C_AF;
    LL_GPIO_Init(I2C_PORT, &gi);
    LL_APB1_GRP1_ForceReset(LL_APB1_GRP1_PERIPH_I2C1);
    LL_APB1_GRP1_ReleaseReset(LL_APB1_GRP1_PERIPH_I2C1);
    LL_I2C_InitTypeDef ii = {0};
    ii.ClockSpeed = 100000; ii.DutyCycle = LL_I2C_DUTYCYCLE_2;
    ii.OwnAddress1 = 0; ii.TypeAcknowledge = LL_I2C_NACK;
    LL_I2C_Init(I2C1, &ii);
    LL_I2C_Enable(I2C1);
}

static int probe(uint8_t addr)
{
    int ack = 0;
    LL_I2C_GenerateStartCondition(I2C1);
    if (wait_flag(LL_I2C_IsActiveFlag_SB)) {
        LL_I2C_TransmitData8(I2C1, (uint8_t)(addr << 1));
        for (uint32_t n = 0; n < TIMEOUT; n++) {
            if (LL_I2C_IsActiveFlag_ADDR(I2C1)) { LL_I2C_ClearFlag_ADDR(I2C1); ack = 1; break; }
            if (LL_I2C_IsActiveFlag_AF(I2C1)) { LL_I2C_ClearFlag_AF(I2C1); break; }
        }
    }
    LL_I2C_GenerateStopCondition(I2C1);
    for (uint32_t n = 0; n < TIMEOUT && LL_I2C_IsActiveFlag_BUSY(I2C1); n++) {}
    return ack;
}

int main(void)
{
    BSP_RCC_HSI_24MConfig();
    BSP_USART_Config(115200);
    i2c_init();
    for (;;) {
        printf("i2c-scan: I2C1 SDA/SCL, 100 kHz\r\n");
        unsigned found = 0;
        for (uint8_t a = 0x08; a < 0x78; a++)
            if (probe(a)) { printf("  found 0x%02x\r\n", a); found++; }
        printf("%u device(s)\r\n", found);
        LL_mDelay(3000);
    }
}
