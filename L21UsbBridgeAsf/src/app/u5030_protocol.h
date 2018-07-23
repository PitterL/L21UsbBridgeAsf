/*
 * protocol_u5030.h
 *
 * Created: 6/20/2018 6:58:25 PM
 *  Author: Pitter.Liao
 */ 


#ifndef PROTOCOL_U5030_H_
#define PROTOCOL_U5030_H_

/*
Communications
*/
#include "conf_usb.h"
#ifndef CONF_USB_COMPOSITE_HID_GENERIC_INTIN_MAXPKSZ
#	define CONF_USB_COMPOSITE_HID_GENERIC_INTIN_MAXPKSZ		UDI_HID_GENERIC_EP_SIZE
#endif

/*******************************
	Configuration Parameters
********************************/
#define CMD_CONFIG 0x80
/*	
	<CMD_CONFIG>
*/
/*
	<CMD_CONFIG>
	Command: DATA1
*/
union config_data1{
	struct {
		uint8_t spi_clk: 2;
		uint8_t ss_pulse: 1;
		uint8_t iic_clk: 2;
		uint8_t use_drdy: 1;
		uint8_t half_dulx: 1;
	}bits;
	uint8_t val;
};
#define CONFIG_DATA1_UART_HALF_DUPLEX_SHIFT 7
#define CONFIG_DATA1_UART_HALF_DUPLEX_MASK 0x1
#define UART_HALF_DUPLEX_HALF 1
#define UART_HALF_DUPLEX_FULL 0

#define CONFIG_DATA1_USE_DRDY_SHIFT 6
#define CONFIG_DATA1_USE_DRDY_MASK 0x1
#define USE_DRDY_ON 1
#define USE_DRDY_OFF 0

#define CONFIG_DATA1_IIC_CLK_SHIFT 4
#define CONFIG_DATA1_IIC_CLK_MASK 0x3
#define IIC_CLK_50KHZ 0
#define IIC_CLK_100KHZ 1
#define IIC_CLK_200KHZ 2
#define IIC_CLK_400KHZ 3

#define CONFIG_DATA1_SS_PULSE_SHIFT 3
#define CONFIG_DATA1_SS_PULSE_MASK 0x1
#define SS_PULSE_OFF 0
#define SS_PULSE_ON 1

#define CONFIG_DATA1_SPI_CLK_SHIFT 0
#define CONFIG_DATA1_SPI_CLK_MASK 0x3
#define SPI_CLK_25KHZ 0
#define SPI_CLK_50KHZ 1
#define SPI_CLK_100KHZ 2
#define SPI_CLK_200KHZ 3
#define SPI_CLK_500KHZ 4
#define SPI_CLK_1MHZ 5
#define SPI_CLK_2MHZ 6
#define SPI_CLK_4MHZ 7
/*
	<CMD_CONFIG>
	Command: DATA2
*/

union config_data2{
	struct {
		uint8_t iic1_addr: 7;
		uint8_t iic_retry: 1;
	}bits;
	uint8_t val;
};
#define CONFIG_DATA2_IIC_RETRY_SHIFT 7
#define CONFIG_DATA2_IIC_RETRY_MASK 0x1
#define IIC_RETRY_OFF 0
#define IIC_RETRY_ON 1

#define CONFIG_DATA2_IIC_ADDR_SHIFT 0
#define CONFIG_DATA2_IIC_ADDR_MASK 0x7f
/*
	<CMD_CONFIG>
	Command: DATA3
*/

union config_data3{
	struct {
		uint8_t iic2_addr: 7;
		uint8_t ss_mode: 1;
	}bits;
	uint8_t val;
};
#define CONFIG_DATA3_SS_MODE_SHIFT 7
#define CONFIG_DATA3_SS_MODE_MASK 0x1
#define SS_MODE_PULL_HIGH 0
#define SS_MODE_CLAMP_LOW 1

#define CONFIG_DATA3_IIC_ADDR2_SHIFT 0
#define CONFIG_DATA3_IIC_ADDR2_MASK 0x7f
/*
	<CMD_CONFIG>
	Command: DATA4
*/

union config_data4{
	struct {
		uint8_t uart_brate: 4;
		uint8_t spi_mode: 2;
		uint8_t rsv: 1;
		uint8_t iic_restart: 1;
	}bits;
	uint8_t val;
};
#define CONFIG_DATA4_IIC_RESTART_SHIFT 7
#define CONFIG_DATA4_IIC_RESTART_MASK 0x1
#define IIC_RESTART_DISABLE 0
#define IIC_RESTART_ENABLE 1

#define CONFIG_DATA4_SPI_MODE_SHIFT 4
#define CONFIG_DATA4_SPI_MODE_MASK 0x3
#define SPI_MODE_0 0
#define SPI_MODE_1 1
#define SPI_MODE_2 2
#define SPI_MODE_3 3

#define CONFIG_DATA4_UART_BAUDRATE_SHIFT 0
#define CONFIG_DATA4_UART_BAUDRATE_MASK 0xf
#define UART_BAUDRATE_DISABLE 0
#define UART_BAUDRATE_9600 1
#define UART_BAUDRATE_19200 2
#define UART_BAUDRATE_28800 3
#define UART_BAUDRATE_38400 4
#define UART_BAUDRATE_57600 5
#define UART_BAUDRATE_115200 6
#define UART_BAUDRATE_250000 7
#define UART_BAUDRATE_500000 8
#define UART_BAUDRATE_1000000 9
/*
	<CMD_CONFIG>
	Command: DATA5
*/
union config_data5{
	struct {
		uint8_t comms_delay: 8;
	}bits;
	uint8_t val;
};
#define CONFIG_DATA5_COMMS_DELAY_SHIFT 0
#define CONFIG_DATA5_COMMS_DELAY_MASK 0xff

/*
	<CMD_CONFIG>
	Command: DATA6
*/
union config_data6{
	struct {
		uint8_t repeat_delay_multiplier: 8;
	}bits;
	uint8_t val;
};
#define CONFIG_DATA6_REPEAT_DELAY_MULTIPLIER_SHIFT 0
#define CONFIG_DATA6_REPEAT_DELAY_MULTIPLIER_MASK 0xff
/*
	<CMD_CONFIG>
	Command: DATA7
*/
union config_data7{
	struct {
		uint8_t iic_retry_delay: 8;
	}bits;
	uint8_t val;
};
#define CONFIG_DATA7_IIC_RETRY_DELAY_SHIFT 0
#define CONFIG_DATA7_IIC_RETRY_DELAY_MASK 0xff
/*
	<CMD_CONFIG>
	Command: DATA8
*/
union config_data8{
	struct {
		uint8_t chg_gpio: 1;
		uint8_t rsv2: 2;
		uint8_t chg_active: 1;
		uint8_t chg_mode: 1;
		uint8_t rsv: 3;
	}bits;
	uint8_t val;
};
#define CONFIG_DATA8_ENABLE_GPIO_AS_CHANGE_SHIFT 4
#define CONFIG_DATA8_ENABLE_GPIO_AS_CHANGE_MASK 0x1
#define ENABLE_GPIO_AS_CHANGE_LED2 0
#define ENABLE_GPIO_AS_CHANGE_GPIO 1

#define CONFIG_DATA8_ACTIVE_LEVEL_SHIFT 3
#define CONFIG_DATA8_ACTIVE_LEVEL_MASK 0x1
#define ACTIVE_LEVEL_LOW 0
#define ACTIVE_LEVEL_HIGH 1

#define CONFIG_DATA8_GPIO_PIN_SHIFT 0
#define CONFIG_DATA8_GPIO_PIN_MASK 0x1
#define GPIO_PIN_0 0
#define GPIO_PIN_1 1
/*
	<CMD_CONFIG>
	Command: DATA9
*/
union config_data9{
	struct {
		uint8_t digitizer_fingers: 8;
	}bits;
	uint8_t val;
};
#define CONFIG_DATA9_NUM_OF_CONTACTS_DIGITIZER_SHIFT 0
#define CONFIG_DATA9_NUM_OF_CONTACTS_DIGITIZER_MASK 0xff
/*
	<CMD_CONFIG>
	Command: DATA10
*/
union config_data10{
	struct {
		uint8_t digitizer_delay: 8;
	}bits;
	uint8_t val;
};
#define CONFIG_DATA10_IDLE_DELAY_DIGITIZER_SHIFT 0
#define CONFIG_DATA10_IDLE_DELAY_DIGITIZER_MASK 0xff
/*
	<CMD_CONFIG>
	Command: DATA11
*/
union config_data11{
	struct {
		uint8_t spi_ss_delay: 8;
	}bits;
	uint8_t val;
};
#define CONFIG_DATA11_SPI_SS_DELAY_SHIFT 0
#define CONFIG_DATA11_SPI_SS_DELAY_MASK 0xff

/*
	<CMD_CONFIG>
	Command: DATA[12-15]
*/

#define CMD_SAVE_CONFIGS_EEPROM 0xEA
/*
	<CMD_SAVE_CONFIGS_EEPROM>
*/
#define CMD_RESTORE_DEFAULT_CONFIGS 0xEB
/*
	<CMD_RESTORE_DEFAULT_CONFIGS>
*/
#define CMD_GET_CONFIG 0x8B
/*
	<CMD_GET_CONFIG>
*/

/*******************************
	Pin Set / Read Functions
*******************************/
#define CMD_CONFIG_READ_PINS 0x81
/*
	<CMD_CONFIG_READ_PINS>
*/

#define CMD_READ_PINS 0x82
/*
	<CMD_READ_PINS>

*/
#define CMD_SET_GPIOS 0x83
/*
	<CMD_SET_GPIOS>
*/

#define CMD_READ_GPIOS 0x84
/*
	<CMD_READ_GPIOS>
*/
#define CMD_PWM 0x85
/*
	<CMD_PWM>
*/

#define PWM_DATA0_START_SHIFT 7
#define PWM_DATA0_START_MASK 0x1
#define STOP_EN 0
#define START_EN 1

#define PWM_DATA0_DRIVE_SHIFT 6
#define PWM_DATA0_DRIVE_MASK 0x1
#define DRIVE_FLOAT 0
#define DRIVE_HIGH 1

#define PWM_DATA0_GPIO_PIN_SHIFT 0
#define PWM_DATA0_GPIO_PIN_MASK 0x7

#define CMD_SET_IO_C 0xE4
/*
	<CMD_SET_IO_C>
*/
#define CMD_GET_IO_C 0xE5
/*
	<CMD_GET_IO_C>
*/
#define CMD_SET_IO_F 0xE6
/*
	<CMD_SET_IO_F>
*/
#define CMD_GET_IO_F 0xE7
/*
	<CMD_GET_IO_F>
*/

/*******************************
	Communications
*******************************/
#define CMD_FIND_IIC_ADDRESS 0xE0
/*
	<CMD_FIND_IIC_ADDRESS>
*/
#define CMD_SPI_UART_DATA 0x50
/*
	<CMD_SPI_UART_DATA>
*/
#define CMD_IIC_DATA_1 0x51
/*
	<CMD_IIC_DATA_1>
*/
#define IIC_DATA_OK 0
#define IIC_DATA_NAK_WRITE 1
#define IIC_DATA_NAK_ADDR 2
#define IIC_DATA_NAK_READ 3 //User add
#define IIC_DATA_FINISHED_WITHOUT_R 4

#define CMD_IIC_DATA_2 0x52
/*
	<CMD_IIC_DATA_2>
*/
#define CMD_REPEAT 0x88
/*
	<CMD_REPEAT>
*/
#define REPEAT_DATA1_OK 0
#define REPEAT_DATA1_FAILED 1

struct config_repeat{
	union {
		struct {
			uint8_t  delay: 3;
			uint8_t chg: 3;
			uint8_t bus: 2;
		}bits;
		uint8_t val;
	} cfg;
	uint8_t lenw;
	uint8_t lenr;
	uint8_t buf[CONF_USB_COMPOSITE_HID_GENERIC_INTIN_MAXPKSZ - 4];
}__packed;

#define REPEAT_BUS_SPI_UART 0
#define REPEAT_BUS_IIC1 1
#define REPEAT_BUS_IIC2 2

#define REPEAT_CHG_BY_TIMER 0
#define REPEAT_CHG_BY_GPIO0_L 1
#define REPEAT_CHG_BY_GPIO0_H 2
#define REPEAT_CHG_BY_GPIO1_L 3
#define REPEAT_CHG_BY_GPIO1_H 4
#define REPEAT_CHG_BY_LED2_L 5
#define REPEAT_CHG_BY_LED2_H 6


#define CMD_AUTO_REPEAT_RESP 0x9A
/*
	<CMD_AUTO_REPEAT_RESP>	
*/

#define CMD_REPEAT_STACK 0x98
/*
	<CMD_REPEAT_STACK>
*/
#define CMD_REPEAT_2_SPI 0x99
/*
	<CMD_REPEAT_2_SPI>
*/

/*******************************
	Hawkeye
*******************************/
#define CMD_START_LISTENING 0xE1
/*
	<CMD_START_LISTENING>
*/
#define CMD_RINGBUFFER_DBG 0xE8
/*
	<CMD_RINGBUFFER_DBG>
*/
#define CMD_STOP_LISTENING 0xE2
/*
	<CMD_STOP_LISTENING>
*/
#define CMD_SEND_DEBUG_DATA 0xE3
/*
	<CMD_SEND_DEBUG_DATA>
*/

/*******************************
	USB Functions
*******************************/

#define CMD_NO_DATA 0x00
/*
	<CMD_NO_DATA>
*/

#define CMD_NULL 0x86
/*
	<CMD_NULL>
*/
#define CMD_START_TEST 0x87
/*
	<CMD_START_TEST>
*/
#define CMD_RESET_BRIDGE 0xEF
/*
	<CMD_RESET_BRIDGE>
*/
#define CMD_JUMP_BOOTLOADER 0xF5
/*
	<CMD_RESET_BRIDGE>
*/
#define CMD_SWITCH_MODE_FAST 0xFA
/*
	<CMD_SWITCH_MODE_FAST>
*/
#define CMD_SWITCH_MODE_5016 0xFB
/*
	<CMD_SWITCH_MODE_5016>
*/
#define CMD_SWITCH_MODE_DEBUG 0xFC
/*
	<CMD_SWITCH_MODE_DEBUG>
*/
#define CMD_SWITCH_MODE_DIGITIZER_SERIAL 0xFD
/*
	<CMD_SWITCH_MODE_DEBUG>
*/
#define CMD_SWITCH_MODE_DIGITIZER_PARALLEL 0xFE
/*
	<CMD_SWITCH_MODE_DEBUG>
*/


/*******************************
	User defined extension 
*******************************/
#define CMD_NONE 0x0
#define CMD_NAK 0xFF
/*
	<CMD_NAK>
		This is response from bridge to host, NAK last command from host
	Command: NA
	Response:
		DATA0: CMD_NAK
*/

#define CMD_EXTENSION_CONFIG 0xF9
/*
	<CMD_EXTENSION_CONFIG>
	Command: 
		DATA1-N: data
	Response:
		DATA0: CMD_EXTENSION_CONFIG 
		DATA1-n: Finial accepted command data content
*/
/*
	<CMD_EXTENSION_CONFIG>
	Command: DATA1
		DATA1:
			[7:4]: Checksum of low byte(Not)
			[3：0]： Com port mode
				0： IIC only
				1:  SPI only
				2:  UART only
				3:  IIC1 + IIC2
				4:  IIC1 + UART
				5:  UART + UART
*/
union config_edat1{
	struct {
		uint8_t com_mode: 4;
		uint8_t checksum: 4;
	}bits;
	uint8_t val;
};
#define EXT_CONFIG_DATA1_COMMUNICATION_MODE_CHECKSUM_SHIFT 4
#define EXT_CONFIG_DATA1_COMMUNICATION_MODE_CHECKSUM_MASK 0xf

#define EXT_CONFIG_COMMUNICATION_MODE_SHIFT 0
#define EXT_CONFIG_COMMUNICATION_MODE_MASK 0xf
#define COM_MODE_IIC_ONLY 0
#define COM_MODE_SPI_ONLY 1
#define COM_MODE_UART_ONLY 2
#define COM_MODE_DUAL_IIC 3
#define COM_MODE_IIC_UART 4
#define COM_MODE_DUAL_UART 5
#define COM_MODE_IIC_UART_SPI_FULL 7

#define DESC_GET(_x, _name) ((_x) >> _name##_SHIFT & _name##_MASK)
#define DESC_SET(_x, _name, _val) (((_x) & ~(_name##_MASK << _name##_SHIFT)) | (((_val) & _name##_MASK) << _name##_SHIFT))

#if defined(UTILS_COMPILER_H_INCLUDED)
#define SET_BIT(_x, _bit) Set_bits((_x), (1 << (_bit)))
#define CLR_BIT(_x, _bit) Clr_bits((_x), (1 << (_bit)))
#define TEST_BIT(_x, _bit) Tst_bits((_x), (1 << (_bit)))
#else
#define SET_BIT(_x, _bit) ((_x) |= (1 << (_bit)))
#define CLR_BIT(_x, _bit) ((_x) &= ~(1 << (_bit)))
#define TEST_BIT(_x, _bit) ((_x) & (1 << (_bit)))
#endif
#define SET_AND_CLR_BIT(_x, _sbit, _cbit) (SET_BIT((_x), (_sbit)), CLR_BIT((_x), (_cbit)))

typedef union {
	uint8_t b[4];
	uint32_t value;
}__packed crc_32_t;

typedef struct {
	union config_data1 data1;
	union config_data2 data2;
	union config_data3 data3;
	union config_data4 data4;
	union config_data5 data5;
	union config_data6 data6;
	union config_data7 data7;
	union config_data8 data8;
	union config_data9 data9;
	union config_data10 data10;
	union config_data11 data11;
}__packed base_setting_t;

typedef struct {
	union config_edat1 data1;
}__packed ext_setting_t;

typedef struct {
	struct config_repeat repeat;
}__packed dym_setting_t;

#define CONFIG_FLAG_BASE_DIRTY_SHIFT 0
#define CONFIG_FLAG_BASE_DIRTY_MASK 0x1

#define CONFIG_FLAG_EXT_DIRTY_SHIFT 1
#define CONFIG_FLAG_EXT_DIRTY_MASK 0x1

typedef struct {
	base_setting_t base;
	ext_setting_t ext;
	crc_32_t crc;

	dym_setting_t dym;
}__packed config_setting_t;

typedef int32_t (*cmd_func) (void *host, uint8_t cmd,  const uint8_t *data, uint32_t count);

struct cmd_func_map{
	uint8_t cmd;
	cmd_func func;
};

#ifdef BOARD_BRIDGE_D21
#define GPIO_P_0 GP_TP2
#define GPIO_P_1 GP_CHG
#define GPIO_P_LED2 GP_IO0
#else
#define GPIO_P_0 USB_ID	//Button on L21 board
#define GPIO_P_1 GP_CHG
#define GPIO_P_LED2 GP_IO0
#endif

int32_t u5030_process_data(const uint8_t *data, uint32_t count);
int32_t u5030_get_response(void **buf_ptr, uint32_t *buf_ptr_size);
int32_t u5030_clear_cache(void);
int32_t u5030_init(void);
void u5030_deinit(void);

#endif /* PROTOCOL_U5030_H_ */