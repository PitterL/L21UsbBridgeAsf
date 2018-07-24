/*
 * protocol_u5030.h
 *
 * Created: 6/20/2018 6:58:25 PM
 *  Author: A41450
 */ 

#ifndef PROTOCOL_U5030_H_
#define PROTOCOL_U5030_H_
/*
Communications

[USB5016 Compatible]
5016-MK2
VID: 0x3EFF Holtek
PID: 0x1398 5016 Bridge

[USB FS Bridge]
QRG-I/F
VID: 0x03EB Atmel
PID: 0x6123 5030 Bridge

[Hawkeye Server]
QRG-DBG
VID: 0x03EB Atmel
PID: 0x6123 5030 Bridge

[Touch-screen Digitizer / Serial Mode]
VID: 0x03EB Atmel
PID: 0x2118 mXT Touchscreen digitizer, serial reporting
Composite device:
Interface 0: Atmel mXT Digitizer (S) Touchscreen digitizer interface
Interface 1: Atmel maXTouch Control 5030 Bridge

[Touchscreen Digtizer / Parallel Mode]
VID: 0x03EB Atmel
PID: 0x2119 mXT Touchscreen digitizer, parallel reporting
Composite device:
Interface 0: Atmel mXT Digitizer (S) Touch-screen digitizer interface
Interface 1: Atmel maXTouch Control 5030 Bridge
This document details communications with the 5030 Bridge, available in various modes as above.

[Packets sent to the bridge]:
Command Data 1 Data 2 Data 3 ... Data 63

[Packets received from the bridge]:
Response ID Data 1 Data 2 Data 3 ... Data 63
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
		Sets the parameters for the communication modes supported by the USB5030.
	Command: [DATA1] ~ [DATA12]
	Response:
		DATA0: CMD_CONFIG 
		DATA1-n: Command data content
*/
/*
	<CMD_CONFIG>
	Command: DATA1
		uart_half_duplex: Bit 7
		Implements half-duplex UART communications
			1 = on (half duplex)
			0 = off (full duplex)
		use_drdy: Bit 6
		Use DRDY pin for SPI / UART Comms
			0 = off Ignore DRDY Pin
			1 = on Wait for DRDY high / 100ms timeout
		iic_clk: Bit5:4
		Clock rate for IIC Comms
			00 = 50kHz
			01 = 100kHz
			10 = 200kHz
			11 = 400kHz
		ss_pulse: Bit 3
		Pulse SS pin before SPI / UART Comms
			0 = off
			1 = on
		Generates a 20us low pulse on SS, then delays for 1ms
		before starting comms
		spi_clk: Bits2:0
			0 = 25kHz
			1 = 50kHz
			2 = 100kHz
			3 = 200kHz
			4 = 500kHz
			5 = 1MHz
			6 = 2MHz
			7 = 4MHz

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
		iic_retry: Bit 7
		Retry IIC comms after Address – NACK
			0 = off
			1 = on
		Iic_address_1: Bit6:0
			7-bit Hardware address for IIC device 1
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
		ss_mode: Bit 7
		Selects whether to pull the SS pin high between bytes
			0 = Pull high (Byte framing)
			1 = Clamp low (Packet framing)
		Iic_address_2: Bits 6:0
			7-bit Hardware address for IIC device 2
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
		iic_restart_enabled: Bit 7
		Selects whether IIC communications should use restarts
			0 = No restarts (Write – Stop – Start – Read)
			1 = Use restarts (Write – ReStart – Read)
		Spi_mode: Bits 5:4
			0 = CPOL: 1, CPHA: 1
			1 = CPOL: 0, CPHA: 1
			2 = CPOL: 1, CPHA: 0
			3 = CPOL: 0, CPHA: 0
		uart_baudrate: Bits 3:0
			0 = UART disabled (SPI mode)
			1 = 9.6k baud
			2 = 19.2k baud
			3 = 28.8k baud
			4 = 38.4k baud
			5 = 57.6k baud
			6 = 115.2k baud
			7 = 250k baud
			8 = 500k baud
			9 = 1M baud
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
		[comms_delay]
		Communications delay to allow processing. Specified in uS, minimum = 1us, default = 200us.
		Delay is applied
			- At each IIC Start condition
			- After each UART / SPI byte exchange
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
		[repeat_delay_multiplier]
		Applied to timing in ‘repeat’ modes, where the selected repeat
		delay is multiplied by repeat_delay_multiplier to calculate the
		polling rate in ms
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
		[iic_retry_delay]
		After an address NACK on IIC communications, the USB5030
		waits for (iic_retry_delay / 8) ms before re-trying
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
		Selects ‘change’ pin and active level for mXT implementations of
		touchscreen digitizer and mouse emulation
		Enable GPIO as change: Bit 4
			0 = Use default (LED 2 IO (Pin 46) Active Low)
			1 = Use GPIO
		Active level: Bit 3
			0 = Low
			1 = High
		GPIO pin used: Bit 0 (GPIO 0 or 1)
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
		Number of contacts to report over digitizer interface
		The number of contacts which will be reported over the digitizer
		interface.
		Default: 10
		Max: 10
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
		IDLE delay for digitizer(s)
		Note: Applies to Parallel report only
		If there are contacts present but not changing / moving, digitizer contact
		report is re-sent to the host after IDLE x 4ms
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
		SPI SS Delay
		Delay in microseconds to be applied after SS goes low and before SCK
		starts and after SCK finishes before SS goes high. Minimum setting is 5us,
		default of 22us applied where config is < 5us.
		Minimum: 5us
		Default: 22us
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
		Bulk mode supoort Tag
		DATA[12]: 0xAA
		DATA[13]: 0x55
		DATA[14]: 0xAA
		DATA[15]: 0xFF
	Response:
		DATA[12]: NC
		DATA[13]: 0x55
		DATA[14]: 0xAA
		DATA[15]: NC
	//For MTA 2.9.9, response check 12~15, 2.9.14 response check 13/14
*/

#define CMD_SAVE_CONFIGS_EEPROM 0xEA
/*
	<CMD_SAVE_CONFIGS_EEPROM>
		Stores the current parameter states to EEPROM, to be reloaded on power-on / reset
	Command: NA
	Response:
		DATA0: CMD_SAVE_CONFIGS_EEPROM
*/
#define CMD_RESTORE_DEFAULT_CONFIGS 0xEB
/*
	<CMD_RESTORE_DEFAULT_CONFIGS>
		Erases parameter states from EEPROM, reloading default settings
	Command: NA
	Response: 
		DATA0: CMD_RESTORE_DEFAULT_CONFIGS
*/
#define CMD_GET_CONFIG 0x8B
/*
	<CMD_GET_CONFIG>
		Reads the parameters for communications modes supported by the USB bridge
	Command: NA
	Response: Same as CMD_CONFIG
*/

/*******************************
	Pin Set / Read Functions
*******************************/
#define CMD_CONFIG_READ_PINS 0x81
/*
	<CMD_CONFIG_READ_PINS>
		Sets the parameters for the communication modes supported by the USB5030.
		Returns the current pin states and ddr register contents for the IO pins used for
		communications.
	Command: TBD
	Response:
			DATA0: CMD_CONFIG_READ_PINS
		//PIN states
			DATA1: [GPIO_3, DRDY, SCK, SS, MOSI, MISO, SDA, SCL]
			DATA2: [ , , , ,LED2, GPIO1, GPIO0, GPIO2]

		//DDR states
			DATA3: Same as DATA1
			DATA4: Same as DATA2 
*/

#define CMD_READ_PINS 0x82
/*
	<CMD_READ_PINS>
		Returns the current pin states and ddr register contents for the IO pins used for
		communications.
	Command: NA
	Resonse:
		Same as CMD_CONFIG_READ_PINS

*/
#define CMD_SET_GPIOS 0x83
/*
	<CMD_SET_GPIOS>
		Sets the DDR and PORT registers of the USB5030 auxiliary GPIO pins.
		Returns the contents of the DDR and PORT registers, and the states of the pins.
	Command: 
		//DDR settings
			DATA[1]: input or output for [E7, E6, GPIO3, DRDY, GPIO2, LED2, GPIO1, GPIO0] (details should see code)
		//PORT register settings
			DATA2: if input, the config of pullup; if out port, the level of output 
		[USER extension]
		//MASK settings: 
			DATA[3]: if masked, skip to operate this pin
		//Toggle delay: 
			DATA[4]: the Toggle delay value(ms), if set, the pin will excute toggle after the delay
	Response:
			DATA[0]: CMD_SET_GPIOS 
		//DDR settings
			DATA[1]: same as command
		//PIN states
			DATA[2]: Pin states
		//PORT register contents
			DATA[3]: same as command
		[USER extension]
		//MASK settings: 
			DATA[4]: same as command
		//Toggle delay
			DATA[5]: same as command
*/

#define CMD_READ_GPIOS 0x84
/*
	<CMD_READ_GPIOS>
		Returns the contents of the DDR and PORT registers, and the states of the USB5030
		auxiliary GPIO pins.
	Command: NA
	Response: Same as CMD_SET_GPIOS
*/
#define CMD_PWM 0x85
/*
	<CMD_PWM>
		Controls generation of a PWM signal at the selected GPIO pin.
	Command: 
		DATA1:	Config
		DATA2:  Low time
		DATA3:	High time
	Response:
		DATA0:  CMD_PWM
		DATA1-3: As per command data
*/
/*
	<CMD_PWM>
	Command:
		DATA1:
			Start: Bit 7
				0 = Stop
				1 = Start
			Drive: Bit 6
				0 = Float (Release) pin for duration of high time
				1 = Drive pin high for duration of high time
			GPIO number: Bits 2:0
				Number of GPIO pin (0 to 3) for PWM signal
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
		Sets the DDR and PORT registers of Port C
	Command: 
		//DDR Register
		DATA1:	
		//PORT Register
		DATA2:
	Response: 
		DATA0: CMD_SET_IO_C
		DATA1-2: As per command data
*/
#define CMD_GET_IO_C 0xE5
/*
	<CMD_GET_IO_C>
		Reads the DDR, PIN and PORT registers of Port C
	Command: NA
	Response:
		DATA0: CMD_GET_IO_C
		//DDR Register
		DATA1:	
		//PIN Register
		DATA2:
		//PORT Register
		DATA3:
*/
#define CMD_SET_IO_F 0xE6
/*
	<CMD_SET_IO_F>
		Sets the DDR and PORT registers of Port F
	Command: 
		//DDR Register
		DATA1:	
		//PORT Register
		DATA2:
	Response: 
		DATA0: CMD_SET_IO_F
		DATA1-2: As per command data
*/
#define CMD_GET_IO_F 0xE7
/*
	<CMD_GET_IO_F>
		Reads the DDR, PIN and PORT registers of Port F
	Command: NA
	Response:
		DATA0: CMD_GET_IO_F
		//DDR Register
		DATA1:	
		//PIN Register
		DATA2:
		//PORT Register
		DATA3:
*/

/*******************************
	Communications
*******************************/
#define CMD_FIND_IIC_ADDRESS 0xE0
/*
	<CMD_FIND_IIC_ADDRESS>
		Hunts for an IIC device at any hardware address, applies the address to the
		communications parameter iic_address_1
	Command: NA
	Response:
		DATA0: CMD_FIND_IIC_ADDRESS
		DATA1:
			0x81: No device found
			<Other>: iic address
*/
#define CMD_SPI_UART_DATA 0x50
/*
	<CMD_SPI_UART_DATA>
		Communications over SPI or UART.
	Command: 
		DATA1: Number of bytes to write
		DATA2: Number of bytes to read
		DATA3-63: transfer data content
	Response:
		DATA0: 
			0: Tranfer OK
			3: Timeout
		DATA1:	Bytes return
		DATA2-63: received data content
*/
#define CMD_IIC_DATA_1 0x51
/*
	<CMD_IIC_DATA_1>
		Communications to IIC Device 1
	Command: 
		DATA1: Number of bytes to write
		DATA2: Number of bytes to read
		DATA3-63: transfer data content
	Response:
		DATA0: 
			0: Transfer OK
			1: IIC NAK to write
			2: IIC NAK to address
			4: Wrtiting finished, without read operation 
		DATA1:	Bytes return
		DATA2-63: received data content
*/
#define IIC_DATA_OK 0
#define IIC_DATA_NAK_WRITE 1
#define IIC_DATA_NAK_ADDR 2
#define IIC_DATA_NAK_READ 3 //User add
#define IIC_DATA_FINISHED_WITHOUT_R 4

#define CMD_IIC_DATA_2 0x52
/*
	<CMD_IIC_DATA_2>
		Same as above, I2C devices 2
*/
#define CMD_REPEAT 0x88
/*
	<CMD_REPEAT>
		Configures the bridge to autonomously carry out communications over IIC, SPI or UART
		and return data to the host after each exchange.
	Command: 
		DATA1:
			[7:6]: 
				0: SPI/UART
				1: IIC1
				2: IIC2
			[5:3]:
				0: Timed
				1: GPIO0 Low
				2: GPIO0 High
				3: GPIO1 Low
				4: GPIO1 High
				5: LED2 Low
				6: LED2 High
			[2:0]:
				Trigger rate(ms): *CONFIG_DATA6_REPEAT_DELAY_MULTIPLIER
		DATA2: Number of bytes to write
		DATA3: Number of bytes to read
		DATA4-63: transfer data content
	Response:
		DATA0: CMD_REPEAT
		DATA1:
			0: OK
			1: Failed
	Repeat Response:
		DATA0: CMD_AUTO_REPEAT_RESP
		DATA1:
			0: Transfer OK
			1: IIC NAK to write
			2: IIC NAK to address
			3: SPI/UART Timeout
			4: Wrtiting finished, without read operation 
		DATA2-63: received data content
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
		autonomously carry out communications over IIC, SPI or UART
		and return data to the host after each exchange.
	Command: NA(Auto run after CMD_REPEAT set)
	Repeat Response:
		DATA0: CMD_AUTO_REPEAT_RESP
		DATA1:
			0: Transfer OK
			1: IIC NAK to write
			2: IIC NAK to address
			3: SPI/UART Timeout
			4: Wrtiting finished, without read operation 
		DATA2-63: received data content	
*/

#define CMD_REPEAT_STACK 0x98
/*
	<CMD_REPEAT_STACK>
		Configures the bridge to autonomously read multiple messages from a message server
		over IIC.
	Command: 
		DATA1:
			[4:3]: 
				1: IIC1
				2: IIC2
			[2:0]:
				0: Timed
				1: GPIO0 Low
				2: GPIO0 High
				3: GPIO1 Low
				4: GPIO1 High
				5: LED2 Low
				6: LED2 High
		DATA2: Message size
		DATA3: CHG re-assert timeout(us)
	Response:
		DATA0： CMD_REPEAT_STACK
		DATA1:
			0: OK
			1: Failed
	Repeat Response:
		DATA0: 1|Num Message|0x0E
		DATA1:
			0: Transfer OK
			1: IIC NAK to write
			2: IIC NAK to address 
*/
#define CMD_REPEAT_2_SPI 0x99
/*
	<CMD_REPEAT_2_SPI>
		Configures the bridge to bundle autonomous communications with 2 SPI devices into a
			single USB packet.
		One of the GPIO pins 0 to 3 is used to switch the SS pin between two target devices on
			the SPI bus. The bridges holds this line ‘high’ to select device 1, and holds it low to select
			device 2.
	Command: 
		DATA1:
			[7:6]:The GPIO (0 to 3) which switches SS between the two
				   target SPI devices
			[5:3]:
				0: Timed
				1: GPIO0 Low
				2: GPIO0 High
				3: GPIO1 Low
				4: GPIO1 High
				5: LED2 Low
				6: LED2 High
			[2:0]:
				Trigger rate(ms): *CONFIG_DATA6_REPEAT_DELAY_MULTIPLIER
		DATA2: Number of bytes to exchange with SPI1
		DATA3: Number of bytes to exchange with SPI2
		DATA4~(4 + nSPI1 -1): transfer data content with SPI1
		DATA(4 + nSPI1)~(4 + nSPI1 + nSPI2 -1):transfer data content with SPI2
	Response:
		DATA0: CMD_REPEAT
		DATA1:
			0: OK
			1: Failed
	Repeat Response:
		DATA0: 0x9B
		DATA1:
			0: Transfer OK
			3: SPI/UART Timeout
		DATA2~(2 + nSPI1 -1): received data content with SPI1
		DATA(4 + nSPI1)~(4 + nSPI1 + nSPI2 -1):received data content with SPI2
*/

/*******************************
	Hawkeye
*******************************/
#define CMD_START_LISTENING 0xE1
/*
	<CMD_START_LISTENING>
		Activates the SPI Slave input for hawkeye-compatible data capture.
	Command: NA
	Response: NA

	Hawkeye Data packet: 64 Data bytes, no Response ID.
		DATA0: Packet number
		DATA1: Number of Packets
		DATA2: Frame number
		DATA3–64: SPI Captured bytes
*/
#define CMD_RINGBUFFER_DBG 0xE8
/*
	<CMD_RINGBUFFER_DBG>
		Activates SPI slave input in ringbuffer mode
	Command: NA
	Response: NA

	Data SPI:
		DATA0: CMD_RINGBUFFER_DEBUG
		DATA1: Number of data bytes in this packet
		DATA2-64: SPI Captured Bytes
*/
#define CMD_STOP_LISTENING 0xE2
/*
	<CMD_STOP_LISTENING>
		De-activates the SPI Slave input for hawkeye-compatible data capture
	Command: NA
	Response: NA
*/
#define CMD_SEND_DEBUG_DATA 0xE3
/*
	<CMD_SEND_DEBUG_DATA>
		Sends the current contents of the debug data buffer
	Command: TBD
	Response: TBD
*/

/*******************************
	USB Functions
*******************************/

#define CMD_NO_DATA 0x00
/*
	<CMD_NO_DATA>
		To cancel Repeat / Test modes without response
	Command: NA
	Response: NA
*/

#define CMD_NULL 0x86
/*
	<CMD_NULL>
		To cancel Repeat / Test modes with response
	Command: NA
	Response: 
		DATA0: CMD_NULL
		[IF support bulk command]
			DATA1: 0x55
			DATA2: 0xAA
			DATA3: verion LSB
			DATA4: verson MSB
		[ELSE]
			DATA[1~7]: Zero
		
		VESRION >= 5, released version
*/
#define CMD_START_TEST 0x87
/*
	<CMD_START_TEST>
		Returns an incrementing byte at Response ID, to test USB communications.
	Command: NA
	Response: 
		DATA0: Incrementing byte at 1ms intervals
*/
#define CMD_RESET_BRIDGE 0xEF
/*
	<CMD_RESET_BRIDGE>
		Soft reset the bridge
	Command: NA
	Response: NA
*/
#define CMD_JUMP_BOOTLOADER 0xF5
/*
	<CMD_RESET_BRIDGE>
		Resets the USB5030, jumping to the ROM location of the factory-programmed USB
		Bootloader to allow firmware update using Atmel’s FLIP application.
	Command: 
		DATA1: 0xA5
		DATA2: 0x05
		DATA3: 0x5F
		DATA4: 0x50
	Response: NA
*/
#define CMD_SWITCH_MODE_FAST 0xFA
/*
	<CMD_SWITCH_MODE_FAST>
		Switches the USB5030 to USB FS Bridge mode
	Command: 
		DATA1: 
			0xE7:
				Make default (power-on / reset) mode
			<Other>:
				NA
	Response: NA
*/
#define CMD_SWITCH_MODE_5016 0xFB
/*
	<CMD_SWITCH_MODE_5016>
		Switches the USB5030 to USB5016 Compatible mode
	Command: 
		DATA1: 
			0xE7:
				Make default (power-on / reset) mode
			<Other>:
				NA
	Response: NA
*/
#define CMD_SWITCH_MODE_DEBUG 0xFC
/*
	<CMD_SWITCH_MODE_DEBUG>
		Switches the USB5030 to Hawkeye Server mode
	Command: 
		DATA1: 
			0xE7:
				Make default (power-on / reset) mode
			<Other>:
				NA
	Response: NA
*/
#define CMD_SWITCH_MODE_DIGITIZER_SERIAL 0xFD
/*
	<CMD_SWITCH_MODE_DEBUG>
		Switches the USB5030 to Touch-screen Digitizer mode with serial reporting
	Command: 
		DATA1: 
			0xE7:
				Make default (power-on / reset) mode
			<Other>:
				NA
	Response: NA
*/
#define CMD_SWITCH_MODE_DIGITIZER_PARALLEL 0xFE
/*
	<CMD_SWITCH_MODE_DEBUG>
		Switches the USB5030 to Touch-screen Digitizer mode with parallel reporting
	Command: 
		DATA1: 
			0xE7:
				Make default (power-on / reset) mode
			<Other>:
				NA
	Response: NA
*/


/*******************************
	User extension 
*******************************/
#define CMD_NONE 0x0
/*
	<CMD_NONE>
		This is micro definition
	Command: NA
	Response: NA
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

#define CMD_SET_GPIO_EXT 0x73
/*
	<CMD_SET_GPIO_EXT>
	Command: 
	Response:
*/


#define CMD_NAK 0xFF
/*
	<CMD_NAK>
		This is response from bridge to host, NAK last command from host
	Command: NA
	Response:
		DATA0: CMD_NAK
*/

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

#ifdef BOARD_D21
#define GPIO_P_0 GP_TP2
#define GPIO_P_1 GP_CHG
#define GPIO_P_LED2 GP_IO_LED2_EN
#define CHG_ACTIVE_LEVEL_INVERT
#else
#define GPIO_P_0 USB_ID	//Button on L21 board
#define GPIO_P_1 GP_CHG
#define GPIO_P_LED2 GP_IO_LED2_EN
#endif


int32_t u5030_parse_command(void *host, const uint8_t *data, uint32_t count);
bool u5030_chg_line_active(void *host);
int32_t set_bridge_ext_config(void *host, uint8_t cmd, const uint8_t *data, uint32_t count);
int32_t u5030_init(void);
void u5030_deinit(void);

#endif /* PROTOCOL_U5030_H_ */