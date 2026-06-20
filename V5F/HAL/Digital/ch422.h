/********************************** (C) COPYRIGHT *******************************
 * File Name          : ch422.h
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2023/10/9
 * Description        :
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/

/******************************************************************************/
#ifndef _CH442_H_
#define _CH442_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "ch32h417.h"

/*********************************************************************
 * CONSTANTS
 */
#define CH422_SCL_GPIO      GPIOC
#define CH422_SDA_GPIO      GPIOC
#define CH422_SCL_PIN       GPIO_Pin_14 //SCL-PC14
#define CH422_SDA_PIN       GPIO_Pin_15 //SDA-PC15

/* IO方向设置 */
#define CH422_SDA_IN()      do{CH422_SDA_GPIO->CFGHR &= 0x0FFFFFFF; CH422_SDA_GPIO->CFGHR |= 4<<28;} while(0)    //PC15输入模式
#define CH422_SDA_OUT()     do{CH422_SDA_GPIO->CFGHR &= 0x0FFFFFFF; CH422_SDA_GPIO->CFGHR |= 7<<28;} while(0)    //PC15输出模式

/* IO输出设置 */
#define CH422_SCL_0()       (GPIO_ResetBits(CH422_SCL_GPIO, CH422_SCL_PIN))         //SCL
#define CH422_SCL_1()       (GPIO_SetBits(CH422_SCL_GPIO, CH422_SCL_PIN))
#define CH422_SDA_0()       (GPIO_ResetBits(CH422_SDA_GPIO, CH422_SDA_PIN))         //SDA
#define CH422_SDA_1()       (GPIO_SetBits(CH422_SDA_GPIO, CH422_SDA_PIN))

/* IO输入获取 */
#define CH422_READ_SDA()    (GPIO_ReadInputDataBit(CH422_SDA_GPIO, CH422_SDA_PIN))  //SDA输入

/* CH422接口参数 */
#define CH422_I2C_ADDR1     0x40    //CH422的地址
#define CH422_I2C_MASK      0x3E    //CH422的高字节命令掩码

/* CH422接口命令 */
#define CH422_SYS_CMD       0x48    //设置系统参数命令，默认方式
#define BIT_OD_EN           0x10    //DIG 输出配置，0-推挽输出，1—开漏输出
#define BIT_A_SCAN          0x04    //动态自动扫描，
#define BIT_IO_OE           0x01    //控制双向输入输出引脚的三态输出，为1允许输出
#define CH422_ADDRESS_1     0x70    //数码管1
#define CH422_ADDRESS_2     0x72    //数码管2
#define CH422_ADDRESS_3     0x74    //数码管3
#define CH422_ADDRESS_4     0x76    //数码管4
#define CH422_RD_IO_CMD     0x4D    //输入I/O引脚当前状态

/*********************************************************************
 * GLOBAL VARIABLES
 */
extern const uint8_t bcdDecode[];

/**
 * @brief   CH422驱动接口初始化
 */
void ch422_init(void);

/**
 * @brief   CH422字节写入
 */
void ch422_write_two_bytes(uint8_t data1, uint8_t data2);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif

