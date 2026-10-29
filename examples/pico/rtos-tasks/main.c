/* FreeRTOS SMP on RP2040 / RP2350: a blink task, a producer and a consumer linked by a
 * queue, and a mutex guarding printf. Console: UART0 GP0/GP1 115200. */
#include "board.h"
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

static QueueHandle_t q;
static SemaphoreHandle_t print_lock;

#define LOG(...) do { xSemaphoreTake(print_lock, portMAX_DELAY); printf(__VA_ARGS__); \
                      xSemaphoreGive(print_lock); } while (0)

static void blink_task(void *arg) {
    (void)arg;
    gpio_init(LED_PIN); gpio_set_dir(LED_PIN, GPIO_OUT);
    for (;;) { gpio_xor_mask(1u << LED_PIN); vTaskDelay(pdMS_TO_TICKS(250)); }
}

static void producer_task(void *arg) {
    (void)arg;
    for (uint32_t n = 0;; n++) {
        xQueueSend(q, &n, portMAX_DELAY);
        LOG("producer: sent %lu (core %u)\n", (unsigned long)n, get_core_num());
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void consumer_task(void *arg) {
    (void)arg;
    uint32_t n;
    for (;;) {
        if (xQueueReceive(q, &n, portMAX_DELAY) == pdTRUE)
            LOG("consumer: got %lu (core %u), free heap %u\n", (unsigned long)n, get_core_num(),
                (unsigned)xPortGetFreeHeapSize());
    }
}

void vApplicationStackOverflowHook(TaskHandle_t t, char *name) { (void)t; panic("stack overflow in %s", name); }
void vApplicationMallocFailedHook(void) { panic("FreeRTOS malloc failed"); }

int main(void) {
    stdio_init_all();
    printf("rtos-tasks: FreeRTOS %s, %d core(s)\n", tskKERNEL_VERSION_NUMBER, configNUMBER_OF_CORES);
    q = xQueueCreate(4, sizeof(uint32_t));
    print_lock = xSemaphoreCreateMutex();
    xTaskCreate(blink_task, "blink", 256, NULL, 1, NULL);
    xTaskCreate(producer_task, "prod", 512, NULL, 2, NULL);
    xTaskCreate(consumer_task, "cons", 512, NULL, 2, NULL);
    vTaskStartScheduler();   /* never returns */
    for (;;) {}
}
