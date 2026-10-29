/* Console, printf, self-test protocol and the newlib system calls for the bare chips. */
#include <stdarg.h>
#include <stdio.h>
#include <sys/stat.h>
#include "bare.h"

static int checks, failures;

void bare_puts(const char *s) { while (*s) { if (*s == '\n') chip_putc('\r'); chip_putc(*s++); } }

void bare_printf(const char *fmt, ...)
{
  char buf[160];
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(buf, sizeof buf, fmt, ap);
  va_end(ap);
  bare_puts(buf);
}

void test_check(int ok, const char *what)
{
  checks++;
  if (!ok) failures++;
  bare_printf("CHECK %s: %s\n", ok ? "ok  " : "FAIL", what);
}

int test_failures(void) { return failures; }

void test_done(void)
{
  if (checks > 0 && failures == 0) { bare_printf("RESULT: PASS (%d checks)\n", checks); chip_exit(0); }
  bare_printf("RESULT: FAIL (%d of %d checks failed)\n", failures, checks);
  chip_exit(1);
}

__attribute__((weak)) void bare_irq_handler(int irq)
{
  bare_printf("unhandled IRQ %d\nRESULT: FAIL (irq)\n", irq);
  chip_exit(1);
}

/* newlib system calls: console only */
int _write(int fd, const char *p, int n) { (void)fd; for (int i = 0; i < n; i++) chip_putc(p[i]); return n; }
int _read(int fd, char *p, int n) { (void)fd; (void)p; (void)n; return 0; }
int _close(int fd) { (void)fd; return -1; }
int _lseek(int fd, int off, int whence) { (void)fd; (void)off; (void)whence; return 0; }
int _fstat(int fd, struct stat *st) { (void)fd; st->st_mode = S_IFCHR; return 0; }
int _isatty(int fd) { (void)fd; return 1; }
int _getpid(void) { return 1; }
int _kill(int pid, int sig) { (void)pid; (void)sig; return -1; }
void _exit(int code) { chip_exit(code); }
void *_sbrk(int incr)
{
  extern char end, __heap_limit;   /* linker script */
  static char *brk = &end;
  if (brk + incr > &__heap_limit) return (void *)-1;
  char *prev = brk;
  brk += incr;
  return prev;
}
