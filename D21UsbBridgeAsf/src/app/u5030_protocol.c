/*
 * protocol_u5030.c
 *
 * Created: 6/20/2018 6:57:50 PM
 *  Author: A41450
 */ 
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <delay.h>

//#include "../driver_init.h"
#include "crc.h"
#include "board/board.h"
#include "bus.h"
#include "external/utils.h"
#include "external/err_codes.h"

static int32_t enpack_response(response_data_t *resp, uint8_t rdata0, const uint8_t *data, uint32_t count);
static void enpack_response_directly(response_data_t *resp);
static uint16_t get_response_cache(response_data_t *resp, void **out_ptr);

static int32_t set_bridge_config(void *host, uint8_t cmd, const uint8_t *data, uint32_t count)
{
    controller_t *hc = (controller_t *)host;
    config_setting_t *scfg = &hc->setting;
    response_data_t *resp = &hc->response;

    uint32_t cfg_size;
    int32_t ret;

    cfg_size = Min(sizeof(scfg->base), count);

    if (cfg_size) {
        if (memcmp(&scfg->base, data, cfg_size)) {
            memcpy(&scfg->base, data, cfg_size);
            ret = u5030_set_bridge_ext_config(hc, cmd, NULL, 0);
            if (ret)
                return ret;
        }
    }
    
    //TBD: Bulk mode test command
    //{ 0x80, 0x30, 0xCA, 0x00, 0x80, 0x00, 0x00, 0xC8, 0x40, 0x54, 0x00, 0x00, 0xAA, 0x55, 0xAA, 0xFF };

    return enpack_response(resp, cmd, data, cfg_size);
}

int32_t u5030_set_bridge_ext_config(void *host, uint8_t cmd, const uint8_t *data, uint32_t count)
{
    controller_t *hc = (controller_t *)host;
    config_setting_t *scfg = &hc->setting;
    response_data_t *resp = &hc->response;    
    uint8_t checksum;
    uint8_t mode;
    uint32_t cfg_size;
    uint32_t baudrate;
    uint8_t edata;
    BUS_TYPE_T btype = BUS_I2C;
    int32_t ret;

    if (count)    {   
        checksum = DESC_GET(data[0], EXT_CONFIG_DATA1_COMMUNICATION_MODE_CHECKSUM);
        mode = DESC_GET(data[0], EXT_CONFIG_COMMUNICATION_MODE);
        if (checksum && mode && (checksum + mode != 0xFF))
            return -ERR_INVALID_DATA;

        cfg_size = Min(sizeof(*scfg), count);
        memcpy(scfg, data, cfg_size);
    }

    if (scfg->ext.data1.bits.com_mode == COM_MODE_IIC_ONLY) {
        switch(scfg->base.data1.bits.iic_clk) {
        case IIC_CLK_50KHZ:
            // FIXME: Some issue work for 50Khz 
            /*
            baudrate = 50;
            break;*/
        case IIC_CLK_100KHZ:
            baudrate = I2C_MASTER_BAUD_RATE_100KHZ;
            break;
        case IIC_CLK_200KHZ:
            baudrate = 200;
            break;
        case IIC_CLK_400KHZ:
        default:
            baudrate = I2C_MASTER_BAUD_RATE_400KHZ;
        }
        
        edata = scfg->base.data2.bits.iic1_addr;
        btype = BUS_I2C;

    } else if (scfg->ext.data1.bits.com_mode == COM_MODE_SPI50) {
        switch(scfg->base.data1.bits.spi_clk) {
        case SPI_CLK_25KHZ:
            baudrate = 25000;
            break;
        case SPI_CLK_50KHZ:
            baudrate = 50000;
            break;
        case SPI_CLK_100KHZ:
            baudrate = 100000;
            break;
        case SPI_CLK_200KHZ:
            baudrate = 200000;
            break;
        case SPI_CLK_500KHZ:
            baudrate = 500000;
            break;
        case SPI_CLK_1MHZ:
            baudrate = 1000000;
            break;
        case SPI_CLK_2MHZ:
            baudrate = 2000000;
            break;
        case SPI_CLK_4MHZ:
        default:
            baudrate = 4000000;
        }

        switch (scfg->base.data4.bits.spi_mode) {
        case SPI_MODE_0:
            edata = 0;
            break;
        case SPI_MODE_1:
            edata = 1;
            break;
        case SPI_MODE_2:
            edata = 2;
            break;
        case SPI_MODE_3:
        default:
            edata = 3;
        }

        btype = BUS_SPI50;
    } else if (scfg->ext.data1.bits.com_mode == COM_MODE_SPI51) {
        // This is simulate 
        baudrate = 1000000;  //SPI baudrate rate
        edata = 3;  //SPI mode 3
        btype = BUS_SPI51;
    } else {
        return ERR_UNSUPPORTED_DEV;
    }

    if (hc->intf && hc->intf->cb_deinit) {
        if (TEST_BIT(hc->flag, BIT_BUS_INITED)) {
            CLR_BIT(hc->flag, BIT_BUS_INITED);
           hc->intf->cb_deinit(hc->intf->dbc);
        }
    }

    bus_attach(hc, btype);
    if (!hc->intf || !hc->intf->cb_init)
        return ERR_NOT_FOUND;

    ret = hc->intf->cb_init(hc->intf->dbc, hc->intf->sercom, baudrate, edata);
    if (ret != ERR_NONE)
        return ERR_NOT_INITIALIZED;

    //clear re-init
    //SET_AND_CLR_BIT(hc->flag, BIT_BUS_INITED, BIT_BUS_REINIT);
        
    //set init tag
    SET_BIT(hc->flag, BIT_BUS_INITED);    

    if (!count)
        return ERR_NONE;
    
    return enpack_response(resp, cmd, data, count);
}

/*
    Transfer i2c bus data (read or write)
    @host: device controller handle
    @cmd: command from upper level(repeat or true command)
    @data: data buffer
    @count: data buffer size
    @return ERR_NONE successful, other value if failed
*/
int32_t u5030_i2c_transfer_bridge_data(void *host, uint8_t cmd, const uint8_t *data, uint32_t count)
{
    controller_t *hc = (controller_t *)host;
    config_setting_t * scfg = &hc->setting;
    response_data_t * resp = &hc->response;
    uint8_t *rcache;
    bus_interface_t * intf = hc->intf;
    uint16_t size, lenw, lenr, read_size_max, len_rsp = 0;
    uint8_t cmd_rsp;
    int32_t ret;
    int32_t i, retry = scfg->base.data2.bits.iic_retry ? 3 : 0;

    if (!intf || !intf->cb_xfer)
        return ERR_NOT_READY;

    if (count < 2)
        return ERR_INVALID_DATA;

    lenw = data[0];
    lenr = data[1];
    if ((uint32_t)(lenw + 2) > count)
        return ERR_INVALID_DATA;

    //Need initialize bus before transfer data
    if (!TEST_BIT(hc->flag, BIT_BUS_INITED)) {
        ret = u5030_set_bridge_ext_config(hc, CMD_EXTENSION_CONFIG, (uint8_t *)&scfg->ext, sizeof(scfg->ext));
        if (ret != ERR_NONE)
            return ret;
    }

    size = get_response_cache(resp, (void **)&rcache);
    read_size_max = intf->cb_trans_size(intf->dbc, size - 2);   // bytes[0:1] is response
    if (lenr > read_size_max)
        lenr = read_size_max;    //count may max than buffer size

    for ( i = 0; i <= retry; i++ ) {
        ret = intf->cb_xfer(intf->dbc, data + 2, lenw, rcache + 2, lenr, &len_rsp, &cmd_rsp);
        if (ret == ERR_NONE) {
            if (cmd == CMD_AUTO_REPEAT_RESP) {
                rcache[0] = cmd;
                rcache[1] = cmd_rsp;
            }else {
                rcache[0] = cmd_rsp;
                rcache[1] = len_rsp;
            }

            enpack_response_directly(resp); //above direct write response cache
            break;
        }
    }

    return ret;
}

/*
    Transfer spi bus data (read or write)
    @host: device controller handle
    @cmd: command from upper level(repeat or true command)
    @data: data buffer
    @count: data buffer size
    @return ERR_NONE successful, other value if failed
*/
int32_t u5030_spi_transfer_bridge_data(void *host, uint8_t cmd, const uint8_t *data, uint32_t count)
{
    controller_t *hc = (controller_t *)host;
    config_setting_t * scfg = &hc->setting;
    response_data_t * resp = &hc->response;
    uint8_t *rcache;
    bus_interface_t * intf = hc->intf;
    uint16_t size, read_size_max, lenw, lenr, len_rsp = 0;
    uint8_t cmd_rsp;
    const SPI_HEADER_PACKET_T *phead;
    int32_t ret;
    int32_t i, retry = 0;

    if (!intf || !intf->cb_xfer)
        return ERR_NOT_READY;

    if (count < sizeof(SPI_HEADER_PACKET_T) + 2)
        return ERR_INVALID_DATA;

    lenw = data[0];
    if ((uint32_t)(lenw + 2) > count)
        return ERR_INVALID_DATA;

    phead = (SPI_HEADER_PACKET_T *)(data + 2);
    if (phead->cmd == SPI_CMD_WRITE) {
        lenr = sizeof(SPI_HEADER_PACKET_T);
    } else if (phead->cmd == SPI_CMD_READ) {
        lenr = sizeof(SPI_HEADER_PACKET_T) + phead->len;      
    }else {
        return ERR_INVALID_DATA;
    }

    //Need initialize bus before transfer data
    if (!TEST_BIT(hc->flag, BIT_BUS_INITED)) {
        ret = u5030_set_bridge_ext_config(hc, CMD_EXTENSION_CONFIG, (uint8_t *)&scfg->ext, sizeof(scfg->ext));
        if (ret != ERR_NONE)
            return ret;
    }

    size = get_response_cache(resp, (void **)&rcache);
    read_size_max = intf->cb_trans_size(intf->dbc, size - 2);   // bytes[0:1] is response
    if (lenr > read_size_max)
        lenr = read_size_max;    //count may max than buffer size
    for ( i = 0; i <= retry; i++ ) {
        ret = intf->cb_xfer(intf->dbc, data + 2, lenw, rcache + 2, lenr, &len_rsp, &cmd_rsp);
        if (ret == ERR_NONE) {
            if (cmd == CMD_AUTO_REPEAT_RESP) {
                rcache[0] = cmd;
                rcache[1] = cmd_rsp;
            }else {
                rcache[0] = cmd_rsp;
                rcache[1] = len_rsp;
            }

            enpack_response_directly(resp); //above direct write response cache
            break;
        }
    }

    return ret;
}

static int32_t set_bridge_auto_repeat(void *host, uint8_t cmd, const uint8_t *data, uint32_t count)
{
    controller_t *hc = (controller_t *)host;
    config_setting_t *scfg = &hc->setting;
    response_data_t *resp = &hc->response;
    uint32_t lenw;
    uint8_t rdata;

    if (count < 3)
        return ERR_INVALID_DATA;

    lenw = data[2];
    //lenr = data[3];
    count -= 3;
    if (lenw > count)
        return ERR_INVALID_DATA;

    memcpy(&scfg->dym.repeat, data, lenw + 3);
    SET_BIT(hc->flag, BIT_AUTO_REPEAT);
    rdata = REPEAT_DATA1_OK;

    return enpack_response(resp, cmd, &rdata, sizeof(rdata));
}

static int32_t set_bridge_stop_listen(void *host, uint8_t cmd, const uint8_t *data, uint32_t count)
{
    return ERR_NONE;
}

static int32_t test_bridge_cmd(void *host, uint8_t cmd, const uint8_t *data, uint32_t count)
{
    controller_t *hc = (controller_t *)host;
    response_data_t *resp = &hc->response;
    uint8_t rdata[6];

    //TBD: Bulk mode (Version > 5)

    memset(rdata, 0, sizeof(rdata));
#ifdef BOARD_D21
    if (data[0] ==  0xaa && data[1] == 0x55) {
        rdata[0] = 0x55;
        rdata[1] = 0xaa;
        rdata[2] = 5;
        rdata[3] = 0;
        rdata[5] = 0x12;
        rdata[6] = 0x34;
    }
#endif
    return enpack_response(resp, cmd, rdata, sizeof(rdata));
}

static int32_t set_bridge_gpios(void *host, uint8_t cmd, const uint8_t *data, uint32_t count)
{
    controller_t *hc = (controller_t *)host;
    response_data_t *resp = &hc->response;
    //never mind [E7, E6, GPIO3, DRDY, GPIO2, LED2, GPIO1, GPIO0]
    const uint8_t pin_list[] = {GP_IO0, GP_IO1, GP_CHG, GP_IO2, GP_IO3, GP_IO4, GP_RST, GP_IO5}; //8 Max
    struct port_config config;
    uint8_t pin;
    uint8_t ddr;
    uint8_t value;
    uint8_t status;
    bool level;
    uint8_t rdata[5];

    if (count < 2)
        return ERR_INVALID_DATA;

    ddr = data[0];
    status = 0;
    value = data[1];

    for (uint8_t i = 0; i < ARRAY_SIZE(pin_list); i++) {
        port_get_config_defaults(&config);
        pin = pin_list[i];
            
        if (TEST_BIT(ddr, i)) {
            config.direction = PORT_PIN_DIR_OUTPUT;
            port_pin_set_output_level(pin, !!TEST_BIT(value, i));
        }else {
            if (config.direction != PORT_PIN_DIR_INPUT) {
                config.direction = PORT_PIN_DIR_INPUT;
                config.input_pull = TEST_BIT(value, i) ? PORT_PIN_PULL_UP : PORT_PIN_PULL_NONE;
            }
        }

        port_pin_set_config(pin, &config);
        level = port_pin_get_input_level(pin);
        
        if (level)
            status |= (1 << i);
    }

    rdata[0] = ddr;
    rdata[1] = status;
    rdata[2] = value;

    return enpack_response(resp, cmd, rdata, sizeof(rdata));
}

static int32_t set_bridge_gpio_ext(void *host, uint8_t cmd, const uint8_t *data, uint32_t count)
{
    controller_t *hc = (controller_t *)host;
    response_data_t *resp = &hc->response;
    const uint8_t *buf = data;
    uint32_t size = count;
    uint8_t scmd;
    uint8_t pin;
    uint8_t val;
    struct system_pinmux_config port_config;

    if (count < 2)
        return ERR_INVALID_DATA;

    scmd = *buf++;
    pin = *buf++;
    size -= 2;
    
    if (TEST_BIT(scmd, SET_GPIO_EXT_DATA1_SCMD_GPIO_SHIFT)) {
        size--;
        if (size < 0)
            return ERR_INVALID_DATA;
        port_pin_set_output_level(pin, !!(*buf++));
    }

    if (TEST_BIT(scmd, SET_GPIO_EXT_DATA1_SCMD_MUX_SHIFT)) {
        size -= 4;
        if (size < 0)
            return ERR_INVALID_DATA;

        port_config.mux_position = *buf++;
        port_config.direction = *buf++;
        port_config.input_pull = *buf++;
        port_config.powersave = *buf++;
        system_pinmux_pin_set_config(pin, &port_config);
    }

    if (TEST_BIT(scmd, SET_GPIO_EXT_DATA1_SCMD_TOGGLE_SHIFT)) {
        size -= 2;
        if (size < 0)
            return ERR_INVALID_DATA;

        val = *buf++;
        if (val)
            delay_ms(val);
        
        val = *buf++;
        if (val)
            delay_us(val);
        
        port_pin_toggle_output_level(pin);
    }

    return enpack_response(resp, cmd, data, count);
}

static int32_t get_bridge_gpios(void *host, uint8_t cmd, const uint8_t *data, uint32_t count)
{
    controller_t *hc = (controller_t *)host;
    response_data_t *resp = &hc->response;
    //never mind [E7, E6, GPIO3, DRDY, GPIO2, LED2, GPIO1, GPIO0]
    const uint8_t pin_list[] = {GP_IO0, GP_IO1, GP_CHG, GP_IO2, GP_IO3, GP_IO4, GP_RST, GP_IO5}; //8 Max
    uint8_t pin;
    uint8_t ddr;
    uint8_t port;
    uint8_t status;
    bool level;
    uint8_t rdata[3];

    if (count < 2)
        return ERR_INVALID_DATA;

    ddr = 0;
    port = 0;
    status = 0;
    for (uint8_t i = 0; i < ARRAY_SIZE(pin_list); i++) {
        pin = pin_list[i];
        level = port_pin_get_input_level(pin);
        if (level)
            status |= (1 << i);
    }

    rdata[0] = ddr;
    rdata[1] = status;
    rdata[2] = port;
    return enpack_response(resp, cmd, rdata, sizeof(rdata));
}

static int32_t find_i2c_address(void *host, uint8_t cmd, const uint8_t *data, uint32_t count)
{
    controller_t *hc = (controller_t *)host;
    response_data_t *resp = &hc->response;
    bus_interface_t * intf = hc->intf;
    uint8_t rdata[1] = { IIC_NO_DEVICE_FOUND };

    uint8_t i2c_addr_list[] = { 0x4A, 0x4B, 0x4C, 0x4D, 0x24, 0x25, 0x26, 0x27 };
    uint8_t spi51_addr_list[] = { SPI51_DUMMY_DEVICE_ADDRESS };
    uint8_t *addr_list;
    uint8_t i, size;
    uint8_t ret;

    if (!intf || !intf->cb_ping)
        return ERR_NOT_READY;

    switch (intf->type) {
        case BUS_SPI50:
            return ERR_UNSUPPORTED_OP;
        case BUS_SPI51:
            addr_list = spi51_addr_list;
            size = ARRAY_SIZE(spi51_addr_list);
        break;
        default:
            addr_list = i2c_addr_list;
            size = ARRAY_SIZE(i2c_addr_list); 
    }


    for (i = 0 ; i < size; i++) {
        ret = intf->cb_ping(intf->dbc, addr_list[i]);
        if (ret) {
            rdata[0] = ret;
            break;
        }
    }

    return enpack_response(resp, cmd, rdata, sizeof(rdata));
}

typedef int32_t (*func_do_cmd) (void *host, uint8_t cmd,  const uint8_t *data, uint32_t count);
struct cmd_func_map{
    uint8_t cmd;
    func_do_cmd func;
};

//default address list
//0x24, 0x25, 0x26, 0x27, 0x34, 0x35, 0x4A, 0x4B, 0x4C, 0x5A, 0x5B, 0x5F
static struct cmd_func_map u5030_command_func_map_list[] = {
    //Base command
    {CMD_CONFIG, set_bridge_config},
    {CMD_SAVE_CONFIGS_EEPROM, NULL},
    {CMD_RESTORE_DEFAULT_CONFIGS, NULL},
    {CMD_GET_CONFIG, NULL},
    {CMD_CONFIG_READ_PINS, NULL},
    {CMD_READ_PINS, NULL},
    {CMD_SET_GPIOS, set_bridge_gpios},
    {CMD_READ_GPIOS, get_bridge_gpios},
    {CMD_PWM, NULL},
    {CMD_SET_IO_C, NULL},
    {CMD_GET_IO_C, NULL},
    {CMD_SET_IO_F, NULL},
    {CMD_GET_IO_F, NULL},
    {CMD_FIND_IIC_ADDRESS, find_i2c_address},
    {CMD_SPI_UART_DATA, u5030_spi_transfer_bridge_data},
    {CMD_IIC_DATA_1, u5030_i2c_transfer_bridge_data},
    {CMD_IIC_DATA_2, NULL},
    {CMD_REPEAT, set_bridge_auto_repeat},
    {CMD_REPEAT_STACK, NULL},
    {CMD_REPEAT_2_SPI, NULL},
    {CMD_START_LISTENING, NULL},
    {CMD_RINGBUFFER_DBG, NULL},
    {CMD_STOP_LISTENING, set_bridge_stop_listen},
    {CMD_SEND_DEBUG_DATA, NULL},
    {CMD_NO_DATA, NULL},
    {CMD_NULL, test_bridge_cmd},
    {CMD_START_TEST, NULL},
    {CMD_RESET_BRIDGE, NULL},
    {CMD_JUMP_BOOTLOADER, NULL},
    {CMD_SWITCH_MODE_FAST, NULL},
    {CMD_SWITCH_MODE_5016, NULL},
    {CMD_SWITCH_MODE_DEBUG, NULL},
    {CMD_SWITCH_MODE_DIGITIZER_SERIAL, NULL},
    {CMD_SWITCH_MODE_DIGITIZER_PARALLEL, NULL},

    //Extension command
    {CMD_NAK, NULL},
    {CMD_EXTENSION_CONFIG, u5030_set_bridge_ext_config},
    {CMD_SET_GPIO_EXT, set_bridge_gpio_ext},
}; 

static int32_t enpack_response(response_data_t *resp, uint8_t rdata, const uint8_t *data, uint32_t count)
{
    response_cache_t *cache = &resp->rcache;
    uint8_t *buf = cache->data;
    uint32_t buf_size = sizeof(cache->data);

    if (count + 1 > buf_size)
        return ERR_NO_MEMORY;
    
    if (data != NULL)   //If data is Null, that means resp has been filled
        buf[0] = rdata;
    buf_size--;
    buf++;

    if (data != NULL)    //If data is Null, that means data already be copied
        memcpy(buf, data, count);
    
    buf_size -= count;
    buf += count;
    
    if (buf_size)
        memset(buf, 0, buf_size);

    resp->dirty = true;

    return ERR_NONE;
}

int32_t enpack_response_nak(response_data_t *resp)
{
    uint8_t dummy = 0;
    return enpack_response(resp, CMD_NAK, &dummy, 1);
}

static void enpack_response_directly(response_data_t *resp)
{
    resp->dirty = true;
}

static uint16_t get_response_cache(response_data_t *resp, void **out_ptr)
{
    if (out_ptr)
        *out_ptr = resp->rcache.data;

    return min(sizeof(resp->rcache.data), MAX_TRANSFER_SIZE_ONCE);
}

int32_t u5030_parse_command(void *host, const uint8_t *data, uint32_t count)
{
    controller_t *hc = (controller_t *)host;
    response_data_t *resp = &hc->response;
    uint8_t cmd;
    struct cmd_func_map *fn_map;
    int32_t result = ERR_NOT_FOUND;
    
    if (count < 1)
        return result;
    
    cmd = data[0];
    for (uint32_t i = 0; i < ARRAY_SIZE(u5030_command_func_map_list); i++)
    {
        fn_map = &u5030_command_func_map_list[i];
        if (cmd == fn_map->cmd){    
            if (fn_map->func)
                result = fn_map->func(hc, cmd, data + 1, count - 1);
            else
                result = ERR_UNSUPPORTED_OP;
            break;
        }
    }

    if (result != ERR_NONE && result != ERR_NOT_FOUND)
        result = enpack_response_nak(resp);
    
    return result;
}

int32_t u5030_check_auto_repeat_avaliable(void *host, const uint8_t *data, uint32_t count)
{
    uint8_t cmd;
    int32_t result = ERR_INVALID_DATA;
    
    if (count < 1)
        return result;
    
    cmd = data[0];
	switch(cmd) {
		case CMD_NULL:
			break;
		default:
			result = ERR_NONE;
	}
	
	return result;
}

bool u5030_chg_line_active(void *host)
{
    controller_t *hc = (controller_t *)host;
    config_setting_t * scfg = &hc->setting;
    int32_t pin = 0;
    bool chkv;

    switch(scfg->dym.repeat.cfg.bits.chg) {
    case REPEAT_CHG_BY_TIMER:
        pin = GPIO_P_1; //Still use default CHG Pin
        chkv = false;
        break;
    case REPEAT_CHG_BY_GPIO0_L:
        pin = GPIO_P_0;
        chkv = false;
        break;
    case REPEAT_CHG_BY_GPIO0_H:
        pin = GPIO_P_0;
        chkv = true;
        break;
    case REPEAT_CHG_BY_GPIO1_L:
        pin = GPIO_P_1;
        chkv = false;
        break;
    case REPEAT_CHG_BY_GPIO1_H:
        pin = GPIO_P_1;
        chkv = true;
        break;
    case REPEAT_CHG_BY_LED2_L:
        pin = GPIO_P_LED2;
        chkv = false;
        break;
    case REPEAT_CHG_BY_LED2_H:
        pin = GPIO_P_LED2;
        chkv = true;
        break;
    }

    if (pin){
        #ifdef CHG_ACTIVE_LEVEL_INVERT
            chkv = !chkv;  //Reserved by N-Mos
        #endif

        if (port_pin_get_input_level(pin) == chkv)
            return true;
    }

    return false;
}

int32_t u5030_init(void)
{
    return ERR_NONE;
}

void u5030_deinit(void)
{

}