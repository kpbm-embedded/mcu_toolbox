#ifndef BOARD_H
#define BOARD_H
#if defined(PY32F071x8)
/* PY32F071: the template has only the HAL driver for PY32F07x. LED PC13 = Puya example HAL/GPIO/GPIO_Toggle (no board
 * for the owned bare chips: unverified), debug UART = USART2 PA2 TX / PA3 RX AF1 (PY32F07x_HAL_BSP py32f07x_bsp_printf.h). */
#include "py32f07x_hal.h"
#include "py32f07x_bsp_clock.h"
#include "py32f07x_bsp_printf.h"
#ifndef LED_PORT
#define LED_PORT GPIOC
#define LED_PIN  GPIO_PIN_13
#define LED_CLK_ENABLE() __HAL_RCC_GPIOC_CLK_ENABLE()
#endif
#else
#include "py32f0xx_ll_rcc.h"
#include "py32f0xx_ll_bus.h"
#include "py32f0xx_ll_system.h"
#include "py32f0xx_ll_exti.h"
#include "py32f0xx_ll_cortex.h"
#include "py32f0xx_ll_utils.h"
#include "py32f0xx_ll_pwr.h"
#include "py32f0xx_ll_gpio.h"
#include "py32f0xx_ll_tim.h"
#include "py32f0xx_ll_adc.h"
#include "py32f0xx_ll_usart.h"
#include "py32f0xx_bsp_clock.h"
#include "py32f0xx_bsp_printf.h"

/* Pins follow the Puya LL examples (Examples/PY32F0xx/LL in IOsetting/py32f0-template): LED PB5, key PA12 (EXTI_Toggle),
 * USART1 PA2/PA3 (BSP printf), ADC channel 4 on PA4 (ADC_SingleConversion_TriggerTimer), TIM1 PWM: PA8 AF2 = CH1 on F003
 * (TIM1_PWM) and F030 (PY32F030 datasheet table 3.1, PA8 AF2 = TIM1_CH1), PA0 AF13 = CH3 on F002A
 * (PY32F002AF15P_TIM1_PWM), I2C PF0 SDA / PF1 SCL AF12 (LL/I2C/AT24C32; PY32F030 datasheet table 3.3).
 * Your PY32F003/F002A dev board and the bare F030 chips are NOT confirmed to wire them that way (no schematic for the
 * owned boards): edit these defines. */
#ifndef LED_PORT
#define LED_PORT GPIOB
#define LED_PIN  LL_GPIO_PIN_5
#define LED_CLK  LL_IOP_GRP1_PERIPH_GPIOB
#endif
#ifndef KEY_PORT
#define KEY_PORT GPIOA
#define KEY_PIN  LL_GPIO_PIN_12
#define KEY_LINE LL_EXTI_LINE_12
#define KEY_CLK  LL_IOP_GRP1_PERIPH_GPIOA
#endif
#ifndef I2C_PORT
#define I2C_PORT GPIOF
#define I2C_SDA  LL_GPIO_PIN_0
#define I2C_SCL  LL_GPIO_PIN_1
#define I2C_AF   LL_GPIO_AF_12
#define I2C_CLK  LL_IOP_GRP1_PERIPH_GPIOF
#endif
#if defined(PY32F003x8) || defined(PY32F030x8)
#define PWM_PORT GPIOA
#define PWM_PIN  LL_GPIO_PIN_8
#define PWM_AF   LL_GPIO_AF_2
#define PWM_SET(v) LL_TIM_OC_SetCompareCH1(TIM1, (v))
#define PWM_CH   LL_TIM_CHANNEL_CH1
#elif defined(PY32F002Ax5)
#define PWM_PORT GPIOA
#define PWM_PIN  LL_GPIO_PIN_0
#define PWM_AF   LL_GPIO_AF_13
#define PWM_SET(v) LL_TIM_OC_SetCompareCH3(TIM1, (v))
#define PWM_CH   LL_TIM_CHANNEL_CH3
#else
#error "unknown device"
#endif
#endif
#endif
