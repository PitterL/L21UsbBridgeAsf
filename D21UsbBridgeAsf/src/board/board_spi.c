/*
 * board.c
 *
 * Created: 7/3/2018 9:55:53 AM
 *  Author: A41450
 */ 

#include <status_codes.h>
#include <string.h>
#include "board.h"
#include "external/err_codes.h"

int32_t spi_set_config(void *hw, struct spi_config *const cfg, uint32_t baudrate, uint8_t spimode)
{
    uint32_t pad0, pad1, pad2, pad3;
    enum spi_signal_mux_setting mux;
    enum spi_transfer_mode mode;

    switch((uint32_t)hw){
    case (uint32_t)SERCOM0:
        /*
        pad0 = PINMUX_PA04D_SERCOM0_PAD0;
        pad1 = PINMUX_PA05D_SERCOM0_PAD1;
        pad2 = PINMUX_PA06D_SERCOM0_PAD2;
        pad3 = PINMUX_PA07D_SERCOM0_PAD3;
        break;*/
    case (uint32_t)SERCOM1:
        //  MUX D: SCK/PDA1, MOSI/PAD0, MISO/PAD3, SS/PAD2
        pad0 = PINMUX_PA16C_SERCOM1_PAD0;
        pad1 = PINMUX_PA17C_SERCOM1_PAD1;
        pad2 = PINMUX_PA18C_SERCOM1_PAD2;
        pad3 = PINMUX_PA19C_SERCOM1_PAD3;
        mux = SPI_SIGNAL_MUX_SETTING_D;
    break;
    default:
      break;
    }

    cfg->pinmux_pad0 = pad0;
    cfg->pinmux_pad1 = pad1;
	cfg->pinmux_pad2 = pad2;
	cfg->pinmux_pad3 = pad3;
	
    switch(spimode) {
        case 0:
            mode = SPI_TRANSFER_MODE_0;
        case 1:
            mode = SPI_TRANSFER_MODE_1;
        case 2:
            mode = SPI_TRANSFER_MODE_2;
        case 3:
            mode = SPI_TRANSFER_MODE_3;
    }

	cfg->transfer_mode = mode;
	cfg->mux_setting = mux;
    if (baudrate)
	    cfg->mode_specific.master.baudrate = baudrate;

    return ERR_NONE;
}

int32_t spi_board_init(void *dbc, void *hw, uint32_t baudrate, uint8_t spimode)
{
    spi_controller_t *ihc = (spi_controller_t *)dbc;

    spi_get_config_defaults(&ihc->config);
    spi_set_config(hw, &ihc->config, baudrate, spimode);
    spi_init(&ihc->module, hw, &ihc->config);
    spi_enable(&ihc->module);

    return ERR_NONE;
}

int32_t spi_board_deinit(void *dbc)
{
    spi_controller_t *ihc = (spi_controller_t *)dbc;

    spi_disable(&ihc->module);
	
    return ERR_NONE;
}

/**
 * \brief I/O write interface
 */
int32_t spi_board_write(void *dbc, const uint8_t *const buf, const uint16_t length)
{
    spi_controller_t *ihc = (spi_controller_t *)dbc;
    enum status_code result;

    result = spi_write_buffer_wait(&ihc->module, buf, length);
    if (result == STATUS_OK)
        return ERR_NONE;
    else
        return ERR_ABORTED;
}

/**
 * \brief I/O read interface
 */
int32_t spi_board_read(void *dbc, uint8_t *const buf, const uint16_t length)
{
    spi_controller_t *ihc = (spi_controller_t *)dbc;
    enum status_code result;

    result = spi_read_buffer_wait(&ihc->module, buf, length, 0xFFFF); //Transfer 0xFF for protocol MOSI line
    if (result == STATUS_OK)
        return ERR_NONE;
    else
        return ERR_ABORTED;
}