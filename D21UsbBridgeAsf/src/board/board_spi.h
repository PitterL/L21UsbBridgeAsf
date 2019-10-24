/*
 * board_spi.h
 *
 * Created: 10/16/2019 2:08:11 PM
 *  Author: A41450
 */ 


#ifndef BOARD_SPI_H_
#define BOARD_SPI_H_

#include <spi.h>

typedef struct{
    struct spi_module module;
    struct spi_config config;
    uint8_t unused;
}spi_controller_t;

//Below value should match protocol
enum {RESULT_SPI_OK, RESULT_SPI_RSV0, RESULT_SPI_RSV1, RESULT_SPI_TIMEOUT};

int32_t spi_bus_init(void *dbc, void *hw, uint32_t baudrate, uint8_t addr);
int32_t spi_bus_deinit(void *dbc);
int32_t spi_bus_write(void *dbc, const uint8_t *const buf, const uint16_t length);
int32_t spi_bus_read(void *dbc, uint8_t *const buf, const uint16_t length);
int32_t spi_bus_xfer_data(void *dbc, const uint8_t *wdata, uint16_t wlen, uint8_t *rdata, uint16_t rlen, uint16_t *readlen, uint8_t *ecode);
uint8_t spi_bus_ping(void *dbc, uint8_t unused);

#endif /* BOARD_SPI_H_ */