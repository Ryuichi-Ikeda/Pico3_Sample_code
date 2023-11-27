/**
 * @file pico_sample.cpp
 * @version 0.1
 * @brief Pico3サンプルコード
 * 　　　 LTE通信モジュールBG770を使用して、MQTTサーバーへ接続
 * 　　　 MQTTからサブスクライブしたら、返事をパブリッシュする
 *
 * @author Iefuji Kohei (iefuji.kohei@kyokko.co.jp)
 * @date 2023-10-10
 * @copyright Copyright (c) 2023 旭光電機株式会社
 */
/**************************************************************************************************
 * INCLUDES
 */
#include <Arduino.h>
#include "bg770.h"
#include "CK_1540_01.h"
#include "setup_define.h"
#include <WiFi.h>
#include <WebServer.h>
#include <Ticker.h>

/***************************************************************************************************
 * LOCAL FUNCTIONS
 */
/**
 * @brief パブリッシュペイロードの作成関数
 * @param[in/out] buf:作成したペイロードを入力するバッファ
 * @return 作成したペイロード長
 */

WebServer server(80);
void Wifi_Setting(void);
Ticker timer;

/**  Main setup **/
void setup() {
  /* GPIOの初期化 */
  initGPIO();

  /* シリアル通信の初期化（デバッグ用） */
  Serial.begin(115200);
  while (!Serial); 
  Serial.println("Starting Serial Monitor");

}
/**  Main loop **/
void loop() {
  static unsigned long pressedTime = 0;
  static bool isPressed = false;

  if (digitalRead(PORT_INP_SW) == LOW) { // スイッチが押されている場合
        if (!isPressed) { // スイッチが初めて押された場合
            isPressed = true;
            pressedTime = millis();
        } else if ((millis() - pressedTime) > 3000) { // スイッチが3秒間押されている場合
            Wifi_Setting();
            isPressed = false;
        }
    } else {
        isPressed = false;
    }
  server.handleClient();
}

void Wifi_Setting() {
  Serial.println("Wifi Setting Mode");
  initWifi();
}