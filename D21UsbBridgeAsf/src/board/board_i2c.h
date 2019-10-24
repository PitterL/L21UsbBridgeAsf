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

//Below value should match protocol
enum {RESULT_I2C_OK, RESULT_I2C_NAK_WRITE, RESULT_I2C_NAK_ADDR, RESULT_I2C_NAK_READ, RESULT_I2C_OK_WITHOUT_R};

int32_t iic_bus_init(void *dbc, void *hw, uint32_t baudrate, uint8_t addr);
int32_t iic_bus_deinit(void *dbc);
int32_t iic_bus_write(void *dbc, const uint8_t *const buf, const uint16_t length);
int32_t iic_bus_read(void *dbc, uint8_t *const buf, const uint16_t length);
int32_t iic_bus_xfer_data(void *dbc, const uint8_t *wdata, uint16_t wlen, uint8_t *rdata, uint16_t rlen, uint16_t *readlen, uint8_t *ecode);
uint8_t iic_bus_ping(void *dbc, uint8_t addr);
void iic_bus_set_address(void *dbc, uint8_t addr);

#endif /* BOARD_I2C_H_ */