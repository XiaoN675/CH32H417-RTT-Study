/********************************** (C) COPYRIGHT *******************************
 * File Name          : adc.h
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2023/10/9
 * Description        :
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/

/******************************************************************************/
#ifndef __ADC_H
#define __ADC_H

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
 * @brief   ADC1初始化
 */
void adc1_single_convert_init(void);

/**
 * @brief   获取ADC(单次采集)
 */
uint16_t adc1_single_convert_value_get(uint8_t ch, uint8_t sample_time);

/**
 * @brief   获取ADC(多次取平均值)
 */
uint16_t adc1_single_convert_average_value_get(uint8_t ch, uint8_t times, uint8_t sample_time);

/**
 * @brief   ADC实际电压值计算
 */
uint32_t adc1_voltage_get(uint16_t val);

/**
 * @brief   ADC连续转换初始化
 */
void adc1_continue_convert_init(uint16_t *adc_buf, uint16_t len, uint8_t channel, uint16_t arr, uint16_t psc);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif
