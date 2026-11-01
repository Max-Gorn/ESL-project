#include <stdint.h>
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "pca10059.h"
#include "blinker.h"

static const uint32_t leds[] = LEDS_LIST;
static const uint32_t buttons[] = BUTTONS_LIST;

void setup()
{
    for (int i = 0; i < LEDS_NUMBER; i++)
    {
        nrf_gpio_cfg_output(leds[i]);
        nrf_gpio_pin_write(leds[i], !LEDS_ACTIVE_STATE);
    }

    for (int i = 0; i < BUTTONS_NUMBER; i++)
    {
        nrf_gpio_cfg_input(buttons[i], BUTTON_PULL);
    }
}

void blink_with_id(const int* blinks_count_list, int delay)
{
    int led_state = !LEDS_ACTIVE_STATE;
    for (int led_number = 0; led_number < LEDS_NUMBER; led_number++)
    {
        for (int blink_number = 0; blink_number < *(blinks_count_list + led_number);)
        {
            if (nrf_gpio_pin_read(BUTTON_1) == BUTTONS_ACTIVE_STATE)
            {
                nrf_gpio_pin_toggle(leds[led_number]);
                nrf_delay_ms(delay);
                
                if (!led_state)
                    blink_number++;
                
                led_state = !led_state;
            }
        }
    }
}
