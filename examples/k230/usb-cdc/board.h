#ifndef BOARD_H
#define BOARD_H
/* Per-board pins for the owned K230 boards. Pins are K230 IO numbers (FPIOA pin == GPIO index).
 * The board is taken from the SDK's generated/autoconf.h (force-included by the Makefile after `make <target>_defconfig`).
 * Sources (k230_rtos_sdk, read only):
 *   01studio: src/uboot/uboot/arch/riscv/dts/k230_canmv_01studio_iomux.dtsi (comments "LED_GPIO52", "KEY_GPIO21", "iic0" on IO48/49)
 *             src/rtsmart/rtsmart/kernel/bsp/maix3/board/configs/k230_canmv_01studio/pinmux_config.c (IO48/49 = IIC0_SCL/SDA)
 *             src/rtsmart/examples/peripheral/pwm/test_pwm.c (PWM0 on IO42), .../uart (UART2 on IO11/12, "GPIO11/12" in the dtsi)
 *   bpi zero: src/uboot/uboot/arch/riscv/dts/k230d_canmv_bpi_zero.dts ("SW3 KEY" over IO0/IO1, "jp1-pin15" IO2,
 *             "jp1-pin3,5 iic2" IO11/12, "JP1-PIN33 pwm" IO60, "UART2" IO44/45)
 *             src/rtsmart/rtsmart/kernel/bsp/maix3/board/configs/k230d_canmv_bpi_zero/pinmux_config.c (IIC2 11/12, PWM0 60, UART2 44/45)
 * UNVERIFIED: LED/key active levels (no schematic in the SDK); the BPI-K230D-Zero has no LED in its dts, so LED_GPIO is the
 * header pin JP1-15 (IO2) for an external LED; SW3 is listed over IO0 and IO1 and IO0 (the one muxed as GPIO) is assumed.
 * Every value is a plain define: override with make EXTRA_CFLAGS="-DLED_GPIO=.. -DLED_ACTIVE_LOW=1". */
#if defined(CONFIG_BOARD_K230_CANMV_01STUDIO)
#  define BOARD_NAME "k230_canmv_01studio"
#  define BOARD_LED_GPIO     52
#  define BOARD_KEY_GPIO     21
#  define BOARD_I2C_ID       0
#  define BOARD_I2C_SCL      48
#  define BOARD_I2C_SDA      49
#  define BOARD_PWM_CH       0
#  define BOARD_PWM_GPIO     42
#  define BOARD_UART_ID      2
#  define BOARD_UART_TX      11
#  define BOARD_UART_RX      12
#elif defined(CONFIG_BOARD_K230D_CANMV_BPI_ZERO)
#  define BOARD_NAME "k230d_canmv_bpi_zero"
#  define BOARD_LED_GPIO     2
#  define BOARD_KEY_GPIO     0
#  define BOARD_I2C_ID       2
#  define BOARD_I2C_SCL      11
#  define BOARD_I2C_SDA      12
#  define BOARD_PWM_CH       0
#  define BOARD_PWM_GPIO     60
#  define BOARD_UART_ID      2
#  define BOARD_UART_TX      44
#  define BOARD_UART_RX      45
#else
#  error "unknown board: select k230_rtos_01studio_defconfig or k230d_rtos_bpi_zero_defconfig in the SDK first"
#endif

#ifndef LED_GPIO
#define LED_GPIO BOARD_LED_GPIO
#endif
#ifndef LED_ACTIVE_LOW
#define LED_ACTIVE_LOW 0
#endif
#ifndef KEY_GPIO
#define KEY_GPIO BOARD_KEY_GPIO
#endif
#ifndef KEY_ACTIVE_LOW
#define KEY_ACTIVE_LOW 1          /* input with pull-up, key to GND (unverified) */
#endif
#ifndef I2C_ID
#define I2C_ID  BOARD_I2C_ID
#endif
#ifndef I2C_SCL_GPIO
#define I2C_SCL_GPIO BOARD_I2C_SCL
#endif
#ifndef I2C_SDA_GPIO
#define I2C_SDA_GPIO BOARD_I2C_SDA
#endif
#ifndef PWM_CH
#define PWM_CH  BOARD_PWM_CH
#endif
#ifndef PWM_GPIO
#define PWM_GPIO BOARD_PWM_GPIO
#endif
#ifndef UART_ID
#define UART_ID BOARD_UART_ID
#endif
#ifndef UART_TX_GPIO
#define UART_TX_GPIO BOARD_UART_TX
#endif
#ifndef UART_RX_GPIO
#define UART_RX_GPIO BOARD_UART_RX
#endif
#endif
