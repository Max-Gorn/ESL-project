#include <stdbool.h>
#include "pca10059.h"
#include "nrfx_gpiote.h"
#include "nrfx_systick.h"

#include "app_config.h"
#include "blinker_button.h"

#define DOUBLE_CLICK_PENDING_PERIOD_US DOUBLE_CLICK_PENDING_PERIOD_MS*1000UL

static nrfx_systick_state_t debounce_timer;
static nrfx_systick_state_t double_click_timer;

static bool is_debouncing = false;
static bool double_click = false;
static bool is_waiting_second_click = false;
static volatile bool button_event_pending = false;

static void button_event_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t polarity);
static bool debounce_timer_check();
static bool double_click_check();

void setup_button()
{
    nrfx_gpiote_in_config_t in_config = NRFX_GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
    in_config.pull = NRF_GPIO_PIN_PULLUP;

    nrfx_err_t err_code = nrfx_gpiote_init();
    NRFX_ASSERT(err_code == NRFX_SUCCESS);

    err_code = nrfx_gpiote_in_init(BUTTON_1, &in_config, button_event_handler);
    NRFX_ASSERT(err_code == NRFX_SUCCESS);
    
    nrfx_gpiote_in_event_enable(BUTTON_1, true);
}

void enable_button()
{
    nrfx_gpiote_in_event_enable(BUTTON_1, true);
}

bool button_event_processor()
{
    // Button event pending process
    if (button_event_pending && !is_debouncing)
    {
        button_event_pending = false;
        is_debouncing = true;
        nrfx_systick_get(&debounce_timer);
    }
    
    // Debouncing
    if (is_debouncing)
    {
        if (debounce_timer_check())
        {
            nrfx_gpiote_in_event_enable(BUTTON_1, true);

            if (nrf_gpio_pin_read(BUTTON_1) != BUTTONS_ACTIVE_STATE)
            {
                return false;
            }

            double_click = double_click_check();

            return true;
        }
    }

    return false;
}

bool get_double_click_state()
{
    return double_click;
}

void reset_double_click_state()
{
    double_click = false;
}

static bool double_click_check()
{
    if (is_waiting_second_click && !nrfx_systick_test(&double_click_timer, DOUBLE_CLICK_PENDING_PERIOD_US))
    {
        is_waiting_second_click = false;
        return true;
    }

    is_waiting_second_click = true;
    nrfx_systick_get(&double_click_timer);
    return false;
}

static bool debounce_timer_check()
{
    if (nrfx_systick_test(&debounce_timer, DEBOUNCE_DELAY_US))
    {
        is_debouncing = false;
        return true;
    }

    return false;
}

void button_event_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t polarity)
{
    if (pin == BUTTON_1)
    {
        nrfx_gpiote_in_event_disable(BUTTON_1);
        button_event_pending = true;
    }
}