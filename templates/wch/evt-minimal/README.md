# {{NAME}} (wch evt-minimal, {{BOARD}})

WCH's own StdPeriph-style API (`GPIO_Init`, `RCC_APB2PeriphClockCmd`, `Delay_Ms`, `USART_Printf_Init`) on CH32V203C8T6 (D6 startup). The five
`ch32v20x_*`/`system_*` files are copies of the EVT `GPIO_Toggle/User` set; the library itself is used read-only from `$WCH_DIR/ch32v20x/EVT/EXAM`.

    source mcu/wch/env.sh
    make BUILD=$PWD/build
    minichlink -w build/app.bin flash -b        # or: wchisp flash build/app.hex

LED is the BluePill Plus user LED (PB2). printf goes to USART1 (PA9) at 115200. Add peripheral drivers by dropping more files from
`$EVT/SRC/Peripheral/src` (all of them are already compiled in).
