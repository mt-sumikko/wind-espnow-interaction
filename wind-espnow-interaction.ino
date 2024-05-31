#include "M5Atom.h"
#include <esp_now.h>
#include <WiFi.h>

//LED -----------
uint8_t brightness;
uint8_t change = 1;//起動時Auto, 1回ボタン押したらInteraction Modeになるようにしておく

CRGB dispColor(uint8_t g, uint8_t r, uint8_t b) {
  return (CRGB)((g << 16) | (r << 8) | b);
}

//------------------


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



//LED点灯タスクを渡す変数
//int ledTask = 0;
int runningMode = 0;//0:Auto, 1:Interaction, 2:Off

//LEDの点灯をコントロール
void windControl(void* arg) {
  while (1) {
    if (runningMode == 0) { //Auto
      //ledTask = 0;


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

      delay(8000);

      //非受信時のテスト用 END-----


    } else if (runningMode == 1) { //Interaction
      //ledTask = 0;


    } else { //Off
      //適度に待つ
      delay(100);
    }
  }
}


//稼働モードを管理するタスク
void modeControl(void* arg) {
  while (1) {

    // ボタンが押されたらLEDの色を変化させる
    if (M5.Btn.wasPressed()) {
      Serial.print("Btn Pressed");
      Serial.println(change);
      switch (change) {
        case 0:
          M5.dis.drawpix(0, dispColor(0, brightness, 0));//green
          runningMode = 0;//Auto
          break;
        case 1:
          M5.dis.drawpix(0, dispColor(0, 0, brightness));//blue
          runningMode =  1;//Interaction
          break;
        case 2:
          M5.dis.drawpix(0, dispColor(brightness, 0, 0));//red
          runningMode = 2;//Off
          break;
        default:
          break;
      }
      if (change >= 2) {
        change = 0;
      } else {
        change++;
      }
    }


    M5.update();
    delay(40);
  }
}



void setup() {
  // シリアルモニタの初期化
  Serial.begin(115200);
  M5.begin(false, false, true);
  brightness = 0xFF;
  M5.dis.drawpix(0, dispColor(0, brightness, 0));//green
  delay(10);

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

  //タスクの設定
  xTaskCreatePinnedToCore(windControl, "windControlTask0", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(modeControl, "modeControlTask0", 4096, NULL, 1, NULL, 1);
}

void loop() {
  // メッセージの送信
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)message, strlen(message));
  if (result == ESP_OK) {
    Serial.println("Message sent");
  } else {
    Serial.println("Message send failed");
  }

  delay(1000);
}
