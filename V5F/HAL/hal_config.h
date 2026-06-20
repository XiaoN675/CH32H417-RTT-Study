/********************************** (C) COPYRIGHT *******************************
 * File Name          : hal_config.h
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2023/10/9
 * Description        :
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/

/******************************************************************************/
#ifndef __HAL_CONFIG_H
#define __HAL_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "hal.h"
#include "led.h"
#include "key.h"


/*********************************************************************
 * CONSTANTS
 */
 /* 配置使用裸机代码还是使用RT-Thread */
#ifndef HAL_RT_TREAD_ENABLE
#define HAL_RT_TREAD_ENABLE         1
#endif

#if HAL_RT_TREAD_ENABLE
/* 头文件包含 */
#include "rtthread.h"
#include "rthw.h"

/* 调试信息打印函数 */
#define hal_printf                  rt_kprintf
/* ms级延时函数 */
#define hal_delay_ms                rt_thread_mdelay
/* us级延时函数 */
#define hal_delay_us                rt_hw_us_delay
/* 内存复制函数 */
#define hal_memcpy                  rt_memcpy
/* 内存设置函数 */
#define hal_memset                  rt_memset
/* 内存比较函数 */
#define hal_memcmp                  rt_memcmp
#else 
/* 调试信息打印函数 */
#define hal_printf                  printf
/* ms级延时函数 */
#define hal_delay_ms                Delay_Ms
/* us级延时函数 */
#define hal_delay_us                Delay_Us
/* 内存复制函数 */
#define hal_memcpy                  my_memcpy
/* 内存设置函数 */
#define hal_memset                  my_memset
/* 内存比较函数 */
#define hal_memcmp                  my_memcmp
#endif

/* 求绝对值 */
#ifndef ABS
#define ABS(n)                      (((n) < 0) ? -(n) : (n))
#endif

/* 求最大值 */
#ifndef MAX
#define MAX(m, n)                   (((m) > (n)) ? (m) : (n))
#endif

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif
