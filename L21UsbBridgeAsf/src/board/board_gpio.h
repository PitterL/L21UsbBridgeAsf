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
#define GP_IO1 PIN_PB11
#define GP_IO2 PIN_PB12
#define GP_RST PIN_PB13
#define GP_CHG PIN_PB14
#define GP_TRANS_EN PIN_PB15
#define GP_IO3 PIN_PA12
#define GP_IO4 PIN_PA13
#define GP_VBUS_EXIST PIN_PA14
#define GP_WIFI_WAKE PIN_PA15
#define GP_IO5 PIN_PA18
#define GP_VDD33_EN PIN_PA19
#define GP_IO_BTLD PIN_PA20
#define GP_VDD5_EN PIN_PA21
#define GP_IO_PWR PIN_PA22
#define GP_IO_LED2_EN PIN_PA23
#define GP_WIFI_EN PIN_PA27
#define GP_WIFI_RST PIN_PB30
#define GP_VDDIO_EN PIN_PB31

void board_gpio_init(void);
bool pwr_switch_on(void);
void pwr_5v_en(bool en);
void pwr_3v3_en(bool en);
void pwr_vio_en(bool en);
void io_trans_en(bool en);
void io_rst_en(bool en);

void por_on(bool en);

#endif