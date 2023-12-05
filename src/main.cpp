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

/***************************************************************************************************
 * LOCAL FUNCTIONS
 */
/**
 * @brief パブリッシュペイロードの作成関数
 * @param[in/out] buf:作成したペイロードを入力するバッファ
 * @return 作成したペイロード長
 */

WebServer server(80);

/**  Main setup **/
void setup() {
  /* GPIOの初期化 */
  initGPIO();
  Serial.println("complete initGPIO");
  /* シリアル通信の初期化（デバッグ用） */
  Serial.begin(115200);
  while (!Serial); 
  Serial.println("Starting Serial Monitor");
  Serial.println("Please fill in the command");
}
/**  Main loop **/
void loop() {
  static unsigned long pressedTime = 0;
  static bool isPressed = false;

  String input;

  if (Serial.available()) {
    input = Serial.readStringUntil('\n');
    String command = input.substring(0, 3);
    String color = input.substring(3);
    if (command.equals("000")) {
      if (color.equals("RED")) {
        LAN_RED_ON();
      } else if (color.equals("GREEN")) {
        LAN_GREEN_ON();
      }
      else{
        LAN_RED_OFF();
        LAN_GREEN_OFF();
      }
    }
    else if (command.equals("001")) {
      if (color.equals("RED")) {
        WAN_RED_ON();
      } else if (color.equals("GREEN")) {
        WAN_GREEN_ON();
      }
      else{
        WAN_RED_OFF();
        WAN_GREEN_OFF();
      }
    }
    else if (command.equals("002")) {
      Serial.println("002 " + String(digitalRead(PORT_INP_SW)));
    }
  }
  if (digitalRead(PORT_INP_SW) == LOW) { 
        if (!isPressed) { 
            isPressed = true;
            pressedTime = millis();
        } else if ((millis() - pressedTime) > 3000) { 
            initWifi();
            isPressed = false;
        }
    } else {
        isPressed = false;
    }
  server.handleClient();
}