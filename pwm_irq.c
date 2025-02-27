 #include "inc/pwm_irq.h"
 #include "hardware/pwm.h"
 #include "hardware/irq.h"
 
 #define BUZZER_PIN_1 10
 #define BUZZER_PIN_2 21
 
 volatile uint32_t pwm_freq = 2700;  // Melhor frequência: 2700 Hz
 
 void pwm_irq_handler() {
     uint slice_num_1 = pwm_gpio_to_slice_num(BUZZER_PIN_1);
     uint slice_num_2 = pwm_gpio_to_slice_num(BUZZER_PIN_2);
     pwm_clear_irq(slice_num_1);
     pwm_clear_irq(slice_num_2);
 }
 
 void pwm_irq_setup(uint gpio_pin) {
     gpio_set_function(gpio_pin, GPIO_FUNC_PWM);
     uint slice_num = pwm_gpio_to_slice_num(gpio_pin);
     pwm_config config = pwm_get_default_config();
     pwm_config_set_clkdiv(&config, 1.0f); // Clock divisor mínimo
     pwm_config_set_wrap(&config, 125000000 / pwm_freq); // 2700 Hz
     pwm_init(slice_num, &config, false);
     pwm_set_chan_level(slice_num, PWM_CHAN_A, (125000000 / pwm_freq) * 9 / 10); // 90% duty cycle
     pwm_clear_irq(slice_num);
     pwm_set_irq_enabled(slice_num, true);
     if (gpio_pin == BUZZER_PIN_1) {
         irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_irq_handler);
         irq_set_enabled(PWM_IRQ_WRAP, true);
     }
     pwm_set_enabled(slice_num, false);
 }
 
 void buzzer_set_frequency(uint32_t freq) {
     pwm_freq = freq;
     uint slice_num_1 = pwm_gpio_to_slice_num(BUZZER_PIN_1);
     uint slice_num_2 = pwm_gpio_to_slice_num(BUZZER_PIN_2);
     pwm_config config = pwm_get_default_config();
     pwm_config_set_clkdiv(&config, 1.0f);
     pwm_config_set_wrap(&config, 125000000 / freq);
     pwm_init(slice_num_1, &config, false);
     pwm_init(slice_num_2, &config, false);
     pwm_set_chan_level(slice_num_1, PWM_CHAN_A, (125000000 / freq) * 9 / 10);
     pwm_set_chan_level(slice_num_2, PWM_CHAN_A, (125000000 / freq) * 9 / 10);
 }
 
 void buzzer_on() {
     uint slice_num_1 = pwm_gpio_to_slice_num(BUZZER_PIN_1);
     uint slice_num_2 = pwm_gpio_to_slice_num(BUZZER_PIN_2);
     pwm_set_enabled(slice_num_1, true);
     pwm_set_enabled(slice_num_2, true);
 }
 
 void buzzer_off() {
     uint slice_num_1 = pwm_gpio_to_slice_num(BUZZER_PIN_1);
     uint slice_num_2 = pwm_gpio_to_slice_num(BUZZER_PIN_2);
     pwm_set_enabled(slice_num_1, false);
     pwm_set_enabled(slice_num_2, false);
 }