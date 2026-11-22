#include <stdbool.h>
#include "pca10059.h"
#include "nrfx_gpiote.h"
#include "nrfx_systick.h"

#define NRF_LOG_MODULE_NAME button
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#include "app_config.h"
#include "blinker_button.h"

#define DOUBLE_CLICK_PENDING_PERIOD_US (DOUBLE_CLICK_PENDING_PERIOD_MS * 1000UL)

typedef enum button_state {
    IDLE,
    FIRST_CLICK,
    WAITING_SECOND_CLICK,
    SECOND_CLICK
} button_state_t;

static nrfx_systick_state_t debounce_timer;
static nrfx_systick_state_t double_click_timer;

static volatile bool button_event_pending = false;
static button_state_t state = IDLE;

void button_event_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t polarity);

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

button_click_t read_button_state()
{
    switch (state)
    {
        case IDLE:
            // Button event pending process
            if (button_event_pending)
            {
                button_event_pending = false;
                state = FIRST_CLICK;
                NRF_LOG_INFO("IDLE -> FIRST_CLICK");
                nrfx_systick_get(&debounce_timer);
            }
            break;

        case FIRST_CLICK:
            // Debouncing
            if (nrfx_systick_test(&debounce_timer, DEBOUNCE_DELAY_US))
            {
                nrfx_gpiote_in_event_enable(BUTTON_1, true);

                if (nrf_gpio_pin_read(BUTTON_1) != BUTTONS_ACTIVE_STATE)
                {
                    state = IDLE;
                    NRF_LOG_INFO("FIRST_CLICK -> IDLE");

                    break;
                }
                state = WAITING_SECOND_CLICK;
                NRF_LOG_INFO("FIRST_CLICK -> WAITING_SECOND_CLICK");
                nrfx_systick_get(&double_click_timer);
                return CLICK;
            }
            break;

        case WAITING_SECOND_CLICK:
            // Button event pending process
            if (button_event_pending)
            {
                button_event_pending = false;
                state = SECOND_CLICK;
                NRF_LOG_INFO("WAITING_SECOND_CLICK -> SECOND_CLICK");
                nrfx_systick_get(&debounce_timer);
            }
            
            if (nrfx_systick_test(&double_click_timer, DOUBLE_CLICK_PENDING_PERIOD_US))
            {
                state = IDLE;
                NRF_LOG_INFO("WAITING_SECOND_CLICK -> IDLE");
            }
            
            break;

        case SECOND_CLICK:
            // Debouncing
            if (nrfx_systick_test(&debounce_timer, DEBOUNCE_DELAY_US))
            {
                nrfx_gpiote_in_event_enable(BUTTON_1, true);

                state = IDLE;
                if (nrf_gpio_pin_read(BUTTON_1) != BUTTONS_ACTIVE_STATE)
                {
                    break;
                }
                NRF_LOG_INFO("IDLE -> FIRST_CLICK");
                return DOUBLE_CLICK;
            }
            break;
    }
    return NONE;
}

void button_event_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t polarity)
{
    if (pin == BUTTON_1)
    {
        nrfx_gpiote_in_event_disable(BUTTON_1);
        button_event_pending = true;
    }
}