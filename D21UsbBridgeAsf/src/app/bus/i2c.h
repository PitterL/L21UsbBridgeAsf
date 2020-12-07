/*
 * common.h
 *
 * Created: 10/25/2019 5:19:22 PM
 *  Author: A41450
 */ 

#ifndef BUS_I2C_H_
#define BUS_I2C_H_

#define MAX_IIC_PACKAGE_LENGTH 255

//Below value should match protocol
enum {RESULT_I2C_OK, RESULT_I2C_NAK_WRITE, RESULT_I2C_NAK_ADDR, RESULT_I2C_NAK_READ, RESULT_I2C_OK_WITHOUT_R};

int32_t iic_bus_xfer_data(void *dbc, const uint8_t *wdata, uint16_t wlen, uint8_t *rdata, uint16_t rlen, uint16_t *readlen, uint8_t *ecode);
uint8_t iic_bus_ping(void *dbc, uint8_t addr);

extern bus_interface_t i2c_interface;

#endif