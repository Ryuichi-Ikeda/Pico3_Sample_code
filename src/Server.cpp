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

bool lan_red_state = false;
bool lan_green_state = false;

void initWifi(){
  /*アクセスポイントとしてESP32を設定*/
  WiFi.softAP("Pico3_AP_Sample", "Photo036F");

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  /*ルートパスにアクセスがあったときのハンドラを設定*/
  server.on("/", handleRoot); 
  server.on("/wifi", handleWifi);
  server.on("/led",handleLed);
  server.on("/red_led_on", handleRedLedOn);
  server.on("/green_led_on", handleGreenLedOn);
  server.on("/led_off", handleLedOff);

  server.begin();
  Serial.println("Server bigin");
}
void handleRoot() {
  String html = "<html><body>";
  html += "<p><a href=\"/wifi\"><button>WiFi設定</button></a></p>";
  html += "<p><a href=\"/led\"><button>LED点灯</button></a></p>";
  html += "</body></html>";
  server.send(200, "text/html; charset=UTF-8", html);
}

void handleWifi(){
  /*リクエストがPOSTの場合*/
  if (server.method() == HTTP_POST) { 
    /*クライアントから送信されたSSIDとパスワードを取得*/
    String new_ssid = server.arg("ssid"); 
    String new_password = server.arg("password"); 
    
    /*新しいSSIDとパスワードが設定されている場合*/
    if (new_ssid != "" && new_password != "") { 
      /*新しいSSIDとパスワードでWiFiに接続*/
      WiFi.begin(new_ssid.c_str(), new_password.c_str()); 
      int attempts = 0;
      while (WiFi.status() != WL_CONNECTED && attempts < 20) { 
        delay(1000); 
        Serial.println("Connecting to WiFi..."); 
        attempts++;
      }
      if(WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected to the WiFi network");
      } else {
        /*エラーメッセージを含めてエラーページを送信*/
        sendErrorPage("Authentication failed. Please check your credentials."); 
        Serial.println("Failed to connect to the WiFi network"); 
        return;
      }
    }
  }
  /*HTMLフォームを作成*/
  String html = "<!DOCTYPE html><html><body><form method=\"post\">"; 
  /*SSIDを選択するためのセレクトボックスを作成*/
  html += "SSID:<br><select name=\"ssid\">"; 
  /*利用可能なネットワークをスキャン*/
  int n = WiFi.scanNetworks(); 
  /*SSIDを保存するベクターを作成*/
  std::vector<String> unique_ssids;
  for (int i = 0; i < n; ++i) { 
    String ssid = WiFi.SSID(i);
    /*重複しないSSIDのみ追加*/
    if (std::find(unique_ssids.begin(), unique_ssids.end(), ssid) == unique_ssids.end()) { 
      unique_ssids.push_back(ssid); 
      html += "<option value=\"" + ssid + "\">" + ssid + "</option>"; 
    }
  }

  /*セレクトボックスの終了タグを追加*/
  html += "</select><br>"; 
  /*パスワードを入力するためのテキストボックスを作成*/
  html += "Password:<br><input type=\"text\" name=\"password\"><br><br>"; 
  /*送信ボタンを作成*/
  html += "<input type=\"submit\" value=\"Submit\">"; 
  /*トップページに戻るボタンを作成*/
  html += "<p><a href=\"/\"><button>Return to Top Page</button></a></p>";
  /*フォームとHTMLの終了タグを追加*/
  html += "</form></body></html>"; 

  /*HTTPレスポンスとしてHTMLフォームを送信*/
  server.send(200, "text/html; charset=UTF-8", html); 
  
}
/*LED点灯ボタンが押された場合の処理*/
void handleLed() {
  String html = "<html><body>";
  html += "<h1>LED点灯ページ</h1>";
  html += "<p><a href=\"/red_led_on\"><button>RED_LED_ON</button></a></p>";
  html += "<p><a href=\"/green_led_on\"><button>GREEN_LED_ON</button></a></p>";
  html += "<p><a href=\"/led_off\"><button>LED_OFF</button></a></p>";
  html += "<p><a href=\"/\"><button>Return to Top Page</button></a></p>";
  html += "</body></html>";
  server.send(200, "text/html; charset=UTF-8", html);
}
/*LAN_RED_ONが押された場合の処理*/
void handleRedLedOn() {
  LAN_RED_ON();
  lan_red_state = true;
  if (lan_green_state) {
    LAN_GREEN_OFF();
    lan_green_state = false;
  }
  server.sendHeader("Location", "/led");
  server.send(303);
}
/*LAN_RED_OFFが押された場合の処理*/
void handleGreenLedOn() {
  LAN_GREEN_ON();
  lan_green_state = true;
  if (lan_red_state) {
    LAN_RED_OFF();
    lan_red_state = false;
  }
  server.sendHeader("Location", "/led");
  server.send(303);
}
/*LAN_OFFが押された場合の処理*/
void handleLedOff() {
  if (lan_red_state) {
    LAN_RED_OFF();
    lan_red_state = false;
  }
  if (lan_green_state) {
    LAN_GREEN_OFF();
    lan_green_state = false;
  }
  server.sendHeader("Location", "/led");
  server.send(303);
}
/*エラーページ送信*/
void sendErrorPage(String message) {
  String errorHtml = "<!DOCTYPE html><html><body><h2>Error</h2><p>" + message + "</p></body></html>";
  server.send(400, "text/html; charset=UTF-8", errorHtml);
}