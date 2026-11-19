#include <stdbool.h>
#include "pca10059.h"
#include "nrfx_systick.h"

#include "app_config.h"
#include "blinker_pwm.h"
#include "blinker_button.h"

#define PWM_PERIOD_US 1000000UL / PWM_FREQ_HZ
#define DUTY_CHANGE_DELAY_US DUTY_CHANGE_DELAY_MS * 1000UL

static nrfx_systick_state_t pwm_timer;
static nrfx_systick_state_t duty_change_timer;

static int pulse_width;
static int duty = 0;
static bool blink_state = false;
static bool is_duty_up = true;

static void pwm_cycle();

void setup_pwm()
{
    nrfx_systick_init();
    nrfx_systick_get(&pwm_timer);
    nrfx_systick_get(&duty_change_timer);
}

void duty_cycle()
{
    while (duty > 0 || is_duty_up)
    {
        pwm_cycle();
        if (get_double_click_state())
        {
            blink_state = !blink_state;
            reset_double_click_state();
        }
    }

    is_duty_up = true;
}

static void pwm_cycle()
{
    button_event_processor();
    if (blink_state && nrfx_systick_test(&duty_change_timer, DUTY_CHANGE_DELAY_US))
    {
        nrfx_systick_get(&duty_change_timer);
        if (duty >= 255)
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

    pulse_width = duty * PWM_PERIOD_US / 255;
    nrfx_systick_get(&pwm_timer);
    while (!nrfx_systick_test(&pwm_timer, PWM_PERIOD_US))
    {
        if (nrfx_systick_test(&pwm_timer, pulse_width))
        {
            pwm_off();
        }
        else
        {
            pwm_on();
        }
    }
}