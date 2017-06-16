/*------------------ PROJETO FINAL ---------------------*/
//THARIK JAHEL E LUCAS MEIRELLES
/* O projeto consiste em um controlador para um jardim automatizado
    em que foram implementados a leitura de temperatura (sensor LM35),
    a leitura de luminosidade (sensor LDR) e a leitura de umidade da
    terra(impedancia entre os pregos). Assim, foram programadas as respostas
    a essas leituras: acendimento das luzes, acionamento dos coolers e ativacao da bomba de agua.
    A leitura da temperatura e o estado da terra(umido ou nao) sao verificaveis pelo display 16x2.
*/

//PINOS
#include <TimerOne.h>
#include <LiquidCrystal.h>
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
#define LM35 0 //Sensor de temperatura
#define LDR 1 //sensor de luminosidade
#define PREGO 2 //sensor de umidade
#define LAMPADA 9 
#define COOLER 10
#define PUMP 13 //bomba de agua
int Vo;
int dia = 0, terra_molhada = 0;
float R1 = 2500;
float logR2, R2, T;
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07; //constantes de calibracao do sensor para graus celcius 


void setup() {
  pinMode(LDR, INPUT_PULLUP);
  pinMode(PREGO, INPUT_PULLUP);
  pinMode(LAMPADA, OUTPUT);
  pinMode(COOLER, OUTPUT);
  pinMode(PUMP, OUTPUT);
  Serial.begin(9600);
  lcd.begin(16, 2);
  Timer1.initialize(25000); // Interrupcao a cada 25ms
  Timer1.attachInterrupt(temp);
}


void loop () {
  //SENSOR DE TEMPERATURA
  // T = (float(analogRead(LM35))*5/(1023))/0.01;

  //SENSOR DE LUMINOSIDADE
  int luz = analogRead(LDR);

  if (luz > 150) {
    dia = 1;
  } else {
    dia = 0;
  }
  //SENSOR DE HUMIDADE DA TERRA
  int terra = analogRead(PREGO);
  Serial.println(terra);

  if (T > 30) {
    analogWrite(COOLER, 150);
    if (T > 35) {
      analogWrite(COOLER, 255);
    }
  } else {
    analogWrite(COOLER, 0);
  }

  if (!dia) {
    digitalWrite(LAMPADA, HIGH);
  } else {
    digitalWrite(LAMPADA, LOW);
  }

  if (terra < 800) {
    digitalWrite(PUMP, HIGH);
    terra_molhada = 0;
  } else {
    digitalWrite(PUMP, LOW);
    terra_molhada = 1;
  }
//LCD
  lcd.home();
  lcd.print("Temp = ");
  lcd.print(T);
  lcd.print(" C");

  lcd.setCursor(0, 1);
  if (terra_molhada) {
    lcd.print("Terra molhada");
  } else {
    lcd.print("Terra seca");
  }
  delay(500);
  lcd.clear();

}

void temp () {
  int i;
  int samples[8];
  for (i = 0; i <= 7; i++) { // Loop que faz a leitura da temperatura 8 vezes
    samples[i] = ( 5.0 * analogRead(LM35) * 100.0) / 1024.0;
    //A cada leitura, incrementa o valor da variavel tempc
    T = T + samples[i];
    delay(100);
  }

  // Divide a variavel tempc por 8, para obter precisão na medição
  T = T / 8.0;

}

