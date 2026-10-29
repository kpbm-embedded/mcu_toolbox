#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "devices.h"
#include "fpioa.h"
#include "board.h"

static QueueHandle_t q;
static SemaphoreHandle_t mtx;
static handle_t gpio;
static unsigned total;

static void producer(void *arg)
{
    (void)arg;
    for (unsigned i = 0;; i++) {
        xQueueSend(q, &i, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

static void consumer(void *arg)
{
    (void)arg;
    unsigned v;
    for (;;) {
        if (xQueueReceive(q, &v, portMAX_DELAY) == pdTRUE) {
            xSemaphoreTake(mtx, portMAX_DELAY);
            total += v;
            xSemaphoreGive(mtx);
            gpio_set_pin_value(gpio, 0, (v & 1) ? GPIO_PV_LOW : GPIO_PV_HIGH);
            printf("consumer: got %u, total %u\n", v, total);
        }
    }
}

int main(void)
{
    fpioa_set_function(PIN_LED_G, FUNC_GPIOHS0);
    gpio = io_open("/dev/gpio0");
    gpio_set_drive_mode(gpio, 0, GPIO_DM_OUTPUT);
    q = xQueueCreate(8, sizeof(unsigned));
    mtx = xSemaphoreCreateMutex();
    xTaskCreate(producer, "prod", 2048, NULL, 2, NULL);
    xTaskCreate(consumer, "cons", 2048, NULL, 3, NULL);
    printf("rtos-tasks started\n");
    vTaskDelete(NULL);   /* main task is done; the scheduler keeps running the others */
    return 0;
}
