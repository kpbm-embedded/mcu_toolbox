/* {{NAME}} startup: vector table (SP, reset, the core exceptions), .data copy, .bss clear, main(). */
#include <stdint.h>
extern uint32_t _estack, _sidata, _sdata, _edata, _sbss, _ebss;
extern int main(void);
void Reset_Handler(void);
static void Fault(void) { for (;;) { } }

__attribute__((section(".isr_vector"), used)) static void (*const vectors[16])(void) = {
  (void (*)(void))&_estack, Reset_Handler, Fault, Fault, Fault, Fault, Fault,
};

void Reset_Handler(void)
{
  uint32_t *s = &_sidata, *d = &_sdata;
  while (d < &_edata) *d++ = *s++;
  for (d = &_sbss; d < &_ebss;) *d++ = 0;
  int rc = main();
  register uint32_t r0 __asm("r0") = 0x18, r1 __asm("r1") = rc == 0 ? 0x20026u : 0x20023u;   /* semihosting exit */
  __asm volatile("bkpt 0xAB" : : "r"(r0), "r"(r1));
  for (;;) { }
}
