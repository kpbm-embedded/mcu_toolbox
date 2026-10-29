#include "board.h"
#include "hardware/flash.h"
#include "pico/flash.h"
#include <stdio.h>
#include <string.h>
/* Last sector of flash, using the board's real flash size (PICO_FLASH_SIZE_BYTES). */
#define STORE_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)
#define MAGIC 0x4D435530u
typedef struct { uint32_t magic, boots; uint8_t pad[FLASH_PAGE_SIZE - 8]; } store_t;

static void do_write(void *param) {
    const store_t *s = param;
    flash_range_erase(STORE_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(STORE_OFFSET, (const uint8_t *)s, sizeof *s);
}
int main(void) {
    stdio_init_all();
    const store_t *flash = (const store_t *)(XIP_BASE + STORE_OFFSET);
    static store_t s;
    memset(&s, 0xFF, sizeof s);
    s.magic = MAGIC;
    s.boots = (flash->magic == MAGIC) ? flash->boots + 1 : 1;
    int rc = flash_safe_execute(do_write, &s, 1000);   /* pauses the other core / IRQs safely */
    sleep_ms(2000);
    printf("boot count = %u (write rc=%d)\n", (unsigned)flash->boots, rc);
    while (true) tight_loop_contents();
}
