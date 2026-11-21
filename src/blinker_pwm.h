#ifndef BLINKER_PWM_H
#define BLINKER_PWM_H

void setup_pwm();
void duty_cycle();
void pwm_write(uint32_t pin, uint32_t duty);
void (*pwm_on)(void);
void (*pwm_off)(void);

#endif