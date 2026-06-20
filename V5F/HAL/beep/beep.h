/********************************** (C) COPYRIGHT *******************************
 * File Name          : beep.h
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2023/10/9
 * Description        :
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/

/******************************************************************************/
#ifndef __BEEP_H
#define __BEEP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ch32h417.h"

/*********************************************************************
 * CONSTANTSA
 */
 #define BEEP_GPIO_CLOCK    RCC_HB2Periph_GPIOB
#define BEEP_GPIO           GPIOB
#define BEEP_PIN            GPIO_Pin_15  //蜂鸣器-PB15

#define BEEP_CONTROL_TIM_CLOCK           100000                                         //蜂鸣器控制定时器时钟100KHz
#define BEEP_CONTROL_TIM_PRESCALER       (HCLKClock / BEEP_CONTROL_TIM_CLOCK - 1)       //定时器的时钟分频系数,频率为100KHz,周期为10us
#define BEEP_CONTROL_TIM_PERIOD_1MS      (BEEP_CONTROL_TIM_CLOCK / 100000 * 100 - 1)    //蜂鸣器控制的PWM周期固定为100*10us=1ms

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
extern uint8_t beep_level;       //当前蜂鸣器音量,百分比
extern const uint16_t beepTone[7];  //不同音调的频率

/**
 * @brief   PWM驱动Beep初始化
 */
void beep_pwm_init(uint16_t arr, uint16_t psc, uint16_t ccp);

/**
 * @brief   改变蜂鸣器的音调
 */
void beep_pwm_control(uint16_t period, uint8_t per);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif
