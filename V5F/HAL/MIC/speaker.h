/********************************** (C) COPYRIGHT *******************************
 * File Name          : speaker.h
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2023/10/9
 * Description        :
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/

/******************************************************************************/
#ifndef __SPEAKER_H
#define __SPEAKER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ch32h417.h"

/*********************************************************************
 * CONSTANTS
 */


/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/**
 * @brief   扬声器DAC输出初始化
 */
void speaker_dac_init(void);

/**
 * @brief   扬声器播放录制的声音
 */
void speaker_dac_play_recording(uint16_t *record_buf, uint16_t len);

/**
 * @brief   扬声器循环播放（Circular DMA），用于实时监听
 */
void speaker_dac_play_continuous(uint16_t *record_buf, uint16_t len);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif
