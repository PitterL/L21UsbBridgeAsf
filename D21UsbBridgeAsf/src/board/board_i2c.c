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

int32_t iic_set_config(void *hw, struct i2c_master_config *const cfg, uint32_t baudrate)
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

void iic_board_set_address(void *dbc, uint8_t addr)
{
    iic_controller_t *ihc = (iic_controller_t *)dbc;

    ihc->addr = addr;
}

int32_t iic_board_init(void *dbc, void *hw, uint32_t baudrate, uint8_t addr)
{
    iic_controller_t *ihc = (iic_controller_t *)dbc;

    i2c_master_get_config_defaults(&ihc->config);
    iic_set_config(hw, &ihc->config, baudrate);
    i2c_master_init(&ihc->module, hw, &ihc->config);
    i2c_master_enable(&ihc->module);
    iic_board_set_address(ihc, addr);

    return ERR_NONE;
}

int32_t iic_board_deinit(void *dbc)
{
    iic_controller_t *ihc = (iic_controller_t *)dbc;
	i2c_master_disable(&ihc->module);
	
    return ERR_NONE;
}

/**
 * \brief I/O write interface
 */
int32_t iic_board_write(void *dbc, const uint8_t *const buf, const uint16_t length)
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
int32_t iic_board_read(void *dbc, uint8_t *const buf, const uint16_t length)
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