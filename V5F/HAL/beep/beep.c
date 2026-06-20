/********************************** (C) COPYRIGHT *******************************
 * File Name          : beep.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2023/10/9
 * Description        :
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/

/******************************************************************************/
/* 头文件包含 */
#include "beep.h"

/**************************************************************************************************
 *                                        GLOBAL VARIABLES
 **************************************************************************************************/
uint8_t beep_level; //当前蜂鸣器音量,百分比

//                             do    re    mi    fa    sol   la    si
const uint16_t beepTone[7] = {1310, 1470, 1650, 1750, 1960, 2200, 2470};

/**************************************************************************************************
 *                                        FUNCTIONS - Local
 **************************************************************************************************/
/* PWM驱动Beep初始化 */
void beep_pwm_init(uint16_t arr, uint16_t psc, uint16_t ccp)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    TIM9_12_TimeBaseInitTypeDef TIM9_12_TimeBaseInitStructure = {0};
    TIM9_12_OCInitTypeDef TIM9_12_OCInitStructure = {0};

    RCC_HB2PeriphClockCmd(RCC_HB2Periph_AFIO | BEEP_GPIO_CLOCK | RCC_HB2Periph_TIM9, ENABLE);

    //GPIO Init
    GPIO_PinAFConfig(BEEP_GPIO, GPIO_PinSource15, GPIO_AF2);
    GPIO_InitStructure.GPIO_Pin = BEEP_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Very_High;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;                     //复用推挽输出
    GPIO_Init(BEEP_GPIO, &GPIO_InitStructure);

    //TIM Init
    TIM9_12_TimeBaseInitStructure.TIM_Period = arr;                     //自动重装载值
    TIM9_12_TimeBaseInitStructure.TIM_Prescaler = psc;                  //预分频系数, 计数器的时钟频率=APB1/(psc+1), APB1=96MHz
    TIM9_12_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //该应用中未使用到
    TIM9_12_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //向上计数模式
    TIM9_12_TimeBaseInit(TIM9, &TIM9_12_TimeBaseInitStructure);         //定时器2初始化

    //Channel Init
    TIM9_12_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;               //向上计数时, TIMx_CNT < TIMx_CH1CVR 输出有效电平，否则输出无效电平
    TIM9_12_OCInitStructure.TIM_Pulse = ccp;                            //设置待装入捕获比较寄存器的比较值 TIMx_CH1CVR
    TIM9_12_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;   //输出使能
    TIM9_12_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;       //高电平有效
    TIM9_12_OC2Init(TIM9, &TIM9_12_OCInitStructure);                    //通道2初始化

    TIM_OC2PreloadConfig(TIM9, TIM_OCPreload_Disable);                  //比较捕获寄存器2预装载失能,即写入比较捕获寄存器中的值会立即起作用
    TIM_ARRPreloadConfig(TIM9, ENABLE);                                 //自动重装载使能
    TIM_Cmd(TIM9, ENABLE);                                              //计数器使能
}

/* 改变蜂鸣器的音调 */
void beep_pwm_control(uint16_t period, uint8_t per)
{
    uint16_t duty_cycle;

    if(per > 100) //百分比大于100
    {
        per = 100;
    }
    duty_cycle = (period + 1) * per / 100;

    /* 更新定时器周期（ARR）来改变频率，更新比较值（CCR）来改变音量 */
    TIM9->ATRLR_32 = period;
    TIM9_12_SetCompare2(TIM9, duty_cycle); //设置定时9通道2的比较值
}
