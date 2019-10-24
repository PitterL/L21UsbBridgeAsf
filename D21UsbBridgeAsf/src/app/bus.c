/*
 * common.c
 *
 * Created: 7/21/2018 11:25:03 AM
 *  Author: A41450
 */ 
#include <stddef.h>
#include <stdint.h>
#include <string.h>

//#include "../driver_init.h"
#include "crc.h"
#include "board/board.h"
#include "bus.h"
#include "external/utils.h"
#include "external/err_codes.h"

iic_controller_t iic_bus_controller;
spi_controller_t spi_bus_controller;

bus_interface_t bus_controller_list[BUS_TYPE_SUM] = {
    //BUS_I2C
    {
        .cb_init = iic_bus_init,
        .cb_deinit = iic_bus_deinit,
        .cb_xfer = iic_bus_xfer_data,
        .cb_ping = iic_bus_ping,
        .sercom = SERCOM1,
        .dbc = &iic_bus_controller
    },

    //BUS_SPI
    {
        .cb_init = spi_bus_init,
        .cb_deinit = spi_bus_deinit,
        .cb_xfer = spi_bus_xfer_data,
        .cb_ping = spi_bus_ping,
        .sercom = SERCOM1,
        .dbc = &spi_bus_controller
    }
};

int32_t bus_init(controller_t *hc, BUS_TYPE_T id)
{
    if (id < BUS_TYPE_SUM) {
        hc->mode = id; 
        hc->intf = &bus_controller_list[id];
       
        return ERR_NONE;
    }

    return ERR_INVALID_ARG;
}

void bus_deinit(controller_t *hc)
{
    hc->mode = BUS_TYPE_SUM;
    hc->intf = NULL;
}

bool bus_mode(controller_t *hc, BUS_TYPE_T id)
{
    return hc->mode == id;
}