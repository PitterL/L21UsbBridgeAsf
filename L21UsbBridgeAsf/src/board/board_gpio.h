/*
 * board.h
 *
 * Created: 7/3/2018 9:56:17 AM
 *  Author: Pitter Liao
 */ 


#ifndef BOARD_GPIO_H_
#define BOARD_GPIO_H_
#include <port.h>

#define USB_ID PIN_PA02
#define GP_TP2 PIN_PA08
#define GP_IO0 PIN_PB10
#define GP_RST PIN_PB13
#define GP_CHG PIN_PB14

void board_gpio_init(void);

#endif