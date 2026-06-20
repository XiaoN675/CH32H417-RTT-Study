/********************************** (C) COPYRIGHT *******************************
 * File Name          : GT911_IIC.h
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2023/10/9
 * Description        :
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/

/******************************************************************************/
#ifndef _GT911_IIC_H_
#define _GT911_IIC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "ch32h417.h"

/*********************************************************************
 * CONSTANTS
 */

/*
 * LCD_TOUCH_SCL   PD7
 * LCD_TOUCH_SDA   PF3
 * LCD_TOUCH_INT   PD6
 * LCD_TOUCH_RST   PD2
 *
 * */
// GPIO设置
#define GT911_SCL_OUT()     {GPIOD->CFGLR &= 0x0FFFFFFF; GPIOD->CFGLR |= 7<<28;}    //PD7 开漏输出
#define GT911_SDA_IN()      {GPIOF->CFGLR &= 0xFFFF0FFF; GPIOF->CFGLR |= 4<<12;}    //PF3 浮空输入
#define GT911_SDA_OUT()     {GPIOF->CFGLR &= 0xFFFF0FFF; GPIOF->CFGLR |= 7<<12;}    //PF3 开漏输出
#define GT911_RST_OUT()     {GPIOD->CFGLR &= 0xFFFFF0FF; GPIOD->CFGLR |= 3<<8;}     //PD2 推挽输出
#define GT911_INT_OUT()     {GPIOD->CFGLR &= 0xF0FFFFFF; GPIOD->CFGLR |= 3<<24;}    //PD6 推挽输出
#define GT911_INT_IN()      {GPIOD->CFGLR &= 0xF0FFFFFF; GPIOD->CFGLR |= 4<<24;}    //PD6 浮空输入

#define GT911_SCL_SET()     (GPIOD->BSHR = GPIO_Pin_7)
#define GT911_SCL_CLR()     (GPIOD->BCR = GPIO_Pin_7)

#define GT911_SDA_SET()     (GPIOF->BSHR = GPIO_Pin_3)
#define GT911_SDA_CLR()     (GPIOF->BCR = GPIO_Pin_3)
#define GT911_SDA_READ()    (GPIOF->INDR & GPIO_Pin_3)

#define GT911_RST_SET()     (GPIOD->BSHR = GPIO_Pin_2)
#define GT911_RST_CLR()     (GPIOD->BCR = GPIO_Pin_2)

#define GT911_INT_SET()     (GPIOD->BSHR = GPIO_Pin_6)
#define GT911_INT_CLR()     (GPIOD->BCR = GPIO_Pin_6)
#define GT911_INT_READ()    (GPIOD->INDR & GPIO_Pin_6)

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

void GT911_iic_gpio_init(void);
void GT911_iic_start(void);
void GT911_iic_stop(void);
uint8_t GT911_iic_wait_ack(void);
void GT911_iic_send_byte(uint8_t data);
uint8_t GT911_iic_read_byte(uint8_t ack);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* end of GT911_IIC.h */

