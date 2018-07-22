/*
 * common.h
 *
 * Created: 7/21/2018 5:19:22 PM
 *  Author: A41450
 */ 

#ifndef PROTOCOL_COMMON_H_
#define PROTOCOL_COMMON_H_

#include "d21_protocol.h"
#include "u5030_protocol.h"

typedef struct response_cache{
	uint8_t data[CONF_USB_COMPOSITE_HID_GENERIC_INTIN_MAXPKSZ];	//must be aligned
}__attribute__((packed, aligned(4))) response_cache_t;

typedef struct command_cache{
#define CONF_CMD_DATA_MAXSIZE 15
	uint8_t data[CONF_CMD_DATA_MAXSIZE];
}command_cache_t;

typedef struct response_data{
    response_cache_t rcache;
    bool dirty;
}response_data_t;

typedef uint8_t (*func_xfer)(void *host, const uint8_t *wdata, uint32_t wlen, uint8_t *rdata, uint32_t rlen, uint8_t *ecode, int32_t retry);
typedef struct transfer_data{
    command_cache_t ccache;
    func_xfer xfer;
}transfer_data_t;

typedef struct {
	config_setting_t setting;
#define BIT_BUS_INITED 1
#define BIT_BUS_REINIT 2
#define BIT_BULK_CMD_READ_CONTINUE 4
#define BIT_AUTO_REPEAT 5
	uint32_t flag;
	iic_controller_t iic;
    transfer_data_t transfer;
    response_data_t response;
}controller_t;


uint8_t i2c_transfer_data(void *host, const uint8_t *wdata, uint32_t wlen, uint8_t *rdata, uint32_t rlen, uint8_t *ecode, int32_t retry);

#endif