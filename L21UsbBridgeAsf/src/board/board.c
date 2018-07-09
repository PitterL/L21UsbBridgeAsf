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

int32_t iic_bus_init(iic_controller_t *ihc, void *hw, uint8_t baudrate, uint8_t addr)
{
	i2c_master_get_config_defaults(&ihc->config);
	i2c_master_init(&ihc->module, hw, &ihc->config);
	ihc->addr = addr;
	
    ihc->hw = hw;

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
	struct i2c_master_packet pkg;
	enum status_code result;
	
	memset(&pkg, 0, sizeof(pkg));
	pkg.address = ihc->addr;
	pkg.data_length = length;
	pkg.data = buf;
	
	result = i2c_master_write_packet_wait(&ihc->module, &pkg);
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
