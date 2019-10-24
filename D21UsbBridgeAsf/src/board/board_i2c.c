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

int32_t iic_master_set_config(void *hw, struct i2c_master_config *const cfg, uint32_t baudrate)
{
    uint32_t pad0, pad1;

    //PAD0: SDA; PAD1: SCL
    switch((uint32_t)hw){
    case (uint32_t)SERCOM0:
      pad0 = PINMUX_PA04D_SERCOM0_PAD0;
      pad1 = PINMUX_PA07D_SERCOM0_PAD3;
      break;
    case (uint32_t)SERCOM4:
      pad0    = PINMUX_PA12D_SERCOM4_PAD0;
      pad1    = PINMUX_PA13D_SERCOM4_PAD1;
      break;
    case (uint32_t)SERCOM3:
      pad0 = PINMUX_PA16D_SERCOM3_PAD0;
      pad1 = PINMUX_PA17D_SERCOM3_PAD1;
      break;
    case (uint32_t)SERCOM1:
    default:
      pad0 = PINMUX_PA16C_SERCOM1_PAD0;
      pad1 = PINMUX_PA17C_SERCOM1_PAD1;
      break;
    }

    cfg->pinmux_pad0 = pad0;
    cfg->pinmux_pad1 = pad1;
    cfg->baud_rate = baudrate;

    return ERR_NONE;
}

void iic_bus_set_address(void *dbc, uint8_t addr)
{
    iic_controller_t *ihc = (iic_controller_t *)dbc;

    ihc->addr = addr;
}

int32_t iic_bus_init(void *dbc, void *hw, uint32_t baudrate, uint8_t addr)
{
    iic_controller_t *ihc = (iic_controller_t *)dbc;

    i2c_master_get_config_defaults(&ihc->config);
    iic_master_set_config(hw, &ihc->config, baudrate);
    i2c_master_init(&ihc->module, hw, &ihc->config);
    i2c_master_enable(&ihc->module);
    iic_bus_set_address(ihc, addr);

    return ERR_NONE;
}

int32_t iic_bus_deinit(void *dbc)
{
    iic_controller_t *ihc = (iic_controller_t *)dbc;
	i2c_master_disable(&ihc->module);
	
    return ERR_NONE;
}

/**
 * \brief I/O write interface
 */
int32_t iic_bus_write(void *dbc, const uint8_t *const buf, const uint16_t length)
{
    iic_controller_t *ihc = (iic_controller_t *)dbc;
    struct i2c_master_packet_w pkg;
    enum status_code result;
    
    memset(&pkg, 0, sizeof(pkg));
    pkg.address = ihc->addr;
    pkg.data_length = length;
    pkg.data = buf;
    
    result = i2c_master_write_packet_wait(&ihc->module, (struct i2c_master_packet *)&pkg);
    if (result == STATUS_OK)
        return ERR_NONE;
    else if (result == STATUS_ERR_BAD_ADDRESS)
        return ERR_BAD_ADDRESS;

    return ERR_ABORTED;
}

/**
 * \brief I/O read interface
 */
int32_t iic_bus_read(void *dbc, uint8_t *const buf, const uint16_t length)
{
    iic_controller_t *ihc = (iic_controller_t *)dbc;
    struct i2c_master_packet pkg;
    enum status_code result;
    
    memset(&pkg, 0, sizeof(pkg));
    pkg.address = ihc->addr;
    pkg.data_length = length;
    pkg.data = buf;
    
    result = i2c_master_read_packet_wait(&ihc->module, &pkg);
    if (result == STATUS_OK)
        return ERR_NONE;
    else if (result == STATUS_ERR_BAD_ADDRESS)
        return ERR_BAD_ADDRESS;
    
    return ERR_ABORTED;
}

/*
    I2c bus transfer data
    @dbc: device bus controller handle
    @wdata: write buffer data pointer
    @wlen: write data length
    @rdata: read buffer data pointer
    @rlen: read buffer length
    @readlen: readout data length output pointer
    @ecode: bus status code
    return error code: ERR_NONE mean successful, otherwise failed
*/
int32_t iic_bus_xfer_data(void *dbc, const uint8_t *wdata, uint16_t wlen, uint8_t *rdata, uint16_t rlen, uint16_t *readlen, uint8_t *ecode)
{
    iic_controller_t *ihc = (iic_controller_t *)dbc;
    int8_t cmd_rsp;
    uint8_t len_rsp;
    int32_t ret;
    int retry = 0;

    do {
        cmd_rsp = RESULT_I2C_OK;
        len_rsp = 0;
        ret = ERR_NONE;
        
        //write
        if (wlen) {
            ret = iic_bus_write(ihc, wdata, wlen);
            if (ret == ERR_BAD_ADDRESS) {
                cmd_rsp = RESULT_I2C_NAK_ADDR;
            }else if(ret) {
                cmd_rsp = RESULT_I2C_NAK_WRITE;
            }
        }

        //read
        if (ret == ERR_NONE) {
            if (rlen) {
                ret = iic_bus_read(ihc, rdata, rlen);
                if (ret == ERR_BAD_ADDRESS) {
                    cmd_rsp = RESULT_I2C_NAK_ADDR;
                }else if(ret) {
                    cmd_rsp = RESULT_I2C_NAK_READ;
                }else {
                    len_rsp = rlen;
                }
            }else {
                cmd_rsp = RESULT_I2C_OK_WITHOUT_R;
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
uint8_t iic_bus_ping(void *dbc, uint8_t addr)
{
    iic_controller_t *ihc = (iic_controller_t *)dbc;
    int32_t ret;

    uint8_t rdata[1];

    iic_bus_set_address(ihc, addr);

    ret = iic_bus_read(ihc, rdata, sizeof(rdata));
    if (ret == ERR_NONE) {
        return addr;
    }

    return 0;
}