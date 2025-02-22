//Diego da Silva Campos do Nascimento-diegocamposrj
//Projeto final controle de versão
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"

// Definições de pinos
#define TRIG_PIN 2
#define ECHO_PIN 3
#define SDA_PIN 0
#define SCL_PIN 1
#define ROW1 4
#define ROW2 8
#define ROW3 9
#define ROW4 16
#define COL1 17
#define COL2 18
#define COL3 19
#define COL4 20
#define VRX_PIN 26
#define VRY_PIN 27
#define SW_PIN 22
#define SERVO_PIN 28

// Constantes
#define DISTANCE_THRESHOLD 50 // cm
#define SUS_CODE "732589687360508"
#define PASSWORD "0873"

// Definições do LCD I2C
#define I2C_PORT i2c0
#define LCD_ADDRESS 0x27
#define LCD_DELAY_US 2000

// Variáveis globais
int medicine_count = 10;
char keypad_input[16];

// Protótipos das funções
void lcd_send_byte(uint8_t data, uint8_t rs);
void lcd_init();
void lcd_clear();
void lcd_set_cursor(uint8_t col, uint8_t row);
void lcd_write_char(char c);
void lcd_write_string(const char* str);
void init_peripherals();
bool detect_presence();
void display_message(const char* message, bool scroll);
char* read_keypad(int max_length);
bool verify_sus(const char* input);
bool verify_password(const char* input);
void display_menu(int* selected);
int select_medicine();
void release_medicine();
void check_presence_and_sleep();

// Funções do LCD permanecem inalteradas
void lcd_send_byte(uint8_t data, uint8_t rs) {
    uint8_t buf[4];
    uint8_t backlight = 0x08;
    uint8_t high_nibble = (data & 0xF0) | rs | backlight;
    uint8_t low_nibble = ((data << 4) & 0xF0) | rs | backlight;

    buf[0] = high_nibble | 0x04;
    buf[1] = high_nibble;
    buf[2] = low_nibble | 0x04;
    buf[3] = low_nibble;

    i2c_write_blocking(I2C_PORT, LCD_ADDRESS, buf, 4, false);
    sleep_us(LCD_DELAY_US);
}

void lcd_init() {
    i2c_init(I2C_PORT, 100 * 1000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    sleep_ms(50);
    lcd_send_byte(0x03 << 4, 0); sleep_ms(5);
    lcd_send_byte(0x03 << 4, 0); sleep_us(150);
    lcd_send_byte(0x03 << 4, 0);
    lcd_send_byte(0x02 << 4, 0);

    lcd_send_byte(0x28, 0);
    lcd_send_byte(0x0C, 0);
    lcd_send_byte(0x01, 0);
    lcd_send_byte(0x06, 0);
}

void lcd_clear() {
    lcd_send_byte(0x01, 0);
    sleep_ms(2);
}

void lcd_set_cursor(uint8_t col, uint8_t row) {
    uint8_t address = (row == 0 ? 0x00 : 0x40) + col;
    lcd_send_byte(0x80 | address, 0);
}

void lcd_write_char(char c) {
    lcd_send_byte(c, 1);
}

void lcd_write_string(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        lcd_write_char(str[i]);
    }
}

// Alteração 1: Mensagem "Bem Vindo" mais lenta
void display_message(const char* message, bool scroll) {
    if (scroll) {
        lcd_set_cursor(0, 0); // Começa na linha 0
        for (int i = 0; message[i] != '\0'; i++) {
            lcd_write_char(message[i]);
            sleep_ms(200); // Delay de 200ms entre cada caractere
        }
    } else {
        lcd_clear();
        lcd_set_cursor(0, 0); // Começa na linha 0
        lcd_write_string(message);
    }
}

void init_peripherals() {
    lcd_init();
    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(SERVO_PIN);
    pwm_set_wrap(slice_num, 20000);
    pwm_set_chan_level(slice_num, PWM_CHAN_A, 1500);
    pwm_set_enabled(slice_num, true);

    adc_init();
    adc_gpio_init(VRX_PIN);
    adc_gpio_init(VRY_PIN);
    gpio_init(SW_PIN);
    gpio_set_dir(SW_PIN, GPIO_IN);
    gpio_pull_up(SW_PIN);

    gpio_init(ROW1); gpio_set_dir(ROW1, GPIO_OUT); gpio_put(ROW1, 1);
    gpio_init(ROW2); gpio_set_dir(ROW2, GPIO_OUT); gpio_put(ROW2, 1);
    gpio_init(ROW3); gpio_set_dir(ROW3, GPIO_OUT); gpio_put(ROW3, 1);
    gpio_init(ROW4); gpio_set_dir(ROW4, GPIO_OUT); gpio_put(ROW4, 1);
    gpio_init(COL1); gpio_set_dir(COL1, GPIO_IN); gpio_pull_up(COL1);
    gpio_init(COL2); gpio_set_dir(COL2, GPIO_IN); gpio_pull_up(COL2);
    gpio_init(COL3); gpio_set_dir(COL3, GPIO_IN); gpio_pull_up(COL3);
    gpio_init(COL4); gpio_set_dir(COL4, GPIO_IN); gpio_pull_up(COL4);

    gpio_init(TRIG_PIN); gpio_set_dir(TRIG_PIN, GPIO_OUT);
    gpio_init(ECHO_PIN); gpio_set_dir(ECHO_PIN, GPIO_IN);
}

// Alteração 2: Sensor ultrassônico com delay de 20 segundos
bool detect_presence() {
    gpio_put(TRIG_PIN, 1);
    sleep_us(10);
    gpio_put(TRIG_PIN, 0);

    uint32_t start = time_us_32();
    while (!gpio_get(ECHO_PIN) && (time_us_32() - start) < 5000);
    start = time_us_32();
    while (gpio_get(ECHO_PIN) && (time_us_32() - start) < 30000);
    uint32_t duration = time_us_32() - start;

    float distance = (duration * 0.0343) / 2;
    if (distance < DISTANCE_THRESHOLD) {
        sleep_ms(20000); // Delay de 20 segundos antes de ativar o LCD
        return true;
    }
    return false;
}

// Alteração 3: Visualização dos caracteres digitados
char* read_keypad(int max_length) {
    char keys[4][4] = {
        {'1', '2', '3', 'A'},
        {'4', '5', '6', 'B'},
        {'7', '8', '9', 'C'},
        {'*', '0', '#', 'D'}
    };
    int pos = 0;
    lcd_set_cursor(0, 1); // Move o cursor para a segunda linha
    while (pos < max_length) {
        for (int row = 0; row < 4; row++) {
            gpio_put(ROW1 + row, 0);
            if (!gpio_get(COL1)) {
                keypad_input[pos] = keys[row][0];
                lcd_write_char(keypad_input[pos]); // Exibe o caractere no LCD
                pos++;
            }
            if (!gpio_get(COL2)) {
                keypad_input[pos] = keys[row][1];
                lcd_write_char(keypad_input[pos]); // Exibe o caractere no LCD
                pos++;
            }
            if (!gpio_get(COL3)) {
                keypad_input[pos] = keys[row][2];
                lcd_write_char(keypad_input[pos]); // Exibe o caractere no LCD
                pos++;
            }
            if (!gpio_get(COL4)) {
                if (keys[row][3] == 'A') {
                    keypad_input[pos] = '\0';
                    return keypad_input;
                }
                keypad_input[pos] = keys[row][3];
                lcd_write_char(keypad_input[pos]); // Exibe o caractere no LCD
                pos++;
            }
            gpio_put(ROW1 + row, 1);
            sleep_ms(50);
        }
    }
    keypad_input[pos] = '\0';
    return keypad_input;
}

bool verify_sus(const char* input) {
    return strcmp(input, SUS_CODE) == 0;
}

bool verify_password(const char* input) {
    return strcmp(input, PASSWORD) == 0;
}

void display_menu(int* selected) {
    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_write_string(*selected == 0 ? ">1. Paracetamol" : " 1. Paracetamol");
    lcd_set_cursor(0, 1);
    lcd_write_string(*selected == 1 ? ">2. Ibuprofeno" : " 2. Ibuprofeno");
}

int select_medicine() {
    int selected = 0;
    while (true) {
        adc_select_input(1); // VRY
        uint16_t vry = adc_read();
        if (vry < 1000 && selected > 0) selected--;
        if (vry > 3000 && selected < 1) selected++;
        if (!gpio_get(SW_PIN)) break;
        display_menu(&selected);
        sleep_ms(200);
    }
    return selected;
}

void release_medicine() {
    pwm_set_chan_level(pwm_gpio_to_slice_num(SERVO_PIN), PWM_CHAN_A, 2500);
    sleep_ms(1000);
    pwm_set_chan_level(pwm_gpio_to_slice_num(SERVO_PIN), PWM_CHAN_A, 1500);
}

void check_presence_and_sleep() {
    sleep_ms(5000);
    if (!detect_presence()) {
        display_message("Desligando...", false);
    }
}

int main() {
    stdio_init_all();
    init_peripherals();

    while (true) {
        if (detect_presence()) {
            display_message("Bem Vindo - Seu remédio esta pronto para ser retirado.", true);
            sleep_ms(2000);

            display_message("Digite SUS:", false);
            char* sus_input = read_keypad(15);
            if (!verify_sus(sus_input)) {
                display_message("Cartão Não Aceito - Digite novamente!", true);
                continue;
            }

            display_message("Digite Senha:", false);
            char* password_input = read_keypad(4);
            if (!verify_password(password_input)) {
                display_message("Senha Incorreta - Tente novamente!", true);
                continue;
            }

            int selected = 0;
            display_menu(&selected);
            int medicine = select_medicine();
            release_medicine();
            medicine_count--;

            check_presence_and_sleep();
        } else {
            sleep_ms(1000);
        }
    }
    return 0;
}