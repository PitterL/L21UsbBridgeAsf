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

#ifdef __SAMD21J18A__
#define BOARD_D21
#endif

void platform_board_init(void);

#endif