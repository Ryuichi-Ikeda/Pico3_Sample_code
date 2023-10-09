/**
 * @file setup_define.h
 * @version 0.1
 * @brief 各種設定ファイル
 *
 * @author Iefuji Kohei (iefuji.kohei@kyokko.co.jp)
 * @date 2023-10-10
 * @copyright Copyright (c) 2023 旭光電機株式会社
 */
#ifndef SETUP_DEFINE_H
#define SETUP_DEFINE_H

/**************************************************************************************************
 * CONSTANTS
 */
/** @brief  デバッグ用プリントモード */
#define DEBUG_PRINT
/** @brief  SIMモードの定義（eSIM or SIM） */
#define eSIMMODE
//#define SIMMODE
/** @brief  サブスクライブTOPIC */
#define SUBSCRIBE_TOPIC "pico/sample/sub"
/** @brief  パブリッシュTOPIC */
#define PUBLISH_TOPIC   "pico/sample/pub"
/** @brief パブリッシュサイズ */
#define PUBLISH_SIZE     1500

#endif
