/**
 * @file BG770.c
 * @version 0.1
 * @brief 無線通信モジュール(BG770)制御コード
 *
 * @author Iefuji Kohei (iefuji.kohei@kyokko.co.jp)
 * @date 2023-10-10
 * @copyright Copyright (c) 2023 旭光電機株式会社
 */
/**************************************************************************************************
 * INCLUDES
 */
#include <stdlib.h>
#include <string.h>
#include "CK_1540_01.h"
#include "bg770.h"
#include "ArduinoJson.h"
#include "setup_define.h"

/**************************************************************************************************
 * CONSTANTS
 */
/** @brief BG770のサブスクライブペイロード位置（+QMTRECV: <client_idx>,<msg_id>,<topic>,<payload>） */
#define BG770_SUB_PAYLOAD_INDEX 4
/** @brief コマンドの最大サイズ */
#define COMMAND_SIZE 64

/**************************************************************************************************
 * TYPEDEFS
 */
/** @brief 基地局オペレータの型(eSIMの場合、IMSIの事業者コードと異なる) */
typedef enum e_operator_states
{
  /** @brief ソフトバンク（事業者コード：44020） */
  OPERATOR_SOFTBANK = 0,
  /** @brief NTTドコモ（事業者コード：44010） */
  OPERATOR_NTTDOCOMO,
} operator_states_t;

/**************************************************************************************************
 * LOCAL VARIABLES
 */
/** @brief ゼロ */
static const char *zero = "0";
/**
 * @brief 初期化コマンドシーケンス
 *
 * この順序でコマンドを実行していく
 */
static const command_executor_t init_command_sequence[] = {
    {NULL, validate_response_ready,  10000, 0},
    {create_command_bg770_setup, validate_response_bg770_setup,  300, 0},
    {create_command_cpin, validate_response_cpin,  300, 0},
    {create_command_cimi, validate_response_cimi,  300, 0},
    {create_command_cgdcont, validate_response_ok,  300, 0},
    {create_command_cops, validate_response_cops,  180000, 0},
    {create_command_csq, validate_response_csq,  1000, 5000},
    {create_command_qicsgp, validate_response_ok,  300, 0},
    {create_command_qiact, validate_response_ok,  150000, 0},
    {create_command_qiopen, validate_response_qiopen,  180000, 0},
    {create_command_qmtopen, validate_response_qmtopen,  180000, 0},
    {create_command_qmtconn, validate_response_qmtconn,  180000, 0},
    {create_command_qmtsub, validate_response_qmtsub,  180000, 0},
    {NULL, NULL, 0}, /* 番兵 */
};
/** @brief 実行しているコマンドのインデックス */
static uint16_t init_command_sequence_index;

/** @brief 基地局オペレータ情報 */
static operator_states_t saved_operator = OPERATOR_SOFTBANK;
/** @brief 基地局オペレータ接続失敗フラグ */
static uint8_t cops_err = false;

/**************************************************************************************************
 * GLOBAL VARIABLES
 */
/** @brief RSSI */
int16_t rssi;
/** @brief IMSI */
char imsi[16];
/** @brief パブリッシュペイロード */
uint8_t Publish_payload[PUBLISH_SIZE];
/** @brief パブリッシュペイロード長 */
uint16_t Publish_length;
/** @brief BG770 の状態 */
bg770_states_t bg_state;

/*************************************************************************************************/
void bg770_init(void)
{
  /* LED点灯 */
  LAN_RED_ON();
  
  /* シリアル設定 */
  Serial1.begin(115200, SERIAL_8N1, PORT_LTEUART_RXD, PORT_LTEUART_TXD);
  while (!Serial);  
  /* パワーオンシーケンス */
  BG770_RESET_ON();
  delay(750);
  BG770_RESET_OFF();

  /* 各変数の初期化 */
  init_command_sequence_index = 0;
  rssi = 99;
  bg_state = BG770_STATE_INIT_COMMAND_SEQUENCE;

  Serial.println("BG770 Power on");
}

/*************************************************************************************************/
api_status_t bg770_reset(void)
{
  if (BG770_STATE_NO_OPEN != bg_state) {
    /* 赤LED点滅 */
    LAN_RED_FLA(5,100);
    LAN_RED_ON();

    /* GPIO */
    delay(1000);
    BG770_RESET_ON();
    delay(1000);
    BG770_RESET_OFF();
    
    /* 変数の初期化 */
    init_command_sequence_index = 0;
    rssi = 99;
    bg_state = BG770_STATE_INIT_COMMAND_SEQUENCE;
  }
  Serial.println("BG770 Reset");

  return API_STATUS_IN_PROGRESS;
}

/*************************************************************************************************/
api_status_t bg770_send_payload(const uint8_t payload[], uint16_t length)
{
  api_status_t result = API_STATUS_FAIL;

  uint16_t ptr = 0;
  while (ptr < length) {
    Serial1.write(payload[ptr]);
    ++ptr;
  }
  Serial1.write('\x1a');
  result = API_STATUS_SUCCESS;

  return result;
}

/*************************************************************************************************/
int16_t bg770_get_rssi(void) { return rssi; }

/*************************************************************************************************/
void bg770_get_imsi(char getimsi[16]) { strcpy(getimsi,imsi); }

/*************************************************************************************************/
api_status_t init_command_sequence_task(void)
{
  api_status_t status = API_STATUS_IN_PROGRESS;

  const command_executor_t *p_executor = &init_command_sequence[init_command_sequence_index];

  if ((NULL != p_executor->validate_response_func) || (NULL != p_executor->create_command_func)) {
    /* コマンド実行 */
    api_status_t result = execute(p_executor);

    if ( result == API_STATUS_SUCCESS) {
      ++init_command_sequence_index;
    }
    else if ( result == API_STATUS_COPS_ERROR ){
        if ( false == cops_err ){
            status = API_STATUS_IN_PROGRESS;
            cops_err = true;
        } else {
            status = API_STATUS_FAIL;
        }
    }
    else {
      status = API_STATUS_FAIL;
    }
  } else {
    /* 番兵に到達(処理終わり) */
    init_command_sequence_index = 0;
    bg_state = BG770_STATE_SUBSCRIBE;
    status = API_STATUS_SUBSCRIBE;
  }

  return status;
}

/*************************************************************************************************/
api_status_t execute(const command_executor_t *p_executor)
{
  /* コマンド送信 */
  if (NULL != p_executor->create_command_func) {
    const char *p = p_executor->create_command_func();

#ifdef DEBUG_PRINT
    Serial.println("command:" + String(p));
#endif

    if (p_executor->command_delay != 0) {
      /*
       * コマンドディレイ処理
       * AT+CSQ
       * など、ネットワークコマンドを実行してから3秒以上は空けないと正しい情報が取得できない。
       */
      delay(p_executor->command_delay);
    }

    while ('\0' != *p) {
      uint8_t data = *p;
      Serial1.write(data);
      ++p;
    }
  }

  /* レスポンス受信 */
  api_status_t result = API_STATUS_IN_PROGRESS;
  /* 受信カウント */
  uint16_t times = 0;
  /* タイムアウトカウント*/
  uint32_t timeout_count = 0;

  while (API_STATUS_IN_PROGRESS == result) {
    if(Serial1.available()) {
      String content = bg770_RxDataGet();
      /* NULLを無視 */
      if(content != ""){
        /* 受信カウントのインクリメント */
        ++times;
        /* String型からChar型へ変換(関数流用のため) */
        const char *content_char = content.c_str();
        /* 受信データ取得 */
        result = p_executor->validate_response_func(content_char, times);
      }
    }
    if(timeout_count > p_executor->timeout){
      result = API_STATUS_FAIL;
    }
    timeout_count++;
    delay(1);
  }

  return result;
}

/*************************************************************************************************/
api_status_t validate_response_ok(const char *content, uint16_t times)
{
  api_status_t result = API_STATUS_FAIL;

  if ((1 == times) && (0 == strcmp(content, zero))) {
    result = API_STATUS_SUCCESS;
  }

  return result;
}

/*************************************************************************************************/
api_status_t validate_response_ready(const char *content, uint16_t times)
{
  api_status_t result = API_STATUS_FAIL;

  if(0 == strcmp(content, "APP RDY")){
    result = API_STATUS_SUCCESS;    
  } else if(0 == strcmp(content, "NORMAL POWER DOWN")){
    result = API_STATUS_FAIL;
  } else {
    result = API_STATUS_IN_PROGRESS;
  }

  return result;
}

/*************************************************************************************************/
const char *create_command_bg770_setup(void)
{
  static const char *command = "ATE0;V0;+CMEE=0\r";
  /*
   * Echo mode OFF
   * Short result code format
   * Disable result code and use ERROR instead
   */
  return command;
}

/*************************************************************************************************/
api_status_t validate_response_bg770_setup(const char *content, uint16_t times)
{
  api_status_t result = API_STATUS_FAIL;

  /* デフォルト設定では1回目に Echo が返ってくる。 */
  if (times <= 2) {
    if (0 == strcmp(content, zero)) {
      result = API_STATUS_SUCCESS;
    } else {
      /* Echo の内容は無視 */
      result = API_STATUS_IN_PROGRESS;
    }
  }

  return result;
}

/*************************************************************************************************/
const char *create_command_cpin(void)
{
  static const char *command = "AT+CPIN?\r";
  
  return command;
}

/*************************************************************************************************/
api_status_t validate_response_cpin(const char *content, uint16_t times)
{
  api_status_t result = API_STATUS_FAIL;
  /*
   * <CR><LF>+CPIN: READY<CR><LF>0<CR>
   * OR
   * <CR><LF>ERROR<CR><LF>
   */
  if ((1 == times) && (0 == strcmp(content, "+CPIN: READY"))) {
    result = API_STATUS_IN_PROGRESS;
  } else if ((2 == times) && (0 == strcmp(content, zero))) {
    result = API_STATUS_SUCCESS;
  }

  return result;
}

/*************************************************************************************************/
const char *create_command_cimi(void)
{
  static const char *command = "AT+CIMI\r";
  return command;
}

/*************************************************************************************************/
const char *create_command_csq(void)
{
  static const char *command = "AT+CSQ\r";
  return command;
}

/*************************************************************************************************/
api_status_t validate_response_csq(const char *content, uint16_t times)
{
  api_status_t result = API_STATUS_FAIL;
  /*
   * <CR><LF>+CSQ: <rssi>,<ber><CR><LF>0<CR>
   * rssi
   *  0-31: -113 to -51 dbm
   *    99: Not known or not detectable
   * ber
   *  0- 7: As RxQual values in the table in 3GPP TS 45.008 subclause 8.2.4
   *    99: Not known or not detectable
   */

  if ((1 == times) && (0 == strncmp(content, "+CSQ:", 5))) {
    char *endptr;
    long csq = (int16_t)strtol(&content[6], &endptr, 10);
    if ((',' == *endptr) && ((99 == csq) || ((0 <= csq) && (csq <= 30)))) {
      if (99 == csq) {
        rssi = (int16_t)csq;
      } else {
        rssi = (int16_t)(2 * csq - 113);
      }
      result = API_STATUS_IN_PROGRESS;
    }
  } else if ((2 == times) && (0 == strcmp(content, zero))) {
    result = API_STATUS_SUCCESS;
  }

  return result;
}

/*************************************************************************************************/
api_status_t validate_response_cimi(const char *content, uint16_t times)
{
  api_status_t result = API_STATUS_FAIL;
  /*
   * <CR><LF><IMSI><CR><LF>0<CR>
   */
  if ((1 == times) && (15 == strlen(content))) {
    /* IMSI */
    strcpy(imsi, content);
    Serial.println("IMSI:[" + String(imsi) + "]");
    result = API_STATUS_IN_PROGRESS;
  } else if ((2 == times) && (0 == strcmp(content, zero))) {
    result = API_STATUS_SUCCESS;
  }

  return result;
}

/*************************************************************************************************/
const char *create_command_cgdcont(void)
{
  static const char *command = "AT+CGDCONT=1,\"IP\",\"soracom.io\"\r";
  return command;
}

/*************************************************************************************************/
const char *create_command_cops(void)
{
  static char command[] = "AT+COPS=1,2,\"00000\",8\r";
#ifdef SIMMODE 
  strncpy(&command[13], imsi, 5);
#endif
#ifdef eSIMMODE
  if(saved_operator == OPERATOR_SOFTBANK){
      strcpy(command,"AT+COPS=1,2,\"44020\",8\r");
  }else{
      strcpy(command,"AT+COPS=1,2,\"44010\",8\r");
  }
#endif

  return command;
}

/*************************************************************************************************/
const char *create_command_qicsgp(void)
{
  static const char *command = "AT+QICSGP=1,1,\"soracom.io\",\"sora\",\"sora\",2\r";
  return command;
}

/*************************************************************************************************/
const char *create_command_qiact(void)
{
  static const char *command = "AT+QIACT=1\r";
  return command;
}

/*************************************************************************************************/
const char *create_command_qideact(void)
{
  static const char *command = "AT+QIDEACT=1\r";
  return command;
}

/*************************************************************************************************/
const char *create_command_qiopen(void)
{
  static const char *command = "AT+QIOPEN=1,0,\"UDP\",\"uni.soracom.io\",23080\r";
  return command;
}

/*************************************************************************************************/
api_status_t validate_response_qiopen(const char *content, uint16_t times)
{
  api_status_t result = API_STATUS_FAIL;
  /*
   * <CR><LF>0<CR><LF>+QIOPEN: 0,0<CR><LF>
   * connect id は 0 固定とする
   */
  if ((1 == times) && (0 == strcmp(content, zero))) {
    result = API_STATUS_IN_PROGRESS;
  } else if ((2 == times) && (0 == strcmp(content, "+QIOPEN: 0,0"))) {
    result = API_STATUS_SUCCESS;
  }

  return result;
}

/*************************************************************************************************/
const char *create_command_qpowd(void)
{
  static const char *command = "AT+QPOWD=0\r";
  return command;
}

/*************************************************************************************************/
api_status_t validate_response_qpowd(const char *content, uint16_t times)
{
  api_status_t result = API_STATUS_FAIL;

  if (1 == times) {
    /* <CR><LF>OK<CR><LF> */
    if (0 == strcmp(content, "OK")) {
      result = API_STATUS_IN_PROGRESS;
    }
  } else if (2 == times) {
    /* POWERED DOWN */
    if (0 == strcmp(content, "POWERED DOWN")) {
      result = API_STATUS_SUCCESS;
    }
  }
  return result;
}


/*************************************************************************************************/
api_status_t validate_response_cops(const char *content, uint16_t times)
{
  api_status_t result = API_STATUS_FAIL;

  if ((1 == times) && (0 == strcmp(content, zero))) {
      result = API_STATUS_SUCCESS;
  } else {
      if ( saved_operator == OPERATOR_SOFTBANK ){ saved_operator = OPERATOR_NTTDOCOMO; }
      else                                      { saved_operator = OPERATOR_SOFTBANK;  }
      result = API_STATUS_COPS_ERROR;
  }

  return result;
}

/*************************************************************************************************/
const char *create_command_qmtopen(void)
{
  static const char *command = "AT+QMTOPEN=0,\"beam.soracom.io\",1883\r";
  return command;
}

/*************************************************************************************************/
api_status_t validate_response_qmtopen(const char *content, uint16_t times)
{
  api_status_t result = API_STATUS_FAIL;
  /*
   * <CR><LF>0<CR><LF>+QMTOPEN: 0,0<CR><LF>
   * client idx は 0 固定とする
   */
  if ((1 == times) && (0 == strcmp(content, zero))) {
    result = API_STATUS_IN_PROGRESS;
  } else if ((2 == times) && (0 == strcmp(content, "+QMTOPEN: 0,0"))) {
    result = API_STATUS_SUCCESS;
  }

  return result;
}

/*************************************************************************************************/
const char *create_command_qmtconn(void)
{
  static const char *command = "AT+QMTCONN=0,\"SampleClient\"\r";
  return command;
}

/*************************************************************************************************/
api_status_t validate_response_qmtconn(const char *content, uint16_t times)
{
  api_status_t result = API_STATUS_FAIL;
  /*
   * <CR><LF>0<CR><LF>+QMTCONN: 0,0,0<CR><LF>
   * client idx は 0 固定とする
   */
  if ((1 == times) && (0 == strcmp(content, zero))) {
    result = API_STATUS_IN_PROGRESS;
  } else if ((2 == times) && (0 == strcmp(content, "+QMTCONN: 0,0,0"))) {
    result = API_STATUS_SUCCESS;
  }

  return result;
}

/*************************************************************************************************/
const char *create_command_qmtsub(void)
{
  /* +QMTSUB: (0-5),(1-65535),<topic>,(0-2) */
  static char command[COMMAND_SIZE];
  strncpy(command,"",COMMAND_SIZE);
  strcpy(command,"AT+QMTSUB=0,1,\"");
  strcat(command,SUBSCRIBE_TOPIC);
  strcat(command,"\",1\r");
  return command;
}

/*************************************************************************************************/
api_status_t validate_response_qmtsub(const char *content, uint16_t times)
{
  api_status_t result = API_STATUS_FAIL;
  /*
   * <CR><LF>0<CR><LF>+QMTSUB: 0,1,0,1<CR><LF>
   * client idx は 0 ,msgID は 1, 固定とする
   */
  if ((1 == times) && (0 == strcmp(content, zero))) {
    result = API_STATUS_IN_PROGRESS;
  } else if ((2 == times) && (0 == strcmp(content, "+QMTSUB: 0,1,0,1"))) {
    result = API_STATUS_SUCCESS;
  }

  return result;
}

/*************************************************************************************************/
const char *create_command_qmtuns(void)
{
  /* +QMTSUB: (0-5),(1-65535),<topic>,(0-2) */
  static char command[COMMAND_SIZE];
  strncpy(command,"",COMMAND_SIZE);
  strcpy(command,"AT+QMTUNS=0,1,\"");
  strcat(command,SUBSCRIBE_TOPIC);
  strcat(command,"\"\r");

  return command;
}

/*************************************************************************************************/
api_status_t validate_response_qmtuns(const char *content, uint16_t times)
{
  api_status_t result = API_STATUS_FAIL;
  /*
   * <CR><LF>0<CR><LF>+QMTUNS: 0,1,0<CR><LF>
   * 但し、コマンド実行中にサブスクライブされた場合、「0」と「+QMTUNS: 0,1,0」の間にデータが入力されるので、
   * エラー処理はタイムアウトのみとする
   */
  if(0 == strcmp(content, "+QMTUNS: 0,1,0")){
    result = API_STATUS_SUCCESS;
  } else {
    result = API_STATUS_IN_PROGRESS;
  }

  return result;
}

/*************************************************************************************************/
String bg770_RxDataGet(){
  String content = Serial1.readStringUntil('\r');
  /* 改行の削除 */
  content.replace("\r",""); content.replace("\0",""); content.replace("\n","");

#ifdef DEBUG_PRINT
  /* NULLを無視 */
  if(content != ""){
    /* 返答確認 */
    Serial.println("content:[" + String(content) + "]");
  }
#endif

  return content;
}

/*************************************************************************************************/
void split(String data, char delimiter, String *dst, uint8_t dst_index){
    uint8_t index = 0; 
    int datalength = data.length();

    for (int i = 0; i < datalength; i++) {
        char tmp = data.charAt(i);
        if ( tmp == delimiter ) {
            if ( index == dst_index-1 ) { dst[index] += tmp; }
            else                      { index++; }
        }
        else dst[index] += tmp;
    }
    
}

/*************************************************************************************************/
String RxData_Analize(String RxData){

    /* 受信データを「,」で区分け */
    String RxData_split[BG770_SUB_PAYLOAD_INDEX];
    /* サブスクライブデータ内のペイロード部分を抽出 */
    split(RxData,',',RxData_split,BG770_SUB_PAYLOAD_INDEX);

    /*ペイロード部分からダブルクォーテーションを排除*/
    String payload = (RxData_split[BG770_SUB_PAYLOAD_INDEX - 1].substring(1, RxData_split[BG770_SUB_PAYLOAD_INDEX - 1].length()-1));

    return payload;
}

/*************************************************************************************************/
const char *create_command_qmtpub(void)
{
  static char command[COMMAND_SIZE];
  strncpy(command,"",COMMAND_SIZE);
  strcpy(command,"AT+QMTPUB=0,1,1,0,\"");
  strcat(command,PUBLISH_TOPIC);
  strcat(command,"\"\r");

  return command;
}

/*************************************************************************************************/
api_status_t validate_response_qmtpub(const char *content, uint16_t times)
{
  api_status_t result = API_STATUS_FAIL;
  /*
   * <CR><LF>0<CR><LF>+QIOPEN: 0,0<CR><LF>
   * connect id は 0 固定とする
   */
  if ((1 == times) && (0 == strcmp(content, "> "))) {
    bg770_send_payload(Publish_payload,Publish_length);
    result = API_STATUS_IN_PROGRESS;
  } else if ((2 == times) && (0 == strcmp(content, zero))) {
    result = API_STATUS_IN_PROGRESS;
  } else if ((3 == times) && (0 == strcmp(content, "+QMTPUB: 0,1,0"))) {
    result = API_STATUS_SUCCESS;
  }

  return result;
}
