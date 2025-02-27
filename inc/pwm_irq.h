#ifndef PWM_IRQ_H
#define PWM_IRQ_H

#include "pico/stdlib.h"

void pwm_irq_setup(uint gpio_pin);
void buzzer_set_frequency(uint32_t freq);
void buzzer_on();
void buzzer_off();

#endif