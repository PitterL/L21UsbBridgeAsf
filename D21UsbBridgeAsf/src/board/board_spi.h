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
    struct spi_slave_inst slave_inst;
    uint8_t unused;

}spi_controller_t;

int32_t spi_board_init(void *dbc, void *hw, uint32_t baudrate, uint8_t spimode);
int32_t spi_board_deinit(void *dbc);
int32_t spi_board_write(void *dbc, const uint8_t *const buf, const uint16_t length);
int32_t spi_board_read(void *dbc, uint8_t *const buf, const uint16_t length);

#endif /* BOARD_SPI_H_ */