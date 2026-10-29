/* {{NAME}}: FreeRTOS (SDK-shipped) skeleton - a blink task and a main task logging on the console UART. Edit here. */
#include <FreeRTOS.h>
#include "task.h"
#include "bflb_gpio.h"
#include "board.h"

/* {{BOARD_NOTE}} -- LED pin NOT verified against the board schematic; change if nothing blinks. */
#define LED_PIN GPIO_PIN_{{LED_PIN}}

static struct bflb_device_s *gpio;

static void blink_task(void *arg)
{
    (void)arg;
    for (unsigned n = 0;; n++) {
        if (n & 1) {
            bflb_gpio_set(gpio, LED_PIN);
        } else {
            bflb_gpio_reset(gpio, LED_PIN);
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

static void main_task(void *arg)
{
    (void)arg;
    for (unsigned n = 0;; n++) {
        printf("{{NAME}} tick %u\r\n", n);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

int main(void)
{
    board_init();
    gpio = bflb_device_get_by_name("gpio");
    bflb_gpio_init(gpio, LED_PIN, GPIO_OUTPUT | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_0);

    xTaskCreate(blink_task, "blink", 256, NULL, 1, NULL);
    xTaskCreate(main_task, "main", 512, NULL, 2, NULL);
    vTaskStartScheduler();
    while (1) {
    }
}
