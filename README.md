# PICO_SAMPLE_CODE

## 1.概要
「Pico3 TypeC」を使用して、LTE通信を行い、SORACOMプラットフォーム（SORACOM BEAM）経由で
「AWS IoT Core」へと接続する

## ソフトウェア構成
ESP32開発環境

|     contents     |         value           |
|:-----------------|:------------------------|
| Selected Board   | ESP32 Dev Module        |
| PSRAM            | Disabled                |
| Partition Scheme | default_16MB.csv        |
| CPU Frequency    | 80MHz                   |
| Flash Mode       | QIO                     |
| Flash Frequency  | 40MHz                   |
| Flash Size       | 4MB                     |
| Upload Speed     | 921600                  |
| BG770Task Runs On| Core 0                  |
| NETTask Run On   | Core 1                  |
| Core Debug Level | None                    |

## 使用ライブラリ
PlatformIOから以下をインストールする
- bblanchon/ArduinoJson@^6.21.3

