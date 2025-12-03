#include <WiFi.h>
#include <HTTPClient.h>

// --- 1. CONFIGURAÇÕES DE REDE E ACESSO ---
const char* ssid = "DiegoDavi";
const char* password = "aa1110840710830210";

// *** ATENÇÃO: PREENCHER ESTES VALORES ***
String readApiKey = "20T2RYQ36XEYEOHO"; // Chave de LEITURA do seu canal ThingSpeak
String writeApiKey = "WTTUCOLHX0BCLCHM";
const int ID_CANAL = 3183297;           // ID Numérico do seu Canal
const int FIELD_DE_CONTROLE = 2;
const int FIELD_DE_CONSUMO = 1;       
// ****************************************

// --- 2. HARDWARE E TEMPO ---
const int LED_PIN = 2;       // LED embutido do ESP8266
const long INTERVALO_POLLING = 5000;   // Checa o comando a cada 5 segundos
unsigned long tempoUltimaLeitura = 0;

int consumoVar = 0;
int statusLuz = 0;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // LED (Lógica Invertida) começa DESLIGADO

  // Conexão Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado! Pronto para Polling.");
}

void loop() {
  // --- ROTINA DE POLLING (LEITURA DO COMANDO) ---
  if (millis() - tempoUltimaLeitura >= INTERVALO_POLLING) {
    tempoUltimaLeitura = millis(); // Reinicia o contador

    // Monta a URL para ler o último valor do Field 1
    // A URL exige o ID do Canal, o número do Field e a Read API Key
    String urlLeitura = "http://api.thingspeak.com/channels/" + String(ID_CANAL) + "/fields/" + String(FIELD_DE_CONTROLE) + "/last.txt?api_key=" + readApiKey;

    WiFiClient client;
    HTTPClient http;
    http.begin(client, urlLeitura);

    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
      String comando = http.getString();
      comando.trim(); // Remove espaços para garantir que '1' e '0' sejam lidos corretamente

      Serial.print("Comando Lido: ");
      Serial.println(comando);

      // --- LÓGICA DE ATUAÇÃO (Lógica Invertida) ---
      if (comando == "1") {
        digitalWrite(LED_PIN, HIGH); // LOW para LIGAR o LED embutido
        Serial.println("-> COMANDO RECEBIDO: LIGAR");
        statusLuz = 1;
      } else if (comando == "0") {
        digitalWrite(LED_PIN, LOW); // HIGH para DESLIGAR o LED embutido
        Serial.println("-> COMANDO RECEBIDO: DESLIGAR");
        statusLuz = 0;
      }
    } else {
      Serial.print("Erro de Polling. Código HTTP: ");
      Serial.println(httpCode);
    }
    http.end(); // Fecha a conexão

    String urlLeituraConsumo = "http://api.thingspeak.com/channels/" + String(ID_CANAL) + "/fields/" + String(FIELD_DE_CONSUMO) + "/last.txt?api_key=" + readApiKey;

    http.begin(client, urlLeituraConsumo);

    httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
      String consumo = http.getString();
      consumo.trim();

      Serial.print("Consumo Lido: ");
      Serial.println(consumo);

      consumoVar = consumo.toInt();

      if (statusLuz == 1) {
        consumoVar++;
      }
    } else {
      Serial.print("Erro de Polling. Código HTTP: ");
      Serial.println(httpCode);
    }
    http.end(); // Fecha a conexão

    delay(5000);

    String urlEscritaConsumo = "http://api.thingspeak.com/update?api_key=" + writeApiKey + "&field" + String(FIELD_DE_CONSUMO) + "=" + String(consumoVar);

    if (statusLuz == 1) {
      http.begin(client, urlEscritaConsumo);
  
      httpCode = http.GET();
  
      if (httpCode == HTTP_CODE_OK) {
        Serial.print("Consumo Escrito: ");
        Serial.println(consumoVar);
      } else {
        Serial.print("Erro de Polling. Código HTTP: ");
        Serial.println(httpCode);
      }
    }

    http.end(); // Fecha a conexão
  }
}
