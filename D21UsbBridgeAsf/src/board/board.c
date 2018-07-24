#include <delay.h>
#include "board.h"

void platform_board_init(void)
{
    delay_init();

    board_gpio_init();
}