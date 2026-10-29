#ifndef USB_CONFIG_H
#define USB_CONFIG_H

#include "cherryusb_config_template.h"

/* BL616: high-speed USB 2.0 OTG (usb_v2 driver, supports test mode). BL702: full-speed, usb_v1 driver without test mode. */
#undef CONFIG_USB_HS
#undef CONFIG_USBDEV_TEST_MODE
#ifdef BL702
#else
#define CONFIG_USB_HS
#define CONFIG_USBDEV_TEST_MODE
#endif

#endif
