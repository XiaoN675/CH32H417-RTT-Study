/********************************** (C) COPYRIGHT *******************************
 * File Name          : led.h
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2023/10/9
 * Description        :
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/

/******************************************************************************/
#ifndef __LED_H
#define __LED_H

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_config.h"

/*********************************************************************
 * CONSTANTS
 */
#define LED_RED_GPIO        GPIOC
#define LED_GREEN_GPIO      GPIOC
#define LED_BLUE_GPIO       GPIOC
#define LED_RED_PIN         GPIO_Pin_9 //红灯PC9
#define LED_GREEN_PIN       GPIO_Pin_8 //绿灯PC8
#define LED_BLUE_PIN        GPIO_Pin_7 //蓝灯PC7

#define RED_ON()            (GPIO_ResetBits(LED_RED_GPIO, LED_RED_PIN))         //开红灯
#define RED_OFF()           (GPIO_SetBits(LED_RED_GPIO, LED_RED_PIN))           //关红灯
#define GREEN_ON()          (GPIO_ResetBits(LED_GREEN_GPIO, LED_GREEN_PIN))     //开绿灯
#define GREEN_OFF()         (GPIO_SetBits(LED_GREEN_GPIO, LED_GREEN_PIN))       //关绿灯
#define BLUE_ON()           (GPIO_ResetBits(LED_BLUE_GPIO, LED_BLUE_PIN))       //开蓝灯
#define BLUE_OFF()          (GPIO_SetBits(LED_BLUE_GPIO, LED_BLUE_PIN))         //关蓝灯

#define LED_CONTROL_TIM_CLOCK           100000                                      //LED控制定时器时钟100KHz
#define LED_CONTROL_TIM_PRESCALER       (HCLKClock / LED_CONTROL_TIM_CLOCK - 1)     //定时器的时钟分频系数,频率为100KHz,周期为10us
#define LED_CONTROL_TIM_PERIOD_1MS      (LED_CONTROL_TIM_CLOCK / 100000 * 100 - 1)  //LED控制的PWM周期为100*10us=1ms

/*********************************************************************
 * TYPEDEFS
 */
typedef enum
{
    kBlack = 0,     //黑色
    kWhite,         //白色
    kRed,           //红色
    kGreen,         //绿色
    kBlue,          //蓝色
    kYellow,        //黄色
    kMagenta,       //品红/红紫色
    kCyan           //青色
}led_color_t;

typedef enum
{
    kALL_LED = 0,   //所有灯
    kRED_LED,       //红 PB3
    kGREEN_LED,     //绿 PB4
    kBLUE_LED,       //蓝 PB5
    KYELLOW_LED,       //黄色
    KMAGENTA_LED,   //品红/红紫色
    KCYAN_LED       //青色
}led_num_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */
extern uint8_t led_level; //当前LED的亮度,百分比

/**
 * @brief   GPIO驱动LED初始化
 */
void led_init(void);

/**
 * @brief   改变LED显示的颜色
 */
void led_color_change(led_color_t color);

/**
 * @brief   PWM驱动LED初始化
 */
void led_pwm_init(uint16_t arr, uint16_t psc, uint16_t ccp);

/**
 * @brief   LED的亮度控制
 */
void led_pwm_control(led_num_t led_num, uint16_t period, uint8_t per);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif
