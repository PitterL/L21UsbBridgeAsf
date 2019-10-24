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
    uint8_t data[CONF_USB_COMPOSITE_HID_GENERIC_INTIN_MAXPKSZ];    //must be aligned
}__attribute__((packed, aligned(4))) response_cache_t;

typedef struct command_cache{
#define CONF_CMD_DATA_MAXSIZE 15
    uint8_t data[CONF_CMD_DATA_MAXSIZE];
}command_cache_t;

typedef struct response_data{
    response_cache_t rcache;
    bool dirty;
}response_data_t;

typedef struct transfer_data{
    command_cache_t ccache;
}transfer_data_t;

typedef int32_t (*func_bus_init_t)(void *dbc, void *hw, uint32_t baudrate, uint8_t addr);
typedef int32_t (*func_bus_deinit_t)(void *dbc);
typedef int32_t (*func_bus_write_t)(void *dbc, const uint8_t *const buf, const uint16_t length);
typedef int32_t (*func_bus_read_t)(void *dbc, uint8_t *const buf, const uint16_t length);
typedef int32_t (*func_bus_xfer_t)(void *dbc, const uint8_t *wdata, uint16_t wlen, uint8_t *rdata, uint16_t rlen, uint16_t *readlen, uint8_t *ecode);
typedef uint8_t (*func_bus_ping_t)(void *dbc, uint8_t addr);
typedef void (*func_bus_set_address_t)(void *dbc, uint8_t addr);

typedef struct bus_interface {
    func_bus_init_t cb_init;
    func_bus_deinit_t cb_deinit;
    //func_bus_write_t cb_write;
    //func_bus_read_t cb_read;
    func_bus_xfer_t cb_xfer;
    func_bus_ping_t cb_ping;    
    //func_bus_set_address_t cb_set_address;
    void *sercom;   //Hw Sercom

    void *dbc;  //Point to the device bus controller handle

}bus_interface_t;

typedef enum BUS_TYPE {BUS_I2C, BUS_SPI, BUS_I2C_WRAP_SPI, BUS_TYPE_SUM} BUS_TYPE_T;

typedef struct {
    config_setting_t setting;
#define BIT_BUS_INITED 1
#define BIT_BUS_REINIT 2
#define BIT_BULK_CMD_READ_CONTINUE 4
#define BIT_AUTO_REPEAT 5
    uint32_t flag;
    transfer_data_t transfer;
    response_data_t response;
    BUS_TYPE_T mode;
    bus_interface_t *intf;
}controller_t;

int32_t bus_init(controller_t *hc, BUS_TYPE_T id);
void bus_deinit(controller_t *hc);
bool bus_mode(controller_t *hc, BUS_TYPE_T id);

#endif