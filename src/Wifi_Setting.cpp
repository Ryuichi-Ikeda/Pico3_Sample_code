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

void handleRoot(void);
void handleWifi(void);
void handleLed(void);
void handleRedLedOn(void);
void handleGreenLedOn(void);
void handleLedOff(void);
void sendErrorPage(String);
bool lan_red_state = false;
bool lan_green_state = false;

void initWifi(){
  // アクセスポイントとしてESP32を設定
  WiFi.softAP("Pico3_AP_Sample", "Photo036F");

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/", handleRoot); // ルートパスにアクセスがあったときのハンドラを設定
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
  if (server.method() == HTTP_POST) { // リクエストがPOSTの場合
    String new_ssid = server.arg("ssid"); // クライアントから送信されたSSIDを取得します
    String new_password = server.arg("password"); // クライアントから送信されたパスワードを取得します

    if (new_ssid != "" && new_password != "") { // 新しいSSIDとパスワードが設定されている場合
      WiFi.begin(new_ssid.c_str(), new_password.c_str()); // 新しいSSIDとパスワードでWiFiに接続します
      int attempts = 0;
      while (WiFi.status() != WL_CONNECTED && attempts < 20) { 
        delay(1000); 
        Serial.println("Connecting to WiFi..."); 
        attempts++;
      }
      if(WiFi.status() == WL_CONNECTED) { // WiFiに接続した場合
        Serial.println("Connected to the WiFi network");
      } else { // WiFiに接続できなかった場合
        sendErrorPage("Authentication failed. Please check your credentials."); // エラーメッセージを含めてエラーページを送信
        Serial.println("Failed to connect to the WiFi network"); 
        return;
      }
    }
  }
  String html = "<!DOCTYPE html><html><body><form method=\"post\">"; // HTMLフォームを作成します
  html += "SSID:<br><select name=\"ssid\">"; // SSIDを選択するためのセレクトボックスを作成します
  int n = WiFi.scanNetworks(); // 利用可能なネットワークをスキャンします
  std::vector<String> unique_ssids; // 一意のSSIDを保存するベクターを作成
  for (int i = 0; i < n; ++i) { 
    String ssid = WiFi.SSID(i);
    if (std::find(unique_ssids.begin(), unique_ssids.end(), ssid) == unique_ssids.end()) { // このSSIDがまだ見つかっていない場合
      unique_ssids.push_back(ssid); // ベクターにSSIDを追加
      html += "<option value=\"" + ssid + "\">" + ssid + "</option>"; 
    }
  }

  html += "</select><br>"; // セレクトボックスの終了タグを追加します
  html += "Password:<br><input type=\"text\" name=\"password\"><br><br>"; // パスワードを入力するためのテキストボックスを作成します
  html += "<input type=\"submit\" value=\"Submit\">"; // 送信ボタンを作成します
  html += "</form></body></html>"; // フォームとHTMLの終了タグを追加します

  server.send(200, "text/html; charset=UTF-8", html); // HTTPレスポンスとしてHTMLフォームを送信します
  
}
void handleLed() {
  String html = "<html><body>";
  html += "<h1>LED点灯ページ</h1>";
  html += "<p><a href=\"/red_led_on\"><button>RED_LED_ON</button></a></p>";
  html += "<p><a href=\"/green_led_on\"><button>GREEN_LED_ON</button></a></p>";
  html += "<p><a href=\"/led_off\"><button>LED_OFF</button></a></p>";
  html += "</body></html>";
  server.send(200, "text/html; charset=UTF-8", html);
}
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
void sendErrorPage(String message) {
  String errorHtml = "<!DOCTYPE html><html><body><h2>Error</h2><p>" + message + "</p></body></html>";
  server.send(400, "text/html; charset=UTF-8", errorHtml);
}