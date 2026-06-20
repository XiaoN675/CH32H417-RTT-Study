/********************************** (C) COPYRIGHT *******************************
 * File Name          : GT911.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2023/10/9
 * Description        :
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/

/******************************************************************************/
/* 头文件包含 */
#include "GT911.h"
#if HAL_RT_TREAD_ENABLE
#include "rtthread.h"
#endif

/**************************************************************************************************
 *                                        GLOBAL VARIABLES
 **************************************************************************************************/
Touch_data_t touchData;
static volatile uint8_t GT911_interruptFlag = 0;

/**************************************************************************************************
 *                                        FUNCTIONS - Local
 **************************************************************************************************/
void GT911_reset(void)
{
    GT911_RST_CLR();
    Delay_Ms(10);
    GT911_RST_SET();
    Delay_Ms(10);
}

//set iic address 0xBA/0xBB
void GT911_set_iic_address(void)
{
    GT911_INT_CLR();
    GT911_RST_CLR();
    Delay_Ms(20);
    GT911_RST_SET();
    Delay_Ms(50);
    GT911_INT_IN();
}

void GT911_read_register(uint16_t reg, uint8_t* buf, uint8_t length)
{
    GT911_iic_start();
    GT911_iic_send_byte(GT911_IIC_WRITE_ADDRESS);
    GT911_iic_wait_ack();
    GT911_iic_send_byte(reg>>8);
    GT911_iic_wait_ack();
    GT911_iic_send_byte(reg&0xff);
    GT911_iic_wait_ack();
    GT911_iic_start();
    GT911_iic_send_byte(GT911_IIC_READ_ADDRESS);
    GT911_iic_wait_ack();

    for(uint8_t i=0; i<length; i++)
    {
        buf[i] = GT911_iic_read_byte(i==(length-1)?0:1);
    }
    GT911_iic_stop();
}

uint8_t GT911_write_register(uint16_t reg, uint8_t* buf, uint8_t length)
{
    uint8_t returnVlaue = 0;
    GT911_iic_start();
    GT911_iic_send_byte(GT911_IIC_WRITE_ADDRESS);
    GT911_iic_wait_ack();
    GT911_iic_send_byte(reg>>8);
    GT911_iic_wait_ack();
    GT911_iic_send_byte(reg&0xff);
    GT911_iic_wait_ack();
    for(uint8_t i=0; i<length; i++)
    {
        GT911_iic_send_byte(buf[i]);
        returnVlaue = GT911_iic_wait_ack();
        if(returnVlaue) break;
    }
    GT911_iic_stop();

    return returnVlaue;
}

void GT911_interrupt_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    EXTI_InitTypeDef EXTI_InitStructure = {0};

    RCC_HB2PeriphClockCmd(RCC_HB2Periph_AFIO | RCC_HB2Periph_GPIOD, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    /* GPIOD ----> EXTI_Line6 */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource6);
    EXTI_InitStructure.EXTI_Line = EXTI_Line6;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_SetPriority(EXTI7_0_IRQn, 10 << 4);
    NVIC_EnableIRQ(EXTI7_0_IRQn);
}

void GT911_init(void)
{
    uint8_t touchID[6]={0};

    GT911_iic_gpio_init();
    GT911_reset();
    GT911_set_iic_address();//0xBA/0xBB
    GT911_read_register(GT911_ID_ADDRESS, touchID, 4);
    printf("touchIC_ID = %s\r\n", touchID);

//    GT911_read_register(GT911_CFGS_REG, touchID, 6);
//    for(uint8_t i = 0; i < 6; i++)
//    {
//        hal_printf("touchID[%d] = 0x%x\r\n",i,touchID[i]);
//    }

    GT911_interrupt_init();
}

void GT911_touch_scanf(Touch_data_t* touchData)
{
    if(GT911_interruptFlag)
    {
        GT911_interruptFlag = 0;
        uint8_t temp = 0;
        GT911_read_register(GT911_BUFFER_STATUS, &temp, 1);
        touchData->TouchState = temp & 0x80;
        touchData->TouchNumber = temp & 0x0f;
        if(touchData->TouchNumber > GT911_TOUCH_MAX_NUM)
        {
            touchData->TouchState = 0;
            temp = 0;
            GT911_write_register(GT911_BUFFER_STATUS, &temp, 1);
        }

        if(touchData->TouchState == 0x80)
        {
            temp = 0;
            GT911_write_register(GT911_BUFFER_STATUS, &temp, 1);

            for(uint8_t i = 0; i < touchData->TouchNumber; i++)
            {
                GT911_read_register((GT911_BUFFER_START + i*8 + 0), &temp, 1); //读出触摸x坐标的低8位
                touchData->Touch_xy[i].PointY  = temp;
                GT911_read_register((GT911_BUFFER_START + i*8 + 1), &temp, 1); //读出触摸x坐标的高8位
                touchData->Touch_xy[i].PointY |= (temp<<8);

                GT911_read_register((GT911_BUFFER_START + i*8 + 2), &temp, 1); //读出触摸y坐标的低8位
                touchData->Touch_xy[i].PointX  = temp;
                GT911_read_register((GT911_BUFFER_START + i*8 + 3), &temp, 1); //读出触摸y坐标的高8位
                touchData->Touch_xy[i].PointX |= (temp<<8);
                touchData->Touch_xy[i].PointX = lcddev.width - touchData->Touch_xy[i].PointX - 1;

                GT911_read_register((GT911_BUFFER_START + i*8 + 4), &temp, 1); //读出触摸大小数据的低8位
                touchData->Touch_xy[i].PointS  = temp;
                GT911_read_register((GT911_BUFFER_START + i*8 + 5), &temp, 1); //读出触摸大小数据的高8位
                touchData->Touch_xy[i].PointS |= (temp<<8);
            }
        }
    }
}

void EXTI7_0_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI7_0_IRQHandler(void)
{
#if HAL_RT_TREAD_ENABLE
    GET_INT_SP();
    rt_interrupt_enter();
#endif

    if(EXTI_GetITStatus(EXTI_Line6) != RESET)
    {
        GT911_interruptFlag = 1;
        EXTI_ClearITPendingBit(EXTI_Line6);     /* Clear Flag */
    }

#if HAL_RT_TREAD_ENABLE
    rt_interrupt_leave();
    FREE_INT_SP();
#endif
}
