#include "nrf_delay.h"
#include "boards.h"
#include "esl_blinky.h"

void blink_with_id(int* blinks_count_list)
{
    for (int led_number = 0; led_number < LEDS_NUMBER; led_number++)
    {
        for (int i = 0; i < *(blinks_count_list + led_number); i++)
        {
            bsp_board_led_invert(led_number);
            nrf_delay_ms(500);
            bsp_board_led_invert(led_number);
            nrf_delay_ms(500);
        }
        nrf_delay_ms(1000);
    }
}
