/********************************** (C) COPYRIGHT *******************************
 * File Name          : key.h
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2023/10/9
 * Description        :
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/

/******************************************************************************/
#ifndef __KEY_H
#define __KEY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_config.h"

/*********************************************************************
 * CONSTANTS
 */
#define KEY1_GPIO           GPIOA
#define KEY2_GPIO           GPIOA
#define KEY3_GPIO           GPIOA
#define KEY4_GPIO           GPIOA
#define KEY1_PIN            GPIO_Pin_8  //按键1-PA8
#define KEY2_PIN            GPIO_Pin_9  //按键2-PA9
#define KEY3_PIN            GPIO_Pin_10 //按键3-PA10
#define KEY4_PIN            GPIO_Pin_15 //按键4-PA15

/* Keys */
#define KEY_BUTTON_1        0x01        //按键1-KEY1
#define KEY_BUTTON_2        0x02        //按键2-KEY2
#define KEY_BUTTON_3        0x04        //按键3-KEY3
#define KEY_BUTTON_4        0x08        //按键4-KEY4

/* Keys Status */
#define KEY_PUSH_BUTTON_1()         (GPIO_ReadInputDataBit(KEY1_GPIO, KEY1_PIN) == Bit_RESET) //按键1按下
#define KEY_PUSH_BUTTON_2()         (GPIO_ReadInputDataBit(KEY2_GPIO, KEY2_PIN) == Bit_RESET) //按键2按下
#define KEY_PUSH_BUTTON_3()         (GPIO_ReadInputDataBit(KEY3_GPIO, KEY3_PIN) == Bit_RESET) //按键3按下
#define KEY_PUSH_BUTTON_4()         (GPIO_ReadInputDataBit(KEY4_GPIO, KEY4_PIN) == Bit_RESET) //按键4按下

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/**
 * @brief   按键驱动GPIO初始化
 */
void key_gpio_init(void);

/**
 * @brief   按键扫描中断初始化
 */
void key_interrupt_init(void);

/**
 * @brief   按键扫描并处理
 */
uint8_t key_polling(void);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif
