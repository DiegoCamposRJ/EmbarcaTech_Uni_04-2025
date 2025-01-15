//Diego da Silva Campos do Nascimento-diegocamposrj@gmail.com
//Esquema-https://github.com/DiegoCamposRJ/EmbarcaTech_Uni_04-2025/blob/main/resources/TCD_M4x4_Led-esquema.JPG
//https://wokwi.com/projects/420112231014771713
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#define LED_R 12
#define LED_G 11
#define LED_B 13

#define ROW1 8
#define ROW2 7
#define ROW3 6
#define ROW4 5
#define COL1 4
#define COL2 3
#define COL3 2
#define COL4 1

char get_keypad_key() {
    const char keys[4][4] = {
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'}
    };

    for (int row = 0; row < 4; row++) {
        gpio_put(ROW1 - row, 0);
        for (int col = 0; col < 4; col++) {
            if (!gpio_get(COL1 - col)) {
                gpio_put(ROW1 - row, 1);
                return keys[row][col];
            }
        }
        gpio_put(ROW1 - row, 1);
    }
    return '\0';
}

void led_rgb_put(bool r, bool g, bool b){
    gpio_put(LED_R, r);
    gpio_put(LED_G, g);
    gpio_put(LED_B, b);
}

int main(){
    stdio_init_all(); // Inicializa a comunicação serial

    gpio_init(LED_R);
    gpio_init(LED_G);
    gpio_init(LED_B);
    gpio_set_dir(LED_R, GPIO_OUT);
    gpio_set_dir(LED_G, GPIO_OUT);
    gpio_set_dir(LED_B, GPIO_OUT);

    for (int row = 0; row < 4; row++) {
        gpio_init(ROW1 - row);
        gpio_set_dir(ROW1 - row, GPIO_OUT);
        gpio_put(ROW1 - row, 1);
    }

    for (int col = 0; col < 4; col++) {
        gpio_init(COL1 - col);
        gpio_set_dir(COL1 - col, GPIO_IN);
        gpio_pull_up(COL1 - col);
    }

    while(true){
        char key = get_keypad_key();

        if (key != '\0') {
            printf("Tecla pressionada: %c\n", key);
        }

        switch (key) {
            case '1':
            case '4':
            case '7':
            case '*':
                led_rgb_put(true, false, false); // LED vermelho
                break;
            case '2':
            case '5':
            case '8':
            case '0':
                led_rgb_put(false, true, false); // LED verde
                break;
            case '3':
            case '6':
            case '9':
            case '#':
                led_rgb_put(false, false, true); // LED azul
                break;
            default:
                led_rgb_put(false, false, false); // Nenhum LED aceso
                break;
        }
        sleep_ms(100);
    }
}
