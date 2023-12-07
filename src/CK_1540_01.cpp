/**
 * @file   CK_1540_01.cpp
 * @brief  このファイルはCK-1540-01基板の設定ライブラリです。
 * 
 * @author Iefuji Kohei (iefuji.kohei@kyokko.co.jp)
 * @date 2023-10-10
 * @copyright Copyright (c) 2023 旭光電機株式会社
 * @{
 */

#include <Arduino.h>
#include "CK_1540_01.h"

/**************************************************************************************************/
void initGPIO(){
  pinMode(PORT_INP_SW, INPUT);          //操作スイッチ
  pinMode(PORT_INP_INT1, INPUT);        //INT1(BG770)
  pinMode(PORT_INP_INT2, INPUT);        //INT2(CC1310)
  pinMode(PORT_OUT_LANLED_G, OUTPUT);   //LAN用LED緑
  pinMode(PORT_OUT_LANLED_R, OUTPUT);   //LAN用LED赤
  pinMode(PORT_OUT_PWRLED_R, OUTPUT);   //電源用LED赤
  pinMode(PORT_OUT_WANLED_G, OUTPUT);   //WAN用LED緑
  pinMode(PORT_OUT_WANLED_R, OUTPUT);   //WAN用LED赤
  pinMode(PORT_OUT_WANLED_B, OUTPUT);   //WAN用LED青
  pinMode(PORT_OUT_BUZZ, OUTPUT);       //ブザー
  pinMode(PORT_OUT_MODULE_RESET, OUTPUT);   //通信モジュールリセット
  /* 各種LED消灯 */
  digitalWrite(PORT_OUT_LANLED_G,HIGH);
  digitalWrite(PORT_OUT_LANLED_R,HIGH);
  digitalWrite(PORT_OUT_PWRLED_R,HIGH);
  digitalWrite(PORT_OUT_WANLED_G,HIGH);
  digitalWrite(PORT_OUT_WANLED_R,HIGH);
  digitalWrite(PORT_OUT_WANLED_B,HIGH);
}

void BG770_RESET_ON(){ digitalWrite(PORT_OUT_MODULE_RESET,LOW); };
void BG770_RESET_OFF(){ digitalWrite(PORT_OUT_MODULE_RESET,HIGH); };
void PWR_RED_ON(){ digitalWrite(PORT_OUT_PWRLED_R,LOW); }
void PWR_RED_OFF(){ digitalWrite(PORT_OUT_PWRLED_R,HIGH); }
void LAN_GREEN_ON(){ digitalWrite(PORT_OUT_LANLED_G,LOW); }
void LAN_GREEN_OFF(){ digitalWrite(PORT_OUT_LANLED_G,HIGH); }
void LAN_RED_ON(){ digitalWrite(PORT_OUT_LANLED_R,LOW); }
void LAN_RED_OFF(){ digitalWrite(PORT_OUT_LANLED_R,HIGH); }
void WAN_GREEN_ON(){ digitalWrite(PORT_OUT_WANLED_G,LOW); }
void WAN_GREEN_OFF(){ digitalWrite(PORT_OUT_WANLED_G,HIGH); }
void WAN_RED_ON(){ digitalWrite(PORT_OUT_WANLED_R,LOW); }
void WAN_RED_OFF(){ digitalWrite(PORT_OUT_WANLED_R,HIGH); }

void LAN_RED_FLA(uint16_t time, uint16_t msec){
  for(uint16_t i=0;i<time;i++){
    LAN_RED_ON();
    delay(msec);
    LAN_RED_OFF();
    delay(msec);
  }
}
/**************************************************************************************************/
/** @} */