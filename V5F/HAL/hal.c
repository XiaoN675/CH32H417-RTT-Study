/********************************** (C) COPYRIGHT *******************************
 * File Name          : hal.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2023/10/9
 * Description        :
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/

/******************************************************************************/
/* 头文件包含 */
#include "hal.h"

/**************************************************************************************************
 *                                        GLOBAL VARIABLES
 **************************************************************************************************/

/**************************************************************************************************
 *                                        FUNCTIONS - Local
 **************************************************************************************************/
/*********************************************************************
 * @fn      my_memcpy
 *
 * @brief   内存数据复制
 *
 * @param   pdst - 目的内存地址
 *          psrc - 源内存地址
 *          len - 复制的字节长度
 *
 * @return  none
 */
void my_memcpy(void *pdst, const void *psrc, uint32_t len)
{
    uint8_t *dst;
    const uint8_t *src;
    if( pdst == 0 || psrc == 0 || len == 0 ){
        return;
    }

    src = (uint8_t *)psrc;
    dst = (uint8_t *)pdst;
    do{
        *dst++ = *src++;
    }while( --len );
}

/*********************************************************************
 * @fn      hal_memset
 *
 * @brief   内存数据值设置
 *
 * @param   pdst - 设置的内存地址
 *          value - 设置的值
 *          len - 设置的字节长度
 *
 * @return  none
 */
void my_memset(void *pdst, int8_t value, uint32_t len)
{
    uint8_t *dst;
    if( pdst == 0 || len == 0 )
    {
        return;
    }

    dst = (uint8_t *)pdst;
    do{
        *dst++ = (uint8_t)value;
    }while(--len );
}

/*********************************************************************
 * @fn      hal_memcmp
 *
 * @brief   内存数据值比较
 *
 * @param   psrc1 - 比较的内存地址1
 *          psrc2 - 比较的内存地址2
 *          len - 比较的字节长度
 *
 * @return  0 - 内存地址1和内存地址2的数据相同
 *          1 - 内存地址1比内存地址2的数据大
 *          -1 - 内存地址1比内存地址2的数据小
 */
int8_t my_memcmp(const void *psrc1, const void *psrc2, uint32_t len)
{
    const uint8_t *src1 = (const uint8_t *)psrc1;
    const uint8_t *src2 = (const uint8_t *)psrc2;
    uint32_t i;

    for(i = 0; i < len; ++i) 
    {
        if(src1[i] < src2[i])
        {
            return -1; //psrc1 < psrc2
        }
        else if(src1[i] > src2[i])
        {
            return 1; //psrc1 > psrc2
        }
    }
    return 0; //内存数据相等
}
