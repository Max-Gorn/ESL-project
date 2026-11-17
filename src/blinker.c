#include <stdint.h>
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrfx_gpiote.h"
#include "nrfx_systick.h"
#include "pca10059.h"
#include "blinker.h"

#define PWM_FREQ_HZ 1000
#define PWM_PERIOD_US 1000000/PWM_FREQ_HZ

#define DUTY_CHANGE_DELAY_MS 5

static void leds_init();
static void buttons_init();

static const uint32_t leds[] = LEDS_LIST;
static const uint32_t buttons[] = BUTTONS_LIST;

static nrfx_systick_state_t pwm_timer;
static nrfx_systick_state_t blink_timer;

void setup()
{
    leds_init();
    buttons_init();
    nrfx_systick_init();
    nrfx_systick_get(&pwm_timer);
    nrfx_systick_get(&blink_timer);
}

void blink_with_id(const int* blinks_count_list, int delay)
{
    int duty = 0;
    int is_duty_up = 1;
    int pulse_width;

    for (int led_number = 0; led_number < LEDS_NUMBER; led_number++)
    {
        for (int blink_number = 0; blink_number < *(blinks_count_list + led_number);)
        {
            while (duty > 0 || is_duty_up)
            {
                pulse_width = duty * PWM_PERIOD_US / 100;
                if (nrf_gpio_pin_read(BUTTON_1) == BUTTONS_ACTIVE_STATE && nrfx_systick_test(&blink_timer, DUTY_CHANGE_DELAY_MS * 1000))
                {
                    nrfx_systick_get(&blink_timer);
                    if (duty >= 100)
                    {
                        is_duty_up = 0;
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

                nrfx_systick_get(&pwm_timer);
                while (!nrfx_systick_test(&pwm_timer, PWM_PERIOD_US))
                {
                    if (duty > 0)
                        nrf_gpio_pin_clear(leds[led_number]);

                    if (nrfx_systick_test(&pwm_timer, pulse_width))
                    {
                        nrf_gpio_pin_set(leds[led_number]);
                    }
                }
            }

            nrf_gpio_pin_set(leds[led_number]);
            is_duty_up = 1;
            blink_number++;

            nrf_delay_ms(delay);
        }
    }
}

static void leds_init()
{
    for (int i = 0; i < LEDS_NUMBER; i++)
    {
        nrf_gpio_cfg_output(leds[i]);
        nrf_gpio_pin_write(leds[i], !LEDS_ACTIVE_STATE);
    }
}

static void buttons_init()
{
    for (int i = 0; i < BUTTONS_NUMBER; i++)
    {
        nrf_gpio_cfg_input(buttons[i], BUTTON_PULL);
    }
}
