/********************************** (C) COPYRIGHT *******************************
 * File Name          : key.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2023/10/9
 * Description        :
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/

/******************************************************************************/
/* 头文件包含 */
#include "key.h"

/**************************************************************************************************
 *                                        GLOBAL VARIABLES
 **************************************************************************************************/
/* 保存的前一次扫描的键值 */
uint8_t preScannedKeys = 0;

/* 保存的前一次有效的键值 */
uint8_t preValidKeys = 0;

/**************************************************************************************************
 *                                        FUNCTIONS - Local
 **************************************************************************************************/
/* 按键驱动GPIO初始化 */
void key_gpio_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    RCC_HB2PeriphClockCmd(RCC_HB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = KEY1_PIN | KEY2_PIN | KEY3_PIN | KEY4_PIN; //PA8:按键1,PA9:按键2,PA10:按键3,PA15:按键4
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //上拉输入模式
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/* 按键扫描中断初始化 */
void key_interrupt_init(void)
{
    EXTI_InitTypeDef EXTI_InitStructure = {0};

    RCC_HB2PeriphClockCmd(RCC_HB2Periph_AFIO, ENABLE);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource8);
    EXTI_InitStructure.EXTI_Line = EXTI_Line8;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);                             //PA8外部中断初始化

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource9);
    EXTI_InitStructure.EXTI_Line = EXTI_Line9;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);                             //PA9外部中断初始化

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource10);
    EXTI_InitStructure.EXTI_Line = EXTI_Line10;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);                             //PA10外部中断初始化

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource15);
    EXTI_InitStructure.EXTI_Line = EXTI_Line15;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);                             //PA15外部中断初始化

    NVIC_SetPriority(EXTI15_8_IRQn, 10 << 4);
    NVIC_EnableIRQ(EXTI15_8_IRQn);                              //外部中断线15_8初始化
}

/* 按键扫描并处理 */
uint8_t key_polling(void)
{
    uint8_t keys = 0;
    uint8_t result = 0;

    if(KEY_PUSH_BUTTON_1())
    {
        keys |= KEY_BUTTON_1;
    }
    if(KEY_PUSH_BUTTON_2())
    {
        keys |= KEY_BUTTON_2;
    }
    if(KEY_PUSH_BUTTON_3())
    {
        keys |= KEY_BUTTON_3;
    }
    if(KEY_PUSH_BUTTON_4())
    {
        keys |= KEY_BUTTON_4;
    }

    if(keys == preScannedKeys) //本次和前一次扫描到的按键结果相同时，为有效键值，起到消抖的作用
    {
        /* 有效键值中有按键被按下，同时前一次有效键值中没有按键被按下，也就是只有按下的动作才会让if判定为真，长按或者抬起动作为假 */
        /* 多按键同时按下，优先处理序号小的按键 */
        /* 多按键一前一后按下，会依次处理有按下动作的按键 */
        if(keys != 0)
        {
            if(((keys & KEY_BUTTON_1)) && ((preValidKeys & KEY_BUTTON_1) == 0))        //按键1
            {
                result |= KEY_BUTTON_1;
            }
            else if(((keys & KEY_BUTTON_2)) && ((preValidKeys & KEY_BUTTON_2) == 0))   //按键2
            {
                result |= KEY_BUTTON_2;
            }
            else if(((keys & KEY_BUTTON_3)) && ((preValidKeys & KEY_BUTTON_3) == 0))   //按键3
            {
                result |= KEY_BUTTON_3;
            }
            else if(((keys & KEY_BUTTON_4)) && ((preValidKeys & KEY_BUTTON_4) == 0))   //按键4
            {
                result |= KEY_BUTTON_4;
            }
        }
        preValidKeys = keys;    //保存有效键值
    }
    preScannedKeys = keys;      //保存扫描键值

    return result; //返回扫描结果
}
