/********************************** (C) COPYRIGHT *******************************
 * File Name          : ch422.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2023/10/9
 * Description        :
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/

/******************************************************************************/
/* 头文件包含 */
#include "ch422.h"


/**************************************************************************************************
 *                                        GLOBAL VARIABLES
 **************************************************************************************************/
//                               0     1     2     3     4     5     6     7     8     9   point  空 
const uint8_t bcdDecode[12] = {0xD7, 0x14, 0xCD, 0x5D, 0x1E, 0x5B, 0xDB, 0x15, 0xDF, 0x5F, 0x20, 0x00};


/**************************************************************************************************
 *                                        FUNCTIONS - Local
 **************************************************************************************************/
//CH422驱动GPIO初始化, PE0:SCL, PE1:SDA
void ch422_gpio_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure={0};

    RCC_HB2PeriphClockCmd( RCC_HB2Periph_GPIOC, ENABLE);

    GPIO_InitStructure.GPIO_Pin = CH422_SCL_PIN|CH422_SDA_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Very_High;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOC, CH422_SCL_PIN|CH422_SDA_PIN);

    CH422_SCL_1();
    CH422_SDA_1();
}

void ch422_i2c_start(void)
{
    CH422_SDA_OUT();
    CH422_SCL_1();
    CH422_SDA_1();
    Delay_Us(10);
    CH422_SDA_0();
    Delay_Us(10);
    CH422_SCL_0();
}

void ch422_i2c_stop(void)
{
    CH422_SDA_0();
    Delay_Us(10);
    CH422_SCL_1();
    Delay_Us(10);
    CH422_SDA_1();
    Delay_Us(10);
}

void ch422_i2c_wirte_byte(uint8_t data)
{
    for(uint8_t i=0; i<8; i++)
    {
        if(data & 0x80)
        {
            CH422_SDA_1();
        }
        else
        {
            CH422_SDA_0();
        }
        Delay_Us(10);
        CH422_SCL_1();
        data <<= 1;
        Delay_Us(10);
        CH422_SCL_0();
    }
    CH422_SDA_1();
    Delay_Us(5);
    CH422_SCL_1();
    Delay_Us(5);
    CH422_SCL_0();
}

uint8_t ch422_i2c_read_byte(void)
{
    uint8_t data = 0;
    CH422_SDA_1();
    CH422_SDA_IN();
    for(uint8_t i=0; i<8; i++)
    {
        Delay_Ms(1);
        CH422_SCL_1();
        Delay_Ms(1);
        data <<= 1;
        if(CH422_READ_SDA()) data++;
        CH422_SCL_0();
    }

    CH422_SDA_OUT();
    CH422_SDA_1();
    Delay_Ms(1);
    CH422_SCL_1();
    Delay_Ms(1);
    CH422_SCL_0();

    return data;
}

void ch422_write_byte(uint8_t data)
{
    ch422_i2c_start();
    ch422_i2c_wirte_byte(data);
    ch422_i2c_stop();
}

void ch422_write_two_bytes(uint8_t data1, uint8_t data2)
{
    ch422_i2c_start();
    ch422_i2c_wirte_byte(data1);
    ch422_i2c_wirte_byte(data2);
    ch422_i2c_stop();
}

uint8_t ch422_read_byte(void)
{
    uint8_t data = 0;

    ch422_i2c_start();
    ch422_i2c_wirte_byte( CH422_RD_IO_CMD );
    data = ch422_i2c_read_byte();
    ch422_i2c_stop();

    return data;
}

//CH422驱动接口初始化
void ch422_init(void)
{
    ch422_gpio_init();

    //CH422初始化
    ch422_write_two_bytes(CH422_SYS_CMD, 0x05);
    ch422_write_two_bytes(CH422_ADDRESS_1, 0);
    ch422_write_two_bytes(CH422_ADDRESS_2, 0);
    ch422_write_two_bytes(CH422_ADDRESS_3, 0);
    ch422_write_two_bytes(CH422_ADDRESS_4, 0);
}
