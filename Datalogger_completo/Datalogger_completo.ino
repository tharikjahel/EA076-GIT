/*Tharik Jahel e Lucas Meirelles */

#include <TimerOne.h>
#include <Key.h>
#include <Keypad.h>
#include <EEPROM.h>
#include <stdio.h>
#include <Wire.h>
#define LDR 0 //Sensor utilizado
#define LED 12
#define MAX_BUFFER_SIZE 15

int end_eeprom = 0x50; //endereco da eeprom no barramento i2c
int posicao_mem = 1; //posicao do cursor de memoria
int flag_med_auto = 0; //flag de controle da medicao automatica

/*-------------------------- Rotina auxiliar para comparacao de strings-------------------------------------------------- */
int str_cmp(char s1[], char s2[], int len) {
  /* Compare two strings up to length len. Return 1 if they are
      equal, and 0 otherwise.
  */
  int i;
  for (i = 0; i < len; i++) {
    if (s1[i] != s2[i]) return 0;
    if (s1[i] == '\0') return 1;
  }
  return 1;
}


/* Buffer de dados recebidos */
#define MAX_BUFFER_SIZE 15
typedef struct {
  char data[MAX_BUFFER_SIZE];
  unsigned int tam_buffer;
} serial_buffer;

serial_buffer Buffer; //buffer que utilizamos

/* Limpa buffer */
void buffer_clean() {
  Buffer.tam_buffer = 0;
}

/* Adiciona caractere ao buffer */
int buffer_add(char c_in) {
  if (Buffer.tam_buffer < MAX_BUFFER_SIZE) {
    Buffer.data[Buffer.tam_buffer++] = c_in;
    return 1;
  }
  return 0;
}

/*--------------------------------------- Flags globais para controle de processos da interrupcao------------------------------- */
volatile int flag_check_command = 0;

/* Rotinas de interrupcao */

/* Ao receber evento da UART */
void serialEvent() {
  char c;
  while (Serial.available()) {
    c = Serial.read();
    if (c == '\n') {
      buffer_add('\0'); /* Se recebeu um fim de linha, coloca um terminador de string no buffer */
      flag_check_command = 1;
    } else {
      buffer_add(c);
    }
  }
}

/*-----------------------------------------------------------EEPROM------------------------------------------------------------ */
//Funcao de escrita na eeprom atraves do barramento i2c
void i2c_eeprom_escrita(unsigned int end_memoria, char valor)
{
  Wire.beginTransmission(end_eeprom);
  Wire.write(end_memoria);
  Wire.write(valor);
  Wire.endTransmission();
  delay(5);
}

/*Funcao de leitura na eeprom, que retorna um byte (char)*/
char i2c_eeprom_leitura(unsigned int end_memoria)
{
  Wire.beginTransmission(end_eeprom);
  Wire.write(end_memoria);
  Wire.endTransmission();
  Wire.requestFrom(end_eeprom, 1);
  int dado;
  if (Wire.available()) {
    dado = Wire.read();
    return dado;
  }
}
/*------------------------------------------------------TECLADO Matricial--------------------------------------------------------------------------*/
/* Para a implementacao do teclado utilizamos a biblioteca keypad.h*/
#define LINHAS 4 //quantidades de linhas e colunas
#define COLUNAS 3
//matriz dos caracteres correspondntes as teclas do teclado
char mapa_teclado[LINHAS][COLUNAS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

byte Pinos_linhas[LINHAS] = {4, 5, 6, 7};
byte Pinos_colunas[COLUNAS] = {8, 9, 10};

/* Inicializa o teclado */
Keypad teclado_matricial = Keypad( makeKeymap(mapa_teclado), Pinos_linhas, Pinos_colunas, LINHAS, COLUNAS);


/*--------------------------------------------------------- Funcoes internas ao void main()--------------------------------------------- */

void setup() {
  /*------------------------------------------------------------ Inicializacao----------------------------------------------------------- */
  pinMode(LED, OUTPUT);
  buffer_clean();
  flag_check_command = 0;
  Wire.begin(); // initialise the connection
  Serial.begin(9600);
  Timer1.initialize(250000); // Interrupcao a cada 250ms para a medicao automatica
  Timer1.attachInterrupt(med_auto);

}
//Medicao automatica, controlada por interrupcao temporal e flag de comando
void med_auto() {
  byte medida2 = analogRead(LDR);
  if (flag_med_auto) { //checa a flag de controle
    i2c_eeprom_escrita(posicao_mem, medida2);
    posicao_mem++; //avanca o cursor da memoria

  }
}

void loop() {
  int x, y;
  char out_buffer[40];
  int flag_write = 0;
  byte medida = analogRead(LDR);

  if (posicao_mem > 2048) {
    posicao_mem = 1;
  }
  /*Nessa parte do programa sao realizadas as checagens correspondentes ao que e digitado no computador, a partir do terminal serial*/
  if (flag_check_command == 1) {                                //checa se houve algum comando no terminal, depois verifica qual comando e executa a acao correspondente
    if (str_cmp(Buffer.data, "PING", 4) ) {
      sprintf(out_buffer, "PONG\n");
      flag_write = 1;
    }

    if (str_cmp(Buffer.data, "ID", 2) ) {
      sprintf(out_buffer, "DATALOGGER do Tharik e do Lucas\n");
      flag_write = 1;
    }
    if (str_cmp(Buffer.data, "MEASURE", 7) ) {
      sprintf(out_buffer, "MEASURE = %d\n", medida);
      flag_write = 1;

    }
    if (str_cmp(Buffer.data, "RESET", 5) ) {
      posicao_mem = 1;
      sprintf(out_buffer, "RESETADO\n");
      flag_write = 1;
    }
    if (str_cmp(Buffer.data, "RECORD", 6) ) {
      i2c_eeprom_escrita(posicao_mem, medida);
      posicao_mem++;
      sprintf(out_buffer, "RECORDED %d\n", medida);
      flag_write = 1;
    }
    if (str_cmp(Buffer.data, "GET", 3) ) {
      sscanf(Buffer.data, "%*s %d", &x);
      y = i2c_eeprom_leitura(x);
      if (x < 2049 && x > 0) {
        sprintf(out_buffer, "Medida no byte %d = %d\n", x, y);
        Serial.println(y);
        flag_write = 1;
      }
    }
    if (str_cmp(Buffer.data, "MEMSTATUS", 9) ) {
      sprintf(out_buffer, "NUM DE ELEMENTOS NA MEMORIA = %d\n", posicao_mem);
      flag_write = 1;

    }


    if (flag_write == 1) { //Se tem algo no buffer, a flag vai estar em nivel alto e o contaudo do buffer sera impresso no terminal
      Serial.write(out_buffer);
      buffer_clean();
      flag_write = 0;
    }

    /*------------------------------------------CASO TECLADO MATRICIAL------------------------------------------*/
    //Parte do programa que verifica as entradas pelo teclado matricial
    char teclas [2], digito;
    int i, n_teclas;
    digito = teclado_matricial.getKey(); // armazena-se o digito

    if (digito) { //verifica-se se algo foi digitado
      teclas[n_teclas] = digito; //inclui o digito na posicao n correspondente a contagem de digitos (ate 3 digitos)
      n_teclas++; //incrementa a contagem de digitos

      if (n_teclas > 2) { //se temos um vetor de 3 digitos completo, partimos para a verificacao e eventual excecucao dos comandos

        if (str_cmp(teclas, "#1*", 3)) {
          for (i = 0; i < 6; i++) {
            digitalWrite(LED, !digitalRead(LED)); //basicamente pisca 5 vezes o led em intervalos de 500ms
            delay(500);
          }
        }

        if (str_cmp(teclas, "#2*", 3)) {
          i2c_eeprom_escrita(posicao_mem, medida); //vai repetir a funcao RECORD
          posicao_mem++;
          sprintf(out_buffer, "RECORDED %d\n", medida);
          flag_write = 1;
        }
        //MEDICAO AUTO
        if (str_cmp(teclas, "#3*", 3)) { //ativa o modo auto
          flag_med_auto = 1;
        }

        if (str_cmp(teclas, "#4*", 3)) { //desativa o modo auto
          flag_med_auto = 0;
        }
        n_teclas = 0;
      }
    }

  }
}

