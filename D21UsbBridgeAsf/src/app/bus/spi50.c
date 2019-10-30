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
#include "app/crc.h"

/**
 * \brief I/O write interface
 */
static int32_t spi50_bus_write(void *dbc, const uint8_t *const buf, const uint16_t length)
{
    spi_controller_t *ihc = (spi_controller_t *)dbc;
    SPI_HEADER_PACKET_T *phead = (SPI_HEADER_PACKET_T *)buf;
    uint8_t crc;

    // Check CRC

    crc = crc8(phead, sizeof(*phead));
    if (crc)  {
         return ERR_INVALID_DATA;
    }

    // Wait bus idle or timeout
    bus_waiting_state(TIMEOUT_SPI_WAITING_BUS_STATE, false);

    return spi_board_write(&ihc->module, buf, length);
}

/**
 * \brief I/O read interface
 */
static int32_t spi50_bus_read(void *dbc, uint8_t *const buf, const uint16_t length)
{
    spi_controller_t *ihc = (spi_controller_t *)dbc;
    SPI_HEADER_PACKET_T *phead = (SPI_HEADER_PACKET_T *)buf;
    uint8_t crc;
    bool active;    

    int32_t result;

    // Wait bus active
    active = bus_waiting_state(TIMEOUT_SPI_WAITING_BUS_STATE, true);
    if (!active)
        return ERR_TIMEOUT;

    result = spi_board_read(&ihc->module, buf, length);
    if (result != ERR_NONE)
        return result;

    // Check CRC
    crc = crc8(phead, sizeof(*phead));
    if (crc || (phead->cmd != SPI_RESP_W_OK &&  phead->cmd != SPI_RESP_R_OK))  {
        return ERR_INVALID_DATA;
    }

    return ERR_NONE;
}


/*
    SPI bus transfer data
    @dbc: device bus controller handle
    @wdata: write buffer data pointer
    @wlen: write data length
    @rdata: read buffer data pointer
    @rlen: read buffer length
    @readlen: readout data length output pointer
    @ecode: bus status code
    return error code: ERR_NONE mean successful, otherwise failed
*/
static int32_t spi50_bus_xfer_data(void *dbc, const uint8_t *wdata, uint16_t wlen, uint8_t *rdata, uint16_t rlen, uint16_t *readlen, uint8_t *ecode)
{
    spi_controller_t *ihc = (spi_controller_t *)dbc;
    int8_t cmd_rsp;
    uint16_t len_rsp;
    int32_t ret;
    int retry = 5;

    do {
        cmd_rsp = RESULT_SPI_OK;
        len_rsp = 0;
        ret = ERR_NONE;
        
        //write
        if (wlen) {
            ret = spi50_bus_write(ihc, wdata, wlen);
        }

        //read
        if (ret == ERR_NONE) {
            if (rlen) {
                ret = spi50_bus_read(ihc, rdata, rlen);
                if (ret == ERR_NONE)
                    len_rsp = rlen;
            }
        }

        //TBD: here need a retry delay

    } while(ret != ERR_NONE && retry-- > 0);


    if (ret != ERR_NONE) {
        cmd_rsp = RESULT_SPI_TIMEOUT;
    }

    if (readlen)
        *readlen = len_rsp;

    if (ecode)
        *ecode = cmd_rsp;

    return ret;
}

static spi_controller_t spi50_bus_controller;

bus_interface_t spi50_interface = {
    .type = BUS_SPI50,
    .cb_init = spi_board_init,
    .cb_deinit = spi_board_deinit,
    .cb_xfer = spi50_bus_xfer_data,
    .cb_ping = spi_bus_ping,
    .cb_trans_size = spi_bus_trans_size,
    .sercom = SERCOM1,
    .dbc = &spi50_bus_controller
};