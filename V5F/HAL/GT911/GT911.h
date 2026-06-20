/********************************** (C) COPYRIGHT *******************************
 * File Name          : GT911.h
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2023/10/9
 * Description        :
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/

/******************************************************************************/
#ifndef _GT911_H_
#define _GT911_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "ch32h417.h"
#include "GT911_IIC.h"
#include "lcd.h"

/*********************************************************************
 * CONSTANTS
 */

#define GT911_IIC_READ_ADDRESS   0xBB   //0x29
#define GT911_IIC_WRITE_ADDRESS  0xBA   //0x28

#define GT911_ID_ADDRESS         0x8140 //id of touch ic(GT911)
#define GT911_CFGS_REG           0x8047
#define GT911_X_OUTPUT_MAX       0x8048
#define GT911_Y_OUTPUT_MAX       0x804A
#define GT911_MOUDULE_SWITCH1    0x804D

#define GT911_BUFFER_STATUS      0x814E //GT911当前检测到的触摸情况,第7位是触摸标志位，低4位是触摸点数个数
#define GT911_BUFFER_START       0x8150 //触摸点起始数据地址
#define GT911_BUFFER_TPD1        0x8150 //第一个触摸点数据地址
#define GT911_BUFFER_TPD2        0x8158
#define GT911_BUFFER_TPD3        0x8160
#define GT911_BUFFER_TPD4        0x8168
#define GT911_BUFFER_TPD5        0x8170

#define GT911_TOUCH_MAX_NUM      5

/*********************************************************************
 * TYPEDEFS
 */
typedef struct
{
   uint16_t PointX; //x坐标
   uint16_t PointY; //y坐标
   uint16_t PointS; //触摸点大小
}xy_coordinate_t;

typedef struct
{
    uint8_t TouchState;  //触摸状态
    uint8_t TouchNumber; //触摸数量
    xy_coordinate_t Touch_xy[GT911_TOUCH_MAX_NUM];
}Touch_data_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */
extern Touch_data_t touchData;

void GT911_init(void);
void GT911_get_point(uint8_t* pointData);
void GT911_read_register(uint16_t reg, uint8_t* buf, uint8_t length);
uint8_t GT911_write_register(uint16_t reg, uint8_t* buf, uint8_t length);
void GT911_touch_scanf(Touch_data_t* touchData);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* end of GT911.h */

