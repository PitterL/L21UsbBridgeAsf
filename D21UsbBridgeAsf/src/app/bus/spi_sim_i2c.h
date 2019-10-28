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

//Below value should match protocol
enum {RESULT_SPI_OK, RESULT_SPI_RSV0, RESULT_SPI_RSV1, RESULT_SPI_TIMEOUT};

int32_t spi_bus_xfer_data(void *dbc, const uint8_t *wdata, uint16_t wlen, uint8_t *rdata, uint16_t rlen, uint16_t *readlen, uint8_t *ecode);
uint8_t spi_bus_ping(void *dbc, uint8_t unused);

#endif