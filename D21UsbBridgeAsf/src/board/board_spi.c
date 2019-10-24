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
        //  SCK/PDA1, MOSI/PAD0, MISO/PAD3, SS/PAD2
        pad0 = PINMUX_PA16C_SERCOM1_PAD0;
        pad1 = PINMUX_PA17C_SERCOM1_PAD1;
        pad2 = PINMUX_PA18C_SERCOM1_PAD2;
        pad3 = PINMUX_PA19C_SERCOM1_PAD3;
        mux = SPI_SIGNAL_MUX_SETTING_O;
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

int32_t spi_bus_init(void *dbc, void *hw, uint32_t baudrate, uint8_t spimode)
{
    spi_controller_t *ihc = (spi_controller_t *)dbc;

    spi_get_config_defaults(&ihc->config);
    spi_set_config(hw, &ihc->config, baudrate, spimode);
    spi_init(&ihc->module, hw, &ihc->config);
    spi_enable(&ihc->module);

    return ERR_NONE;
}

int32_t spi_bus_deinit(void *dbc)
{
    spi_controller_t *ihc = (spi_controller_t *)dbc;

    spi_disable(&ihc->module);
	
    return ERR_NONE;
}

/**
 * \brief I/O write interface
 */
int32_t spi_bus_write(void *dbc, const uint8_t *const buf, const uint16_t length)
{
    spi_controller_t *ihc = (spi_controller_t *)dbc;
    enum status_code result;
       
    result = spi_write_buffer_wait(&ihc->module, buf, length);
    if (result == STATUS_OK)
        return ERR_NONE;
    else if (result == STATUS_ERR_BAD_ADDRESS)
        return ERR_BAD_ADDRESS;

    return ERR_ABORTED;
}

/**
 * \brief I/O read interface
 */
int32_t spi_bus_read(void *dbc, uint8_t *const buf, const uint16_t length)
{
    spi_controller_t *ihc = (spi_controller_t *)dbc;
    enum status_code result;
    
    result = spi_read_buffer_wait(&ihc->module, buf, length, 0);
    if (result == STATUS_OK)
        return ERR_NONE;
    else if (result == STATUS_ERR_BAD_ADDRESS)
        return ERR_BAD_ADDRESS;
    
    return ERR_ABORTED;
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
    uint8_t len_rsp;
    int32_t ret;
    int retry = 0;

    do {
        cmd_rsp = RESULT_SPI_OK;
        len_rsp = 0;
        ret = ERR_NONE;
        
        //write
        if (wlen) {
            ret = spi_bus_write(ihc, wdata, wlen);
            if(ret) {
                cmd_rsp = RESULT_SPI_TIMEOUT;;
            }
        }

        //read
        if (ret == ERR_NONE) {
            if (rlen) {
                ret = spi_bus_read(ihc, rdata, rlen);
                if(ret) {
                    cmd_rsp = RESULT_SPI_TIMEOUT;
                }else {
                    len_rsp = rlen;
                }
            }else {
                cmd_rsp = RESULT_SPI_OK;
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
    @dbc: device bus controller handle
    return i2c device addr if get
*/
uint8_t spi_bus_ping(void *dbc, uint8_t unused)
{
    spi_controller_t *ihc = (spi_controller_t *)dbc;
    int32_t ret;

    uint8_t rdata[1];

    ret = spi_bus_read(ihc, rdata, sizeof(rdata));
    if (ret == ERR_NONE) {
        return rdata[0];
    }

    return 0;
}