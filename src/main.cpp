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
#include "ArduinoJson.h"

/***************************************************************************************************
 * LOCAL FUNCTIONS
 */
/**
 * @brief パブリッシュペイロードの作成関数
 * @param[in/out] buf:作成したペイロードを入力するバッファ
 * @return 作成したペイロード長
 */

WebServer server(80);
uint16_t publish_payload_build(char buf[],String command);

/**  Main setup **/
void setup() {
  /* GPIOの初期化 */
  initGPIO();
  /* シリアル通信の初期化（デバッグ用） */
  Serial.begin(115200);
  while (!Serial); 
  Serial.println("Starting Serial Monitor");

  bg770_init();
}
/**  Main loop **/
void loop() {
  static unsigned long pressedTime = 0;
  static bool isPressed = false;
  String jsonString;
  String command;
  String color;
  String SW;
  StaticJsonDocument<200> doc;

  if(bg_state == BG770_STATE_INIT_COMMAND_SEQUENCE){
   while(bg_state != BG770_STATE_SUBSCRIBE){
     if(init_command_sequence_task() == API_STATUS_FAIL){ bg770_reset(); };
     delay(1);
   }
   Serial.println("Subscribe Start");
  }

   if (command.length() == 0) {
    Serial.println("Please enter a command");
    while (!Serial.available()) {
      delay(100); // データが利用可能になるまで待機
    }
    command = Serial.readStringUntil('\n');
    doc["command"] = command; // JSON形式のcommandに格納
  }
    if (command == "002") {
      if(digitalRead(PORT_INP_SW) == 0){
        doc["command"] = command;
        doc["SW"] = "ON";
      }
      if(digitalRead(PORT_INP_SW) == 1){
        doc["command"] = command;
        doc["SW"] = "OFF";
      }
      serializeJson(doc,jsonString);
      serializeJson(doc,Serial);
      Publish_length = publish_payload_build((char *)Publish_payload,jsonString);
      if(execute(&unsubscribe_command) == API_STATUS_FAIL){ bg770_reset(); }
      if(execute(&publish_command) == API_STATUS_FAIL){ bg770_reset(); }

      command = ""; // コマンドをリセットして、次の入力を待つ
      return;
    }

  if (color.length() == 0) {
    Serial.println("Please enter a color");
    while (!Serial.available()) {
      delay(100); // データが利用可能になるまで待機
    }
    color = Serial.readStringUntil('\n');
    doc["color"] = color; // JSON形式のcolorに格納
  }
  // コマンドと色が両方とも入力されたら、判別を行う
  serializeJson(doc, jsonString);
  serializeJson(doc,Serial);
  if (command.length() > 0 && color.length() > 0) {
    Publish_length = publish_payload_build((char *)Publish_payload,jsonString);
    if(execute(&unsubscribe_command) == API_STATUS_FAIL){ bg770_reset(); }
    if(execute(&publish_command) == API_STATUS_FAIL){ bg770_reset(); }

    if (command.equals("000")) {
      if (color.equals("RED")) {
        LAN_RED_ON();
        LAN_GREEN_OFF();
      } else if (color.equals("GREEN")) {
        LAN_GREEN_ON();
        LAN_RED_OFF();
      }
      else{
        LAN_RED_OFF();
        LAN_GREEN_OFF();
      }
    }
    else if (command.equals("001")) {
      if (color.equals("RED")) {
        WAN_RED_ON();
        WAN_GREEN_OFF();
      } else if (color.equals("GREEN")) {
        WAN_GREEN_ON();
        WAN_RED_OFF();
      }
      else{
        WAN_RED_OFF();
        WAN_GREEN_OFF();
      }
    }
    // 判別が終わったら、コマンドと色をリセット
    command = "";
    color = "";
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

uint16_t publish_payload_build(char* buf,String jsonString)
{
  uint16_t len = 0;
  /* message */
  len += sprintf(buf,"%s",jsonString.c_str());
  
  return len;
}