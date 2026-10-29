/* {{NAME}}: STM32 LL drivers without HAL. USART1 is configured again with LL_USART_Init (115200 8N1 from the reset
 * clock) and PA5 becomes an output with LL_GPIO_Init, set and reset; the ODR read-back is checked. */
#include "bare.h"
#include LL_FAMILY_H
#if defined(STM32F103xB)
#include "stm32f1xx_ll_bus.h"
#include "stm32f1xx_ll_usart.h"
#elif defined(STM32H743xx)
#include "stm32h7xx_ll_bus.h"
#include "stm32h7xx_ll_usart.h"
#else
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_usart.h"
#endif

int main(void)
{
#if defined(STM32F103xB)
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
#elif defined(STM32H743xx)
  LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOA);
#else
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
#endif
  LL_GPIO_InitTypeDef g;
  LL_GPIO_StructInit(&g);
  g.Pin = LL_GPIO_PIN_5;
  g.Mode = LL_GPIO_MODE_OUTPUT;
  int gpio_ok = LL_GPIO_Init(GPIOA, &g) == SUCCESS;
  LL_USART_InitTypeDef u;
  LL_USART_StructInit(&u);
  u.BaudRate = 115200;
  LL_USART_Disable(USART1);
  int usart_ok = LL_USART_Init(USART1, &u) == SUCCESS;   /* baud from SystemCoreClock (system_stm32xxxx.c, reset clock) */
  LL_USART_Enable(USART1);
  bare_printf("{{NAME}}: LL GPIO %s, LL USART %s\n", gpio_ok ? "ok" : "error", usart_ok ? "ok" : "error");
  LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_5);
  int high = LL_GPIO_IsOutputPinSet(GPIOA, LL_GPIO_PIN_5);
  LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_5);
  test_check(gpio_ok && high && !LL_GPIO_IsOutputPinSet(GPIOA, LL_GPIO_PIN_5), "PA5 set/reset via LL (ODR read-back)");
  test_check(LL_USART_IsEnabled(USART1), "USART1 enabled via LL and still printing");
  test_done();
}
