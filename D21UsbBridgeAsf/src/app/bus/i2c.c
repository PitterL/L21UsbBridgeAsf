/*
 * common.c
 *
 * Created: 7/21/2018 11:25:03 AM
 *  Author: A41450
 */ 
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "board/board.h"
#include "external/utils.h"
#include "external/err_codes.h"
#include "app/bus.h"

/*
    I2c bus transfer data
    @dbc: device bus controller handle
    @wdata: write buffer data pointer
    @wlen: write data length
    @rdata: read buffer data pointer
    @rlen: read buffer length
    @readlen: readout data length output pointer
    @ecode: bus status code
    return error code: ERR_NONE mean successful, otherwise failed
*/
int32_t iic_bus_xfer_data(void *dbc, const uint8_t *wdata, uint16_t wlen, uint8_t *rdata, uint16_t rlen, uint16_t *readlen, uint8_t *ecode)
{
    iic_controller_t *ihc = (iic_controller_t *)dbc;
    int8_t cmd_rsp;
    uint16_t len_rsp;
    int32_t ret;
    int retry = 0;

    do {
        cmd_rsp = RESULT_I2C_OK;
        len_rsp = 0;
        ret = ERR_NONE;
        
        //write
        if (wlen) {
            ret = iic_board_write(ihc, wdata, wlen);
            if (ret == ERR_BAD_ADDRESS) {
                cmd_rsp = RESULT_I2C_NAK_ADDR;
            }else if(ret) {
                cmd_rsp = RESULT_I2C_NAK_WRITE;
            }else {
                // Write OK
            }
        }

        //read
        if (ret == ERR_NONE) {
            if (rlen) {
                ret = iic_board_read(ihc, rdata, rlen);
                if (ret == ERR_BAD_ADDRESS) {
                    cmd_rsp = RESULT_I2C_NAK_ADDR;
                }else if(ret) {
                    cmd_rsp = RESULT_I2C_NAK_READ;
                }else {
                    len_rsp = rlen;
                }
            }else {
                cmd_rsp = RESULT_I2C_OK_WITHOUT_R;
            }
        }

        if (ret == ERR_NONE)
            break;

        //TBD: here need a retry delay

    }while(ret != ERR_NONE && retry-- > 0);

    if (readlen)
        *readlen = len_rsp;

    if (ecode)
        *ecode = cmd_rsp;

    return ret == ERR_IO ? ERR_IO : ERR_NONE;
}

/*
    Scan first i2c devices on bus
    @dbc: device bus controller handle
    return i2c device addr if get, else 0
*/
uint8_t iic_bus_ping(void *dbc, uint8_t addr)
{
    iic_controller_t *ihc = (iic_controller_t *)dbc;
    int32_t ret;

    uint8_t rdata[1];

    iic_board_set_address(ihc, addr);

    ret = iic_board_read(ihc, rdata, sizeof(rdata));
    if (ret == ERR_NONE) {
        return addr;
    }

    return 0;
}

/*
    Get bus max transfer size
    @dbc: device bus controller handle
    return size supported
*/
uint16_t iic_bus_trans_size(void *dbc, uint16_t size)
{
    if (size > MAX_IIC_PACKAGE_LENGTH)
        return MAX_IIC_PACKAGE_LENGTH;

    return size;
}

static iic_controller_t iic_bus_controller;

bus_interface_t i2c_interface = {
    .type = BUS_I2C,
    .cb_init = iic_board_init,
    .cb_deinit = iic_board_deinit,
    .cb_xfer = iic_bus_xfer_data,
    .cb_ping = iic_bus_ping,
    .cb_trans_size = iic_bus_trans_size,
    .sercom = SERCOM1,
    .dbc = &iic_bus_controller
};

