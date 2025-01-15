//Aula 14-01-2024 Exer01-https://github.com/DiegoCamposRJ/EmbarcaTech_Uni_04-2025/blob/main/resources/exer_GPIO-Led_leitura2Button.JPG
//https://wokwi.com/projects/420100735187586049
//esquema https://github.com/DiegoCamposRJ/EmbarcaTech_Uni_04-2025/blob/main/resources/GPIO_LED_S_2Button-esquema.JPG

#include <stdio.h>
#include "pico/stdlib.h"

#define LED_R 12
#define LED_G 11
#define LED_B 13
#define BUTTON_A 14
#define BUTTON_B 15

void led_rgb_put(bool r, bool g, bool b){
    gpio_put(LED_R, r);
    gpio_put(LED_G, g);
    gpio_put(LED_B, b);
}

int main(){
    //uso do serial só se aplica para o Wokwi
    Serial.begin(115200);
    //stdio_init_all();

    gpio_init(LED_R);
    gpio_init(LED_G);
    gpio_init(LED_B);
    gpio_set_dir(LED_R, true);
    gpio_set_dir(LED_G, true);
    gpio_set_dir(LED_B, true);

    gpio_init(BUTTON_A);
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_A, false);
    gpio_set_dir(BUTTON_B, false);
    gpio_pull_up(BUTTON_A);
    gpio_pull_up(BUTTON_B);

    while(true){
        bool button_a_pressed = !gpio_get(BUTTON_A);
        bool button_b_pressed = !gpio_get(BUTTON_B);

        if(button_a_pressed){
            led_rgb_put(false, false, true); // LED azul
        } else if(button_b_pressed){
            led_rgb_put(true, false, false); // LED vermelho
        } else {
            led_rgb_put(false, true, false); // LED verde
        }
        sleep_ms(100);
    }
}
