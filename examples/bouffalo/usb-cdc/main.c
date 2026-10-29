/* USB CDC-ACM echo device (CherryUSB, SDK v2). Plug the board's USB port into the host: a /dev/ttyACMx appears,
 * everything typed is echoed back; the console UART prints connection events. USB VID/PID 0xFFFF are the SDK demo
 * placeholders: use your own before shipping. Pattern follows the SDK example examples/cherryusb/cherryusb_cli/device_demo/cdc_acm_template.c. */
#include "bflb_mtimer.h"
#include "board.h"
#include "usbd_core.h"
#include "usbd_cdc_acm.h"
#include "FreeRTOS.h"
#include "task.h"

#define CDC_IN_EP  0x83
#define CDC_OUT_EP 0x04
#define CDC_INT_EP 0x85

#define USBD_VID       0xFFFF
#define USBD_PID       0xFFFF
#define USBD_MAX_POWER 100
#define USB_CONFIG_SIZE (9 + CDC_ACM_DESCRIPTOR_LEN)

#ifdef CONFIG_USB_HS
#define CDC_MAX_MPS 512
#else
#define CDC_MAX_MPS 64
#endif

static const uint8_t device_descriptor[] = {
    USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0xEF, 0x02, 0x01, USBD_VID, USBD_PID, 0x0100, 0x01)
};
static const uint8_t config_descriptor[] = {
    USB_CONFIG_DESCRIPTOR_INIT(USB_CONFIG_SIZE, 0x02, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
    CDC_ACM_DESCRIPTOR_INIT(0x00, CDC_INT_EP, CDC_OUT_EP, CDC_IN_EP, CDC_MAX_MPS, 0x02)
};
static const uint8_t device_quality_descriptor[] = { 0x0a, USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER, 0x00, 0x02, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00 };
static const char *string_descriptors[] = { (const char[]){ 0x09, 0x04 }, "mcu-examples", "Bouffalo CDC echo", "0001" };

static const uint8_t *device_descriptor_callback(uint8_t speed) { return device_descriptor; }
static const uint8_t *config_descriptor_callback(uint8_t speed) { return config_descriptor; }
static const uint8_t *device_quality_descriptor_callback(uint8_t speed) { return device_quality_descriptor; }
static const char *string_descriptor_callback(uint8_t speed, uint8_t index) { return index > 3 ? NULL : string_descriptors[index]; }

static const struct usb_descriptor cdc_descriptor = {
    .device_descriptor_callback = device_descriptor_callback,
    .config_descriptor_callback = config_descriptor_callback,
    .device_quality_descriptor_callback = device_quality_descriptor_callback,
    .string_descriptor_callback = string_descriptor_callback
};

static USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX uint8_t read_buffer[CDC_MAX_MPS];
static volatile bool configured;

static void usbd_event_handler(uint8_t busid, uint8_t event)
{
    switch (event) {
        case USBD_EVENT_RESET:
        case USBD_EVENT_DISCONNECTED:
        case USBD_EVENT_SUSPEND:
            configured = false;
            break;
        case USBD_EVENT_CONFIGURED:
            configured = true;
            usbd_ep_start_read(busid, CDC_OUT_EP, read_buffer, sizeof(read_buffer));
            break;
        default:
            break;
    }
}

static void bulk_out(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    if (nbytes) {
        usbd_ep_start_write(busid, CDC_IN_EP, read_buffer, nbytes); /* echo; re-arm the read when the write completes */
    } else {
        usbd_ep_start_read(busid, CDC_OUT_EP, read_buffer, sizeof(read_buffer));
    }
}

static void bulk_in(uint8_t busid, uint8_t ep, uint32_t nbytes)
{
    if (nbytes && (nbytes % CDC_MAX_MPS) == 0) {
        usbd_ep_start_write(busid, CDC_IN_EP, NULL, 0); /* zero-length packet ends the transfer */
    } else {
        usbd_ep_start_read(busid, CDC_OUT_EP, read_buffer, sizeof(read_buffer));
    }
}

static struct usbd_endpoint out_ep = { .ep_addr = CDC_OUT_EP, .ep_cb = bulk_out };
static struct usbd_endpoint in_ep = { .ep_addr = CDC_IN_EP, .ep_cb = bulk_in };
static struct usbd_interface intf0, intf1;

void usbd_cdc_acm_set_dtr(uint8_t busid, uint8_t intf, bool dtr) { printf("DTR %d\r\n", dtr); }

static void app_task(void *param)
{
    usbd_desc_register(0, &cdc_descriptor);
    usbd_add_interface(0, usbd_cdc_acm_init_intf(0, &intf0));
    usbd_add_interface(0, usbd_cdc_acm_init_intf(0, &intf1));
    usbd_add_endpoint(0, &out_ep);
    usbd_add_endpoint(0, &in_ep);
    usbd_initialize(0, 0, usbd_event_handler);

    while (1) {
        printf("usb-cdc: %s\r\n", configured ? "configured" : "waiting for host");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

int main(void)
{
    board_init();
#if defined(BOARD_USB_VIA_GPIO)
    board_usb_gpio_init();
#endif
    xTaskCreate(app_task, "usb_cdc", 1024, NULL, 15, NULL);
    vTaskStartScheduler();
    while (1) {
    }
}
