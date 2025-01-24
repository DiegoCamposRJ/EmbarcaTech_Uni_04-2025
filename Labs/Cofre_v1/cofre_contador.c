#include <stdio.h>
#include "pico/stdlib.h"
#include <string.h>

#define SET 1
#define RESET 0
#define COMP 4

char password1[COMP+1] = {0};

// Define os pinos GPIO para as linhas e para as colunas
const uint8_t Linha[] = {2, 3, 4, 5};
const uint8_t Coluna[] ={6, 7, 8, 9};

// Mapeamento das teclas em uma matriz 4x4
char teclas[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

// Inicializando os pinos de GPIO pins
void init_teclado() {
    for (int i = 0; i < 4; i++) {
      // Configuração das linhas como saídas
      gpio_init(Linha[i]);
      gpio_set_dir(Linha[i], GPIO_OUT);
      gpio_put(Linha[i],RESET);
      
      // Configuração das linhas como saídas
      gpio_init(Coluna[i]);
      gpio_set_dir(Coluna[i], GPIO_IN);
      gpio_pull_down(Coluna[i]);  // força as colunas para zero.
    }
  
}
 
// Varredura do teclado e retorno da tecla pressionada
char leitura_teclado() {
    for (int row = 0; row < 4; row++) {
        // Coloca a linha atual em nível alto
        gpio_put(Linha[row], SET);

        for (int col = 0; col < 4; col++) {
            if (gpio_get(Coluna[col])) {
                // Espera um tempo para estabilização da tecla pressionada
                sleep_ms(100);
                while (gpio_get(Coluna[col])); // Espera até a tecla ser liberada
                gpio_put(Linha[row], RESET); // Reseta a linha atual
                return teclas[row][col];
            }
        }
        // Coloca a linha atual novamente para nível baixo
        gpio_put(Linha[row], RESET);
    }

    return 0; 
}
// Função para pegar a senha digitada 
void get_senha(char *senha, int tam) {
  
    int index = 0;
        while (index < tam) {
          //printf("Estou aqui\n");
        char key = leitura_teclado();
        
        if (key != 0) {
            printf("%c", key); //adicionei para saber qual senha era digitada
            senha[index] = key;
            index=index+1; //funcionou melhor separando
            sleep_ms(100); 
        }
    }
    senha[tam] = '\0'; 
}

// Função para comparar duas senhas
bool compare_senhas(const char *password1, const char *password2) {
    return strcmp(password1, password2) == 0; 
}

int main() {
    stdio_init_all();
    init_teclado();

     printf("Cadastro da senha do cofre com 4 digitos!\n");
     get_senha(password1, COMP);
     printf("\nSenha gravada!\n");

    int cont=0;
    bool acesso = 1;
    
    while (acesso) {
      printf("Digite a senha do cofre de 4 digitos!\n");
          char input[COMP+1] = {0};
                get_senha(input, COMP);
               
        if (compare_senhas(input, password1)) {
            printf("\nSenha Correta: Acesso ao cofre!\n");
            acesso = !acesso;
        }
        else{
          printf("\nSenha Incorreta: Acesso ao cofre negado !\n");
          cont = cont +1;
        }
        sleep_ms(50); // estabilização
        if(cont>2){
          printf("\n Três tentativas bloqueadas por 5 segundos!\n");
          sleep_ms(5000);
          cont= 0;
        }
        
    }
  

}
