#include <stdint.h>
#include <stdbool.h>

#include "pca10059.h"
#include "nrf_delay.h"
#include "nrfx_systick.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "nrf_log_backend_usb.h"

#include "app_usbd.h"
#include "app_usbd_serial_num.h"

#include "app_config.h"
#include "blinker_button.h"
#include "blinker_pwm.h"

#define DUTY_CHANGE_DELAY_US (DUTY_CHANGE_DELAY_MS * 1000UL)

static nrfx_systick_state_t duty_change_timer;
static int led_number;
static bool blink_state = false;

static const uint32_t leds[] = LEDS_LIST;
static const int blinks_count_list[] = BLINK_SEQUENCE;

static void setup();
static void loop();
static void duty_cycle();
static void leds_init();
static void logs_init();

int main(void)
{
    setup();
    while (true)
    {
        loop();
    }
}

static void setup()
{
    logs_init();
    leds_init();
    setup_button();
    nrfx_systick_init();
}

static void loop()
{
    for (led_number = 0; led_number < LEDS_NUMBER; led_number++)
    {
        for (int blink_number = 0; blink_number < *(blinks_count_list + led_number);)
        {
            duty_cycle();
            // nrf_gpio_pin_write(leds[led_number], !LEDS_ACTIVE_STATE);
            blink_number++;

            nrf_delay_ms(BLINK_DELAY_MS);
        }
    }
}

static void duty_cycle()
{
    uint32_t duty = 0;
    bool is_duty_up = true;

    nrfx_systick_get(&duty_change_timer);

    while (duty > 0 || is_duty_up)
    {
        // NRF_LOG_INFO("Duty: %u", duty);
        pwm_write(leds[led_number], duty);

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

        button_click_t button_state = read_button_state();

        if (button_state == CLICK)
        {
            NRF_LOG_INFO("Click");
        }

        if (button_state == DOUBLE_CLICK)
        {
            blink_state = !blink_state;

            NRF_LOG_INFO("Double click");
            if (blink_state)
            {
                NRF_LOG_INFO("Started");
            }
            else
            {
                NRF_LOG_INFO("Paused on duty: %u", duty);
            }
        }

        while (NRF_LOG_PROCESS())
        {
            LOG_BACKEND_USB_PROCESS();
        }
        LOG_BACKEND_USB_PROCESS();
    }
}

static void logs_init()
{
    ret_code_t ret = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(ret);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

static void leds_init()
{
    for (int i = 0; i < LEDS_NUMBER; i++)
    {
        nrf_gpio_cfg_output(leds[i]);
        nrf_gpio_pin_write(leds[i], !LEDS_ACTIVE_STATE);
    }
}