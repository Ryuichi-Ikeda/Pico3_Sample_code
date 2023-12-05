/**
 * @file   CK_1540_01.h
 * @brief  CK-1540-01ポートライブラリ
 * @author KOHEI Iefuji
 * @{
 */
#ifndef CK_1540_01_H
#define CK_1540_01_H

#include <Arduino.h>
#include <WebServer.h>

/***************************************************************************************************
 * CONSTANTS
 */
/** @brief CAN RXD ピン番号 */
#define PORT_CAN_RXD        35
/** @brief CAN TXD ピン番号 */
#define PORT_CAN_TXD        25
/** @brief SW ピン番号 */
#define	PORT_INP_SW         36
/** @brief INT1(BG770) ピン番号 */
#define	PORT_INP_INT1       39
/** @brief INT2(CC1310) ピン番号 */
#define	PORT_INP_INT2       19
/** @brief LAN用LED緑 ピン番号 */
#define	PORT_OUT_LANLED_G   32  
/** @brief LAN用LED赤 ピン番号 */
#define	PORT_OUT_LANLED_R   27  
/** @brief 電源用LED赤 ピン番号 */
#define	PORT_OUT_PWRLED_R   26 
/** @brief WAN用LED緑 ピン番号 */
#define	PORT_OUT_WANLED_G   12
/** @brief WAN用LED赤 ピン番号 */
#define	PORT_OUT_WANLED_R   33
/** @brief WAN用LED青 ピン番号 */
#define	PORT_OUT_WANLED_B   13
/** @brief ブザー ピン番号 */
#define	PORT_OUT_BUZZ       14
/** @brief LTE用RXD(BG770) ピン番号 */
#define PORT_LTEUART_RXD    34
/** @brief LTE用TXD(BG770) ピン番号 */
#define PORT_LTEUART_TXD    15
/** @brief Sub-GHz用RXD(CC1310) ピン番号 */
#define PORT_SUBGUART_RXD   4
/** @brief Sub-GHz用TXD(CC1310) ピン番号 */
#define PORT_SUBGUART_TXD   5
/** @brief PC接続用RXD ピン番号 */
#define PORT_UART0_RXD      3
/** @brief PC接続用TXD ピン番号 */
#define PORT_UART0_TXD      1
/** @brief 通信モジュールリセット ピン番号 */
#define PORT_OUT_MODULE_RESET  18

/***************************************************************************************************
 * GLOBAL FUNCTIONS
 */
/** @brief GPIO初期化 **/
void initGPIO();
/** @brief wifi初期化 **/
void initWifi();
/** @brief BG770リセット（負論理) **/
void BG770_RESET_ON();
void BG770_RESET_OFF(); 
/** @brief 電源LED ON/OFF **/
void PWR_RED_ON();
void PWR_RED_OFF();
/** @brief LAN用LED ON/OFF **/
void LAN_GREEN_ON();
void LAN_GREEN_OFF();
void LAN_RED_ON();
void LAN_RED_OFF();
/** @brief WAN用LED ON/OFF **/
void WAN_GREEN_ON();
void WAN_GREEN_OFF();
void WAN_RED_ON();
void WAN_RED_OFF();
/*ハンドラ設定*/
void handleRoot(void);
void handleWifi(void);
void handleLed(void);
void handleRedLedOn(void);
void handleGreenLedOn(void);
void handleLedOff(void);
void sendErrorPage(String);
/**
 * @brief LAN赤LED点滅関数
 * @param[in] time ：点滅回数
 * @param[in] msec ：点滅時間（Duty50％固定）
 */
void LAN_RED_FLA(uint16_t time, uint16_t msec);

extern WebServer server;

/**************************************************************************************************/
#endif
/** @} */