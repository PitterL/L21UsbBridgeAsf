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
#include "common.h"
#include "external/utils.h"
#include "external/err_codes.h"

/*
    I2c bus transfer data
    @host: device controller handle
    @wdata: write buffer data pointer
    @wlen: write data length
    @rdata: read buffer data pointer
    @rlen: raad buffer length
    @readlen: readout data length output pointer
    @ecode: bus status code
    return error code: ERR_NONE mean successful, otherwist failed
*/
int32_t i2c_transfer_data(void *host, const uint8_t *wdata, uint16_t wlen, uint8_t *rdata, uint16_t rlen, uint16_t *readlen, uint8_t *ecode)
{
    controller_t *hc = (controller_t *)host;
    int8_t cmd_rsp;
    uint8_t len_rsp;
    int32_t ret;
    int retry = 0;

    do {
        cmd_rsp = IIC_DATA_OK;
        len_rsp = 0;
        ret = ERR_NONE;
        
        //write
        if (wlen) {
            ret = iic_write(&hc->iic, wdata, wlen);
            if (ret == ERR_BAD_ADDRESS) {
                cmd_rsp = IIC_DATA_NAK_ADDR;
            }else if(ret) {
                cmd_rsp = IIC_DATA_NAK_WRITE;
            }
        }

        //read
        if (ret == ERR_NONE) {
            if (rlen) {
                ret = iic_read(&hc->iic, rdata, rlen);
                if (ret == ERR_BAD_ADDRESS) {
                    cmd_rsp = IIC_DATA_NAK_ADDR;
                }else if(ret) {
                    cmd_rsp = IIC_DATA_NAK_READ;
                }else {
                    len_rsp = rlen;
                }
            }else {
                cmd_rsp = IIC_DATA_FINISHED_WITHOUT_R;
            }
        }

        if (ret == ERR_NONE)
            break;
        retry--;
        //TBD: here need a retry delay

    }while(ret != ERR_NONE && retry > 0);

    if (readlen)
        *readlen = len_rsp;

    if (ecode)
        *ecode = cmd_rsp;

    return ret;
}

/*
    Scan first i2c devices on bus
    @host: device controller handle
    return i2c device addr if get
*/
uint8_t i2c_ping(void *host, uint8_t addr)
{
    controller_t *hc = (controller_t *)host;
    int32_t ret;

    uint8_t rdata[1];

    iic_set_address(&hc->iic, addr);

    ret = iic_read(&hc->iic, rdata, sizeof(rdata));
    if (ret == ERR_NONE) {
        return addr;
    }

    return 0;
}