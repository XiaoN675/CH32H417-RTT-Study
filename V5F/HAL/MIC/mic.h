/********************************** (C) COPYRIGHT *******************************
 * File Name          : mic.h
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2023/10/9
 * Description        :
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/

/******************************************************************************/
#ifndef __MIC_H
#define __MIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ch32h417.h"

/*********************************************************************
 * CONSTANTS
 */
 /* MIC高采样频率使能 */
#define MIC_HIGH_PERFORMANCE_EN         0

#define MIC_CONTROL_TIM_CLOCK           1000000                                             //使用定时器时钟1MHz
#define MIC_CONTROL_TIM_PRESCALER       (HCLKClock / MIC_CONTROL_TIM_CLOCK - 1)       //定时器的时钟分频系数,频率为1MHz,周期为1us
#if MIC_HIGH_PERFORMANCE_EN
#define MIC_CONTROL_TIM_PERIOD          (MIC_CONTROL_TIM_CLOCK / 1000000 * 50 - 1)         //定时器计数周期为50*1us=50us,采样频率为20KHz
#else 
// #define MIC_CONTROL_TIM_PERIOD          (MIC_CONTROL_TIM_CLOCK / 1000000 * 100 - 1)         //定时器计数周期为100*1us=100us,采样频率为10KHz
#define MIC_CONTROL_TIM_PERIOD          (MIC_CONTROL_TIM_CLOCK / 1000000 * 125 - 1)         //定时器计数周期为125*1us=125us,采样频率为8KHz
#endif

 /* ADC采集的通道 */
#define MIC_ADC_CHANNEL                 ADC_Channel_15

/* ADC转换值的中值 */
#define MIC_ADC_MID                     2048

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/**
 * @brief   MIC采集放大器初始化
 */
void mic_opa_init(void);

/**
 * @brief   MIC采集ADC初始化
 */
void mic_adc_init(uint16_t *adc_buf, uint16_t len);

/**
 * @brief   MIC ADC采集使能设置
 */
void mic_adc_collection_set(uint8_t cap_en);

/**
 * @brief   重置麦克风录制内容
 */
void mic_adc_reset_recording(uint16_t *record_buf, uint16_t len);

/**
 * @brief   麦克风音频数据滤波
 */
void mic_audio_filter(uint16_t *data, uint32_t len);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif
