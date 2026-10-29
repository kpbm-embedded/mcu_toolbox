/* RV32 startup shared by the RISC-V chips: stack, gp, .data copy, .bss clear, chip_init(), constructors, main(),
 * chip_exit(). Machine mode, no interrupts set up (mtvec is left alone). */
  .section .init, "ax"
  .globl _start
_start:
  .option push
  .option norelax
  la gp, __global_pointer$
  .option pop
  la sp, _estack
  la a0, _sidata
  la a1, _sdata
  la a2, _edata
1: bgeu a1, a2, 2f
  lw t0, 0(a0)
  sw t0, 0(a1)
  addi a0, a0, 4
  addi a1, a1, 4
  j 1b
2: la a0, _sbss
  la a1, _ebss
3: bgeu a0, a1, 4f
  sw zero, 0(a0)
  addi a0, a0, 4
  j 3b
4: call chip_init
  la s0, __init_array_start            /* C++ / constructor functions */
  la s1, __init_array_end
6: bgeu s0, s1, 7f
  lw t0, 0(s0)
  jalr t0
  addi s0, s0, 4
  j 6b
7: call main
  call chip_exit
5: j 5b
