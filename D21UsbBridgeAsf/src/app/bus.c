/*
 * common.c
 *
 * Created: 7/21/2018 11:25:03 AM
 *  Author: A41450
 */ 
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "systick_counter.h"
#include "crc.h"
#include "board/board.h"
#include "hiddf_intf.h"
#include "external/utils.h"
#include "external/err_codes.h"
#include "bus.h"

#define WAITING_LOOP_UNIT_SHIFT 4
bool bus_waiting_state(uint32_t delay_us, bool active)
{
    bool status;
    int count = delay_us >> WAITING_LOOP_UNIT_SHIFT;

    do {
        status = hid_chg_line_active();
        if (status == active)
            break;

        delay_cycles_us(1 << WAITING_LOOP_UNIT_SHIFT);
    } while(count-- > 0);

    return count > 0;
}

bus_interface_t *bus_controller_list[BUS_TYPE_SUM] = {
    //BUS_I2C
    &i2c_interface,
    //BUS_SPI50
    &spi50_interface,
    //BUS_SPI51
    &spi51_interface,
};

int32_t bus_init(controller_t *hc, BUS_TYPE_T id)
{
    if (id < BUS_TYPE_SUM) {
        hc->mode = id; 
        hc->intf = bus_controller_list[id];
       
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