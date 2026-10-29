# CH32V203 has the USBD device block, CH32V307 the USBFS one: different ch32fun extralibs driver
ADDITIONAL_C_FILES += $(CH32FUN)/../extralibs/$(if $(filter ch32v203,$(TARGET_NAME)),usbd.c,fsusb.c)
