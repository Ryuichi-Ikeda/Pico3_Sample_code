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

  server.begin();

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

  WiFiClient client = server.available();   // クライアントが接続を待つ

  if (client) { // クライアントが接続した場合
    String currentLine = ""; // 現在の行を保存するための文字列
    String new_ssid = ""; // 新しいSSIDを保存するための文字列
    String new_password = ""; // 新しいパスワードを保存するための文字列

    while (client.connected()) { // クライアントが接続中の間ループ
      if (client.available()) { // クライアントからデータが利用可能な場合
        char c = client.read(); // クライアントから1文字読み込む
        if (c == '\n') { // 改行文字が読み込まれた場合
          if (currentLine.length() == 0) { // 現在の行が空の場合（HTTPリクエストの終わり）
            client.println("HTTP/1.1 200 OK"); // HTTPレスポンスのステータスラインを送信
            client.println("Content-type:text/html"); // HTTPレスポンスのヘッダーを送信
            client.println(); // 空行を送信してヘッダーの終わりを示す
            Serial.println(currentLine);
            Serial.println(new_ssid);
            Serial.println(new_password);

            client.println("<!DOCTYPE html><html>"); // HTMLの開始タグを送信
            client.println("<body><form method=\"post\">"); // フォームの開始タグを送信
            client.println("SSID:<br><select name=\"ssid\">"); // SSIDを選択するためのセレクトボックスを作成
            Serial.println(currentLine);
            Serial.println(new_ssid);
            Serial.println(new_password);

            int n = WiFi.scanNetworks(); // 利用可能なネットワークをスキャン
            for (int i = 0; i < n; ++i) { // 各ネットワークに対して
              client.println("<option value=\"" + WiFi.SSID(i) + "\">" + WiFi.SSID(i) + "</option>"); // オプションタグを送信
            }

            client.println("</select><br>"); // セレクトボックスの終了タグを送信
            client.println("Password:<br><input type=\"text\" name=\"password\"><br><br>"); // パスワードを入力するためのテキストボックスを作成
            client.println("<input type=\"submit\" value=\"Submit\">"); // 送信ボタンを作成
            client.println("</form></body></html>"); // フォームとHTMLの終了タグを送信
            client.println(); // 空行を送信してレスポンスの終わりを示す
            
            Serial.println(currentLine);
            Serial.println(new_ssid);
            Serial.println(new_password);
            break; // whileループを抜ける
          } else { // 現在の行が空でない場合（HTTPリクエストの本文を処理）
            currentLine = currentLine.substring(currentLine.indexOf(':') + 1); // コロンの後の文字列を取得
            if (currentLine.startsWith("ssid=")) { // 現在の行が"ssid="で始まる場合
              new_ssid = currentLine.substring(5); // "ssid="の後の文字列を新しいSSIDとして保存
            } else if (currentLine.startsWith("password=")) { // 現在の行が"password="で始まる場合
              new_password = currentLine.substring(9); // "password="の後の文字列を新しいパスワードとして保存
            }
            currentLine = ""; // 現在の行を空にする
            
            Serial.println(currentLine);
            Serial.println(new_ssid);
            Serial.println(new_password);
          }
        } else if (c != '\r') { // 改行文字でない場合
          currentLine += c; // 現在の行に読み込んだ文字を追加
        }
      }
    }

    client.stop(); // クライアントとの接続を閉じる
    
            Serial.println(currentLine);
            Serial.println(new_ssid);
            Serial.println(new_password);
    Serial.println("client stop");

    if (new_ssid != "" && new_password != "") { // 新しいSSIDとパスワードが設定されている場合
      WiFi.begin(new_ssid.c_str(), new_password.c_str()); // 新しいSSIDとパスワードでWiFiに接続
      while (WiFi.status() != WL_CONNECTED) { // WiFiに接続するまで待機
        delay(500); // 0.5秒待つ
        Serial.println("Connecting to WiFi..."); // "Connecting to WiFi..."とシリアルに出力
      }
      if(WiFi.status() == WL_CONNECTED) { // WiFiに接続した場合
        Serial.println("Connected to the WiFi network"); // "Connected to the WiFi network"とシリアルに出力
      } else { // WiFiに接続できなかった場合
        Serial.println("Failed to connect to the WiFi network"); // "Failed to connect to the WiFi network"とシリアルに出力
      }
    }
  }
}