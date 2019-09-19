/*
 * board.h
 *
 * Created: 7/15/2018 2:15:17 PM
 *  Author: A41450
 */ 


#ifndef PLATFORM_BOARD_H_
#define PLATFORM_BOARD_H_

#include "board_i2c.h"
#include "board_gpio.h"
#include "board_cdc.h"
#include "board_clock.h"

#define BOARD_BRIDGE_D21_PROTOCOL

struct i2c_master_packet_w {
	/** Address to slave device  */
	uint16_t address;
	/** Length of data array */
	uint16_t data_length;
	/** Data array containing all data to be transferred */
	const uint8_t *data;
	/** Use 10-bit addressing. Set to false if the feature is not supported by the device  */
	bool ten_bit_address;
	/** Use high speed transfer. Set to false if the feature is not supported by the device */
	bool high_speed;
	/** High speed mode master code (0000 1XXX), valid when high_speed is true */
	uint8_t hs_master_code;
};

void platform_board_init(void);

#endif