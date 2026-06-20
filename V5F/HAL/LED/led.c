/********************************** (C) COPYRIGHT *******************************
 * File Name          : led.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2023/10/9
 * Description        :
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/

/******************************************************************************/
/* 头文件包含 */
#include "led.h"

/**************************************************************************************************
 *                                        GLOBAL VARIABLES
 **************************************************************************************************/
uint8_t led_level;       //当前LED的亮度,百分比

/**************************************************************************************************
 *                                        FUNCTIONS - Local
 **************************************************************************************************/
/* GPIO驱动LED初始化 */
void led_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure = {0};

    RCC_HB2PeriphClockCmd(RCC_HB2Periph_GPIOC, ENABLE);

    GPIO_InitStructure.GPIO_Pin = LED_RED_PIN | LED_GREEN_PIN | LED_BLUE_PIN; //PC9:红色 PC8:绿色 PC7:蓝色
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Very_High;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //推挽输出
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    //灯默认为关闭状态
    RED_OFF();
    GREEN_OFF();
    BLUE_OFF();
}

/* 改变LED显示的颜色 */
void led_color_change(led_color_t color)
{
    switch(color)
    {
    case kBlack:    //黑色
        RED_OFF();
        GREEN_OFF();
        BLUE_OFF();
        break;

    case kWhite:    //白色
        RED_ON();
        GREEN_ON();
        BLUE_ON();
        break;

    case kRed:      //红色
        RED_ON();
        GREEN_OFF();
        BLUE_OFF();
        break;

    case kGreen:    //绿色
        RED_OFF();
        GREEN_ON();
        BLUE_OFF();
        break;

    case kBlue:     //蓝色
        RED_OFF();
        GREEN_OFF();
        BLUE_ON();
        break;

    case kYellow:   //黄色
        RED_ON();
        GREEN_ON();
        BLUE_OFF();
        break;

    case kMagenta:  //品红/红紫色
        RED_ON();
        GREEN_OFF();
        BLUE_ON();
        break;

    case kCyan:     //青色
        RED_OFF();
        GREEN_ON();
        BLUE_ON();
        break;

    default:
        break;
    }
}

/* PWM驱动LED初始化 */
void led_pwm_init(uint16_t arr, uint16_t psc, uint16_t ccp)
{
    GPIO_InitTypeDef  GPIO_InitStructure = {0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};
    TIM_OCInitTypeDef TIM_OCInitStructure = {0};

    RCC_HB1PeriphClockCmd(RCC_HB1Periph_TIM3, ENABLE);
    RCC_HB2PeriphClockCmd(RCC_HB2Periph_AFIO | RCC_HB2Periph_GPIOC, ENABLE);

	GPIO_PinAFConfig(LED_RED_GPIO, GPIO_PinSource9, GPIO_AF2);
    GPIO_PinAFConfig(LED_GREEN_GPIO, GPIO_PinSource8, GPIO_AF2);
    GPIO_PinAFConfig(LED_BLUE_GPIO, GPIO_PinSource7, GPIO_AF2);
    GPIO_InitStructure.GPIO_Pin = LED_RED_PIN | LED_GREEN_PIN | LED_BLUE_PIN; //PB3:红色   PB4:绿色   PB5:蓝色
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Very_High;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;                 //复用推挽输出
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    TIM_TimeBaseInitStructure.TIM_Period = arr;                     //自动重装载值
    TIM_TimeBaseInitStructure.TIM_Prescaler = psc;                  //预分频系数, 计数器的时钟频率=TIMxCLK/(psc+1), TIMxCLK=APB1*2=96MHz
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;     //输入捕获时滤波所用的采样时钟
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //向上计数模式
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStructure);             //定时器初始化

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;               //向上计数时, TIMx_CNT < TIMx_CH1CVR 输出有效电平，否则输出无效电平
    TIM_OCInitStructure.TIM_Pulse = ccp;                            //设置待装入捕获比较寄存器的比较值 TIMx_CH1CVR
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;   //输出使能
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;        //低电平有效
    TIM_OC4Init(TIM3, &TIM_OCInitStructure);                        //通道4初始化
    TIM_OC3Init(TIM3, &TIM_OCInitStructure);                        //通道3初始化
    TIM_OC2Init(TIM3, &TIM_OCInitStructure);                        //通道2初始化

    TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Disable);              //比较捕获寄存器4预装载失能,写入比较捕获寄存器中的值会立即起作用
    TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Disable);              //比较捕获寄存器3预装载失能,写入比较捕获寄存器中的值会立即起作用
    TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Disable);              //比较捕获寄存器2预装载失能,写入比较捕获寄存器中的值会立即起作用
    TIM_ARRPreloadConfig(TIM3, ENABLE );                            //自动重装载使能
    TIM_Cmd(TIM3, ENABLE);                                          //计数器使能
}

/* LED的亮度控制 */
void led_pwm_control(led_num_t led_num, uint16_t period, uint8_t per)
{
    uint16_t duty_cycle;

    if(per > 100) //百分比大于100
    {
        per = 100;
    }
    duty_cycle = (period + 1) * per / 100;
//    hal_printf(" per:%d | duty_cycle:%d\r\n",per, duty_cycle);

    switch(led_num)
    {
    case kALL_LED:
        TIM_SetCompare4(TIM3, duty_cycle); //设置定时3通道4的比较值，设置红灯的亮度
        TIM_SetCompare3(TIM3, duty_cycle); //设置定时3通道3的比较值，设置绿灯的亮度
        TIM_SetCompare2(TIM3, duty_cycle); //设置定时3通道2的比较值，设置蓝灯的亮度
        break;

    case kRED_LED:
        TIM_SetCompare4(TIM3, duty_cycle); //设置定时3通道4的比较值，设置红灯的亮度
        break;

    case kGREEN_LED:
        TIM_SetCompare3(TIM3, duty_cycle); //设置定时3通道3的比较值，设置绿灯的亮度
        break;

    case kBLUE_LED:
        TIM_SetCompare2(TIM3, duty_cycle); //设置定时3通道2的比较值，设置蓝灯的亮度
        break;

    case KYELLOW_LED:   
        TIM_SetCompare4(TIM3, duty_cycle); //设置定时3通道4的比较值，设置红灯的亮度
        TIM_SetCompare3(TIM3, duty_cycle); //设置定时3通道3的比较值，设置绿灯的亮度
        break;
    case KMAGENTA_LED:  
        TIM_SetCompare4(TIM3, duty_cycle); //设置定时3通道4的比较值，设置红灯的亮度
        TIM_SetCompare2(TIM3, duty_cycle); //设置定时3通道2的比较值，设置蓝灯的亮度
        break;
    case KCYAN_LED:     
        TIM_SetCompare3(TIM3, duty_cycle); //设置定时3通道3的比较值，设置绿灯的亮度
        TIM_SetCompare2(TIM3, duty_cycle); //设置定时3通道2的比较值，设置蓝灯的亮度
        break;

    default:
        break;
    }
}
