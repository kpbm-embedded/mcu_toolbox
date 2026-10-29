/* {{NAME}} on {{TARGET}}: sensor task -> queue -> logger task (mutex-protected stats), a software timer feeding an event group,
 * and a supervisor waiting on event bits. Replace the "sensor" with your own source. */
#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "event_groups.h"
#include "rtos_glue.h"

#define EV_SAMPLE  (1U << 0)    /* set by the sensor task every sample */
#define EV_TICK1S  (1U << 1)    /* set by the 1 s software timer */

static QueueHandle_t q;
static SemaphoreHandle_t stats_mx;
static EventGroupHandle_t ev;
static struct { uint32_t samples, sum; } stats;      /* shared: guarded by stats_mx */

static void sensor_task(void *arg)
{
  (void)arg;
  uint32_t v = 0;
  TickType_t last = xTaskGetTickCount();
  for (;;) {
    v = (v * 1103515245U + 12345U) >> 16 & 0xff;           /* fake sample */
    xQueueSend(q, &v, 0);
    xEventGroupSetBits(ev, EV_SAMPLE);
    vTaskDelayUntil(&last, pdMS_TO_TICKS(100));
  }
}
static void logger_task(void *arg)
{
  (void)arg;
  uint32_t v;
  for (;;) {
    if (xQueueReceive(q, &v, portMAX_DELAY) == pdPASS) {
      xSemaphoreTake(stats_mx, portMAX_DELAY);
      stats.samples++; stats.sum += v;
      xSemaphoreGive(stats_mx);
    }
  }
}
static void timer_cb(TimerHandle_t t) { (void)t; xEventGroupSetBits(ev, EV_TICK1S); }
static void supervisor_task(void *arg)
{
  (void)arg;
  for (;;) {
    EventBits_t b = xEventGroupWaitBits(ev, EV_TICK1S, pdTRUE, pdFALSE, portMAX_DELAY);
    (void)b;
    xSemaphoreTake(stats_mx, portMAX_DELAY);
    uint32_t n = stats.samples, avg = n ? stats.sum / n : 0;
    xSemaphoreGive(stats_mx);
    rtos_log("samples=%lu avg=%lu heap=%u B", (unsigned long)n, (unsigned long)avg, (unsigned)xPortGetFreeHeapSize());
    board_led_toggle();
  }
}

int main(void)
{
  board_init();
  rtos_log_init();
  q = xQueueCreate(16, sizeof(uint32_t));
  stats_mx = xSemaphoreCreateMutex();
  ev = xEventGroupCreate();
  TimerHandle_t t = xTimerCreate("1s", pdMS_TO_TICKS(1000), pdTRUE, NULL, timer_cb);
  xTimerStart(t, 0);
  xTaskCreate(sensor_task, "sensor", 256, NULL, 3, NULL);
  xTaskCreate(logger_task, "logger", 256, NULL, 2, NULL);
  xTaskCreate(supervisor_task, "super", 384, NULL, 1, NULL);
  vTaskStartScheduler();
  for (;;) {}
}
