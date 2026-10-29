/* {{NAME}} on {{TARGET}}: task "led" blinks {{LED_PIN}} every 500 ms, task "log" prints once a second (USART1 PA9, 115200).
 * FreeRTOS heap_4 with a {{HEAP_KB}} KiB heap; config in mcu/freertos/common/FreeRTOSConfig.h. */
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "rtos_glue.h"

static void led_task(void *arg)
{
  (void)arg;
  for (;;) { board_led_toggle(); vTaskDelay(pdMS_TO_TICKS(500)); }
}
static void log_task(void *arg)
{
  (void)arg;
  for (unsigned n = 0;; n++) {
    rtos_log("hello #%u, free heap %u B", n, (unsigned)xPortGetFreeHeapSize());
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

int main(void)
{
  board_init();
  rtos_log_init();
  xTaskCreate(led_task, "led", 128, NULL, 1, NULL);
  xTaskCreate(log_task, "log", 384, NULL, 1, NULL);
  vTaskStartScheduler();          /* never returns unless the heap is too small for the idle/timer tasks */
  for (;;) {}
}
