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
#include "crc.h"
#include "board/board.h"
#include "u5030_protocol.h"
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
static uint8_t transfer_data(void *host, const uint8_t *wdata, uint32_t wlen, uint8_t *rdata, uint32_t rlen, uint8_t *ecode, int32_t retry);
static int32_t enpack_command(uint8_t cmd, uint8_t resp, const uint8_t *data, uint32_t count);

static int32_t set_bridge_config(void *host, uint8_t cmd, const uint8_t *data, uint32_t count)
{
	controller_t *hc = (controller_t *)host;
	config_setting_t *scfg = &hc->setting;
	uint32_t cfg_size;

	cfg_size = Min(sizeof(scfg->base), count);

	if (cfg_size){
		memcpy(&scfg->base, data, cfg_size);
	
		if (TEST_BIT(hc->flag, BIT_BUS_INITED))
			SET_BIT(hc->flag, BIT_BUS_REINIT);
	}
	
	//TBD: Bulk mode test command
	//{ 0x80, 0x30, 0xCA, 0x00, 0x80, 0x00, 0x00, 0xC8, 0x40, 0x54, 0x00, 0x00, 0xAA, 0x55, 0xAA, 0xFF };

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
		iic_bus_init(&hc->iic, SERCOM3, baudrate, scfg->base.data2.bits.iic1_addr);
	}
	
	//TODO: Inlitialize UART
	//TODO: Inlitialize SPI

	//set re-init
	SET_AND_CLR_BIT(hc->flag, BIT_BUS_INITED, BIT_BUS_REINIT);
	
	//clear auto repeat
	CLR_BIT(hc->flag, BIT_AUTO_REPEAT);
	
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
	uint32_t lenr, read_size_max;
	uint8_t cmd_rsp;
	uint8_t len_rsp;
	int32_t ret;
	int32_t retry = scfg->base.data2.bits.iic_retry ? 3 : 0;

	if (count < 2)
		return -ERR_INVALID_DATA;

	lenw = data[0];
	lenr = data[1];
	if (lenw + 2 > count)
		return -ERR_INVALID_DATA;

	//Need initialize bus before transfer data
	if (TEST_BIT(hc->flag, BIT_BUS_REINIT) || !TEST_BIT(hc->flag, BIT_BUS_INITED)) {
		ret = set_bridge_ext_config(hc, CMD_EXTENSION_CONFIG, (uint8_t *)&scfg->ext, sizeof(scfg->ext));
		if (ret != ERR_NONE)
			return ret;
	}

	read_size_max = sizeof(cache->data) - 2;
	if (lenr > read_size_max)
		lenr = read_size_max;	//count may max than buffer size
	len_rsp = transfer_data(host, data + 2, lenw, cache->data + 2, lenr, &cmd_rsp, retry);
	if (cmd == CMD_AUTO_REPEAT_RESP) {
		cache->data[0] = cmd;
		cache->data[1] = cmd_rsp;
	}else {
		cache->data[0] = cmd_rsp;
		cache->data[1] = len_rsp;
	}
	
	//clear auto repeat
	CLR_BIT(hc->flag, BIT_AUTO_REPEAT);
	
	return enpack_command(cmd, 0, NULL, len_rsp + 1);
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

static int32_t test_bridge_cmd(void *host, uint8_t cmd, const uint8_t *data, uint32_t count)
{
	uint8_t resp[7];

	//TBD: Bulk mode (Version > 5)

	memset(resp, 0, sizeof(resp));
	return enpack_command(cmd, cmd, resp, sizeof(resp));
}

//default address list
//0x24, 0x25, 0x26, 0x27, 0x34, 0x35, 0x4A, 0x4B, 0x4C, 0x5A, 0x5B, 0x5F


static int32_t set_bridge_gpios(void *host, uint8_t cmd, const uint8_t *data, uint32_t count)
{
	//never mind [E7, E6, GPIO3, DRDY, GPIO2, LED2, GPIO1, GPIO0]
	const uint8_t pin_list[] = {GP_IO0, GP_IO1, GP_IO2, GP_IO3, GP_IO4, GP_IO5, GP_RST, GP_CHG}; //8 Max
	struct port_config config;
	uint8_t pin;
	uint8_t ddr;
	uint8_t port;
	uint8_t status;
	bool level;
	uint8_t resp[3];

	if (count < 2)
		return -ERR_INVALID_DATA;

	ddr = data[0];
	port = data[1];
	status = 0;
	for (uint8_t i = 0; i < ARRAY_SIZE(pin_list); i++) {
		port_get_config_defaults(&config);
		pin = pin_list[i];
		if (TEST_BIT(ddr, i)) {
			config.direction = PORT_PIN_DIR_OUTPUT;
			port_pin_set_output_level(pin, !!TEST_BIT(port, i));
		}else {
			config.direction = PORT_PIN_DIR_INPUT;
			config.input_pull = TEST_BIT(port, i) ? PORT_PIN_PULL_UP : PORT_PIN_PULL_NONE;
		}
		
		port_pin_set_config(pin, &config);
		level = port_pin_get_input_level(pin);
		if (level)
			status |= (1 << i);
	}

	resp[0] = ddr;
	resp[1] = status;
	resp[2] = port;
	return enpack_command(cmd, cmd, resp, sizeof(resp));
}

static int32_t get_bridge_gpios(void *host, uint8_t cmd, const uint8_t *data, uint32_t count)
{
	//never mind [E7, E6, GPIO3, DRDY, GPIO2, LED2, GPIO1, GPIO0]
	const uint8_t pin_list[] = {GP_IO0, GP_IO1, GP_IO2, GP_IO3, GP_IO4, GP_IO5, GP_RST, GP_CHG}; //8 Max
	uint8_t pin;
	uint8_t ddr;
	uint8_t port;
	uint8_t status;
	bool level;
	uint8_t resp[3];

	if (count < 2)
		return -ERR_INVALID_DATA;

	ddr = 0;
	port = 0;
	status = 0;
	for (uint8_t i = 0; i < ARRAY_SIZE(pin_list); i++) {
		pin = pin_list[i];
		level = port_pin_get_input_level(pin);
		if (level)
			status |= (1 << i);
	}

	resp[0] = ddr;
	resp[1] = status;
	resp[2] = port;
	return enpack_command(cmd, cmd, resp, sizeof(resp));
}

static struct cmd_func_map command_func_map_list[] = {
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
	{CMD_STOP_LISTENING, NULL},
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
}; 

static uint8_t transfer_data(void *host, const uint8_t *wdata, uint32_t wlen, uint8_t *rdata, uint32_t rlen, uint8_t *ecode, int32_t retry)
{
	controller_t *hc = (controller_t *)host;
	uint8_t cmd_rsp;
	uint8_t len_rsp;
	int32_t ret;
	
	do {
		cmd_rsp = IIC_DATA_OK;
		len_rsp = 0;
		ret = ERR_NONE;
		
		//write
		if (wlen) {
			ret = iic_write(&hc->iic, wdata, wlen);
			if (ret == ERR_BAD_ADDRESS) {
				cmd_rsp = IIC_DATA_NAK_ADDR;
			}else if(ret) {
				cmd_rsp = IIC_DATA_NAK_WRITE;
			}
		}

		//read
		if (ret == ERR_NONE) {
			if (rlen) {
				ret = iic_read(&hc->iic, rdata, rlen);
				if (ret == ERR_BAD_ADDRESS) {
					cmd_rsp = IIC_DATA_NAK_ADDR;
				}else if(ret) {
					cmd_rsp = IIC_DATA_NAK_READ;
				}else {
					len_rsp = rlen;
				}
			}else {
				cmd_rsp = IIC_DATA_FINISHED_WITHOUT_R;
			}
		}

		if (ret == ERR_NONE)
			break;
		retry--;
		//TBD: here need a retry delay

	}while(ret != ERR_NONE && retry > 0);

	if (ecode)
		*ecode = cmd_rsp;

	return len_rsp;
}

static int32_t enpack_command(uint8_t cmd, uint8_t resp, const uint8_t *data, uint32_t count)
{
	controller_t *hc = &g_host_controller;
	response_cache_t *cache = &hc->rcache;
	uint8_t *buf = cache->data;
	uint32_t buf_size = sizeof(cache->data);

	if (cache->cmd != CMD_NONE) {
		return -ERR_BUSY; 
	}

	if (count + 1 > buf_size)
		return -ERR_NO_MEMORY;
	
	cache->cmd = cmd;
	if (data != NULL)   //If data is Null, that means resp has been filled
		buf[0] = resp;
	buf_size--;
	buf++;

	if (data != NULL)	//If data is Null, that means data already be copied
		memcpy(buf, data, count);
	
	buf_size -= count;
	buf += count;
	
	if (buf_size)
		memset(buf, 0, buf_size);

	return ERR_NONE;
}

static void enpack_command_nak(uint8_t cmd)
{
	uint8_t dummy = 0;
	enpack_command(cmd, CMD_NAK, &dummy, 1);
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

	config_setting_t * scfg = &hc->setting;
	const uint8_t *data;
	uint32_t count;
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
				pin = GPIO_P_LED2;
				chkv = false;
				break;
			case REPEAT_CHG_BY_LED2_H:
				pin = GPIO_P_LED2;
				chkv = true;
				break;
			}

			if (pin){
				#ifdef BOARD_D21
					chkv = !chkv;  //Reserved by N-Mos
				#endif

				if (port_pin_get_input_level(pin) == chkv) {
					if (scfg->dym.repeat.cfg.bits.bus == REPEAT_BUS_SPI_UART) {
						;//TODO
					}else if (scfg->dym.repeat.cfg.bits.bus == REPEAT_BUS_IIC1){
						data = (uint8_t *)&scfg->dym.repeat.lenw;
						count = scfg->dym.repeat.lenw + 2;
						send_bridge_data(hc, CMD_AUTO_REPEAT_RESP, data, count);
					}else if (scfg->dym.repeat.cfg.bits.bus == REPEAT_BUS_IIC2){
						//TODO
					}
				}
			}
		}
	}

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
		
	cache->cmd = CMD_NONE;
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

	platform_board_init();
	load_default_config(&hc->setting);

	return ERR_NONE;
}

void u5030_deinit(void)
{

}