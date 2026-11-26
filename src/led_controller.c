#include <stdbool.h>

#include "pca10059.h"
#include "nrfx_pwm.h"
#include "nrfx_systick.h"

#include "app_config.h"
#include "led_controller.h"

#define PWM_TOP_VALUE 1000
#define RAMP_STEPS 25
#define TOTAL_STEPS (25 * 2)
static nrfx_pwm_t m_pwm0 = NRFX_PWM_INSTANCE(0);

static nrf_pwm_values_individual_t m_seq_values[TOTAL_STEPS];

static uint16_t m_blink_step_delay = 0;

static nrf_pwm_sequence_t m_seq = {
    .values.p_individual = m_seq_values,
    .length = NRF_PWM_VALUES_LENGTH(m_seq_values),
    .repeats = 0,
    .end_delay = 0
};

static void hsv_to_pwm_ticks(uint16_t h, uint8_t s, uint8_t v, uint16_t *r_out, uint16_t *g_out, uint16_t *b_out);

void led_ctrl_init()
{
    nrfx_pwm_config_t const config = {
        .output_pins = {
            LED1_G | NRFX_PWM_PIN_INVERTED,
            LED2_R | NRFX_PWM_PIN_INVERTED,
            LED2_G | NRFX_PWM_PIN_INVERTED,
            LED2_B | NRFX_PWM_PIN_INVERTED
        },
        .irq_priority = 6,
        .base_clock = NRF_PWM_CLK_1MHz,
        .count_mode = NRF_PWM_MODE_UP,
        .top_value = PWM_TOP_VALUE,
        .load_mode = NRF_PWM_LOAD_INDIVIDUAL,
        .step_mode = NRF_PWM_STEP_AUTO
    };

    nrfx_pwm_init(&m_pwm0, &config, NULL);

    nrfx_pwm_simple_playback(&m_pwm0, &m_seq, 1, NRFX_PWM_FLAG_LOOP);
}

void led_ctrl_set_led1_brightness(uint8_t brightness)
{
    uint16_t ticks = (uint16_t)((brightness * PWM_TOP_VALUE) / 100);

    if (m_blink_step_delay != 0)
    {
        m_blink_step_delay = 0;
        nrfx_pwm_stop(&m_pwm0, true);
        m_seq.repeats = 0;
        nrfx_pwm_simple_playback(&m_pwm0, &m_seq, 1, NRFX_PWM_FLAG_LOOP);
    }

    for (uint8_t i = 0; i < TOTAL_STEPS; i++)
    {
        m_seq_values[i].channel_0 = ticks;
    }
}

void led_ctrl_start_blink_led1(uint16_t step_delay_ms)
{
    if (step_delay_ms == 0)
    {
        led_ctrl_set_led1_brightness(0);
        return;
    }

    m_blink_step_delay = step_delay_ms;

    for (uint8_t i = 0; i < TOTAL_STEPS; i++)
    {
        if (i < RAMP_STEPS)
            m_seq_values[i].channel_0 = (PWM_TOP_VALUE * i) / RAMP_STEPS;
        else
            m_seq_values[i].channel_0 = (PWM_TOP_VALUE * (TOTAL_STEPS - i)) / RAMP_STEPS;
    }

    nrfx_pwm_stop(&m_pwm0, true);
    m_seq.repeats = m_blink_step_delay - 1;
    nrfx_pwm_simple_playback(&m_pwm0, &m_seq, 1, NRFX_PWM_FLAG_LOOP);
}

void led_ctrl_set_hsv(uint16_t h, uint8_t s, uint8_t v)
{
    uint16_t r, g, b;
    hsv_to_pwm_ticks(h, s, v, &r, &g, &b);

    for (uint8_t i = 0; i < TOTAL_STEPS; i++)
    {
        m_seq_values[i].channel_1 = r;
        m_seq_values[i].channel_2 = g;
        m_seq_values[i].channel_3 = b;
    }
}

static void hsv_to_pwm_ticks(uint16_t h, uint8_t s, uint8_t v, uint16_t *r_out, uint16_t *g_out, uint16_t *b_out)
{
    if (s == 0)
    {
        uint16_t val = (v * PWM_TOP_VALUE) / 100;
        *r_out = val;
        *g_out = val;
        *b_out = val;
        return;
    }

    uint8_t region = h / 60;
    uint16_t remainder = (h % 60) * 100 / 60;

    uint32_t p = (v * (100 - s)) / 100;
    uint32_t q = (v * (100 - ((s * remainder) / 100))) / 100;
    uint32_t t = (v * (100 - ((s * (100 - remainder)) / 100))) / 100;

    uint32_t r = 0, g = 0, b = 0;
    switch (region)
    {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        default: r = v; g = p; b = q; break;
    }

    *r_out = (uint16_t)((r * PWM_TOP_VALUE) / 100);
    *g_out = (uint16_t)((g * PWM_TOP_VALUE) / 100);
    *b_out = (uint16_t)((b * PWM_TOP_VALUE) / 100);
}