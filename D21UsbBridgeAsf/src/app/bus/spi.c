/*
 * spi.c
 *
 * Created: 10/28/2019 12:14:02 PM
 *  Author: A41450
 */ 

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "board/board.h"
#include "external/utils.h"
#include "external/err_codes.h"
#include "app/bus.h"
#include "spi.h"

/*
    Get bus max transfer size
    @dbc: device bus controller handle
    return size supported
*/
uint16_t spi_bus_trans_size(void *dbc, uint16_t size)
{
    if (size > MAX_SPI_PACKAGE_LENGTH)
        return MAX_SPI_PACKAGE_LENGTH;

    return size;
}

/*
    Scan spi devices on bus
    @dbc: device bus controller handle
    return (dummy) address if get
*/
uint8_t spi_bus_ping(void *dbc, uint8_t unused)
{

    spi_controller_t *ihc = (spi_controller_t *)dbc;
    int32_t ret;

    uint8_t rdata[1];

    ret = spi51_bus_read(ihc, 0, (uint8_t *)rdata, sizeof(rdata));
    if (ret == ERR_NONE) {
        return unused;
    }

    return 0;
}