/**
 * \file
 *
 * \brief Descriptors for an USB Composite Device
 *
 * Copyright (c) 2009-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#include "conf_usb.h"
#include "udd.h"
#include "udc_desc.h"

/**
 * \defgroup udi_group_desc Descriptors for a USB Device
 * composite
 *
 * @{
 */

/**INDENT-OFF**/

//! USB Device Descriptor
COMPILER_WORD_ALIGNED
UDC_DESC_STORAGE usb_dev_desc_t udc_device_desc = {
	.bLength                   = sizeof(usb_dev_desc_t),
	.bDescriptorType           = USB_DT_DEVICE,
	.bcdUSB                    = LE16(USB_V2_0),
	.bDeviceClass              = 0,
	.bDeviceSubClass           = 0,
	.bDeviceProtocol           = 0,
	.bMaxPacketSize0           = USB_DEVICE_EP_CTRL_SIZE,
	.idVendor                  = LE16(USB_DEVICE_VENDOR_ID),
	.idProduct                 = LE16(USB_DEVICE_PRODUCT_ID),
	.bcdDevice                 = LE16((USB_DEVICE_MAJOR_VERSION << 8)
		| USB_DEVICE_MINOR_VERSION),
	.iManufacturer             = USB_DEVICE_MANUFACTURE_STRING_ID,
	.iProduct                  = 0 /*USB_DEVICE_PRODUCT_STRING_ID*/,
	.iSerialNumber             = USB_DEVICE_SERIAL_STRING_ID,
	.bNumConfigurations        = 1
};


#ifdef USB_DEVICE_HS_SUPPORT
//! USB Device Qualifier Descriptor for HS
COMPILER_WORD_ALIGNED
UDC_DESC_STORAGE usb_dev_qual_desc_t udc_device_qual = {
	.bLength                   = sizeof(usb_dev_qual_desc_t),
	.bDescriptorType           = USB_DT_DEVICE_QUALIFIER,
	.bcdUSB                    = LE16(USB_V2_0),
	.bDeviceClass              = 0,
	.bDeviceSubClass           = 0,
	.bDeviceProtocol           = 0,
	.bMaxPacketSize0           = USB_DEVICE_EP_CTRL_SIZE,
	.bNumConfigurations        = 1
};
#endif

//! Structure for USB Device Configuration Descriptor
COMPILER_PACK_SET(1)
typedef struct {
	usb_conf_desc_t conf;
	//UDI_COMPOSITE_DESC_T;
	#ifdef USB_COMPOSITE_DEVICE_UDI_CDC_EN
	usb_iad_desc_t       udi_cdc_iad;
	udi_cdc_comm_desc_t  udi_cdc_comm;
	udi_cdc_data_desc_t  udi_cdc_data;
	#endif
	#ifdef USB_COMPOSITE_DEVICE_MSC_EN
	udi_msc_desc_t       udi_msc;
	#endif
	#ifdef USB_COMPOSITE_DEVICE_HID_MOUSE
	udi_hid_mouse_desc_t udi_hid_mouse;
	#endif
	#ifdef USB_COMPOSITE_DEVICE_HID_KBD
	udi_hid_kbd_desc_t   udi_hid_kbd;
	#endif
	#ifdef USB_COMPOSITE_DEVICE_HID_GENERIC_EN
	udi_hid_generic_desc_t   udi_hid_generic;
	#endif
} udc_desc_t;
COMPILER_PACK_RESET()

//! USB Device Configuration Descriptor filled for FS
COMPILER_WORD_ALIGNED
UDC_DESC_STORAGE udc_desc_t udc_desc_fs = {
	.conf.bLength              = sizeof(usb_conf_desc_t),
	.conf.bDescriptorType      = USB_DT_CONFIGURATION,
	.conf.wTotalLength         = LE16(sizeof(udc_desc_t)),
	.conf.bNumInterfaces       = USB_DEVICE_NB_INTERFACE,
	.conf.bConfigurationValue  = 1,
	.conf.iConfiguration       = 0 /*USB_CONFIG_STR_DESC_STRING_ID*/,
	.conf.bmAttributes         = USB_CONFIG_ATTR_MUST_SET | USB_DEVICE_ATTR,
	.conf.bMaxPower            = USB_CONFIG_MAX_POWER(USB_DEVICE_POWER),
	//UDI_COMPOSITE_DESC_FS
	#ifdef USB_COMPOSITE_DEVICE_UDI_CDC_EN
	.udi_cdc_iad   = UDI_CDC_IAD_DESC_0,
	.udi_cdc_comm  = UDI_CDC_COMM_DESC_0,
	.udi_cdc_data  = UDI_CDC_DATA_DESC_0_FS,
	#endif
	#ifdef USB_COMPOSITE_DEVICE_MSC_EN
	.udi_msc       = UDI_MSC_DESC_FS,
	#endif
	#ifdef USB_COMPOSITE_DEVICE_HID_MOUSE
	.udi_hid_mouse = UDI_HID_MOUSE_DESC,
	#endif
	#ifdef USB_COMPOSITE_DEVICE_HID_KBD
	.udi_hid_kbd   = UDI_HID_KBD_DESC,
	#endif
	#ifdef USB_COMPOSITE_DEVICE_HID_GENERIC_EN
	.udi_hid_generic = UDI_HID_GENERIC_DESC,
	#endif
};

#ifdef USB_DEVICE_HS_SUPPORT
//! USB Device Configuration Descriptor filled for HS
COMPILER_WORD_ALIGNED
UDC_DESC_STORAGE udc_desc_t udc_desc_hs = {
	.conf.bLength              = sizeof(usb_conf_desc_t),
	.conf.bDescriptorType      = USB_DT_CONFIGURATION,
	.conf.wTotalLength         = LE16(sizeof(udc_desc_t)),
	.conf.bNumInterfaces       = USB_DEVICE_NB_INTERFACE,
	.conf.bConfigurationValue  = 1,
	.conf.iConfiguration       = 0 /*USB_CONFIG_STR_DESC_STRING_ID*/,
	.conf.bmAttributes         = USB_CONFIG_ATTR_MUST_SET | USB_DEVICE_ATTR,
	.conf.bMaxPower            = USB_CONFIG_MAX_POWER(USB_DEVICE_POWER),
	//UDI_COMPOSITE_DESC_HS
	#ifdef USB_COMPOSITE_DEVICE_UDI_CDC_EN
	.udi_cdc_iad   = UDI_CDC_IAD_DESC_0,
	.udi_cdc_comm  = UDI_CDC_COMM_DESC_0,
	.udi_cdc_data  = UDI_CDC_DATA_DESC_0_HS,
	#endif
	#ifdef USB_COMPOSITE_DEVICE_MSC_EN
	.udi_msc       = UDI_MSC_DESC_HS,
	#endif
	#ifdef USB_COMPOSITE_DEVICE_HID_MOUSE
	.udi_hid_mouse = UDI_HID_MOUSE_DESC,
	#endif
	#ifdef USB_COMPOSITE_DEVICE_HID_KBD
	.udi_hid_kbd   = UDI_HID_KBD_DESC,
	#endif
	#ifdef USB_COMPOSITE_DEVICE_HID_GENERIC_EN
	.udi_hid_generic = UDI_HID_GENERIC_DESC,
	#endif
};
#endif


/**
 * \name UDC structures which contains all USB Device definitions
 */
//@{

//! Associate an UDI for each USB interface
UDC_DESC_STORAGE udi_api_t *udi_apis[USB_DEVICE_NB_INTERFACE] = {
	//UDI_COMPOSITE_API
	#ifdef USB_COMPOSITE_DEVICE_UDI_CDC_EN
	&udi_api_cdc_comm,
	&udi_api_cdc_data,
	#endif
	#ifdef USB_COMPOSITE_DEVICE_MSC_EN
	&udi_api_msc,
	#endif
	#ifdef USB_COMPOSITE_DEVICE_HID_MOUSE
	&udi_api_hid_mouse,
	#endif
	#ifdef USB_COMPOSITE_DEVICE_HID_KBD
	&udi_api_hid_kbd,
	#endif
	#ifdef USB_COMPOSITE_DEVICE_HID_GENERIC_EN
	&udi_api_hid_generic,
	#endif
};

//! Add UDI with USB Descriptors FS
UDC_DESC_STORAGE udc_config_speed_t   udc_config_lsfs[1] = {{
	.desc          = (usb_conf_desc_t UDC_DESC_STORAGE*)&udc_desc_fs,
	.udi_apis      = udi_apis,
}};

#ifdef USB_DEVICE_HS_SUPPORT
//! Add UDI with USB Descriptors HS
UDC_DESC_STORAGE udc_config_speed_t   udc_config_hs[1] = {{
	.desc          = (usb_conf_desc_t UDC_DESC_STORAGE*)&udc_desc_hs,
	.udi_apis      = udi_apis,
}};
#endif

//! Add all information about USB Device in global structure for UDC
UDC_DESC_STORAGE udc_config_t udc_config = {
	.confdev_lsfs = &udc_device_desc,
	.conf_lsfs = udc_config_lsfs,
#ifdef USB_DEVICE_HS_SUPPORT
	.confdev_hs = &udc_device_desc,
	.qualifier = &udc_device_qual,
	.conf_hs = udc_config_hs,
#endif
};

//@}
/**INDENT-ON**/
//@}
