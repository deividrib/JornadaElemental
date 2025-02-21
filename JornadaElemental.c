#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ssd1306.h"
#include "font.h"
#include "matriz_led_control.h"

// -----------------------------
// Configurações e definições
// -----------------------------
#define OLED_WIDTH      128
#define OLED_HEIGHT     64

// Definição dos pinos para o LED RGB (ajuste conforme sua placa)
#define LED_R_PIN       13
#define LED_G_PIN       11
#define LED_B_PIN       12

// Definição do pino para o BUZZER
#define BUZZER_PIN      10

// Definição dos pinos para os botões
#define BUTTON_ACCEPT   5   // Botão para aceitar a aventura
#define BUTTON_DENY     6   // Botão para recusar a aventura

// Pino de saída para a matriz LED 5x5
#define MATRIX_OUT_PIN  7

// -----------------------------
// Variáveis globais e instâncias
// -----------------------------
ssd1306_t display;

// Estrutura para controle da matriz LED via PIO
pio_t meu_pio = {
    .pio = pio0,
    .ok  = false,
    .i   = 0,
    // Valores padrão; serão alterados para definir a cor do glifo
    .r   = 0.0,
    .g   = 0.0,
    .b   = 0.0,
    .sm  = 0
};

// Definição de um "glifo" enigmático para a matriz LED 5x5
double glifo[25] = {
    0.0, 0.3, 0.0, 0.3, 0.0,
    0.3, 0.0, 0.3, 0.0, 0.3,
    0.0, 0.3, 0.3, 0.3, 0.0,
    0.3, 0.0, 0.3, 0.0, 0.3,
    0.0, 0.3, 0.0, 0.3, 0.0
};

// -----------------------------
// Funções auxiliares
// -----------------------------

// Configura o LED RGB para emitir um suave tom azul
void set_led_blue() {
    gpio_put(LED_R_PIN, 0);
    gpio_put(LED_G_PIN, 0);
    gpio_put(LED_B_PIN, 1);
}

// Função para tocar uma única nota no buzzer usando PWM
void play_note(uint frequency, uint duration_ms) {
    if (frequency == 0) {
        sleep_ms(duration_ms);
        return;
    }
    
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
    uint channel = pwm_gpio_to_channel(BUZZER_PIN);
    
    float divider = 100.0f;
    pwm_set_clkdiv(slice_num, divider);
    
    uint32_t wrap = (uint32_t)(125000000 / (divider * frequency)) - 1;
    pwm_set_wrap(slice_num, wrap);
    
    pwm_set_chan_level(slice_num, channel, wrap / 2);
    pwm_set_enabled(slice_num, true);
    
    printf("Tocando nota: %d Hz, wrap: %ld\n", frequency, wrap);
    sleep_ms(duration_ms);
    
    pwm_set_enabled(slice_num, false);
}

// Função para tocar uma sequência de notas (musiquinha)
void play_tune() {
    printf("Iniciando a musiquinha...\n");
    play_note(262, 500);  // C4 por 500 ms
    sleep_ms(50);
    play_note(294, 500);  // D4
    sleep_ms(50);
    play_note(330, 500);  // E4
    sleep_ms(50);
    play_note(262, 500);  // C4 novamente
    printf("Musiquinha finalizada.\n");
}

// Função para exibir um frame no display OLED
void display_frame(const char *lines[], uint8_t y_positions[], int num_lines) {
    ssd1306_fill(&display, 0);
    for (int i = 0; i < num_lines; i++) {
        ssd1306_draw_string(&display, lines[i], 8, y_positions[i]);
    }
    ssd1306_send_data(&display);
}

// -----------------------------
// main()
// -----------------------------
int main() {
    stdio_init_all();
    sleep_ms(2000);
    printf("Capitulo 1: O Chamado do Destino\n");

    // Inicializa o I2C para o display OLED (pinos SDA = 14, SCL = 15)
    i2c_init(i2c1, 100 * 1000);
    gpio_set_function(14, GPIO_FUNC_I2C);
    gpio_set_function(15, GPIO_FUNC_I2C);
    gpio_pull_up(14);
    gpio_pull_up(15);

    ssd1306_init(&display, OLED_WIDTH, OLED_HEIGHT, false, 0x3C, i2c1);
    ssd1306_config(&display);
    ssd1306_fill(&display, 0);
    ssd1306_send_data(&display);

    init_pio_routine(&meu_pio, MATRIX_OUT_PIN);

    gpio_init(LED_R_PIN);
    gpio_set_dir(LED_R_PIN, GPIO_OUT);
    gpio_init(LED_G_PIN);
    gpio_set_dir(LED_G_PIN, GPIO_OUT);
    gpio_init(LED_B_PIN);
    gpio_set_dir(LED_B_PIN, GPIO_OUT);

    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);

    gpio_init(BUTTON_ACCEPT);
    gpio_set_dir(BUTTON_ACCEPT, GPIO_IN);
    gpio_pull_up(BUTTON_ACCEPT);

    gpio_init(BUTTON_DENY);
    gpio_set_dir(BUTTON_DENY, GPIO_IN);
    gpio_pull_up(BUTTON_DENY);

    // -----------------------------
    // Exibe o Frame 1 no OLED (primeira parte da narrativa)
    // -----------------------------
    const char *frame1_lines[] = {
        "O Chamado do",
        "Destino",
        "Em uma noite",
        "calma,"
    };
    uint8_t frame1_y[] = {0, 10, 20, 30};
    display_frame(frame1_lines, frame1_y, 4);
    sleep_ms(7000);

    // -----------------------------
    // Exibe o Frame 2 no OLED (continuação da narrativa)
    // -----------------------------
    const char *frame2_lines[] = {
        "uma visao",
        "misteriosa",
        "revela um",
        "segredo antigo"
    };
    uint8_t frame2_y[] = {0, 10, 20, 30};
    display_frame(frame2_lines, frame2_y, 4);
    sleep_ms(7000);

    // -----------------------------
    // Exibe o Frame 3: Pergunta se aceita a aventura
    // -----------------------------
    const char *frame3_lines[] = {
        "Aceita aventura?",
        "Aceitar: Botao 5",
        "Recusar: Botao 6"
    };
    uint8_t frame3_y[] = {10, 30, 40};
    display_frame(frame3_lines, frame3_y, 3);

    // Define novos valores de cor para o glifo – ex.: tom alaranjado
    meu_pio.r = 1.0;   // Vermelho em máximo
    meu_pio.g = 0.5;   // Verde com intensidade média
    meu_pio.b = 0.0;   // Azul desligado

    // Em vez de usar a função "desenho_pio", usamos "desenho_pio_rgb" para aplicar a cor
    desenho_pio_rgb(glifo, &meu_pio);

    set_led_blue();              // Mantém o LED RGB azul
    play_tune();                 // Toca a musiquinha

    printf("Aguardando resposta: Aceitar (botao 5) ou Recusar (botao 6)...\n");
    while (gpio_get(BUTTON_ACCEPT) && gpio_get(BUTTON_DENY)) {
        tight_loop_contents();
    }

    if (!gpio_get(BUTTON_ACCEPT)) {
        ssd1306_fill(&display, 0);
        ssd1306_draw_string(&display, "Chamado Aceito!", 0, 20);
        ssd1306_draw_string(&display, "Iniciando aventura...", 0, 40);
        ssd1306_send_data(&display);
        printf("Chamado Aceito! Aventura iniciada.\n");
    } else if (!gpio_get(BUTTON_DENY)) {
        ssd1306_fill(&display, 0);
        ssd1306_draw_string(&display, "Chamado Recusado!", 20, 20);
        ssd1306_draw_string(&display, "Fim da jornada.", 20, 40);
        ssd1306_send_data(&display);
        printf("Chamado Recusado! Jornada encerrada.\n");
    }

    while (1) {
        tight_loop_contents();
    }

    return 0;
}
