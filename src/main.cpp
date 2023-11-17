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

/***************************************************************************************************
 * LOCAL FUNCTIONS
 */
/**
 * @brief パブリッシュペイロードの作成関数
 * @param[in/out] buf:作成したペイロードを入力するバッファ
 * @return 作成したペイロード長
 */
uint16_t publish_payload_build(char buf[]);

/**  Main setup **/
void setup() {
  /* GPIOの初期化 */
  initGPIO();

  /* シリアル通信の初期化（デバッグ用） */
  Serial.begin(115200);
  while (!Serial); 
  Serial.println("Starting Serial Monitor");

  /* bg770の初期化 */
  bg770_init();

}
/**  Main loop **/
void loop() {

  /* BG770が初期化コマンドシーケンス中なら、サブスクライブ状態になるまで、ループ */
  if(bg_state == BG770_STATE_INIT_COMMAND_SEQUENCE){
    while(bg_state != BG770_STATE_SUBSCRIBE){
      if(init_command_sequence_task() == API_STATUS_FAIL){ bg770_reset(); };
      delay(1);
    }
    /* LED消灯 */
    LAN_GREEN_OFF();
    LAN_RED_OFF();
    Serial.println("Subscribe Start");
  }

  /* bg770からの受信待ち */
  if(Serial1.available()) {
    
    /* データ受信 */
    String RxData = bg770_RxDataGet();
    
    /* NULLは無視 */
    if(RxData != ""){
      /* サブスクライブした場合のBG770のレスポンス */
      if(RxData.startsWith("+QMTRECV: 0")){
        
        /* 受信データの解析 */
        String Subscribe_payload = RxData_Analize(RxData);
        Serial.println("Subscribe Payload[" + String(Subscribe_payload) + "]");

        /*AWSからのデータが「RED」なら、LANの赤LEDを光らす。*/
        if (Subscribe_payload.equals("RED")){
          LAN_RED_ON();
          LAN_GREEN_OFF();
        }
       /*AWSからのデータが「GREEN」なら、LANの緑LEDを光らす。*/
        if (Subscribe_payload.equals("GREEN")){
          LAN_RED_OFF();
          LAN_GREEN_ON();
        }
      /*AWSからのデータが「RED」「GREEN」以外なら、LANのLEDを消す*/
        if (!Subscribe_payload.equals("RED") && !Subscribe_payload.equals("GREEN")){
          LAN_RED_OFF();
          LAN_GREEN_OFF();
        }            
    
        /* サブスクライブの中止 */
        if(execute(&unsubscribe_command) == API_STATUS_FAIL){ bg770_reset(); }

        /* サブスクライブ状態へ戻す */
        if(execute(&subscribe_command) == API_STATUS_FAIL){ bg770_reset(); }

      } else {
        /* サブスクライブデータ以外はエラーとして、リセット */
        bg770_reset();
      }                                        
    }

  }

}

/*************************************************************************************************/
uint16_t publish_payload_build(char buf[])
{
  uint16_t len = 0;
  /* JSON open */
  buf[len++] = '{';

  /* message */
  len += sprintf(&buf[len], "\"message\":\"Pico3からの挨拶\"");

  /* JSON close */
  buf[len++] = '}';

  return len;
}
