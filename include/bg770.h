/**
 * @file bg770.h
 * @version 0.1
 * @brief 無線通信モジュール(bg770) API
 *
 * @author Iefuji Kohei (iefuji.kohei@kyokko.co.jp)
 * @date 2023-10-10
 * @copyright Copyright (c) 2023 旭光電機株式会社
 */
#ifndef BG770_H
#define BG770_H
/**************************************************************************************************
 * INCLUDES
 */
#include <stdbool.h>
#include <stdint.h>
#include "setup_define.h"

/**************************************************************************************************
 * TYPEDEFS
 */
/** @brief API 戻り値の型 */
typedef enum e_api_status
{
  /** @brief 成功 */
  API_STATUS_SUCCESS = 0,
  /** @brief 失敗 */
  API_STATUS_FAIL,
  /** @brief タイムアウト */
  API_STATUS_TIMEOUT,
  /** @brief 処理中 */
  API_STATUS_IN_PROGRESS,
  /** @brief ペイロード送信要求 */
  API_STATUS_REQUEST_TO_SEND_PAYLOAD,
  /** @brief COPSエラー */
  API_STATUS_COPS_ERROR,
  /** @brief サブスクライブ中 */
  API_STATUS_SUBSCRIBE,
} api_status_t;

/** @brief BG770状態の型 */
typedef enum e_bg770_states
{
  /** @brief BG770初期化前 */
  BG770_STATE_NO_OPEN = 0,
  /** @brief BG770初期化シーケンス中*/
  BG770_STATE_INIT_COMMAND_SEQUENCE,
  /** @brief BG770 パブリッシュ中 */
  BG770_STATE_PUBLISH,
  /** @brief BG770 エラー */
  BG770_STATE_ERROR,
  /** @brief BG770 サブスクライブ中 */
  BG770_STATE_SUBSCRIBE,
  /** @brief BG770 操作完了待機中 */
  BG770_STATE_OPERATION_WAIT,
} bg770_states_t;

/** @brief コマンド実行構造体の型 */
typedef struct st_command_executor
{
  /** @brief コマンド文字列作成関数 */
  const char *(*create_command_func)(void);
  /** @brief コマンド返答関数 */
  api_status_t (*validate_response_func)(const char *, uint16_t);
  /** @brief タイムアウト */
  uint32_t timeout;
  /** @brief コマンド間のディレイ */
  uint32_t command_delay;
} command_executor_t;


/**************************************************************************************************
 * GLOBAL FUNCTIONS
 */
/**
 * @brief 無線通信モジュール初期化関数
 */
void bg770_init(void);

/**
 * @brief 初期化シーケンスタスク関数
 * @return BG770状態
 */
api_status_t init_command_sequence_task(void);
/**
 * @brief コマンド実行関数
 * @param[in] p_executor :コマンド実行ポインタ
 * @return BG770状態
 */
api_status_t execute(const command_executor_t *p_executor);
/**
 * @brief 受信データ取得関数
 * @return 受信データ文字列
 */
String bg770_RxDataGet();
/**
 * @brief 文字列分割関数（ペイロード部分取得用）
 * @param[in] data:元データ
 * @param[in] delimiter:分割する基準の文字列（「,」）
 * @param[out] dst:分割後の文字列
 * @param[in] dst_index：必要な文字列迄の「delimiter」の数
 */
void split(String data, char delimiter, String *dst, uint8_t dst_index);
/**
 * @brief 受信データ（JSON）を解析する関数
 * @param[in] RxData:解析受信データ文字列
 * @return：サブスクライブペイロード文字列
 */
String RxData_Analize(String RxData);
/**
 * @brief BG770のリセット関数
 * @return：API_STATUS_IN_PROGRESS（初期化コマンドシーケンスに戻す）
 */
api_status_t bg770_reset(void);
/**
 * @brief ペイロード送信
 * @param payload:送信するペイロード
 * @param length：送信ペイロード長
 * @return API_STATUS_SUCCESSのみ
 */
api_status_t bg770_send_payload(const uint8_t payload[], uint16_t length);
/**
 * @brief IMSI 取得関数
 */
void bg770_get_imsi(char imsi[16]);
/**
 * @brief RSSI 取得関数
 *
 * @return RSSI
 */
int16_t bg770_get_rssi(void);
/**
 * @brief コマンド文字列作成関数
 * @return コマンド文字列
 */
/** @brief BG770初期設定コマンド **/
const char *create_command_bg770_setup(void);
/** @brief SIM確認コマンド **/
const char *create_command_cpin(void);
/** @brief IMSI取得コマンド **/
const char *create_command_cimi(void);
/** @brief パケットデータプロトコル（PDP)設定コマンド **/
const char *create_command_cgdcont(void);
/** @brief 基地局確認コマンド **/
const char *create_command_cops(void);
/** @brief RSSI取得コマンド **/
const char *create_command_csq(void);
/** @brief APN/Username/Password設定コマンド **/
const char *create_command_qicsgp(void);
/** @brief PDPのアクティベートコマンド **/
const char *create_command_qiact(void);
/** @brief PDPのデアクティベートコマンド **/
const char *create_command_qideact(void);
/** @brief ソケットオープンコマンド **/
const char *create_command_qiopen(void);
/** @brief BG770 パワーダウンコマンド **/
const char *create_command_qpowd(void);
/** @brief BG770 MQTTサーバーオープンコマンド **/
const char *create_command_qmtopen(void);
/** @brief BG770 サブスクライブコマンド **/
const char *create_command_qmtsub(void);
/** @brief BG770 MQTTサーバー接続コマンド **/
const char *create_command_qmtconn(void);
/** @brief BG770 サブスクライブ中止コマンド **/
const char *create_command_qmtuns(void);
/** @brief BG770 パブリッシュコマンド **/
const char *create_command_qmtpub(void);
/** @brief BG770 NTPサーバー接続コマンド **/
const char *create_command_qntp(void);

/**
 * @brief コマンド返答確認関数
 * @param[in] content：BG770からの返答文字列
 * @param[in] times:BG770返答回数
 * @return API_STATUS_FAIL：コマンド実行失敗
 *         API_STATUS_SUCCESS：コマンド実行成功
 *         API_STATUS_PROGRESS：コマンド継続中
 */
/* 標準返答確認（「0」が返ってくるだけ） */
api_status_t validate_response_ok(const char *content, uint16_t times);
/** @brief BG770の準備完了確認 */
api_status_t validate_response_ready(const char *content, uint16_t times);
/** @brief BG770初期設定完了確認 */
api_status_t validate_response_bg770_setup(const char *content, uint16_t times);
/** @brief SIM確認完了確認 */
api_status_t validate_response_cpin(const char *content, uint16_t times);
/** @brief IMSI取得完了確認 */
api_status_t validate_response_cimi(const char *content, uint16_t times);
/** @brief RSSI取得確認 */
api_status_t validate_response_csq(const char *content, uint16_t times);
/** @brief ソケットオープン完了確認 */
api_status_t validate_response_qiopen(const char *content, uint16_t times);
/** @brief パワーダウン完了確認 */
api_status_t validate_response_qpowd(const char *content, uint16_t times);
/** @brief 基地局接続完了確認 */
api_status_t validate_response_cops(const char *content, uint16_t times);
/** @brief MQTTサーバーオープン完了確認 */
api_status_t validate_response_qmtopen(const char *content, uint16_t times);
/** @brief MQTTサーバー接続確認 */
api_status_t validate_response_qmtconn(const char *content, uint16_t times);
/** @brief サブスクライブ完了確認 */
api_status_t validate_response_qmtsub(const char *content, uint16_t times);
/** @brief サブスクライブ中止完了確認 */
api_status_t validate_response_qmtuns(const char *content, uint16_t times);
/** @brief パブリッシュ完了確認 */
api_status_t validate_response_qmtpub(const char *content, uint16_t times);
/** @brief NTPサーバー接続完了確認 */
api_status_t validate_response_qntp(const char *content, uint16_t times);

/**************************************************************************************************
 * GLOBAL VARIABLES
 */
/** @brief サブスクライブ中止実行コマンド */
const command_executor_t unsubscribe_command = {create_command_qmtuns, validate_response_qmtuns,  180000, 0};
/** @brief パワーオフ実行コマンド */
const command_executor_t poweroff_command =    {create_command_qpowd,  validate_response_qpowd,   180000, 0};
/** @brief パブリッシュ実行コマンド */
const command_executor_t publish_command =     {create_command_qmtpub, validate_response_qmtpub,  180000, 0};
/** @brief サブスクライブ実行コマンド */
const command_executor_t subscribe_command =   {create_command_qmtsub, validate_response_qmtsub,  180000, 0};
/** @brief NTPサーバー接続実行コマンド */
const command_executor_t ntp_command =     {create_command_qntp, validate_response_qntp,  180000, 0};
/** @brief MQTTサーバーオープン実行コマンド */
const command_executor_t qmtopen_command = {create_command_qmtopen, validate_response_qmtopen,  180000, 0};
/** @brief PDPアクティブ実行コマンド */
const command_executor_t qiact_command =   {create_command_qiact, validate_response_ok,  150000, 0};
/** @brief PDPデアクティブ実行コマンド */
const command_executor_t qideact_command = {create_command_qideact, validate_response_ok,  40000, 0};
/** @brief MQTT接続実行コマンド */
const command_executor_t qmtconn_command = {create_command_qmtconn, validate_response_qmtconn,  180000, 0};
/** @brief サブスクライブ実行コマンド */
const command_executor_t qmtsub_command = {create_command_qmtsub, validate_response_qmtsub,  180000, 0};
/** @brief RSSI */
extern int16_t rssi;
/** @brief IMSI */
extern char imsi[16];
/** @brief パブリッシュペイロード */
extern uint8_t Publish_payload[PUBLISH_SIZE];
/** @brief パブリッシュペイロード長 */
extern uint16_t Publish_length;
/** @brief BG770 の状態 */
extern bg770_states_t bg_state;

#endif
