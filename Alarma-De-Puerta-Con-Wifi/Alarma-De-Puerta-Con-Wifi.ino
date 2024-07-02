#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// remplazar con tu informacion
const char* ssid = "*************";
const char* password = "***********";

// iniciar Telegram BOT
#define BOTtoken "7257081526:AAHWGQc8tk0k8Zag3E9hZKP53MIKe-2iTt0"  // your Bot Token (Get from Botfather)

// ingresar chat id del usuario o del grupo
// recuerda darle start al bot para que se active
// enviale un mensaje al bot(cualquier cosa)
#define CHAT_ID "6793359298"

X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

const int pinBuzzer = 14; // D5 -> GPIO14
const int HALLPin = 12;   // D6 -> GPIO12
const int PIRPin = 13;    // D7 -> GPIO13

bool motionDetected = false;
unsigned long lastMessageTime = 0;
const unsigned long messageInterval = 5000; // Intervalo mínimo entre mensajes en milisegundos

void ICACHE_RAM_ATTR detectsMovement() {
  motionDetected = true;
}

void setup() {
  Serial.begin(115200);
  configTime(0, 0, "pool.ntp.org"); // tomamos la hora 
  client.setTrustAnchors(&cert);    // certificamos la api de telegram

  pinMode(HALLPin, INPUT); // Set sensor hall como input
  pinMode(PIRPin, INPUT_PULLUP);  // et sensor Pir como input internal pull-up
  pinMode(pinBuzzer, OUTPUT); // et sensor buzzer como output

  // se genera una interrupcion:Las interrupciones son útiles para responder rápidamente a eventos, como la señal de un sensor de movimiento, sin tener que comprobar constantemente el estado del sensor en el programa principal (lo que se conoce como "sondeo").
  attachInterrupt(digitalPinToInterrupt(PIRPin), detectsMovement, RISING);

  // aca intenta conectarse a la red
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  bot.sendMessage(CHAT_ID, "Bot started up", "");
}

void loop() {
  // lee los valores de los pines
  int hallValue = digitalRead(HALLPin);
  int pirValue = digitalRead(PIRPin);
  unsigned long currentMillis = millis();

  // si ambos sensores detectan
  if (hallValue == HIGH && pirValue == HIGH) {
    // genera sonido el buzzer
    tone(pinBuzzer, 523);

    // envia el mensaje pasado el tiempo de envio del otro
    if (currentMillis - lastMessageTime > messageInterval) {
      bot.sendMessage(CHAT_ID, "PUERTA ABIERTA Y MOVIMIENTO ADENTRO", "");
      Serial.println("PUERTA ABIERTA Y MOVIMIENTO ADENTRO");
      lastMessageTime = currentMillis; // pasado el tiempo se actualiza y vuelve a leer los sensores para ver si hay moviemiento o algo
    }
  } 
  // si solo el pir detecta
  else if (pirValue == HIGH && hallValue == LOW) {
    if (currentMillis - lastMessageTime > messageInterval) {
      // envia mensaje por telegram
      bot.sendMessage(CHAT_ID, "MOVIMIENTO DETECTADO", "");
      Serial.println("SE DETECTO MOVIMIENTO,LA PUERTA ESTA CERRADA");
      lastMessageTime = currentMillis; // pasado el tiempo se actualiza y vuelve a leer los sensores para ver si hay moviemiento o algo
    }
    noTone(pinBuzzer); // buzzer apagado
  }
  // si el sensor esta bajo
  else {
    // detener el buzzer
    noTone(pinBuzzer);
  }

  delay(10); // ajustado para que no sobre cargue el cpu
}