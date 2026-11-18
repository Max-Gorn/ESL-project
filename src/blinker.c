#include <stdint.h>
#include <stdbool.h>
#include "nrf_delay.h"
#include "pca10059.h"

#include "app_config.h"
#include "blinker.h"
#include "blinker_button.h"
#include "blinker_pwm.h"

static void leds_init();

static void task_pin_on();
static void task_pin_off();
static void pin_on(uint32_t pin);
static void pin_off(uint32_t pin);

static const uint32_t leds[] = LEDS_LIST;

static int led_number;

void setup()
{
    leds_init();
    setup_pwm();
    setup_button();
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

            nrf_gpio_pin_set(leds[led_number]);
            blink_number++;

            nrf_delay_ms(BLINK_DELAY_MS);
        }
    }
}

static void task_pin_on()
{
    pin_on(leds[led_number]);
}

static void task_pin_off()
{
    pin_off(leds[led_number]);
}

static void pin_on(uint32_t pin)
{
    nrf_gpio_pin_clear(pin);
}

static void pin_off(uint32_t pin)
{
    nrf_gpio_pin_set(pin);
}

static void leds_init()
{
    for (int i = 0; i < LEDS_NUMBER; i++)
    {
        nrf_gpio_cfg_output(leds[i]);
        nrf_gpio_pin_write(leds[i], !LEDS_ACTIVE_STATE);
    }
}