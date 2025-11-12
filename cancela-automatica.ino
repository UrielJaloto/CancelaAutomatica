#include <ESP32Servo.h>

const int pinoTrig = 32;
const int pinoEcho = 34;
const int pinoServo = 25;
const int pinoBotaoAumenta = 26;
const int pinoBotaoDiminui = 27;

int distanciaLimite = 20;

Servo meuServo;

unsigned long ultimoTempoBotao = 0;
const int intervaloDebounce = 200; 

void setup() {
  Serial.begin(115200);

  pinMode(pinoTrig, OUTPUT);
  pinMode(pinoEcho, INPUT);

  pinMode(pinoBotaoAumenta, INPUT_PULLUP);
  pinMode(pinoBotaoDiminui, INPUT_PULLUP);

  meuServo.attach(pinoServo);

  meuServo.write(91);
  
  Serial.println(">>> Sistema de Proximidade Iniciado (Pinos Re-mapeados) <<<");
  Serial.print("Distancia limite inicial: ");
  Serial.println(distanciaLimite);
  Serial.println("---------------------------------------------------------");
}

void loop() {
  
  if (millis() - ultimoTempoBotao > intervaloDebounce) {
    if (digitalRead(pinoBotaoAumenta) == LOW) {
      distanciaLimite += 5;
      Serial.print(">>> Limite AUMENTADO para: ");
      Serial.println(distanciaLimite);
      ultimoTempoBotao = millis();
    }

    if (digitalRead(pinoBotaoDiminui) == LOW) {
      if (distanciaLimite > 5) {
        distanciaLimite -= 5;
        Serial.print(">>> Limite DIMINUIDO para: ");
        Serial.println(distanciaLimite);
      }
      ultimoTempoBotao = millis();
    }
  }

  digitalWrite(pinoTrig, LOW);
  delayMicroseconds(2);
  digitalWrite(pinoTrig, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinoTrig, LOW);

  long duracao = pulseIn(pinoEcho, HIGH, 25000);
  float distancia = (duracao * 0.034) / 2;

  Serial.print("Distancia Atual: ");
  Serial.print(distancia);
  Serial.print(" cm  |  Limite de Acionamento: ");
  Serial.println(distanciaLimite);

  if (distancia < distanciaLimite && distancia > 0) {
    meuServo.write(0);
    (delay(3000));
  } else {
    meuServo.write(91);
  }

  delay(200);  
}