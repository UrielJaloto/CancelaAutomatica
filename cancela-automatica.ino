#include <ESP32Servo.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "NOME_DA_SUA_REDE_WIFI";
const char* password = "SENHA_DA_SUA_REDE_WIFI";
const char* backendIp = "192.168.X.X";

const int pinoTrig = 32;
const int pinoEcho = 34;
const int pinoServo = 25;

int distanciaLimite = 20;
String logCache = "";
int loopCounter = 0;
const int BATCH_INTERVAL = 10;

Servo meuServo;
String apiGatewayUrl = "http://" + String(backendIp) + ":8000";

void setupWiFi() {
  Serial.println("Conectando ao WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Conectado!");
  Serial.print("Endereco IP: ");
  Serial.println(WiFi.localIP());
}

void buscarConfiguracao() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String getUrl = apiGatewayUrl + "/controle";
    
    Serial.print("Buscando config: ");
    Serial.println(getUrl);

    http.begin(getUrl);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      StaticJsonDocument<128> doc;
      DeserializationError error = deserializeJson(doc, payload);

      if (error) {
        Serial.print("Falha ao parsear JSON: ");
        Serial.println(error.c_str());
      } else {
        distanciaLimite = doc["proximidade"];
        Serial.print(">>> Limite de distancia atualizado para: ");
        Serial.println(distanciaLimite);
      }
    } else {
      Serial.print("Erro no GET /controle: ");
      Serial.println(httpCode);
    }
    http.end();
  } else {
    Serial.println("WiFi Desconectado. Nao foi possivel buscar config.");
  }
}

void enviarLogs() {
  if (WiFi.status() == WL_CONNECTED && logCache.length() > 0) {
    HTTPClient http;
    String postUrl = apiGatewayUrl + "/logging";
    
    http.begin(postUrl);
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<1024> doc;
    doc["mensagem"] = logCache;

    String jsonPayload;
    serializeJson(doc, jsonPayload);

    Serial.println("Enviando logs para o backend...");
    int httpCode = http.POST(jsonPayload);

    if (httpCode == HTTP_CODE_CREATED) {
      Serial.println("Logs enviados com sucesso.");
      logCache = "";
    } else {
      Serial.print("Erro no POST /logging: ");
      Serial.println(httpCode);
    }
    http.end();
  } else if (logCache.length() == 0) {
      Serial.println("Cache de log vazio. Nada a enviar.");
  } else {
    Serial.println("WiFi Desconectado. Nao foi possivel enviar logs.");
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(pinoTrig, OUTPUT);
  pinMode(pinoEcho, INPUT);

  meuServo.attach(pinoServo);
  meuServo.write(91);
  
  setupWiFi();
  
  Serial.println(">>> Sistema de Proximidade Iniciado (Modo Rede) <<<");
  
  buscarConfiguracao();
  
  Serial.println("---------------------------------------------------------");
}

void loop() {
  digitalWrite(pinoTrig, LOW);
  delayMicroseconds(2);
  digitalWrite(pinoTrig, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinoTrig, LOW);

  long duracao = pulseIn(pinoEcho, HIGH, 25000);
  float distancia = (duracao * 0.034) / 2;

  String logEntry = "Distancia Atual: " + String(distancia, 1) + 
                    " cm  |  Limite de Acionamento: " + String(distanciaLimite) + "\n";
  
  Serial.print(logEntry);
  logCache += logEntry;

  if (distancia < distanciaLimite && distancia > 0) {
    meuServo.write(0);
    (delay(3000));
  } else {
    meuServo.write(91);
  }
  
  delay(200); 

  loopCounter++;
  
  if (loopCounter >= BATCH_INTERVAL) {
    Serial.println("-------------------- [Ciclo de Rede] --------------------");
    enviarLogs();
    buscarConfiguracao();
    loopCounter = 0;
    Serial.println("---------------------------------------------------------");
  }
}