#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* Three FreeRTOS tasks (kernel from py32f0-template Libraries/FreeRTOS, config from FreeRTOS/Tasks/LL_Blink, 1.5 KB heap):
 * blink toggles the LED, producer sends a counter through a queue, consumer prints it; a mutex guards printf. */
static QueueHandle_t q;
static SemaphoreHandle_t print_lock;

static void blink(void *arg)
{
    (void)arg;
    for (;;) {
#if defined(PY32F071x8)
        HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
#else
        LL_GPIO_TogglePin(LED_PORT, LED_PIN);
#endif
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}

static void producer(void *arg)
{
    (void)arg;
    for (uint32_t n = 0;; n++) { xQueueSend(q, &n, portMAX_DELAY); vTaskDelay(pdMS_TO_TICKS(1000)); }
}

static void consumer(void *arg)
{
    (void)arg;
    uint32_t n;
    for (;;) {
        if (xQueueReceive(q, &n, portMAX_DELAY) == pdTRUE) {
            xSemaphoreTake(print_lock, portMAX_DELAY);
            printf("consumer got %lu, free heap %u\r\n", (unsigned long)n, (unsigned)xPortGetFreeHeapSize());
            xSemaphoreGive(print_lock);
        }
    }
}

int main(void)
{
#if defined(PY32F071x8)
    HAL_Init();                               /* HAL driver (F07x); SysTick is taken over by the FreeRTOS port */
    BSP_USART_Config();
    LED_CLK_ENABLE();
    GPIO_InitTypeDef gi = { .Pin = LED_PIN, .Mode = GPIO_MODE_OUTPUT_PP, .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_LOW };
    HAL_GPIO_Init(LED_PORT, &gi);
#else
    BSP_RCC_HSI_24MConfig();
    BSP_USART_Config(115200);
    LL_IOP_GRP1_EnableClock(LED_CLK);
    LL_GPIO_SetPinMode(LED_PORT, LED_PIN, LL_GPIO_MODE_OUTPUT);
#endif
    printf("rtos-tasks: FreeRTOS %s\r\n", tskKERNEL_VERSION_NUMBER);
    q = xQueueCreate(4, sizeof(uint32_t));
    print_lock = xSemaphoreCreateMutex();
    if (!q || !print_lock ||
        xTaskCreate(blink, "blink", configMINIMAL_STACK_SIZE, NULL, 1, NULL) != pdPASS ||
        xTaskCreate(producer, "prod", configMINIMAL_STACK_SIZE, NULL, 1, NULL) != pdPASS ||
        xTaskCreate(consumer, "cons", configMINIMAL_STACK_SIZE + 64, NULL, 1, NULL) != pdPASS) {
        printf("out of heap\r\n");
        for (;;) {}
    }
    vTaskStartScheduler();
    for (;;) {}
}
