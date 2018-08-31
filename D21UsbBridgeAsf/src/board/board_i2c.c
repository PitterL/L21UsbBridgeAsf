/*
 * board.c
 *
 * Created: 7/3/2018 9:55:53 AM
 *  Author: A41450
 */ 

#include <status_codes.h>
#include <string.h>
#include "board.h"
#include "external/err_codes.h"

int32_t i2c_master_get_default_pads(void *hw, struct i2c_master_config *const cfg)
{
	uint32_t pad0, pad1;

	//PAD0: SDA; PAD1: SCL
	switch((uint32_t)hw){
	case (uint32_t)SERCOM0:
		pad0 = PINMUX_PA04D_SERCOM0_PAD0;
		pad1 = PINMUX_PA07D_SERCOM0_PAD3;
		break;
	case (uint32_t)SERCOM4:
		pad0    = PINMUX_PA12D_SERCOM4_PAD0;
		pad1    = PINMUX_PA13D_SERCOM4_PAD1;
		break;
	case (uint32_t)SERCOM3:
		pad0 = PINMUX_PA16D_SERCOM3_PAD0;
		pad1 = PINMUX_PA17D_SERCOM3_PAD1;
		break;
	case (uint32_t)SERCOM1:
	default:
		pad0 = PINMUX_PA16C_SERCOM1_PAD0;
		pad1 = PINMUX_PA17C_SERCOM1_PAD1;
		break;
	}

	cfg->pinmux_pad0 = pad0;
	cfg->pinmux_pad1 = pad1;

	return ERR_NONE;
}

int32_t iic_bus_init(iic_controller_t *ihc, void *hw, uint8_t baudrate, uint8_t addr)
{
	i2c_master_get_config_defaults(&ihc->config);
	i2c_master_get_default_pads(hw, &ihc->config);
	i2c_master_init(&ihc->module, hw, &ihc->config);
	i2c_master_enable(&ihc->module);
	ihc->addr = addr;

    return ERR_NONE;
}

int32_t iic_bus_deinit(iic_controller_t *ihc)
{
	i2c_master_disable(&ihc->module);
	
    return ERR_NONE;
}


/**
 * \brief I/O write interface
 */
int32_t iic_write(iic_controller_t *ihc, const uint8_t *const buf, const uint16_t length)
{
	struct i2c_master_packet_w pkg;
	enum status_code result;
	
	memset(&pkg, 0, sizeof(pkg));
	pkg.address = ihc->addr;
	pkg.data_length = length;
	pkg.data = buf;
	
	result = i2c_master_write_packet_wait(&ihc->module, (struct i2c_master_packet *)&pkg);
	if (result == STATUS_OK)
		return ERR_NONE;
	
	return -ERR_ABORTED;
}

/**
 * \brief I/O read interface
 */
int32_t iic_read(iic_controller_t *ihc, uint8_t *const buf, const uint16_t length)
{
	struct i2c_master_packet pkg;
	enum status_code result;
	
	memset(&pkg, 0, sizeof(pkg));
	pkg.address = ihc->addr;
	pkg.data_length = length;
	pkg.data = buf;
	
	result = i2c_master_read_packet_wait(&ihc->module, &pkg);
	if (result == STATUS_OK)
		return ERR_NONE;
	
	return -ERR_ABORTED;
}
