#include "pico/stdlib.h"
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "config.h"

static void blink_task(void *arg) {
    (void)arg;
    gpio_init(APP_LED_PIN); gpio_set_dir(APP_LED_PIN, GPIO_OUT);
    for (;;) { gpio_xor_mask(1u << APP_LED_PIN); vTaskDelay(pdMS_TO_TICKS(500)); }
}

static void main_task(void *arg) {
    (void)arg;
    for (unsigned n = 0;; n++) {
        printf("{{NAME}}: tick %u (core %u)\n", n, get_core_num());
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void vApplicationStackOverflowHook(TaskHandle_t t, char *name) { (void)t; panic("stack overflow in %s", name); }
void vApplicationMallocFailedHook(void) { panic("FreeRTOS malloc failed"); }

int main(void) {
    stdio_init_all();
    xTaskCreate(blink_task, "blink", 256, NULL, 1, NULL);
    xTaskCreate(main_task, "main", 1024, NULL, 2, NULL);
    vTaskStartScheduler();
    for (;;) {}
}
