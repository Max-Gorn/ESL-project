#ifndef BLINKER_PWM_H
#define BLINKER_PWM_H

void led_ctrl_init();
void led_ctrl_start_blink_led1(uint16_t step_delay_ms);
void led_ctrl_set_hsv(uint16_t h, uint8_t s, uint8_t v);
void led_ctrl_set_led1_brightness(uint8_t brightness);

#endif