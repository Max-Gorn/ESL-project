#include <stdbool.h>

#include "pca10059.h"

#include "nrfx_systick.h"

#include "app_config.h"
#include "blinker_pwm.h"

#define PWM_PERIOD_US (1000000UL / PWM_FREQ_HZ)

static nrfx_systick_state_t pwm_timer;

void pwm_write(uint32_t pin, uint32_t duty)
{
    uint32_t pulse_width = duty * PWM_PERIOD_US / PWM_MAX_DUTY;
    nrfx_systick_get(&pwm_timer);

    while (!nrfx_systick_test(&pwm_timer, PWM_PERIOD_US))
    {
        if (nrfx_systick_test(&pwm_timer, pulse_width))
        {
            nrf_gpio_pin_write(pin, !LEDS_ACTIVE_STATE);
        }
        else
        {
            nrf_gpio_pin_write(pin, LEDS_ACTIVE_STATE);
        }
    }
}