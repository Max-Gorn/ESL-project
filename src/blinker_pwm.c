#include <stdbool.h>
#include "pca10059.h"
#include "nrfx_systick.h"

#include "app_config.h"
#include "blinker_pwm.h"
#include "blinker_button.h"

#define PWM_PERIOD_US 1000000/PWM_FREQ_HZ
#define DUTY_CHANGE_DELAY_US DUTY_CHANGE_DELAY_MS*1000

static nrfx_systick_state_t pwm_timer;
static nrfx_systick_state_t blink_timer;

static int pulse_width;
static int duty = 0;
static bool is_duty_up = true;

static void pwm_cycle();

void setup_pwm()
{
    nrfx_systick_init();
    nrfx_systick_get(&pwm_timer);
    nrfx_systick_get(&blink_timer);
}

void duty_cycle()
{
    while (duty > 0 || is_duty_up)
    {
        pwm_cycle();
    }

    is_duty_up = true;
}

static void pwm_cycle()
{
    if (button_event_processor() && nrfx_systick_test(&blink_timer, DUTY_CHANGE_DELAY_US))
    {
        nrfx_systick_get(&blink_timer);
        if (duty >= 100)
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

    nrfx_systick_get(&pwm_timer);
    while (!nrfx_systick_test(&pwm_timer, PWM_PERIOD_US))
    {
        if (duty > 0)
        {
            pwm_on();
        }

        pulse_width = duty * PWM_PERIOD_US / 100;

        if (nrfx_systick_test(&pwm_timer, pulse_width))
        {
            pwm_off();
        }
    }

    enable_button();
}