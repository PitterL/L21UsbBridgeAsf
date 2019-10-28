/*
 * common.h
 *
 * Created: 7/21/2018 5:19:22 PM
 *  Author: A41450
 */ 

#ifndef BUS_SPI_H_
#define BUS_SPI_H_

#define MAX_SPI_PACKAGE_LENGTH 64
#define TIMEOUT_SPI_WAITING_BUS_STATE 500 //us

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

//Below value should match protocol
enum {RESULT_SPI_OK, RESULT_SPI_RSV0, RESULT_SPI_RSV1, RESULT_SPI_TIMEOUT};

int32_t spi_bus_xfer_data(void *dbc, const uint8_t *wdata, uint16_t wlen, uint8_t *rdata, uint16_t rlen, uint16_t *readlen, uint8_t *ecode);
uint8_t spi_bus_ping(void *dbc, uint8_t unused);

#endif