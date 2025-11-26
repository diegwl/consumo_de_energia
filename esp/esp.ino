#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// --- 1. CONFIGURAÇÕES DE REDE E ACESSO ---
const char* ssid = "HTOIOT";
const char* password = "iot123456";

// *** ATENÇÃO: PREENCHER ESTES VALORES ***
String readApiKey = "20T2RYQ36XEYEOHO"; // Chave de LEITURA do seu canal ThingSpeak
const int ID_CANAL = 3183297;           // ID Numérico do seu Canal
const int FIELD_DE_CONTROLE = 1;       // Usaremos o Field 1 para o comando (1 ou 0)
// ****************************************

// --- 2. HARDWARE E TEMPO ---
const int LED_PIN = LED_BUILTIN;       // LED embutido do ESP8266
const long INTERVALO_POLLING = 5000;   // Checa o comando a cada 5 segundos
unsigned long tempoUltimaLeitura = 0;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // LED (Lógica Invertida) começa DESLIGADO

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
        digitalWrite(LED_PIN, LOW); // LOW para LIGAR o LED embutido
        Serial.println("-> COMANDO RECEBIDO: LIGAR");
      } else if (comando == "0") {
        digitalWrite(LED_PIN, HIGH); // HIGH para DESLIGAR o LED embutido
        Serial.println("-> COMANDO RECEBIDO: DESLIGAR");
      }
    } else {
      Serial.print("Erro de Polling. Código HTTP: ");
      Serial.println(httpCode);
    }
    http.end(); // Fecha a conexão
  }
}
