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
#include "common.h"
#include "external/utils.h"
#include "external/err_codes.h"

static int32_t enpack_response(response_data_t *resp, uint8_t rdata0, const uint8_t *data, uint32_t count);
static int32_t set_bridge_config(void *host, uint8_t cmd, const uint8_t *data, uint32_t count)
{
	controller_t *hc = (controller_t *)host;
	config_setting_t *scfg = &hc->setting;
	response_data_t *resp = &hc->response;

	uint32_t cfg_size;

	cfg_size = Min(sizeof(scfg->base), count);

	if (cfg_size){
		memcpy(&scfg->base, data, cfg_size);
	
		if (TEST_BIT(hc->flag, BIT_BUS_INITED))
			SET_BIT(hc->flag, BIT_BUS_REINIT);
	}
	
	//TBD: Bulk mode test command
	//{ 0x80, 0x30, 0xCA, 0x00, 0x80, 0x00, 0x00, 0xC8, 0x40, 0x54, 0x00, 0x00, 0xAA, 0x55, 0xAA, 0xFF };

	return enpack_response(resp, cmd, data, cfg_size);
}

int32_t set_bridge_ext_config(void *host, uint8_t cmd, const uint8_t *data, uint32_t count)
{
	controller_t *hc = (controller_t *)host;
	config_setting_t *scfg = &hc->setting;
	transfer_data_t *trans = &hc->transfer;
	response_data_t *resp = &hc->response;	
	uint8_t checksum;
	uint8_t mode;
	uint32_t cfg_size = sizeof(scfg->ext);
	uint32_t baudrate;
	bool dummy = data == (uint8_t *)&scfg->ext;

	if (cfg_size > count)
		return ERR_INVALID_DATA;

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
		trans->xfer = i2c_transfer_data;
	}
	
	//TODO: Inlitialize UART
	//TODO: Inlitialize SPI

	//set re-init
	SET_AND_CLR_BIT(hc->flag, BIT_BUS_INITED, BIT_BUS_REINIT);
	
	if (dummy)
		return ERR_NONE;
	
	return enpack_response(resp, cmd, data, count);
}

int32_t send_bridge_data(void *host, uint8_t cmd, const uint8_t *data, uint32_t count)
{
	controller_t *hc = (controller_t *)host;
	config_setting_t * scfg = &hc->setting;
	response_data_t * resp = &hc->response;
	uint8_t *rdata = resp->rcache.data;
	transfer_data_t * trans = &hc->transfer;
	uint32_t size = sizeof(resp->rcache.data);
	uint32_t lenw;
	uint32_t lenr, read_size_max;
	uint8_t cmd_rsp;
	uint8_t len_rsp;
	int32_t ret;
	int32_t retry = scfg->base.data2.bits.iic_retry ? 3 : 0;

	if (count < 2)
		return ERR_INVALID_DATA;

	lenw = data[0];
	lenr = data[1];
	if (lenw + 2 > count)
		return ERR_INVALID_DATA;

	//Need initialize bus before transfer data
	if (TEST_BIT(hc->flag, BIT_BUS_REINIT) || !TEST_BIT(hc->flag, BIT_BUS_INITED)) {
		ret = set_bridge_ext_config(hc, CMD_EXTENSION_CONFIG, (uint8_t *)&scfg->ext, sizeof(scfg->ext));
		if (ret != ERR_NONE)
			return ret;
	}

	read_size_max = size - 2;
	if (lenr > read_size_max)
		lenr = read_size_max;	//count may max than buffer size
	len_rsp = trans->xfer(host, data + 2, lenw, rdata + 2, lenr, &cmd_rsp, retry);
	if (cmd == CMD_AUTO_REPEAT_RESP) {
		rdata[0] = cmd;
		rdata[1] = cmd_rsp;
	}else {
		rdata[0] = cmd_rsp;
		rdata[1] = len_rsp;
	}
	resp->dirty = true;

	return ERR_NONE;
}

static int32_t set_bridge_auto_repeat(void *host, uint8_t cmd, const uint8_t *data, uint32_t count)
{
	controller_t *hc = (controller_t *)host;
	config_setting_t *scfg = &hc->setting;
	response_data_t *resp = &hc->response;
	uint32_t lenw = data[1];
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
	uint8_t mask = 0;
	uint8_t status;
	bool level;
	uint8_t rdata[5];

	if (count < 2)
		return ERR_INVALID_DATA;

	ddr = data[0];
	value = data[1];
	//User extension
	if (count >= 3)
		mask = data[2];

	status = 0;
	for (uint8_t i = 0; i < ARRAY_SIZE(pin_list); i++) {
		if (TEST_BIT(mask, i))
			continue;

		port_get_config_defaults(&config);
		pin = pin_list[i];
			
		if (TEST_BIT(ddr, i)) {
			config.direction = PORT_PIN_DIR_OUTPUT;
			port_pin_set_output_level(pin, !!TEST_BIT(value, i));
			port_pin_set_config(pin, &config);
			level = port_pin_get_output_level(pin);
		}else {
			config.direction = PORT_PIN_DIR_INPUT;
			config.input_pull = TEST_BIT(value, i) ? PORT_PIN_PULL_UP : PORT_PIN_PULL_NONE;
			port_pin_set_config(pin, &config);
			level = port_pin_get_input_level(pin);
		}
		
		if (level)
			status |= (1 << i);
	}

	rdata[0] = ddr;
	rdata[1] = status;
	rdata[2] = value;
	rdata[3] = mask;

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
	{CMD_FIND_IIC_ADDRESS, NULL},
	{CMD_SPI_UART_DATA, NULL},
	{CMD_IIC_DATA_1, send_bridge_data},
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
	{CMD_EXTENSION_CONFIG, set_bridge_ext_config},
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

	if (data != NULL)	//If data is Null, that means data already be copied
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

bool u5030_chg_line_active(void *host)
{
	controller_t *hc = (controller_t *)host;
	config_setting_t * scfg = &hc->setting;
	int32_t pin = 0;
	bool chkv;

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