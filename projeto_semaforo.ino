/* 
  Nome: Lucas Campos Meirelles RA: 156339
        Tharik Moreira Jahel   RA: 177561
        
  Projeto 1 - EA076C - Semáforo 
*/

#include <TimerOne.h>

// Definimos os pins de cada componente, parâmetros iniciais e variáveis do programa
#define redCar 6
#define yellowCar 5 
#define greenCar 4
#define redPeople 2
#define greenPeople 3
#define button A1
#define LDR 0
#define buzzer 8
int botao=0, tempo=0;
int sensor;
int estado;
int dia=0;

void setup() // Função Setup: Inicializamos as entradas e saídas e as funções do programa 
{
  pinMode(redCar, OUTPUT);
  pinMode(yellowCar, OUTPUT);
  pinMode(greenCar, OUTPUT);
  pinMode(redPeople, OUTPUT);
  pinMode(greenPeople, OUTPUT);
  pinMode(button, INPUT);
  pinMode(buzzer, OUTPUT);
  Timer1.initialize(250000); // Função Timer1: Interrupcao temporal a cada 250ms
  Timer1.attachInterrupt(semaforo);
  Serial.begin(9600);
}

void semaforo() //implementacao da maquina de estados correspondentes ao semaforo
{
  if(dia) //checa se esta durante o dia/ periodo de funcionamento
  {
    if (botao) //se botao foi apertado
    {
      tempo++; //variavel incrementada a cada 250 ms
      if(tempo < 10) //espera 4 segundos apos aperto de botao
      {
        digitalWrite(greenCar, HIGH); 
        digitalWrite(redPeople, HIGH);
      }
      if(tempo>=10 && tempo <15)
      {
        digitalWrite(greenCar, LOW);
        digitalWrite(yellowCar, HIGH);
      }
      if(tempo>=15 && tempo<25)
      {
        digitalWrite(yellowCar, LOW);
        digitalWrite(redCar, HIGH);
        digitalWrite(redPeople, LOW);
        digitalWrite(greenPeople, HIGH);
        if(tempo%2==0)
        {
          digitalWrite(buzzer,!digitalRead(buzzer));
        }
      }
      if (tempo>=25 && tempo <35)
      {
        digitalWrite(greenPeople, LOW);
        digitalWrite(redPeople,!digitalRead(redPeople));
        digitalWrite(buzzer,!digitalRead(buzzer));
      }
      if(tempo>=35)
      {
        digitalWrite(redCar, LOW);
        botao=0;
        tempo=0;
      }
    } 
    else
    {
      digitalWrite(redPeople, HIGH);
      digitalWrite(greenCar, HIGH);
      digitalWrite(redCar, LOW);
      digitalWrite(yellowCar, LOW);
      digitalWrite(greenPeople, LOW);
      digitalWrite(buzzer,LOW);
    }
  }
  else //caso noite, amarelo do carro e vermelho do pedestre piscam
  { 
    digitalWrite(greenCar, LOW);
    digitalWrite(redCar, LOW);
    digitalWrite(yellowCar, !digitalRead(yellowCar));
    digitalWrite(greenPeople, LOW);
    digitalWrite(redPeople, !digitalRead(redPeople));
  }
}

void loop ()
{
  sensor=analogRead(LDR); 
  Serial.println(sensor);
  estado=digitalRead(button);
  if(sensor>150) //conversor A/D
  {
    dia=1; 
  } 
  else 
  {
    dia=0;
  }
  if (!estado && dia) //aqui a variavel do botao e setada caso um aperto seja detectado
  {
    botao=1;
  }
}

 

