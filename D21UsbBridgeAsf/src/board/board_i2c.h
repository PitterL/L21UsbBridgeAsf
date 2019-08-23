/*
 * board.h
 *
 * Created: 7/3/2018 9:56:17 AM
 *  Author: A41450
 */ 


#ifndef BOARD_I2C_H_
#define BOARD_I2C_H_
#include <i2c_master.h>

typedef struct{
	struct i2c_master_module module;
	struct i2c_master_config config;
	uint8_t addr;
}iic_controller_t;

void iic_set_address(iic_controller_t *ihc, uint8_t addr);
int32_t iic_bus_init(iic_controller_t *ihc, void *hw, uint8_t baudrate, uint8_t addr);
int32_t iic_bus_deinit(iic_controller_t *ihc);
int32_t iic_write(iic_controller_t *ihc, const uint8_t *const buf, const uint16_t length);
int32_t iic_read(iic_controller_t *ihc, uint8_t *const buf, const uint16_t length);

#endif /* BOARD_H_ */