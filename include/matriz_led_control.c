#include "matriz_led_control.h"
// #include "buzzer_functions.h"

void init_pio_routine(pio_t * meu_pio, uint OUT_PIN)
{
    //coloca a frequência de clock para 128 MHz, facilitando a divisão pelo clock
    meu_pio->ok = set_sys_clock_khz(128000, false);

    stdio_init_all();

    printf("iniciando a transmissão PIO");
    if (meu_pio->ok) printf("clock set to %ld\n", clock_get_hz(clk_sys));

    //configurações da PIO
    uint offset = pio_add_program(meu_pio->pio, &pio_matrix_program);
    meu_pio->sm = pio_claim_unused_sm(meu_pio->pio, true);
    pio_matrix_program_init(meu_pio->pio, meu_pio->sm, offset, OUT_PIN);
}

//imprimir valor binário
void imprimir_binario(int num) 
{
 int i;
 for (i = 31; i >= 0; i--) {
  (num & (1 << i)) ? printf("1") : printf("0");
 }
}

//rotina para definição da intensidade de cores do led
uint32_t matrix_rgb(double b, double r, double g)
{
  unsigned char R, G, B;
  R = r * 255;
  G = g * 255;
  B = b * 255;
  return (G << 24) | (R << 16) | (B << 8);
}

void desenho_pio(double *desenho, pio_t *meu_pio)
{
    uint32_t valor_led = 0;

    for (int16_t i = 0; i < NUM_PIXELS; i++) {
        valor_led = matrix_rgb(desenho[24-i], 0, 0);  // Apenas o canal Verde ativado
        pio_sm_put_blocking(meu_pio->pio, meu_pio->sm, valor_led);
        //imprimir_binario(valor_led);
    }
    printf("clock set to %ld\n", clock_get_hz(clk_sys));
}


void desenho_pio_rgb(double *desenho, pio_t * meu_pio)
{
    uint32_t valor_led = 0;

    for (int16_t i = 0; i < NUM_PIXELS; i++) {
        valor_led = matrix_rgb(desenho[24-i]*meu_pio->b, desenho[24-i]*meu_pio->r, desenho[24-i]*meu_pio->g);
        pio_sm_put_blocking(meu_pio->pio, meu_pio->sm, valor_led);
        //imprimir_binario(valor_led);
    }
    printf("clock set to %ld\n", clock_get_hz(clk_sys));
}


void desliga_tudo(pio_t * meu_pio)
{
    uint32_t valor_led = 0;

    for (int16_t i = 0; i < NUM_PIXELS; i++) {
        valor_led = matrix_rgb(0.0, 0.0, 0.0);
        pio_sm_put_blocking(meu_pio->pio, meu_pio->sm, valor_led);
        //imprimir_binario(valor_led);
    }
    printf("clock set to %ld\n", clock_get_hz(clk_sys));
}