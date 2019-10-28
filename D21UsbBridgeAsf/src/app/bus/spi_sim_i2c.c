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
#include "spi.h"


typedef struct SPI_HEADER_PACKET {
    uint8_t cmd;
    /*
    union {
        struct {
            uint8_t lo;
            uint8_t hi;
        };
        uint16_t value;
    } addr;
    */
    uint16_t addr;
    uint16_t len;
    uint8_t crc;
}__attribute__((packed, aligned(1))) SPI_HEADER_PACKET_T;

#define SPI_CMD_WRITE 0x01
#define SPI_CMD_READ 0x02

#define SPI_RESP_W_OK 0x81
#define SPI_RESP_W_FAILED 0x41
#define SPI_RESP_R_OK 0x82
#define SPI_RESP_R_FAILED 0x42
#define SPI_INVALID_REQ 0x4
#define SPI_INVALID_CRC 0x8

typedef struct SPI_DATA_PACKET {
    SPI_HEADER_PACKET_T header;
    uint8_t buf[MAX_SPI_PACKAGE_LENGTH];
}__attribute__((packed, aligned(1))) SPI_DATA_PACKET_T;


/**
 * \brief I/O write interface
 */
int32_t spi_bus_write(void *dbc, uint16_t addr, const uint8_t *const buf, const uint16_t length)
{
    spi_controller_t *ihc = (spi_controller_t *)dbc;
    int16_t i;
    uint16_t size;
    uint8_t crc;
    int32_t result;
    int32_t retry = 1;

    SPI_DATA_PACKET_T packet;

    for ( i = 0; i < length; ) {
        // wait bus idle or timeout
        bus_waiting_state(TIMEOUT_SPI_WAITING_BUS_STATE, false);

        size = length - i;
        if (size > sizeof(packet.buf))
            size = sizeof(packet.buf);

        memcpy(packet.buf, buf + i, size);
        packet.header.cmd = SPI_CMD_WRITE;
        packet.header.len = size;
        packet.header.addr = addr + i;
        packet.header.crc = crc8(&packet.header, offsetof(SPI_HEADER_PACKET_T, crc));

        result = spi_board_write(&ihc->module, (uint8_t *)&packet, sizeof(packet.header) + size);
        if (result != ERR_NONE)
            break;

        // wait for chg status to get ACK
        if (!bus_waiting_state(TIMEOUT_SPI_WAITING_BUS_STATE, true)) {
            result= ERR_TIMEOUT;
            break;
        }

        result = spi_board_read(&ihc->module, (uint8_t *)&packet.header, sizeof(packet.header));
        if (result != ERR_NONE)
            break;

        crc = crc8(&packet.header, sizeof(packet.header));

        if (crc || packet.header.cmd != SPI_RESP_W_OK)  {
            if (--retry) {
                continue;
            } else {
                result = ERR_INVALID_DATA;
                break;
            }
        }
        
        i += size;
    }

    return result;
}

/**
 * \brief I/O read interface
 */
int32_t spi_bus_read(void *dbc, uint16_t addr, uint8_t *const buf, const uint16_t length)
{
    spi_controller_t *ihc = (spi_controller_t *)dbc;
    int16_t i;
    uint16_t size;
    uint8_t crc;
    int32_t result;
    int32_t retry = 0;

    SPI_DATA_PACKET_T packet;

    for ( i = 0; i < length; ) {
        // wait bus idle or timeout
        bus_waiting_state(TIMEOUT_SPI_WAITING_BUS_STATE, false);

        size = length - i;
        if (size > sizeof(packet.buf))
            size = sizeof(packet.buf);

        packet.header.cmd = SPI_CMD_READ;
        packet.header.len = size;
        packet.header.addr = addr + i;
        packet.header.crc = crc8(&packet.header, offsetof(SPI_HEADER_PACKET_T, crc));

        result = spi_board_write(&ihc->module, (uint8_t *)&packet.header, sizeof(packet.header));
        if (result != ERR_NONE)
            break;

        // wait for chg status to get ACK
        if (!bus_waiting_state(TIMEOUT_SPI_WAITING_BUS_STATE, true)) {
            result= ERR_TIMEOUT;
            break;
        }

        result = spi_board_read(&ihc->module, (uint8_t *)&packet.header, sizeof(packet.header) + size);
        if (result != ERR_NONE)
            break;

        crc = crc8(&packet.header, sizeof(packet.header));

        if (crc || packet.header.cmd != SPI_RESP_R_OK)  {
            if (retry--) {
                continue;
            } else {
                result = ERR_INVALID_DATA;
                break;
            }
        }

        memcpy(buf + i, packet.buf, size);
        i += size;
    }

    return result;
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
int32_t spi_bus_xfer_data(void *dbc, const uint8_t *wdata, uint16_t wlen, uint8_t *rdata, uint16_t rlen, uint16_t *readlen, uint8_t *ecode)
{
    spi_controller_t *ihc = (spi_controller_t *)dbc;
    int8_t cmd_rsp;
    uint16_t address, len_rsp;
    int32_t ret;
    int retry = 0;

    if (wlen < 2) {
        address = 0;    //Default set address zero
        wlen = 0;
    } else {
        address = wdata[0] | (uint16_t)(wdata[1] << 8);
        wlen -= 2;
        wdata += 2;
    }

    do {
        cmd_rsp = RESULT_SPI_OK;
        len_rsp = 0;
        ret = ERR_NONE;
        
        //write
        if (wlen) {
            ret = spi_bus_write(ihc, address, wdata, wlen);
        }

        //read
        if (ret == ERR_NONE) {
            if (rlen) {
                ret = spi_bus_read(ihc, address, rdata, rlen);
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

/*
    Scan first i2c devices on bus
    @dbc: device bus controller handle
    return i2c device addr if get
*/
uint8_t spi_bus_ping(void *dbc, uint8_t unused)
{
    spi_controller_t *ihc = (spi_controller_t *)dbc;
    int32_t ret;

    uint8_t rdata[1];

    ret = spi_bus_read(ihc, 0, (uint8_t *)rdata, sizeof(rdata));
    if (ret == ERR_NONE) {
        return rdata[0];
    }

    return 0;
}


static spi_controller_t spi_bus_controller;

bus_interface_t spi_interface = {
    .cb_init = spi_board_init,
    .cb_deinit = spi_board_deinit,
    .cb_xfer = spi_bus_xfer_data,
    .cb_ping = spi_bus_ping,
    .sercom = SERCOM1,
    .dbc = &spi_bus_controller
};