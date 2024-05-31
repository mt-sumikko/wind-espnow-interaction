//omom m5atom curtain wind relay control
#include <esp_now.h>
#include <WiFi.h>

#include <Arduino.h>
#include <FastLED.h>  //LED

//LED -----------
#define PIN_LED 21  // 本体フルカラーLEDの使用端子（G21）S3
#define NUM_LEDS 1  // 本体フルカラーLEDの数

CRGB leds[NUM_LEDS];
//------------------ -


#define FAN_PIN_1 22
#define FAN_PIN_2 19
#define FAN_PIN_3 23
#define FAN_PIN_4 33
#define FAN_PIN_5 21

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
const char *message = "Hello";

// 送信コールバック関数
void onSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

// 受信コールバック関数
void onReceive(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  Serial.print("Received Message: ");
  // バイトデータを文字列に変換して表示
  char receivedMessage[data_len + 1];
  memcpy(receivedMessage, data, data_len);
  receivedMessage[data_len] = '\0';  // 終端文字を追加
  Serial.println(receivedMessage);

  // メッセージが "Hello" の場合、LEDを点灯
  if (strcmp(receivedMessage, "Hello") == 0) {
    Serial.println("FAN1");
    digitalWrite(FAN_PIN_1, HIGH);
    delay(1000);  // 1秒間LEDを点灯
    digitalWrite(FAN_PIN_1, LOW);
  }
}

void setup() {
  // シリアルモニタの初期化
  Serial.begin(115200);

  // Wi-Fiモードを設定
  WiFi.mode(WIFI_STA);

  // ESP-NOWの初期化
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW initialization failed");
    return;
  }

  // 送信コールバック関数の登録
  esp_now_register_send_cb(onSent);

  // 受信コールバック関数の登録
  esp_now_register_recv_cb(onReceive);

  // ピアの追加
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  // ピンの初期化
  pinMode(FAN_PIN_1, OUTPUT);
  pinMode(FAN_PIN_2, OUTPUT);
  pinMode(FAN_PIN_3, OUTPUT);
  pinMode(FAN_PIN_4, OUTPUT);
  pinMode(FAN_PIN_5, OUTPUT);

  digitalWrite(FAN_PIN_1, LOW);
  digitalWrite(FAN_PIN_2, LOW);
  digitalWrite(FAN_PIN_3, LOW);
  digitalWrite(FAN_PIN_4, LOW);
  digitalWrite(FAN_PIN_5, LOW);

  //マイコン起動サインとしてLEDつけておく
  FastLED.addLeds<WS2812B, PIN_LED, GRB>(leds, NUM_LEDS);
  leds[0] = CRGB(0, 20, 0);  //CRGB::Blue;
  FastLED.show();
}

void loop() {
  // メッセージの送信
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)message, strlen(message));
  if (result == ESP_OK) {
    Serial.println("Message sent");
  } else {
    Serial.println("Message send failed");
  }

  //非受信時のテスト用 START-----
  digitalWrite(FAN_PIN_1, HIGH);
  delay(600);  // 1秒間ファンをを回す

  digitalWrite(FAN_PIN_2, HIGH);
  delay(600);  // 1秒間ファンをを回す
  digitalWrite(FAN_PIN_1, LOW);

  digitalWrite(FAN_PIN_3, HIGH);
  delay(600);  // 1秒間ファンをを回す
  digitalWrite(FAN_PIN_2, LOW);

  digitalWrite(FAN_PIN_4, HIGH);
  delay(600);  // 1秒間ファンをを回す
  digitalWrite(FAN_PIN_3, LOW);

  digitalWrite(FAN_PIN_5, HIGH);
  delay(600);  // 1秒間ファンをを回す
  digitalWrite(FAN_PIN_4, LOW);

  delay(600);  // 1秒間ファンをを回す
  digitalWrite(FAN_PIN_5, LOW);

  delay(300);

  //非受信時のテスト用 END-----
}
