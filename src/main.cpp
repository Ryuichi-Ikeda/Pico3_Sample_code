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
uint16_t publish_payload_build(char buf[]);
WebServer server(80);

void handleRoot(void);

/**  Main setup **/
void setup() {
  /* GPIOの初期化 */
  initGPIO();

  /* シリアル通信の初期化（デバッグ用） */
  Serial.begin(115200);
  while (!Serial); 
  Serial.println("Starting Serial Monitor");

  // アクセスポイントとしてESP32を設定
  WiFi.softAP("Pico3_AP_Sample", "Photo036F");

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/", handleRoot); // ルートパスにアクセスがあったときのハンドラを設定
  server.begin();
  Serial.println("Server bigin");
}
/**  Main loop **/
void loop() {
  Serial.println("a");
  delay(1000);
  server.handleClient();
}

void handleRoot() {
  Serial.println("in the handleRoot");
  if (server.method() == HTTP_POST) { // リクエストがPOSTの場合
    String new_ssid = server.arg("ssid"); // クライアントから送信されたSSIDを取得します
    String new_password = server.arg("password"); // クライアントから送信されたパスワードを取得します

    if (new_ssid != "" && new_password != "") { // 新しいSSIDとパスワードが設定されている場合
      WiFi.begin(new_ssid.c_str(), new_password.c_str()); // 新しいSSIDとパスワードでWiFiに接続します
      while (WiFi.status() != WL_CONNECTED) { // WiFiに接続するまで待機します
        delay(500); // 0.5秒待ちます
        Serial.println("Connecting to WiFi..."); // "Connecting to WiFi..."とシリアルに出力します
      }
      if(WiFi.status() == WL_CONNECTED) { // WiFiに接続した場合
        Serial.println("Connected to the WiFi network"); // "Connected to the WiFi network"とシリアルに出力します
      } else { // WiFiに接続できなかった場合
        Serial.println("Failed to connect to the WiFi network"); // "Failed to connect to the WiFi network"とシリアルに出力します
      }
    }
  }

  String html = "<!DOCTYPE html><html><body><form method=\"post\">"; // HTMLフォームを作成します
  html += "SSID:<br><select name=\"ssid\">"; // SSIDを選択するためのセレクトボックスを作成します

  int n = WiFi.scanNetworks(); // 利用可能なネットワークをスキャンします
  for (int i = 0; i < n; ++i) { // 各ネットワークに対して
    html += "<option value=\"" + WiFi.SSID(i) + "\">" + WiFi.SSID(i) + "</option>"; // オプションタグを作成します
  }

  html += "</select><br>"; // セレクトボックスの終了タグを追加します
  html += "Password:<br><input type=\"text\" name=\"password\"><br><br>"; // パスワードを入力するためのテキストボックスを作成します
  html += "<input type=\"submit\" value=\"Submit\">"; // 送信ボタンを作成します
  html += "</form></body></html>"; // フォームとHTMLの終了タグを追加します

  server.send(200, "text/html", html); // HTTPレスポンスとしてHTMLフォームを送信します
  
  Serial.println("out the handleRoot");
}