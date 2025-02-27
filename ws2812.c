#include <stdio.h>
#include <string.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "ws2812.pio.h"
#include "hardware/i2c.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include "inc/pwm_irq.h"

// Definições de hardware
#define BUTTON_A 5
#define BUTTON_B 6
#define WS2812_PIN 7
#define BUZZER_PIN_1 10   
#define LED_RGB_G 11
#define LED_RGB_B 12
#define LED_RGB_R 13
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
#define I2C_PORT i2c1
#define BUZZER_PIN_2 21   
#define JOYSTICK_PB 22
#define JOYSTICK_Y_PIN 27
#define NUM_PIXELS 25
#define IS_RGBW false
#define JOY_CENTER 2048
#define JOY_THRESHOLD 100

// Intervalo de tempo
const int intervalo_ms = 400;
const int intervalo_micro = intervalo_ms * 1000;

// Variáveis globais
volatile int contador = 0;
volatile uint32_t ultimo_estado_a = 0;
volatile uint32_t ultimo_estado_b = 0;
volatile uint32_t ultimo_estado_jpb = 0;
bool estado_sistema = true;
bool configuracao = true;
uint16_t adc_value_y;
uint64_t tempo_inicio = 0;
bool tempo_rodando = false;
uint64_t tempo_pausado = 0;
ssd1306_t ssd;
char ultimo_buffer[20] = "";
volatile bool alarme_ativo = false;
uint64_t ultimo_tempo_alarme = 0;
bool buzzer_estado = false;
volatile int contador_bips = 0;
bool display_atualizado = false; // Flag para controle de atualização

// Funções
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

bool numeros[11][NUM_PIXELS] = {
    { // Zero
        1, 1, 1, 1, 1,
        1, 1, 1, 1, 1,
        1, 1, 1, 1, 1,
        1, 1, 1, 1, 1,
        1, 1, 1, 1, 1
    },
    { // Um
        0, 1, 0, 0, 0,
        0, 0, 0, 1, 0,
        0, 1, 0, 0, 0,
        0, 0, 0, 1, 0,
        0, 1, 0, 0, 0
    },
    { // Dois
        0, 1, 1, 1, 0,
        0, 1, 0, 0, 0,
        0, 1, 1, 1, 0,
        0, 0, 0, 1, 0,
        0, 1, 1, 1, 0
    },
    { // Três
        0, 1, 1, 1, 0,
        0, 0, 0, 1, 0,
        0, 1, 1, 0, 0,
        0, 0, 0, 1, 0,
        0, 1, 1, 1, 0
    },
    { // Quatro
        0, 1, 0, 0, 0,
        0, 0, 0, 1, 0,
        0, 1, 1, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 0, 1, 0
    },
    { // Cinco
        0, 1, 1, 1, 0,
        0, 0, 0, 1, 0,
        0, 1, 1, 1, 0,
        0, 1, 0, 0, 0,
        0, 1, 1, 1, 0
    },
    { // Seis
        0, 1, 1, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 1, 1, 0,
        0, 1, 0, 0, 0,
        0, 1, 1, 1, 0
    },
    { // Sete
        0, 1, 0, 0, 0,
        0, 0, 0, 1, 0,
        0, 1, 0, 0, 0,
        0, 0, 0, 1, 0,
        0, 1, 1, 1, 0
    },
    { // Oito
        0, 1, 1, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 1, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 1, 1, 0
    },
    { // Nove
        0, 1, 1, 1, 0,
        0, 0, 0, 1, 0,
        0, 1, 1, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 1, 1, 0
    },
    { // Dez (apagado)
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0,
        0, 0, 0, 0, 0
    }
};

void display_numeros(int numero) {
    uint32_t color = urgb_u32(100, 0, 0);
    for (int i = 0; i < NUM_PIXELS; i++) {
        if (numero == 10) {
            put_pixel(0);
        } else {
            put_pixel(numeros[numero][i] ? color : 0);
        }
    }
}

void iniciar_temporizador() {
    tempo_inicio = time_us_64();
    tempo_rodando = true;
    tempo_pausado = 0; // Reseta o tempo pausado ao iniciar
    printf("Contagem iniciada\n");
    display_atualizado = false;
    ssd1306_fill(&ssd, false);
    sprintf(ultimo_buffer, "%d s", contador * 60);
    ssd1306_draw_string(&ssd, ultimo_buffer, 0, 20);
    ssd1306_send_data(&ssd);
}

void iniciar_alarme() {
    alarme_ativo = true;
    ultimo_tempo_alarme = time_us_64();
    contador_bips = 0;
}

void atualizar_alarme() {
    static uint64_t inicio_alarme = 0;
    if (alarme_ativo) {
        uint64_t tempo_atual = time_us_64();
        if (inicio_alarme == 0) {
            inicio_alarme = tempo_atual;
        }

        if (tempo_atual - ultimo_tempo_alarme >= 200 * 1000) {
            buzzer_estado = !buzzer_estado;
            if (buzzer_estado) {
                buzzer_on();
                display_numeros(0);
                contador_bips++;
            } else {
                buzzer_off();
                display_numeros(10);
            }
            ultimo_tempo_alarme = tempo_atual;
        }

        if (contador_bips >= 5 && !buzzer_estado) {
            alarme_ativo = false;
            buzzer_off();
            display_numeros(10);
            contador_bips = 0;
        }
    }
}

void exibir_tempo(float tempo) {
    char buffer[16];
    int tempo_segundos = (int)tempo;
    sprintf(buffer, "%d s", tempo_segundos);

    if (strcmp(buffer, ultimo_buffer) != 0) {
        strcpy(ultimo_buffer, buffer);
        ssd1306_draw_string(&ssd, "        ", 0, 20); // Limpa apenas a área dos segundos
        ssd1306_draw_string(&ssd, buffer, 0, 20);
        ssd1306_send_data(&ssd);
    }

    if (tempo_segundos <= 10 && tempo_segundos >= 0) {
        display_numeros(tempo_segundos);
    } else {
        display_numeros(10);
    }
}

void verificar_tempo() {
    if (estado_sistema && !configuracao) {
        if (tempo_rodando) {
            uint64_t tempo_decorrido = (time_us_64() - tempo_inicio) / 1000000;
            float tempo_restante = contador * 60 - tempo_decorrido;
            exibir_tempo(tempo_restante);

            if (tempo_restante <= 0) {
                tempo_rodando = false;
                iniciar_alarme();
                ssd1306_fill(&ssd, false);
                if (!configuracao) {
                    ssd1306_draw_string(&ssd, "Timer encerrado", 0, 20);
                }
                ssd1306_send_data(&ssd);
                display_atualizado = false;
            }
        }
    }
}

void display_sistema() {
    if (estado_sistema && !display_atualizado) {
        char buffer[20];
        if (configuracao) {
            snprintf(buffer, sizeof(buffer), "%d min", contador);
        } else {
            snprintf(buffer, sizeof(buffer), "Timer de %d min", contador);
        }
        // Não limpa o display inteiro, apenas atualiza a linha superior
        ssd1306_draw_string(&ssd, "                ", 0, 0); // Limpa apenas a área superior
        ssd1306_draw_string(&ssd, buffer, 0, 0);
        ssd1306_send_data(&ssd);
        display_atualizado = true;
    }
}

void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    if (gpio == BUTTON_A && current_time - ultimo_estado_a > intervalo_micro) {
        ultimo_estado_a = current_time;
        if (!configuracao) {
            iniciar_temporizador();
        }
    }
    if (gpio == BUTTON_B && current_time - ultimo_estado_b > intervalo_micro) {
        ultimo_estado_b = current_time;
        if (estado_sistema) {
            configuracao = !configuracao;
            display_atualizado = false; // Força atualização
            if (configuracao) { // Ao entrar no modo de configuração
                if (tempo_rodando) { // Pausar o temporizador se estava rodando
                    tempo_pausado = (time_us_64() - tempo_inicio) / 1000000;
                    tempo_rodando = false;
                }
                ssd1306_fill(&ssd, false); // Limpa o display inteiro
                char buffer[20];
                snprintf(buffer, sizeof(buffer), "%d min", contador);
                ssd1306_draw_string(&ssd, buffer, 0, 0);
                if (tempo_pausado > 0) { // Exibe o tempo pausado, se houver
                    ssd1306_draw_string(&ssd, ultimo_buffer, 0, 20);
                }
                ssd1306_send_data(&ssd);
            } else if (!configuracao && tempo_pausado > 0) { // Retomar o temporizador
                tempo_inicio = time_us_64() - (tempo_pausado * 1000000);
                tempo_rodando = true;
                tempo_pausado = 0;
            }
        }
    }
    if (gpio == JOYSTICK_PB && current_time - ultimo_estado_jpb > intervalo_micro) {
        ultimo_estado_jpb = current_time;
        estado_sistema = !estado_sistema;
        if (!estado_sistema) {
            configuracao = true;
            tempo_rodando = false;
            ssd1306_fill(&ssd, false);
            ssd1306_send_data(&ssd);
            display_numeros(10);
            buzzer_off();
        }
        display_atualizado = false; // Força atualização
    }
}

int main() {
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    stdio_init_all();
    gpio_init(LED_RGB_R); gpio_set_dir(LED_RGB_R, GPIO_OUT);
    gpio_init(LED_RGB_G); gpio_set_dir(LED_RGB_G, GPIO_OUT);
    gpio_init(LED_RGB_B); gpio_set_dir(LED_RGB_B, GPIO_OUT);
    gpio_init(BUZZER_PIN_1); gpio_set_dir(BUZZER_PIN_1, GPIO_OUT);
    gpio_init(BUZZER_PIN_2); gpio_set_dir(BUZZER_PIN_2, GPIO_OUT);

    pwm_irq_setup(BUZZER_PIN_1);
    pwm_irq_setup(BUZZER_PIN_2);

    gpio_init(BUTTON_A); gpio_set_dir(BUTTON_A, GPIO_IN); gpio_pull_up(BUTTON_A);
    gpio_init(BUTTON_B); gpio_set_dir(BUTTON_B, GPIO_IN); gpio_pull_up(BUTTON_B);
    gpio_init(JOYSTICK_PB); gpio_set_dir(JOYSTICK_PB, GPIO_IN); gpio_pull_up(JOYSTICK_PB);
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(JOYSTICK_PB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    adc_init();
    adc_gpio_init(JOYSTICK_Y_PIN);

    while (1) {
        verificar_tempo();
        atualizar_alarme();
        
        if (configuracao && estado_sistema) {
            adc_select_input(1);
            adc_value_y = adc_read();
            int16_t delta_y = adc_value_y - JOY_CENTER;
            if (abs(delta_y) > JOY_THRESHOLD) {
                if (delta_y > 0 && contador > 0) {
                    contador--;
                } else if (delta_y < 0) {
                    contador++;
                }
                printf("Contador: %d\n", contador);
                display_atualizado = false; // Força atualização
                sleep_ms(200);
            }
        }
        display_sistema();
        sleep_ms(40);
    }
    return 0;
}