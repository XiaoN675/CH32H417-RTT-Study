/********************************** (C) COPYRIGHT *******************************
 * File Name          :speaker.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2023/10/9
 * Description        :
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/

/******************************************************************************/
/* 头文件包含 */
#include "speaker.h"
#include "mic.h"


/**************************************************************************************************
 *                                        GLOBAL VARIABLES
 **************************************************************************************************/


/**************************************************************************************************
 *                                        FUNCTIONS - Local
 **************************************************************************************************/
/* 扬声器DAC输出初始化 */
void speaker_dac_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure={0};
    DAC_InitTypeDef DAC_InitStructure={0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};
    DMA_InitTypeDef DMA_InitStructure={0};

    RCC_HBPeriphClockCmd(RCC_HBPeriph_DMA2, ENABLE);
    RCC_HB1PeriphClockCmd(RCC_HB1Periph_DAC | RCC_HB1Periph_TIM6, ENABLE );
    RCC_HB2PeriphClockCmd(RCC_HB2Periph_GPIOA, ENABLE );

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 配置DAC通道1
    DAC_InitStructure.DAC_Trigger = DAC_Trigger_T6_TRGO;  // 使用TIM6触发
    DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
    DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
    DAC_Init(DAC_Channel_1, &DAC_InitStructure);
    DAC_Cmd(DAC_Channel_1, ENABLE); // 使能DAC通道1

    DMA_DeInit(DMA2_Channel3);                                              //复位DMA Channel1
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&DAC->R12BDHR1;    //写入DAC数据寄存器
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)NULL;                  //DAC数据发送缓冲区
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;                      //数据目标地址是DAC
    DMA_InitStructure.DMA_BufferSize = 0;                                   //缓冲区的大小
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;        //DAC数据源的地址不要递增
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                 //缓冲区的地址要递增
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; //外设数据大小为半字
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;     //缓冲区数据大小为半字
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                           //DMA存储模式不要循环
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;                 //通道优先级为中
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                            //不是缓冲区到缓冲区
    DMA_Init( DMA2_Channel3, &DMA_InitStructure );
    DMA_MuxChannelConfig(DMA_MuxChannel11, 103); 
    DAC_DMACmd(DAC_Channel_1, ENABLE);

    TIM_TimeBaseInitStructure.TIM_Period = MIC_CONTROL_TIM_PERIOD;          //自动重装值
    TIM_TimeBaseInitStructure.TIM_Prescaler = MIC_CONTROL_TIM_PRESCALER;    //预分频器
    TIM_TimeBaseInitStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM6, &TIM_TimeBaseInitStructure);

    TIM_ARRPreloadConfig(TIM6, ENABLE);
    TIM_SelectOutputTrigger(TIM6, TIM_TRGOSource_Update);
    TIM_Cmd(TIM6, ENABLE);
}

/* 扬声器播放录制的声音 */
void speaker_dac_play_recording(uint16_t *record_buf, uint16_t len)
{
    DMA_Cmd(DMA2_Channel3, DISABLE);                //设置DMA结束数据的个数,DMA不使能才能够设置
    DMA2_Channel3->MADDR = (uint32_t)record_buf;   //dac输出
    DMA_SetCurrDataCounter(DMA2_Channel3, len);     //更新DMA传输长度
    DMA_Cmd(DMA2_Channel3, ENABLE);                 //启动DAC DMA
}

/* 扬声器连续循环播放（Circular DMA模式），用于实时麦克风监听 */
void speaker_dac_play_continuous(uint16_t *record_buf, uint16_t len)
{
    DMA_Cmd(DMA2_Channel3, DISABLE);
    DMA2_Channel3->MADDR = (uint32_t)record_buf;
    DMA_SetCurrDataCounter(DMA2_Channel3, len);
    /* 切换到循环模式（Circular） */
    DMA2_Channel3->CFGR |= DMA_Mode_Circular;
    DMA_Cmd(DMA2_Channel3, ENABLE);
}
