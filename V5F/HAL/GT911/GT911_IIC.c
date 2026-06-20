/********************************** (C) COPYRIGHT *******************************
 * File Name          : GT911_IIC.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2023/10/9
 * Description        :
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/

/******************************************************************************/
/* 头文件包含 */
#include "GT911_IIC.h"

#define DELAY_TIME_US 50

/**************************************************************************************************
 *                                        GLOBAL VARIABLES
 **************************************************************************************************/

/**************************************************************************************************
 *                                        FUNCTIONS - Local
 **************************************************************************************************/
void GT911_iic_gpio_init(void)
{
    RCC_HB2PeriphClockCmd(RCC_HB2Periph_GPIOD | RCC_HB2Periph_GPIOF, ENABLE);
    
    GT911_SCL_OUT();
    GT911_SDA_OUT();
    GT911_RST_OUT();
    GT911_INT_OUT();

    GT911_SCL_SET();
    GT911_SDA_SET();
    GT911_INT_SET();
    GT911_RST_SET();
}

void GT911_iic_start(void)
{
    GT911_SDA_OUT();
    GT911_SCL_SET();
    GT911_SDA_SET();
    Delay_Us(DELAY_TIME_US);
    GT911_SDA_CLR();
    Delay_Us(DELAY_TIME_US);
    GT911_SCL_CLR();
    Delay_Us(DELAY_TIME_US);
}

void GT911_iic_stop(void)
{
    GT911_SDA_OUT();
    GT911_SCL_CLR();
    GT911_SDA_CLR();
    Delay_Us(DELAY_TIME_US);
    GT911_SCL_SET();
    Delay_Us(DELAY_TIME_US);
    GT911_SDA_SET();
}

void GT911_iic_ack(void)
{
    GT911_SCL_CLR();
    GT911_SDA_OUT();
    GT911_SDA_CLR();
    Delay_Us(DELAY_TIME_US);
    GT911_SCL_SET();
    Delay_Us(DELAY_TIME_US);
    GT911_SCL_CLR();

}

void GT911_iic_nak(void)
{
    GT911_SCL_CLR();
    GT911_SDA_OUT();
    GT911_SDA_SET();
    Delay_Us(DELAY_TIME_US);
    GT911_SCL_SET();
    Delay_Us(DELAY_TIME_US);
    GT911_SCL_CLR();
}

uint8_t GT911_iic_wait_ack(void)
{
    uint16_t waitTime = 0;
    GT911_SDA_IN();
    GT911_SDA_SET();
    Delay_Us(DELAY_TIME_US);
    GT911_SCL_SET();
    Delay_Us(DELAY_TIME_US);
    while(GT911_SDA_READ())
    {
        waitTime++;
        if(waitTime > 750)
        {
            GT911_iic_stop();
            return 1;
        }
    }
    GT911_SCL_CLR();
    Delay_Us(DELAY_TIME_US);
    return 0;
}

void GT911_iic_send_byte(uint8_t data)
{
    GT911_SCL_CLR();
    GT911_SDA_OUT();

    for(uint8_t i = 0; i < 8; i++)
    {
        if((data & 0x80) >> 7)
        {
            GT911_SDA_SET();
        }
        else
        {
            GT911_SDA_CLR();
        }
        data <<= 1;
        Delay_Us(5);
        GT911_SCL_SET();
        Delay_Us(DELAY_TIME_US);
        GT911_SCL_CLR();
        Delay_Us(DELAY_TIME_US);
    }
}

/*
 * ack = 1, send ack
 * ack = 0, send nak
 */
uint8_t GT911_iic_read_byte(uint8_t ack)
{
   uint8_t receiveData = 0;
   GT911_SDA_IN();
   for(uint8_t i = 0; i < 8; i++)
   {
       GT911_SCL_CLR();
       Delay_Us(DELAY_TIME_US);
       GT911_SCL_SET();
       Delay_Us(DELAY_TIME_US);
       receiveData <<= 1;
       if(GT911_SDA_READ()) receiveData++;
   }
   if(ack)
   {
       GT911_iic_ack();
   }
   else
   {
       GT911_iic_nak();
   }

   return receiveData;
}
