/* Bare-metal helpers shared by every chip in mcu/bare (chips/<chip>/ provides chip_init/chip_putc/chip_exit).
 * Self-test protocol for qemu/Renode runs: test_check() lines, then "RESULT: PASS" or "RESULT: FAIL". */
#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

void chip_init(void);                         /* clocks/console; called by the startup code before main() */
void chip_putc(char c);                       /* blocking console write */
void chip_exit(int code) __attribute__((noreturn)); /* qemu: exit status; Renode/boards: park the CPU */

void bare_puts(const char *s);
void bare_printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void bare_irq_handler(int irq);               /* weak: every peripheral IRQ (Cortex-M) lands here with its number */

void test_check(int ok, const char *what);
int  test_failures(void);
void test_done(void) __attribute__((noreturn));

#ifdef __cplusplus
}
#endif

#define REG32(addr) (*(volatile uint32_t *)(addr))
