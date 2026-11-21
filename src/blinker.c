#include <stdint.h>
#include <stdbool.h>
#include "pca10059.h"
#include "nrf_delay.h"
#include "nrfx_systick.h"

#include "app_config.h"
#include "blinker.h"
#include "blinker_button.h"
#include "blinker_pwm.h"

#define DUTY_CHANGE_DELAY_US (DUTY_CHANGE_DELAY_MS * 1000UL)

static nrfx_systick_state_t duty_change_timer;
static int led_number;

static const uint32_t leds[] = LEDS_LIST;

static void leds_init();
static void task_pin_on();
static void task_pin_off();
static void pin_on(uint32_t pin);
static void pin_off(uint32_t pin);

void setup()
{
    leds_init();
    setup_pwm();
    setup_button();
    nrfx_systick_get(&duty_change_timer);
}

void blink_with_id(const int* blinks_count_list)
{
    for (led_number = 0; led_number < LEDS_NUMBER; led_number++)
    {
        pwm_on = task_pin_on;
        pwm_off = task_pin_off;

        for (int blink_number = 0; blink_number < *(blinks_count_list + led_number);)
        {
            duty_cycle();
            nrf_gpio_pin_write(leds[led_number], !LEDS_ACTIVE_STATE);
            blink_number++;

            nrf_delay_ms(BLINK_DELAY_MS);
        }
    }
}

void duty_cycle()
{
    uint32_t duty = 0;
    bool is_duty_up = true;
    bool blink_state = false;

    while (duty > 0 || is_duty_up)
    {
        if (blink_state && nrfx_systick_test(&duty_change_timer, DUTY_CHANGE_DELAY_US))
        {
            nrfx_systick_get(&duty_change_timer);
            if (duty >= PWM_MAX_DUTY)
            {
                is_duty_up = false;
            }

            if (is_duty_up)
            {
                duty++;
            }
            else
            {
                duty--;
            }
        }
        
        pwm_write(leds[led_number], duty);

        if (get_is_double_click())
        {
            blink_state = !blink_state;
        }
    }

    is_duty_up = true;
}

static void leds_init()
{
    for (int i = 0; i < LEDS_NUMBER; i++)
    {
        nrf_gpio_cfg_output(leds[i]);
        nrf_gpio_pin_write(leds[i], !LEDS_ACTIVE_STATE);
    }
}