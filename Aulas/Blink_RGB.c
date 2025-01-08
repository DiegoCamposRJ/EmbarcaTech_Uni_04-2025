//Diego da Silva Campos do Nascimento - diegocamposrj@gmail.com
// Exercício da Aula Síncrona 07/01/2025 Blink RGB_Led no BitDogLab
#include <stdio.h>
#include "pico/stdlib.h"
//#include "hardware/gpio.h"

#define LED_R 12
#define LED_G 11
#define LED_B 13

void led_rgb_put(bool r, bool g, bool b){
    gpio_put(LED_R, r);
    gpio_put(LED_G, g);
    gpio_put(LED_B, b);
}
int main(){
    //para usar no Wokwi troque por 
    //Serial.begin(115200); 
    stdio_init_all();

    gpio_init(LED_R);
    gpio_init(LED_G);
    gpio_init(LED_B);
    gpio_set_dir(LED_R, true);
    gpio_set_dir(LED_G, true);
    gpio_set_dir(LED_B, true);

    while(true){
        led_rgb_put(true, false, false);
        sleep_ms(500);

        led_rgb_put(false, true, false);
        sleep_ms(500);

        led_rgb_put(false, false, true);
        sleep_ms(500);

        led_rgb_put(true, true, true);
        sleep_ms(500);
    }
}