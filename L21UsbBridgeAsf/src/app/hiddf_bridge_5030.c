/*
 * bridge_u5030.c
 *
 * Created: 6/20/2018 9:13:58 AM
 *  Author: A41450
 */ 
#include "hiddf_generic.h"
#include "u5030_protocol.h"

/* HID report descriptor for bridge u5030 */
const uint8_t u5030_report_desc[CONF_USB_COMPOSITE_HID_GENERIC_REPORT_LEN] = {

	0x06, 0xFF, 0xFF,  // Usage Page (Vendor Defined 0xFFFF)
	0x09, 0x01,        // Usage (0x01)
	0xA1, 0x01,        // Collection (Application)
	0x09, 0x02,        //   Usage (0x02)
	0x15, 0x00,        //   Logical Minimum (0)
	0x25, 0xFF,        //   Logical Maximum (-1)
	0x35, 0x00,        //   Physical Minimum (0)
	0x45, 0x00,        //   Physical Maximum (0)
	0x65, 0x00,        //   Unit (None)
	0x55, 0x00,        //   Unit Exponent (0)
	0x75, 0x08,        //   Report Size (8)
	0x95, 0x01,        //   Report Count (1)
	0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0x09, 0x03,        //   Usage (0x03)
	0x95, 0x3F,        //   Report Count (63)
	0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0x09, 0x04,        //   Usage (0x04)
	0x95, 0x01,        //   Report Count (1)
	0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
	0x09, 0x05,        //   Usage (0x05)
	0x95, 0x3F,        //   Report Count (63)
	0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
	0x09, 0x06,        //   Usage (0x06)
	0x95, 0x01,        //   Report Count (1)
	0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
	0x09, 0x07,        //   Usage (0x07)
	0x95, 0x03,        //   Report Count (3)
	0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
	0xC0        // End Collection
};

/**
 * \brief Initialize the USB HID BRIDGE U5030 Function Driver
 */
int32_t hiddf_generic_u5030_init(void)
{
	return hiddf_generic_init(u5030_report_desc, ARRAY_SIZE(u5030_report_desc));
}


static bool hiddf_bridge_interrupt_in(const uint8_t ep, const enum usb_xfer_code rc, const uint32_t count);
static void hiddf_bridge_sof_event(void)
{
	int32_t ret;
	uint8_t *buf;
	uint32_t count;
	
	uint8_t data[CONF_USB_COMPOSITE_HID_GENERIC_INTIN_MAXPKSZ];
	//OUT Token
	ret = hiddf_generic_read(data, sizeof(data));
	if (ret == ERR_NONE)
	{
		ret = u5030_process_data(data, sizeof(data));
		if (ret != ERR_NONE)
			; //TODO: need do something
	}else {
		; //TODO: need do something
	}

/*
	//IN Token	
	ret = u5030_get_response((void **)&buf, &count);
	if (ret == ERR_NONE) {
		ret = hiddf_generic_write(buf, count-1);
		if (ret != ERR_NONE)
			; //TODO: should do something
		u5030_clear_cache();
	}

	if (ret != ERR_NONE){
		;  //TODO: how to handle error
	}
*/
}
static struct usbdc_handler hiddf_bridge_sof_event_h = {NULL, (FUNC_PTR)hiddf_bridge_sof_event};

/**
 * \brief Callback invoked when bulk IN data received
 * \param[in] ep Endpoint number
 * \param[in] rc transfer return status
 * \param[in] count the amount of bytes has been transferred
 * \return Operation status.
 */

static bool hiddf_bridge_interrupt_in(const uint8_t ep, const enum usb_xfer_code rc, const uint32_t count)
{
	uint8_t buf[] = {0xff, 0, 1, 2, 3};
	(void)ep;
	(void)rc;

	if (rc == USB_XFER_RESET) {
		return true;
	} else if (rc == USB_XFER_UNHALT) {
		return hiddf_generic_write(buf, sizeof(buf));
	}
	
	return false;
}

/**
 * \brief Callback invoked when bulk OUT data received
 * \param[in] ep Endpoint number
 * \param[in] rc transfer return status
 * \param[in] count the amount of bytes has been transferred
 * \return Operation status.
 */

static bool hiddf_bridge_interrupt_out(const uint8_t ep, const enum usb_xfer_code rc, const uint32_t count)
{
	uint8_t buf[count];
	int32_t result = ERR_NONE;

	(void)ep;
	if (rc == USB_XFER_RESET) {
		;
	} else if (rc == USB_XFER_UNHALT) {
		result = hiddf_generic_read(buf, count);
		if (result == ERR_NONE)
		{
			result = u5030_process_data(buf, count);
		}
	}
	
	if (result != ERR_NONE)
		;  //FIXME: need handle error
	
	return result;
}

void hiddf_bridge_change_event (enum usbdc_change_type change, uint32_t value)
{
	//uint8_t buf[1];
	int ret;

	if (change == USBDC_C_STATE) {
		if (value == USBD_S_CONFIG) {
			if (hiddf_generic_is_enabled()) {
				ret = hiddf_generic_register_callback(HIDDF_GENERIC_CB_READ, (FUNC_PTR)hiddf_bridge_interrupt_out);
				if (ret != ERR_NONE)
					; //FIXME: not handle the error
				hiddf_generic_register_callback(HIDDF_GENERIC_CB_WRITE, (FUNC_PTR)hiddf_bridge_interrupt_in);
				//hiddf_generic_read(buf, 0);
			}
		}
	}
}
static struct usbdc_handler hiddf_bridge_change_event_h = {NULL, (FUNC_PTR)hiddf_bridge_change_event};

bool hiddf_bridge_set_report(uint8_t *buf, uint16_t size)
{
	struct usb_req *req = (struct usb_req *)buf;

	if (req->bmRequestType != 0x21)
		return false;

	//TODO: do something here
	return true;
}

void bridge_u5030_init(void)
{
	u5030_init();
	usbdc_register_handler(USBDC_HDL_SOF, &hiddf_bridge_sof_event_h);
	usbdc_register_handler(USBDC_HDL_CHANGE, &hiddf_bridge_change_event_h);
	//hiddf_generic_register_callback(HIDDF_GENERIC_CB_READ, (FUNC_PTR)hiddf_bridge_interrupt_out);
	//hiddf_generic_register_callback(HIDDF_GENERIC_CB_WRITE, hiddf_bridge_interrupt_in);
	hiddf_generic_register_callback(HIDDF_GENERIC_CB_SET_REPORT, (FUNC_PTR)hiddf_bridge_set_report);
}
