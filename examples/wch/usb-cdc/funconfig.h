#ifndef _FUNCONFIG_H
#define _FUNCONFIG_H
/* Derived from ch32fun examples_usb/USBD/usbd_cdc_tty and USBFS/usbfs_cdc_tty (MIT). USB needs an exact 48 MHz clock: HSE. */
#define FUNCONF_USE_DEBUGPRINTF     0   /* only one printf backend: the USB CDC port */
#define FUNCONF_USE_USBPRINTF       1
#define FUNCONF_DEBUG_HARDFAULT     0
#define FUNCONF_USE_HSI             0
#define FUNCONF_USE_HSE             1
#if defined(BOARD_CH32V307)
#define FUNCONF_ENABLE_HPE          0
#define FUNCONF_SYSTICK_USE_HCLK    1
#endif
#endif
