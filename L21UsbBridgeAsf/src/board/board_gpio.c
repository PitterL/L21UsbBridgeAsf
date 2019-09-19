#include <compiler.h>
#include <board.h>
#include <conf_board.h>
#include <delay.h>

#include "board_gpio.h"
#include "external/utils.h"

struct pin_config{
    /** Pin **/
    uint8_t pin;

    /** config **/
    struct port_config config;

    /** default level (output only)**/
    bool level;

};

static struct pin_config g_board_pins_list[] = 
{
    //USB_ID
    {PIN_PA02, {PORT_PIN_DIR_INPUT, PORT_PIN_PULL_UP}, false},

    //GP_TP2
    {PIN_PA08, {PORT_PIN_DIR_INPUT, PORT_PIN_PULL_UP}, false},

    //GP_IO0
    {PIN_PB10, {PORT_PIN_DIR_INPUT, PORT_PIN_PULL_UP}, false},

    //GP_IO1
    {PIN_PB11, {PORT_PIN_DIR_OUTPUT, PORT_PIN_PULL_NONE}, false},

    //GP_IO2
    {PIN_PB12, {PORT_PIN_DIR_OUTPUT, PORT_PIN_PULL_NONE}, false},

    //GP_RST
    {PIN_PB13, {PORT_PIN_DIR_OUTPUT, PORT_PIN_PULL_NONE}, false},

    //GP_CHG
    {PIN_PB14, {PORT_PIN_DIR_INPUT, PORT_PIN_PULL_UP}, false},

    //GP_TRANS_EN
    {PIN_PB15, {PORT_PIN_DIR_OUTPUT, PORT_PIN_PULL_NONE}, false},

    //GP_IO3
    {PIN_PA12, {PORT_PIN_DIR_OUTPUT, PORT_PIN_PULL_NONE}, false},

    //GP_IO4
    {PIN_PA13, {PORT_PIN_DIR_OUTPUT, PORT_PIN_PULL_NONE}, false},

    //GP_VBUS_EXIST
    {PIN_PA14, {PORT_PIN_DIR_INPUT, PORT_PIN_PULL_NONE}, false},

    //GP_WIFI_WAKE
    {PIN_PA15, {PORT_PIN_DIR_OUTPUT, PORT_PIN_PULL_NONE}, false},

    //GP_IO5
    {PIN_PA18, {PORT_PIN_DIR_OUTPUT, PORT_PIN_PULL_NONE}, false},

    //GP_VDD33_EN
    {PIN_PA19, {PORT_PIN_DIR_OUTPUT, PORT_PIN_PULL_NONE}, true},

    //GP_IO_BTLD
    {PIN_PA20, {PORT_PIN_DIR_INPUT, PORT_PIN_PULL_NONE}, false},

    //GP_VDD5_EN
    {PIN_PA21, {PORT_PIN_DIR_OUTPUT, PORT_PIN_PULL_NONE}, false},

    //GP_IO_PWR
    {PIN_PA22, {PORT_PIN_DIR_INPUT, PORT_PIN_PULL_NONE}, false},

    //GP_IO_LED2_EN
    {PIN_PA23, {PORT_PIN_DIR_OUTPUT, PORT_PIN_PULL_NONE}, true},

    //GP_WIFI_EN
    {PIN_PA27, {PORT_PIN_DIR_OUTPUT, PORT_PIN_PULL_NONE}, false},

    //GP_WIFI_RST
    {PIN_PB30, {PORT_PIN_DIR_OUTPUT, PORT_PIN_PULL_NONE}, true},

    //GP_VDDIO_EN
    {PIN_PB31, {PORT_PIN_DIR_OUTPUT, PORT_PIN_PULL_NONE}, true},
};

struct mux_config {
    /** pad **/
    uint32_t pad;

    /** Port buffer input/output direction */
	enum system_pinmux_pin_dir direction;

    /** Logic level pull of the input buffer */
	enum system_pinmux_pin_pull input_pull;

    bool powersave;
};

static struct mux_config g_board_ports_list[] =
{
    //SERCOM1: IIC
    {PINMUX_PA16C_SERCOM1_PAD0, SYSTEM_PINMUX_PIN_DIR_OUTPUT_WITH_READBACK, SYSTEM_PINMUX_PIN_PULL_UP, false},
    {PINMUX_PA17C_SERCOM1_PAD1, SYSTEM_PINMUX_PIN_DIR_OUTPUT_WITH_READBACK, SYSTEM_PINMUX_PIN_PULL_UP, false},
    
    //SERCOM0: UART
    {PINMUX_PA04D_SERCOM0_PAD0, SYSTEM_PINMUX_PIN_DIR_OUTPUT, SYSTEM_PINMUX_PIN_PULL_NONE, false},
    {PINMUX_PA07D_SERCOM0_PAD3, SYSTEM_PINMUX_PIN_DIR_INPUT, SYSTEM_PINMUX_PIN_PULL_NONE, false},
};

void board_gpio_init(void)
{
    const struct pin_config *pin_cfg;
    const struct mux_config *mux_cfg;
    struct system_pinmux_config port_conf;
    uint32_t i;
	bool en;

    for(i = 0; i < ARRAY_SIZE(g_board_pins_list); i++) {
        pin_cfg = &g_board_pins_list[i];

        if (pin_cfg->config.direction == PORT_PIN_DIR_OUTPUT)
            port_pin_set_output_level(pin_cfg->pin, pin_cfg->level);
        port_pin_set_config(pin_cfg->pin, &pin_cfg->config);
    }
	
    for (i = 0; i < ARRAY_SIZE(g_board_ports_list); i++) {
        mux_cfg = &g_board_ports_list[i];
        
        port_conf.mux_position = mux_cfg->pad & 0xFFFF;
        port_conf.direction = mux_cfg->direction;
        port_conf.input_pull = mux_cfg->input_pull;
        port_conf.powersave = mux_cfg->powersave;

        system_pinmux_pin_set_config(mux_cfg->pad >> 16, &port_conf);
    }

    //por_on(false);

    en = pwr_switch_on();
    por_on(en);
}

bool pwr_switch_on(void)
{
    return port_pin_get_input_level(GP_IO_PWR) == false;
}

void pwr_5v_en(bool en)
{
    port_pin_set_output_level(GP_VDD5_EN, en);
}

void pwr_3v3_en(bool en)
{
    port_pin_set_output_level(GP_VDD33_EN, !en);
}

void pwr_vio_en(bool en)
{
    port_pin_set_output_level(GP_VDDIO_EN, !en);
}

void io_trans_en(bool en)
{
    port_pin_set_output_level(GP_TRANS_EN, en);
}

void io_rst_en(bool en)
{
    port_pin_set_output_level(GP_RST, en);
}

void por_on(bool en)
{
    if (en) {
        //io_rst_en(true);  //In GPIO init, RST default value been set
        io_trans_en(true);

        pwr_vio_en(true);
        pwr_3v3_en(true);
        pwr_5v_en(true);
        //here need a delay for at least 90ns
        delay_ms(5);

        io_rst_en(false);
        //delay 150ms for reset finished, but i don't wanto wait so long
        delay_ms(150);
    }else{
        pwr_5v_en(false);
        pwr_3v3_en(false);
        pwr_vio_en(false);
        io_rst_en(true);

        delay_ms(20);
    }
}