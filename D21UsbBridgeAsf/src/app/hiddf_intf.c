/*
 * hiddf_intf.c
 *
 * Created: 6/20/2018 6:57:50 PM
 *  Author: A41450
 */ 
#include <stddef.h>
#include <stdint.h>
#include <string.h>

//#include "../driver_init.h"
#include "crc.h"
#include "board/board.h"
#include "common.h"
#include "external/utils.h"
#include "external/err_codes.h"
#include "hiddf_intf.h"

static controller_t g_host_controller;

extern int32_t enpack_response_nak(response_data_t *resp);
void hiddf_intf_receive(uint8_t *data, uint32_t size)
{
    controller_t *hc = &g_host_controller;
    response_data_t *resp = &hc->response;
    int32_t result;

    if (size < 1)
        return;

    //clear auto repeat tag
    CLR_BIT(hc->flag, BIT_AUTO_REPEAT);
    //clear continue to send tag
    CLR_BIT(hc->flag, BIT_BULK_CMD_READ_CONTINUE);

    result = u5030_parse_command(hc, data, size);
    if (result == ERR_NOT_FOUND) {
        result = d21_parse_command(hc, data, size);
    }

    if (result != ERR_NONE){
        enpack_response_nak(resp);
    }
}

extern int32_t transfer_bridge_data(void *host, uint8_t cmd, const uint8_t *data, uint32_t count);
int32_t hiddf_intf_get_data(void **buf_ptr, uint32_t *buf_ptr_size)
{
    controller_t *hc = &g_host_controller;
    response_data_t *resp = &hc->response;
    response_cache_t *rcache = &resp->rcache;
    command_cache_t *ccache = &hc->transfer.ccache;

    config_setting_t * scfg = &hc->setting;
    const uint8_t *data;
    uint32_t count;

    if (!resp->dirty) {
        if (TEST_BIT(hc->flag, BIT_BULK_CMD_READ_CONTINUE)) {
            hiddf_intf_receive(ccache->data, sizeof(ccache->data));
        }else if (TEST_BIT(hc->flag, BIT_AUTO_REPEAT)) {
            if (u5030_chg_line_active(hc)) {
                if (scfg->dym.repeat.cfg.bits.bus == REPEAT_BUS_SPI_UART) {
                        ;//TODO
                }else if (scfg->dym.repeat.cfg.bits.bus == REPEAT_BUS_IIC1) {
                    data = (uint8_t *)&scfg->dym.repeat.lenw;
                    count = scfg->dym.repeat.lenw + 2;
                    transfer_bridge_data(hc, CMD_AUTO_REPEAT_RESP, data, count);
                }else if (scfg->dym.repeat.cfg.bits.bus == REPEAT_BUS_IIC2) {
                    //TODO
                }
            }
        }
    }

    if (resp->dirty) {
        if(buf_ptr)
            *buf_ptr = (void *)&rcache->data;
    
        if(buf_ptr_size)
            *buf_ptr_size = sizeof(rcache->data);
        
        return ERR_NONE;
    }
    
    return -ERR_NOT_READY;
}

int32_t hiddf_intf_clear_cache(void)
{
    controller_t *hc = &g_host_controller;
    response_data_t *resp = &hc->response;
    
    resp->dirty = false;
    memset(&resp->rcache, 0, sizeof(resp->rcache));
    
    return ERR_NONE;
}

void hiddf_intf_send(cb_hiddf_func_data_send cb_send)
{
    uint8_t *buf;
    uint32_t count;
    int32_t ret;
    
    ret = hiddf_intf_get_data((void **)&buf, &count);
    if (ret == ERR_NONE) {
        cb_send(buf);
        if (ret != ERR_NONE)
          ; //TODO: should do something
        hiddf_intf_clear_cache();
    }

    if (ret != ERR_NONE){
        ;  //TODO: how to handle error
    }
}

static int32_t load_config_from_flash(config_setting_t * scfg)
{
    return -ERR_DENIED;
}

static void load_default_config(config_setting_t *scfg)
{
    if (load_config_from_flash(scfg) == ERR_NONE)
        return;

    memset(scfg, 0, sizeof(*scfg));

    scfg->base.data1.bits.iic_clk = IIC_CLK_400KHZ;
    scfg->base.data2.bits.iic_retry = IIC_RETRY_ON;
    scfg->base.data2.bits.iic1_addr = 0x4A;
    scfg->base.data4.bits.iic_restart = IIC_RESTART_DISABLE;
    scfg->base.data4.bits.uart_brate = UART_BAUDRATE_115200;
    scfg->base.data5.bits.comms_delay = 0;
    scfg->base.data6.bits.repeat_delay_multiplier = 10;
    scfg->base.data7.bits.iic_retry_delay = 200;
    scfg->base.data8.bits.chg_mode = ENABLE_GPIO_AS_CHANGE_GPIO;
    scfg->base.data8.bits.chg_active = ACTIVE_LEVEL_LOW;
    scfg->base.data8.bits.chg_gpio = GPIO_PIN_1;

    scfg->ext.data1.bits.com_mode = COM_MODE_IIC_ONLY;
    scfg->ext.data1.bits.checksum = ~scfg->ext.data1.bits.com_mode;

    scfg->crc.value = crc24((uint8_t *)scfg, offsetof(config_setting_t, crc));
}

int32_t hiddf_intf_init(void)
{
    controller_t *hc = &g_host_controller;

    platform_board_init();
    load_default_config(&hc->setting);

    u5030_init();
    d21_init();

    return ERR_NONE;
}

void hiddf_intf_deinit(void)
{
    d21_deinit();
    u5030_deinit();    
}
