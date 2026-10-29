/* FreeRTOS (shipped in the SDK, components/os/freertos): a blink task, a producer and a consumer linked by a queue,
 * and a mutex guarding printf on the console UART. SDK 2.3 for bl616/bl702, legacy SDK 2.2.0 for bl808 (M0 core). */
#include <FreeRTOS.h>
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "bflb_gpio.h"
#include "board.h"
#include "board_pins.h"

static QueueHandle_t q;
static SemaphoreHandle_t print_lock;
static struct bflb_device_s *gpio;

#define LOG(...) do { xSemaphoreTake(print_lock, portMAX_DELAY); printf(__VA_ARGS__); \
                      xSemaphoreGive(print_lock); } while (0)

static void blink_task(void *arg)
{
    (void)arg;
    for (unsigned n = 0;; n++) {
        if (n & 1) {
            bflb_gpio_set(gpio, LED_PIN);
        } else {
            bflb_gpio_reset(gpio, LED_PIN);
        }
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}

static void producer_task(void *arg)
{
    (void)arg;
    for (uint32_t n = 0;; n++) {
        xQueueSend(q, &n, portMAX_DELAY);
        LOG("producer: sent %lu\r\n", (unsigned long)n);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void consumer_task(void *arg)
{
    (void)arg;
    uint32_t n;
    for (;;) {
        if (xQueueReceive(q, &n, portMAX_DELAY) == pdTRUE) {
            LOG("consumer: got %lu, stack headroom %u words\r\n", (unsigned long)n, (unsigned)uxTaskGetStackHighWaterMark(NULL));
        }
    }
}

void vApplicationStackOverflowHook(TaskHandle_t t, char *name)
{
    (void)t;
    printf("stack overflow in %s\r\n", name);
    while (1) {
    }
}

void vApplicationMallocFailedHook(void)
{
    printf("FreeRTOS malloc failed\r\n");
    while (1) {
    }
}

int main(void)
{
    board_init();
    gpio = bflb_device_get_by_name("gpio");
    bflb_gpio_init(gpio, LED_PIN, GPIO_OUTPUT | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_0);

    printf("rtos-tasks: FreeRTOS %s\r\n", tskKERNEL_VERSION_NUMBER);
    q = xQueueCreate(4, sizeof(uint32_t));
    print_lock = xSemaphoreCreateMutex();
    xTaskCreate(blink_task, "blink", 256, NULL, 1, NULL);
    xTaskCreate(producer_task, "prod", 512, NULL, 2, NULL);
    xTaskCreate(consumer_task, "cons", 512, NULL, 2, NULL);
    vTaskStartScheduler();
    while (1) {
    }
}
