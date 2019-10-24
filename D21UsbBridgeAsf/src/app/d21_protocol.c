/*
 * d21_protocol.c
 *
 * Created: 7/21/2018 11:25:03 AM
 *  Author: A41450
 */ 
#include <stddef.h>
#include <stdint.h>
#include <string.h>

//#include "../driver_init.h"
#include "crc.h"
#include "board/board.h"
#include "bus.h"
#include "external/utils.h"
#include "external/err_codes.h"

static int32_t enpack_d21_response(response_data_t *resp, uint8_t rdata0, uint8_t rdata1, const uint8_t *data, uint32_t count);

static int32_t get_bridge_io(void *host, uint8_t cmd, uint8_t bcmd, const uint8_t *data, uint32_t count)
{
    return ERR_NOT_READY;
}

static int32_t set_bridge_io(void *host, uint8_t cmd, uint8_t bcmd, const uint8_t *data, uint32_t count)
{
    return ERR_NOT_READY;
}

static int32_t send_bridge_data_bulk(void *host, uint8_t cmd, uint8_t bcmd, const uint8_t *data, uint32_t count)
{
    controller_t *hc = (controller_t *)host;
    config_setting_t * scfg = &hc->setting;
    response_data_t * resp = &hc->response;
    uint8_t *rdata = resp->rcache.data;
    transfer_data_t * trans = &hc->transfer;
    uint8_t * cdata = trans->ccache.data;
    bus_interface_t * intf = hc->intf;
    uint32_t size = sizeof(resp->rcache);
    uint16_t addr, lenr, read_size, read_size_max, len_rsp = 0;
    uint8_t cmd_rsp = OBP_DATA4_BULK_TRANSFER_COMPLETED;
    int32_t i, retry = scfg->base.data2.bits.iic_retry ? 3 : 0;
    int32_t ret;
	
    if (!intf || !intf->cb_xfer)
        return ERR_NOT_READY;

    if (count < 7)
        return ERR_INVALID_DATA;

    read_size = data[0] | (data[1] << 8);
    //data[2~4]: reserved 
    addr = data[5] | (data[6] << 8);

    //Need initialize bus before transfer data
    if (TEST_BIT(hc->flag, BIT_BUS_REINIT) || !TEST_BIT(hc->flag, BIT_BUS_INITED)) {
        ret = u5030_set_bridge_ext_config(hc, CMD_EXTENSION_CONFIG, (uint8_t *)&scfg->ext, sizeof(scfg->ext));
        if (ret != ERR_NONE)
            return ret;
    }

    read_size_max = size - 5;
    if (read_size > read_size_max)
        lenr = read_size_max;    //count may max than buffer size
    else
        lenr = read_size;
    
    for (i = 0; i < retry; i++ ) {
        ret = intf->cb_xfer(intf->dbc, (const uint8_t *)&addr, sizeof(addr), rdata + 5, lenr, &len_rsp, &cmd_rsp);
        if (ret == ERR_NONE) {
            read_size -= len_rsp;
            addr += len_rsp;
            if (read_size > 0) {    //Not finished, put command into command cache
                SET_BIT(hc->flag, BIT_BULK_CMD_READ_CONTINUE);
                cdata[0] = cmd;
                cdata[1] = bcmd;
                cdata[2] = read_size & 0xff;
                cdata[3] = (read_size >> 8) & 0xff;
                cdata[4] = data[2];
                cdata[5] = data[3];
                cdata[6] = data[4];
                cdata[7] = addr & 0xff;
                cdata[8] = (addr >> 8) & 0xff;
            }else {
                cmd_rsp = OBP_DATA4_BULK_TRANSFER_COMPLETED;
            }
            break;
        }
    }

    rdata[0] = cmd;
    rdata[1] = bcmd;
    rdata[2] = 0;
    rdata[3] = cmd_rsp;
    rdata[4] = len_rsp;
    resp->dirty = true;

    return ERR_NONE;
}

static int32_t get_bridge_adcs(void *host, uint8_t cmd, uint8_t bcmd, const uint8_t *data, uint32_t count)
{
    return ERR_NOT_READY;
}

static int32_t set_bridge_adc_cal(void *host, uint8_t cmd, uint8_t bcmd, const uint8_t *data, uint32_t count)
{
    return ERR_NOT_READY;
}

static int32_t get_bridge_adc_cal(void *host, uint8_t cmd, uint8_t bcmd, const uint8_t *data, uint32_t count)
{
    return ERR_NOT_READY;
}

struct pwr_param{
    void (*ctrl)(bool en);
    uint8_t delay;
};

static int32_t set_bridge_power(void *host, uint8_t cmd, uint8_t bcmd, const uint8_t *data, uint32_t count)
{
    controller_t *hc = (controller_t *)host;
    response_data_t * resp = &hc->response;
    uint8_t on;
    uint8_t delay_5v;
    uint8_t delay_3v3;
    uint8_t delay_io;
    struct pwr_param param[] = {{.ctrl=pwr_5v_en}, {.ctrl=pwr_3v3_en}, {.ctrl=pwr_vio_en}};
    uint32_t i;

    if (count < 4)
        return -ERR_INVALID_DATA;

    on = data[0];
    delay_5v = data[1];
    delay_3v3 = data[2];
    delay_io = data[3];

    //param[0].ctrl = pwr_5v_en;
    param[0].delay = delay_5v;
    //param[1].ctrl = pwr_3v3_en;
    param[1].delay = delay_3v3;
    //param[2].ctrl = pwr_vio_en;
    param[2].delay = delay_io;
    
    if (delay_5v || delay_3v3 || delay_io ) {
        //qsort(&param); //TBD need sort here
    }
            
    for (i = 0; i < ARRAY_SIZE(param); i++)
        param[i].ctrl(on);
    
    return enpack_d21_response(resp, cmd, 0, &on, sizeof(on));
}

static int32_t set_bridge_fw_profile(void *host, uint8_t cmd, uint8_t bcmd, const uint8_t *data, uint32_t count)
{
    //test4: cmd, bcmd, 0x00, 0x10, 0x01, 0x05, 0x05, 0x4, 0x3, 0x2, 0x1
    return ERR_UNSUPPORTED_OP;
}

static int32_t check_bridge_frame_ready(void *host, uint8_t cmd, uint8_t bcmd, const uint8_t *data, uint32_t count)
{
    //buf[2] & 0x80: CMD Error
    //buf[3] = 0x1: frame not ready within 100ms.
    return ERR_UNSUPPORTED_OP;
}

typedef int32_t (*func_do_cmd_d21) (void *host, uint8_t cmd,  uint8_t bcmd, const uint8_t *data, uint32_t count);
struct cmd_func_map{
    uint8_t bcmd;
    func_do_cmd_d21 func;
};

static struct cmd_func_map d21_command_func_map_list[] = {
    {CMD61_OBP, send_bridge_data_bulk},
    {CMD61_HID, NULL},
    {CMD61_I2CSCAN, NULL},
    {CMD61_GETIO, get_bridge_io},
    {CMD61_SETIO, set_bridge_io},
    {CMD61_GetBoardNo, NULL},
    {CMD61_SetBoardNo, NULL},
    {CMD61_T37_OBP, NULL},
    {CMD61_GetADC, get_bridge_adcs},
    {CMD61_CalibrateADC, set_bridge_adc_cal},
    {CMD61_GetCalibrate, get_bridge_adc_cal},
    {CMD61_SetPower, set_bridge_power},
    {CMD61_SETRAMDataOBP, NULL},
    {CMD61_SETRAMDataHID, NULL},
    {CMD61_GetFWProfile, NULL},
    {CMD61_SetFWProfile, set_bridge_fw_profile},
    {CMD61_SetFWData, NULL},
    {CMD61_StartFWUpg, NULL},
    {CMD61_IsFrameReady, check_bridge_frame_ready},
    {CMD61_UKNOWN, NULL},
};

static int32_t enpack_d21_response(response_data_t *resp, uint8_t rdata0, uint8_t rdata1, const uint8_t *data, uint32_t count)
{
    response_cache_t *cache = &resp->rcache;
    uint8_t *buf = cache->data;
    uint32_t buf_size = sizeof(cache->data);

    if (count + 2 > buf_size)
        return -ERR_NO_MEMORY;
    
    if (data != NULL) {   //If data is Null, that means resp has been filled
        buf[0] = rdata0;
        buf[1] = rdata1;
    }
    buf_size -= 2;
    buf += 2;

    if (data != NULL)    //If data is Null, that means data already be copied
        memcpy(buf, data, count);
    
    buf_size -= count;
    buf += count;
    
    if (buf_size)
        memset(buf, 0, buf_size);

    resp->dirty = true;

    return ERR_NONE;
}

static int32_t enpack_d21_response_nak(response_data_t *resp)
{
    uint8_t dummy[8];
    
    memset(dummy, 0xff, sizeof(dummy));
    return enpack_d21_response(resp, CMD_NAK, CMD_NAK, dummy, sizeof(dummy));
}

int32_t d21_parse_command(void *host, const uint8_t *data, uint32_t count)
{
    controller_t *hc = (controller_t *)host;
    response_data_t * resp = &hc->response;
    uint8_t cmd, bcmd;
    struct cmd_func_map *fn_map;
    int32_t result = ERR_NOT_FOUND;
    
    if (count < 2)
        return ERR_INVALID_ARG;
    
    cmd = data[0];
    bcmd = data[1];

    if (cmd != CMD_D21_EXTENTION_BULK_DATA)
        return result;

    for (uint32_t i = 0; i < ARRAY_SIZE(d21_command_func_map_list); i++)
    {
        fn_map = &d21_command_func_map_list[i];
        if (bcmd == fn_map->bcmd){                
            if (fn_map->func)
                result = fn_map->func(hc, cmd, bcmd, data + 2, count - 2);
            else
                result = ERR_UNSUPPORTED_OP;
            break;
        }
    }

    if (result != ERR_NONE && result != ERR_NOT_FOUND)
        result = enpack_d21_response_nak(resp);
        
    return result;
}

int32_t d21_init(void)
{
    return ERR_NONE;
}

void d21_deinit(void)
{

}