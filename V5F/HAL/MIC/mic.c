/********************************** (C) COPYRIGHT *******************************
 * File Name          : mic.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2023/10/9
 * Description        :
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/

/******************************************************************************/
/* 头文件包含 */
#include "mic.h"
#include "adc.h"
#if HAL_RT_TREAD_ENABLE
#include "rtthread.h"
#endif

/**************************************************************************************************
 *                                        GLOBAL VARIABLES
 **************************************************************************************************/

/**************************************************************************************************
 *                                        FUNCTIONS - Local
 **************************************************************************************************/
/*********************************************************************
 * @fn      mic_opa_init
 *
 * @brief   Initializes OPA3 collection.
 *
 * @return  none
 */
void mic_opa_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure={0};
    OPA_InitTypeDef  OPA_InitStructure={0};

    RCC_HB2PeriphClockCmd( RCC_HB2Periph_GPIOB | RCC_HB2Periph_GPIOF | RCC_HB2Periph_OPCM, ENABLE );//OPCM是用来开启内部OPA(运算放大器)和CM(比较器)的时钟

    /*
        PF11CHP，同向输入端
        PF12CHN，反相输入端
    */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOF, &GPIO_InitStructure);
    /*
        PB1，输出端
    */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    OPA_InitStructure.PSEL = CHP1; //放大器同向输入
    OPA_InitStructure.NSEL = CHN1; //放大器反向输入
    OPA_InitStructure.Mode = OUT_IO_OUT1; //放大器输出引脚
    OPA_InitStructure.PGADIF = DIF_OFF; //差分输入失能
    OPA_InitStructure.FB = FB_OFF;
	OPA_InitStructure.HS = HS_ON;
    OPA_Init(OPA2, &OPA_InitStructure);
    OPA_Cmd(OPA2, ENABLE);
}

/*********************************************************************
 * @fn      mic_adc_init
 *
 * @brief   Initializes ADC collection.
 *
 * @return  none
 */
void mic_adc_init(uint16_t *adc_buf, uint16_t len)
{
    GPIO_InitTypeDef GPIO_InitStructure={0};

    RCC_HB2PeriphClockCmd(RCC_HB2Periph_GPIOC, ENABLE );

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    adc1_continue_convert_init(adc_buf, len, MIC_ADC_CHANNEL, MIC_CONTROL_TIM_PERIOD, MIC_CONTROL_TIM_PRESCALER);
}

/*********************************************************************
 * @fn      mic_adc_capture_set
 *
 * @brief   Set ADC collection.
 *
 * @return  none
 */
void mic_adc_collection_set(uint8_t cap_en)
{
    if(cap_en) 
    {
        TIM_Cmd(TIM8, ENABLE);  //定时器使能后将会开启ADC采集
    }
    else 
    {
        TIM_Cmd(TIM8, DISABLE);  //定时器使能后将会开启ADC采集
    }
}

/* 重置麦克风录制内容 */
void mic_adc_reset_recording(uint16_t *record_buf, uint16_t len)
{
    DMA_Cmd(DMA1_Channel1, DISABLE);                //设置DMA结束数据的个数,DMA不使能才能够设置
    DMA1_Channel1->MADDR = (uint32_t)record_buf;    //mic_adc_buffer_1采集完成，切换到mic_adc_buffer_2
    DMA_SetCurrDataCounter(DMA1_Channel1, len);     //更新DMA传输长度
    DMA_Cmd(DMA1_Channel1, ENABLE);                 //启动ADC DMA
}

/* 去直流 */
int16_t dc_remove(int16_t input)
{
    static int32_t last_out = 0;
    static int32_t last_in = 0;
    float alpha = 0.995f; // α 取 0.995 ~ 0.999

    int32_t out = alpha * (last_out + input - last_in);

    last_in = input;
    last_out = out;

    return (int16_t)out;
}

/* 低通滤波 */
int16_t low_pass_filter(int16_t input)
{
    static int32_t y = 0;
    uint8_t k = 4; // 滤波强度：1~10之间，数值越大，滤波越强

    y = y + (input - y) / k;

    return (int16_t)y;
}

/* 麦克风音频数据滤波 */
void mic_audio_filter(uint16_t *data, uint32_t len)
{
    int16_t val;
    int32_t result;

    for(uint32_t i = 0; i < len; ++i) 
    {
        val = (int16_t)data[i] - MIC_ADC_MID; //转换成有符号数
        val = dc_remove(val); //去直流（使用返回值）
        val = low_pass_filter(val); //低通滤波（使用返回值）
        result = val + MIC_ADC_MID; //转回无符号数
        if(result < 0)
        {
            result = 0;
        }
        if(result > 4095)
        {
            result = 4095;
        }
        data[i] = (uint16_t)result;
    }
}