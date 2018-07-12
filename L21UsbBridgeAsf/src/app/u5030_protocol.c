/*
 * protocol_u5030.c
 *
 * Created: 6/20/2018 6:57:50 PM
 *  Author: A41450
 */ 
#include <stddef.h>
#include <stdint.h>
#include <string.h>

//#include "../driver_init.h"
#include "u5030_protocol.h"
#include "crc.h"
#include "board/board.h"
#include "external/utils.h"
#include "external/err_codes.h"


typedef struct response_cache{
	uint8_t data[CONF_USB_COMPOSITE_HID_GENERIC_INTIN_MAXPKSZ];	//must be aligned
	uint8_t cmd;
	uint8_t rsv[3];
}__attribute__((packed, aligned(4))) response_cache_t;

typedef struct {
	config_setting_t setting;
#define BIT_BUS_INITED 1
#define BIT_BUS_REINIT 2
#define BIT_AUTO_REPEAT 5
	uint32_t flag;
	iic_controller_t iic;
	response_cache_t rcache;
}controller_t;

//static config_setting_t g_config_setting;
static controller_t g_host_controller;
static int32_t enpack_command(uint8_t cmd, uint8_t resp, const uint8_t *buf, uint32_t size);

static int32_t set_bridge_config(void *host, uint8_t cmd, const uint8_t *data, uint32_t count)
{
	controller_t *hc = (controller_t *)host;
	config_setting_t *scfg = &hc->setting;
	uint32_t cfg_size = sizeof(scfg->base);

	if (cfg_size > count)
		return -ERR_INVALID_DATA;

	memcpy(&scfg->base, data, cfg_size);
	
	if (TEST_BIT(hc->flag, BIT_BUS_INITED))
		SET_BIT(hc->flag, BIT_BUS_REINIT);

	return enpack_command(cmd, cmd, data, cfg_size);
}

static int32_t set_bridge_ext_config(void *host, uint8_t cmd, const uint8_t *data, uint32_t count)
{
	controller_t *hc = (controller_t *)host;
	config_setting_t *scfg = &hc->setting;
	uint8_t checksum;
	uint8_t mode;
	uint32_t cfg_size = sizeof(scfg->ext);
	uint32_t baudrate;
	bool dummy = data == (uint8_t *)&scfg->ext;

	if (cfg_size > count)
		return -ERR_INVALID_DATA;

	if (!dummy)	{ //here may be itself outside	
		checksum = DESC_GET(data[0], EXT_CONFIG_DATA1_COMMUNICATION_MODE_CHECKSUM);
		mode = DESC_GET(data[0], EXT_CONFIG_COMMUNICATION_MODE);
		if (checksum + mode != 0xFF)
			return -ERR_INVALID_DATA;

		memcpy(&scfg->ext, data, cfg_size);
	}

	if (scfg->ext.data1.bits.com_mode == COM_MODE_IIC_ONLY)
	{
		if(TEST_BIT(hc->flag, BIT_BUS_REINIT)) {
			iic_bus_deinit(&hc->iic);
		}
		switch(scfg->base.data1.bits.iic_clk) {
		case IIC_CLK_50KHZ:
			baudrate = 50;
			break;
		case IIC_CLK_100KHZ:
			baudrate = 100;
			break;
		case IIC_CLK_200KHZ:
			baudrate = 200;
			break;
		case IIC_CLK_400KHZ:
			baudrate = 400;
			break;
    	}
		iic_bus_init(&hc->iic, SERCOM1, baudrate, scfg->base.data2.bits.iic1_addr);
	}
	
	//TODO: Inlitialize UART
	//TODO: Inlitialize SPI

	SET_AND_CLR_BIT(hc->flag, BIT_BUS_INITED, BIT_BUS_REINIT);

	if (dummy)
		return ERR_NONE;
	
	return enpack_command(cmd, cmd, data, cfg_size);
}

static int32_t send_bridge_data(void *host, uint8_t cmd, const uint8_t *data, uint32_t count)
{
	controller_t *hc = (controller_t *)host;
	config_setting_t * scfg = &hc->setting;
	response_cache_t *cache = &hc->rcache;
	uint32_t lenw;
	uint32_t lenr;
	uint8_t cmd_rsp;
	uint8_t len_rsp;
	int32_t ret;
	int32_t retry = scfg->base.data2.bits.iic_retry ? 3 : 0;

	if (count < 2)
		return -ERR_INVALID_DATA;

	lenw = data[0];
	lenr = data[1];
	count -= 2;
	if (lenw > count)
		return -ERR_INVALID_DATA;

	//Need initialize bus before transfer data
	if (TEST_BIT(hc->flag, BIT_BUS_REINIT) || !TEST_BIT(hc->flag, BIT_BUS_INITED)) {
		ret = set_bridge_ext_config(hc, CMD_EXTENSION_CONFIG, (uint8_t *)&scfg->ext, sizeof(scfg->ext));
		if (ret != ERR_NONE)
			return ret;
	}

	do {
		cmd_rsp = IIC_DATA_OK;
		len_rsp = 0;
		ret = ERR_NONE;
		
		//write
		if (lenw) {
			ret = iic_write(&hc->iic, data + 2, lenw);
			if (ret == ERR_BAD_ADDRESS) {
				cmd_rsp = IIC_DATA_NAK_ADDR;
			}else if(ret) {
				cmd_rsp = IIC_DATA_NAK_WRITE;
			}
		}

		//read
		if (ret == ERR_NONE) {
			if (lenr) {
				ret = iic_read(&hc->iic, cache->data + 2, lenr);
				if (ret == ERR_BAD_ADDRESS) {
					cmd_rsp = IIC_DATA_NAK_ADDR;
				}else if(ret) {
					cmd_rsp = IIC_DATA_NAK_READ;
				}else {
					len_rsp = lenr;
				}
			}else {
				cmd_rsp = IIC_DATA_FINISHED_WITHOUT_R;
			}
		}

		if (ret == ERR_NONE)
			break;
		retry--;

	}while(ret != ERR_NONE && retry > 0);

	cache->data[1] = len_rsp;
	return enpack_command(cmd, cmd_rsp, cache->data, 0);
}

static int32_t set_bridge_auto_repeat(void *host, uint8_t cmd, const uint8_t *data, uint32_t count)
{
	controller_t *hc = (controller_t *)host;
	config_setting_t *scfg = &hc->setting;
	uint32_t lenw = data[1];
	uint8_t resp;

	if (count < 3)
		return -ERR_INVALID_DATA;

	lenw = data[2];
	//lenr = data[3];
	count -= 3;
	if (lenw > count)
		return -ERR_INVALID_DATA;

	memcpy(&scfg->dym.repeat, data, lenw + 3);
	SET_BIT(hc->flag, BIT_AUTO_REPEAT);
	resp = REPEAT_DATA1_OK;

	return enpack_command(cmd, cmd, &resp, 1);
}

static struct cmd_func_map command_func_map_list[] = {
	//Base command
	{CMD_CONFIG, set_bridge_config},
	{CMD_SAVE_CONFIGS_EEPROM, NULL},
	{CMD_RESTORE_DEFAULT_CONFIGS, NULL},
	{CMD_GET_CONFIG, NULL},
	{CMD_CONFIG_READ_PINS, NULL},
	{CMD_READ_PINS, NULL},
	{CMD_SET_GPIOS, NULL},
	{CMD_READ_GPIOS, NULL},
	{CMD_PWM, NULL},
	{CMD_SET_IO_C, NULL},
	{CMD_GET_IO_C, NULL},
	{CMD_SET_IO_F, NULL},
	{CMD_GET_IO_F, NULL},
	{CMD_FIND_IIC_ADDRESS, NULL},
	{CMD_SPI_UART_DATA, NULL},
	{CMD_IIC_DATA_1, send_bridge_data},
	{CMD_IIC_DATA_2, NULL},
	{CMD_REPEAT, set_bridge_auto_repeat},
	{CMD_REPEAT_STACK, NULL},
	{CMD_REPEAT_2_SPI, NULL},
	{CMD_START_LISTENING, NULL},
	{CMD_RINGBUFFER_DBG, NULL},
	{CMD_STOP_LISTENING, NULL},
	{CMD_SEND_DEBUG_DATA, NULL},
	{CMD_NO_DATA, NULL},
	{CMD_NULL, NULL},
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
	{CMD_EXTENSION_CONFIG, set_bridge_ext_config},
}; 

static int32_t enpack_command(uint8_t cmd, uint8_t resp, const uint8_t *buf, uint32_t size)
{
	controller_t *hc = &g_host_controller;
	response_cache_t *cache = &hc->rcache;

	if (cmd == CMD_NONE)
		return -ERR_NOT_FOUND;

	if (cache->cmd) {
		return -ERR_BUSY;//FIXME: last package not be handled there should be something wrong 
	}

	if (size + 1 > sizeof(cache->data))
		return -ERR_NO_MEMORY;
		
	cache->cmd = cmd;
	if (buf != NULL) {
		cache->data[0] = resp;
		if (size)
			memcpy(cache->data + 1, buf, size);
	}else
		memset(cache->data, 0, sizeof(cache->data));

	return ERR_NONE;
}

static void enpack_command_nak(uint8_t cmd)
{
	enpack_command(cmd, CMD_NAK, NULL, 0);
}

static int32_t parse_command(const uint8_t *data, uint32_t count)
{
	controller_t *hc = &g_host_controller;
	uint8_t cmd;
	struct cmd_func_map *fn_map;
	int32_t result = -ERR_INVALID_DATA;
	
	if (count < 1)
		return result;
	
	cmd = data[0];
	for (uint32_t i = 0; i < ARRAY_SIZE(command_func_map_list); i++)
	{
		fn_map = &command_func_map_list[i];
		if (cmd == fn_map->cmd){
			if (fn_map->func)
				result = fn_map->func(hc, cmd, data + 1, count - 1);
			else
				result = -ERR_NOT_FOUND;
			break;
		}
	}
	
	if (result != ERR_NONE){
		enpack_command_nak(cmd);
	}
	
	return result;
}

int32_t u5030_process_data(const uint8_t *data, uint32_t size)
{
	return parse_command(data, size);
}

int32_t u5030_get_response(void **buf_ptr, uint32_t *buf_ptr_size)
{

	controller_t *hc = &g_host_controller;
	response_cache_t *cache = &hc->rcache;
/*
	config_setting_t * scfg = &hc->setting;
	int32_t pin = 0;
	bool chkv;

	if (!cache->cmd) {
		if (TEST_BIT(hc->flag, BIT_AUTO_REPEAT)) {
			switch(scfg->dym.repeat.cfg.bits.chg) {
			case REPEAT_CHG_BY_TIMER:
				//TODO
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
				//TODO
				break;
			case REPEAT_CHG_BY_LED2_H:
				//TODO
				break;
			}

			if (pin){
				if (port_pin_get_input_level(pin) == chkv) {
					if (scfg->dym.repeat.cfg.bits.bus == REPEAT_BUS_SPI_UART) {
						;//TODO
					}else if (scfg->dym.repeat.cfg.bits.bus == REPEAT_BUS_IIC1){
						send_bridge_data(hc, CMD_IIC_DATA_1, (uint8_t *)&scfg->dym.repeat, sizeof(scfg->dym.repeat));
					}else if (scfg->dym.repeat.cfg.bits.bus == REPEAT_BUS_IIC2){
						//TODO
					}
				}
			}
		}
	}
*/
	if (cache->cmd) {
		if(buf_ptr)
			*buf_ptr = cache->data;
	
		if(buf_ptr_size)
			*buf_ptr_size = sizeof(cache->data);
		
		return ERR_NONE;
	}
	
	return -ERR_NOT_READY;
}

int32_t u5030_clear_cache(void)
{
	controller_t *hc = &g_host_controller;
	response_cache_t *cache = &hc->rcache;
		
	cache->cmd = 0;
	cache->data[0] = 0;
	
	return ERR_NONE;
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

int32_t u5030_init(void)
{
	controller_t *hc = &g_host_controller;

	load_default_config(&hc->setting);
	
	return ERR_NONE;
}

void u5030_deinit(void)
{

}