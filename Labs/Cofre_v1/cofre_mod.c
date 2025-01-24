#include <stdio.h>
#include "pico/stdlib.h"
#include <string.h>
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "ssd1306.h"

// Definições
#define SET 1
#define RESET 0
#define COMP 4

// Pinos GPIO
const uint8_t Linha[] = {2, 3, 4, 5};
const uint8_t Coluna[] = {6, 7, 8, 9};
const uint8_t Segmentos[] = {10, 11, 12, 13, 14, 15, 16, 17}; // Pinos dos displays de 7 segmentos
const uint8_t Buzzer = 18;
const uint8_t PushButton_A = 19;
const uint8_t PushButton_B = 20;
const uint8_t LED_R = 21;
const uint8_t LED_G = 22;
const uint8_t LED_B = 26;

// Mapeamento das teclas em uma matriz 4x4
char teclas[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

// Variáveis globais
char password1[COMP + 1] = {0};
int tentativas = 3;
bool alarme_ativo = false;

// Inicializando os pinos de GPIO
void init_gpio() {
    for (int i = 0; i < 4; i++) {
        gpio_init(Linha[i]);
        gpio_set_dir(Linha[i], GPIO_OUT);
        gpio_put(Linha[i], RESET);

        gpio_init(Coluna[i]);
        gpio_set_dir(Coluna[i], GPIO_IN);
        gpio_pull_down(Coluna[i]);
    }

    for (int i = 0; i < 8; i++) {
        gpio_init(Segmentos[i]);
        gpio_set_dir(Segmentos[i], GPIO_OUT);
        gpio_put(Segmentos[i], RESET);
    }

    gpio_init(Buzzer);
    gpio_set_dir(Buzzer, GPIO_OUT);
    gpio_put(Buzzer, RESET);

    gpio_init(PushButton_A);
    gpio_set_dir(PushButton_A, GPIO_IN);
    gpio_pull_up(PushButton_A);

    gpio_init(PushButton_B);
    gpio_set_dir(PushButton_B, GPIO_IN);
    gpio_pull_up(PushButton_B);

    gpio_init(LED_R);
    gpio_set_dir(LED_R, GPIO_OUT);
    gpio_put(LED_R, RESET);

    gpio_init(LED_G);
    gpio_set_dir(LED_G, GPIO_OUT);
    gpio_put(LED_G, RESET);

    gpio_init(LED_B);
    gpio_set_dir(LED_B, GPIO_OUT);
    gpio_put(LED_B, RESET);
}

// Varredura do teclado e retorno da tecla pressionada
char leitura_teclado() {
    for (int row = 0; row < 4; row++) {
        gpio_put(Linha[row], SET);
        for (int col = 0; col < 4; col++) {
            if (gpio_get(Coluna[col])) {
                sleep_ms(100);
                while (gpio_get(Coluna[col]));
                gpio_put(Linha[row], RESET);
                return teclas[row][col];
            }
        }
        gpio_put(Linha[row], RESET);
    }
    return 0;
}

// Função para pegar a senha digitada
void get_senha(char *senha, int tam) {
    int index = 0;
    while (index < tam) {
        char key = leitura_teclado();
        if (key != 0) {
            printf("%c", key);
            senha[index] = key;
            index++;
            sleep_ms(100);
        }
    }
    senha[tam] = '\0';
}

// Função para comparar duas senhas
bool compare_senhas(const char *password1, const char *password2) {
    return strcmp(password1, password2) == 0;
}

// Função para exibir a contagem regressiva nos displays de 7 segmentos
void exibir_contagem(int contagem) {
    // Código para exibir a contagem nos displays de 7 segmentos
}

// Função para exibir mensagem de erro no display SSD1306
void exibir_mensagem_erro(int tentativas_restantes) {
    // Código para exibir a mensagem de erro no display SSD1306
}

// Função para ativar o alarme sonoro
void ativar_alarme() {
    gpio_put(Buzzer, SET);
    alarme_ativo = true;
}

// Função para desativar o alarme sonoro
void desativar_alarme() {
    gpio_put(Buzzer, RESET);
    alarme_ativo = false;
}

// Função para controlar o LED RGB
void controlar_led(bool senha_correta) {
    if (senha_correta) {
        gpio_put(LED_R, RESET);
        gpio_put(LED_G, SET);
        gpio_put(LED_B, RESET);
    } else {
        for (int i = 0; i < 3; i++) {
            gpio_put(LED_R, SET);
            gpio_put(LED_G, RESET);
            gpio_put(LED_B, RESET);
            sleep_ms(500);
            gpio_put(LED_R, RESET);
            sleep_ms(500);
        }
    }
}

int main() {
    stdio_init_all();
    init_gpio();

    while (gpio_get(PushButton_A)) {
        // Espera até que o botão_A seja pressionado
    }

    printf("Cadastro da senha do cofre com 4 digitos!\n");
    get_senha(password1, COMP);
    printf("\nSenha gravada!\n");

    while (1) {
        printf("Digite a senha do cofre de 4 digitos!\n");
        char input[COMP + 1] = {0};
        get_senha(input, COMP);

        if (compare_senhas(input, password1)) {
            printf("\nSenha Correta: Acesso ao cofre!\n");
            controlar_led(true);
            break;
        } else {
            printf("\nSenha Incorreta: Acesso ao cofre negado!\n");
            tentativas--;
            controlar_led(false);
            exibir_mensagem_erro(tentativas);
            if (tentativas == 0) {
                printf("\nTentativas esgotadas! Ativando alarme...\n");
                ativar_alarme();
                while (alarme_ativo) {
                    if (!gpio_get(PushButton_B)) {
                        desativar_alarme();
                    }
                }
                tentativas = 3;
            }
        }

        if (!gpio_get(PushButton_A)) {
            for (int i = 10; i >= 0; i--) {
                exibir_contagem(i);
                sleep_ms(1000);
            }
            ativar_alarme();
        }
    }

    return 0;
}
