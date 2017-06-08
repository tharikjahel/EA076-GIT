#include <TimerOne.h>

volatile byte meia_volta;
unsigned int rpm;
unsigned long timeold;
//sensor IR esta no pin 2
#define LED_IR 11 //Pino do Led IR emissor 
#define LED_RESP 13 //Led de responsividade, na propria placa do Arduino
#define MOTOR 3 //Pino PWM de controle do motor
#define MAX_BUFFER_SIZE 15 //Tammanho maximo da string recebida por serial
#define PERIODO 500000 //Intervalo entre as medidas de RPM de 500ms

void rpm_motor () { //Conta toda vez que a helice interrompe o sensor, portanto meia voltaa (2 helices)
  meia_volta++;
  digitalWrite(LED_RESP, !digitalRead(LED_RESP));
}
volatile float rpm_verificado = 0; //RPM calculado
volatile int rpm_pretendido = 0; //RPM desejado - Input
volatile int rpm_pretendido_previo = 0;
float erro = 0, soma_erro = 0; //erro e acumulo de erro
void calculo_rpm() { // Calculo do RPM no periodo de 500ms

  detachInterrupt(0); //Para a interrupcao para evitar erros
  rpm_verificado = ((30 * 1000000) / PERIODO) * meia_volta;
  meia_volta = 0;
  Serial.println(rpm_verificado);

  attachInterrupt(0, rpm_motor, RISING);

}

/*---------------------------------------------COMUNICAÇÃO SERIAL--------------------------------------------  */


volatile int flag_nova_velocidade = 0; //checa se nova velocidade foi pedida

typedef struct {
  char data[MAX_BUFFER_SIZE];
  unsigned int tam_buffer;
} serial_buffer;

volatile serial_buffer Buffer;

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

/* Ao receber evento da UART */
void serialEvent() {
  char c;
  while (Serial.available()) {
    c = Serial.read();
    if ((c == '\n') || (c == '\r') || (c == 0)) {
      buffer_add('\0'); /* Se recebeu um fim de linha, coloca um terminador de string no buffer */
      flag_nova_velocidade = 1;
    } else {
      buffer_add(c);
    }
  }
}

int getInt(char *s) { //transforma string em decimal
  unsigned int number = 0;
  byte i;
  for (i = 0; s[i] != 0; i++) {
    number += s[i] - '0';
    number *= 10;
  }
  return number / 10;
}

void setup()
{
  attachInterrupt(0, rpm_motor, RISING);
  Timer1.initialize(PERIODO);
  Timer1.attachInterrupt(calculo_rpm);
  meia_volta = 0;
  rpm_verificado = 0;
  timeold = 0;
  pinMode(MOTOR, OUTPUT);
  pinMode(2, INPUT_PULLUP);
  pinMode(LED_RESP, OUTPUT);
  digitalWrite(LED_IR, HIGH);
  digitalWrite(LED_RESP, HIGH);
  Serial.begin(9600);
}


void loop() {
  float pot;
  if (flag_nova_velocidade) {
    rpm_pretendido = getInt((char*)Buffer.data);
    buffer_clean();
    flag_nova_velocidade = 0;
    pot = 0;
  }
  erro = rpm_pretendido - rpm_verificado; //erro entre medida e RPM desejado
  if (rpm_pretendido != rpm_pretendido_previo || rpm_pretendido == rpm_verificado) soma_erro = 0;
  soma_erro += erro;

  //O calculo da potencia fornecida ao arduino varia de 0 a 255 e eh alcancada pelo erro associaldo
  //multiplicado por um indice k1 e o aculmulo de erro multiplcicado por k2, ambos os k`s sao constantes e
  //podem mudar de acordo com as condicoes do motor.

  pot = pot  + (0.00001) * soma_erro + (0.01) * erro;


  if (rpm_pretendido == 0)pot = 0;
  if (pot > 255) pot = 255;
  analogWrite(MOTOR, pot);

  rpm_pretendido_previo = rpm_pretendido;

}
