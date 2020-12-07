/*
 * board_i2c.h
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

int32_t iic_board_init(void *dbc, void *hw, uint32_t baudrate, uint8_t addr);
int32_t iic_board_deinit(void *dbc);
int32_t iic_board_write(void *dbc, const uint8_t *const buf, const uint16_t length);
int32_t iic_board_read(void *dbc, uint8_t *const buf, const uint16_t length);
void iic_board_set_address(void *dbc, uint8_t addr);

#endif /* BOARD_I2C_H_ */