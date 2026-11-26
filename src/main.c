#include <stdint.h>
#include <stdbool.h>

#include "pca10059.h"
#include "nrf_delay.h"
#include "nrfx_systick.h"
#include "nrfx_pwm.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "nrf_log_backend_usb.h"

#include "app_usbd.h"
#include "app_usbd_serial_num.h"

#include "app_config.h"
#include "blinker_button.h"
#include "led_controller.h"

typedef enum input_mode
{
    NO_INPUT,
    HUE,
    SATURATION,
    BRIGHTNESS
} input_mode_t;

#define DUTY_CHANGE_DELAY_US (DUTY_CHANGE_DELAY_MS * 1000UL)

static nrfx_systick_state_t step_timer; // Добавить тесты таймера для каждого шага изменения у каждого свойства

static const uint32_t leds[] = LEDS_LIST;
static const int blinks_count_list[] = BLINK_SEQUENCE;
static input_mode_t mode = NO_INPUT;
static uint16_t hue = 360 * (blinks_count_list[2] * 10 + blinks_count_list[3]) / 100.0;
static uint8_t saturation = 100;
static uint8_t value = 100; // brightness
static int8_t saturation_step = 1;
static int8_t value_step = 1;

static void setup();
static void loop();
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
    led_ctrl_init();
    led_ctrl_set_hsv(hue, saturation, value);
    nrfx_systick_get(&step_timer);
}

static void loop()
{
    button_click_t button_state = read_button_state();

    if (button_state == CLICK)
    {
        NRF_LOG_INFO("Click");
    }

    if (button_state == DOUBLE_CLICK)
    {
        mode = (mode + 1) % 4;
        switch (mode)
        {
            case NO_INPUT:
                NRF_LOG_INFO("mode: no input");
                led_ctrl_set_led1_brightness(0);
                break;

            case HUE:
                NRF_LOG_INFO("mode: hue modification");
                led_ctrl_start_blink_led1(60);
                break;

            case SATURATION:
                NRF_LOG_INFO("mode: saturation modification");
                led_ctrl_start_blink_led1(10);
                break;

            case BRIGHTNESS:
                NRF_LOG_INFO("mode: brightness modification");
                led_ctrl_set_led1_brightness(100);
                break;
        }
    }

    if (button_state == HOLDING && nrfx_systick_test(&step_timer, MODIFICATION_STEP_DELAY_US))
    {
        switch (mode)
        {
            case NO_INPUT:
                break;

            case HUE:
                hue = (hue + 1) % 360;
                break;

            case SATURATION:
                if (saturation >= 100)
                {
                    saturation_step = -1;
                }
                if (saturation <= 0)
                {
                    saturation_step = 1;
                }

                saturation += saturation_step;
                break;

            case BRIGHTNESS:
                if (value >= 100)
                {
                    value_step = -1;
                }
                if (value <= 0)
                {
                    value_step = 1;
                }

                value += value_step;
                break;
        }

        led_ctrl_set_hsv(hue, saturation, value);
        nrfx_systick_get(&step_timer);
    }

    while (NRF_LOG_PROCESS())
    {
        LOG_BACKEND_USB_PROCESS();
    }
    LOG_BACKEND_USB_PROCESS();
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