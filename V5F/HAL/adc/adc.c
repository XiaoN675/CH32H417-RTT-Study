/********************************** (C) COPYRIGHT *******************************
 * File Name          : adc.c
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2023/10/9
 * Description        :
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * SPDX-License-Identifier: Apache-2.0
 *******************************************************************************/

/******************************************************************************/
/* 头文件包含 */
#include "adc.h"

/**************************************************************************************************
 *                                        GLOBAL VARIABLES
 **************************************************************************************************/

/**************************************************************************************************
 *                                        FUNCTIONS - Local
 **************************************************************************************************/
/*******************************************************************************
 * @fn      adc1_single_convert_init
 *
 * @brief   ADC1初始化
 *
 * @param   none
 *
 * @return  none
 *******************************************************************************/
void adc1_single_convert_init(void)
{
    ADC_InitTypeDef  ADC_InitStructure = {0};
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    RCC_HB2PeriphClockCmd(RCC_HB2Periph_ADC1|RCC_HB2Periph_GPIOC, ENABLE );
    RCC_ADCCLKConfig(RCC_ADCCLKSource_HCLK);
    /* PPRE2_DIV0 = 保持APB2时钟不变（不改PPRE2，否则GPIO/I2C/TIM8全受影响） */
    RCC_ADCHCLKCLKAsSourceConfig(RCC_PPRE2_DIV0,RCC_HCLK_ADCPRE_DIV8);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    ADC_DeInit(ADC1);                                                       //复位ADC1,将外设 ADC1的全部寄存器重设为缺省值
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;                      //ADC1工作在独立模式
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;                           //关闭扫描模式
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;                     //关闭连续转换模式
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;     //转换由软件而不是外部触发启动
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;                  //ADC数据右对齐
    ADC_InitStructure.ADC_NbrOfChannel = 1;                                 //规则转换的ADC通道的数目
    ADC_Init(ADC1, &ADC_InitStructure);                                     //ADC1初始化
    ADC_Cmd(ADC1, ENABLE);                                              //使能ADC1

    ADC_BufferCmd(ADC1, DISABLE);
    ADC_ResetCalibration(ADC1);                     //使能复位校准
    while(ADC_GetResetCalibrationStatus(ADC1));     //等待复位校准结束
    ADC_StartCalibration(ADC1);                     //开启ADC校准
    while(ADC_GetCalibrationStatus(ADC1));          //等待校准结束
}

/*******************************************************************************
 * @fn      adc_value_get
 *
 * @brief   获取ADC(单次采集)
 *
 * @param   ch - 采样通道选择
 *          sample_time - 采样时间选择
 *
 * @return  ADC的值
 *******************************************************************************/
uint16_t adc1_single_convert_value_get(uint8_t ch, uint8_t sample_time)
{
    uint16_t val;

    ADC_RegularChannelConfig(ADC1, ch, 1, sample_time);     //设置指定ADC的规则组通道，一个序列
    ADC_ClearFlag(ADC1, ADC_FLAG_EOC);                      //清楚EOC标志以防前一次转换未被读取
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);                 //使能指定的ADC1的软件转换启动功能
    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));          //等待转换结束
    val = ADC_GetConversionValue(ADC1);                     //ADC1规则组的转换结果

    return val;
}

/*******************************************************************************
 * @fn      adc_average_value_get
 *
 * @brief   获取ADC(多次取平均值)
 *
 * @param   ch - 采样通道选择
 *          times - 采集次数
 *          sample_time - 采样时间选择
 *
 * @return  (uint16_t)ADC的值
 *******************************************************************************/
uint16_t adc1_single_convert_average_value_get(uint8_t ch, uint8_t times, uint8_t sample_time)
{
    uint32_t val_sum = 0;
    uint16_t val = 0;
    uint8_t i;

    adc1_single_convert_value_get(ch, ADC_SampleTime_CyclesMode7); //第一次转换值丢弃
    Delay_Us(50);

    ADC_RegularChannelConfig(ADC1, ch, 1, sample_time);     //设置指定ADC的规则组通道，一个序列
    ADC_ClearFlag(ADC1, ADC_FLAG_EOC);                      //清楚EOC标志以防前一次转换未被读取
    for(i = 0; i < times; ++i)
    {
        ADC_SoftwareStartConvCmd(ADC1, ENABLE);             //使能指定的ADC1的软件转换启动功能
        while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));      //等待转换结束
        val_sum += ADC_GetConversionValue(ADC1);            //ADC1规则组的转换结果
        Delay_Us(50);
    }

    val = (val_sum * 10 / times + 5) / 10;                  //计算平均值，四舍五入

    return val;
}

/*******************************************************************************
 * @fn      adc_voltage_get
 *
 * @brief   ADC实际电压值计算
 *
 * @param   val - ADC的值
 *
 * @return  实际电压值,单位为mV
 *******************************************************************************/
uint32_t adc1_voltage_get(uint16_t val)
{
    return (((((uint32_t) val * 8250) >> 10) + 5) / 10); // val / 4096 * 3300 + 0.5 四舍五入
}

/*******************************************************************************
 * @fn      adc1_continue_convert_init
 *
 * @brief   ADC连续转换初始化
 *
 * @param   adc_buf - 用于保存ADC采集数值的缓冲区
 *
 * @return  
 *******************************************************************************/
void adc1_continue_convert_init(uint16_t *adc_buf, uint16_t len, uint8_t channel, uint16_t arr, uint16_t psc)
{
    ADC_InitTypeDef ADC_InitStructure={0};
    DMA_InitTypeDef DMA_InitStructure={0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};

    RCC_HBPeriphClockCmd(RCC_HBPeriph_DMA1, ENABLE);
    RCC_HB2PeriphClockCmd(RCC_HB2Periph_AFIO | RCC_HB2Periph_ADC1 | RCC_HB2Periph_TIM8, ENABLE );
    RCC_ADCCLKConfig(RCC_ADCCLKSource_HCLK);
    /* PPRE2_DIV0 = 保持APB2时钟不变 */
    RCC_ADCHCLKCLKAsSourceConfig(RCC_PPRE2_DIV0,RCC_HCLK_ADCPRE_DIV8);

    ADC_DeInit(ADC1);                                                       //复位ADC1,将外设 ADC1的全部寄存器重设为缺省值
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;                      //ADC1工作在独立模式
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;                           //关闭扫描模式
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;                     //关闭连续转换模式
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_Ext_IT11_TIM8_TRGO;  //转换由外部触发启动
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;                  //ADC数据右对齐
    ADC_InitStructure.ADC_NbrOfChannel = 1;                                 //规则转换的ADC通道的数目
    ADC_Init(ADC1, &ADC_InitStructure);                                     //ADC1初始化
    ADC_Cmd(ADC1, ENABLE);

    ADC_BufferCmd(ADC1, DISABLE);
    ADC_ResetCalibration(ADC1);                     //使能复位校准
    while(ADC_GetResetCalibrationStatus(ADC1));     //等待复位校准结束
    ADC_StartCalibration(ADC1);                     //开启ADC校准
    while(ADC_GetCalibrationStatus(ADC1));          //等待校准结束

    DMA_DeInit(DMA1_Channel1);                                              //复位DMA Channel1
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->RDATAR;     //从ADC的规则数据寄存器获取数据
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)adc_buf;               //ADC数据接收缓冲区
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;                      //数据源来自于ADC
    DMA_InitStructure.DMA_BufferSize = len;                                 //缓冲区的大小
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;        //ADC数据源的地址不要递增
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                 //缓冲区的地址要递增
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; //外设数据大小为半字
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;     //缓冲区数据大小为半字
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                        //循环模式，持续采集
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;                 // 通道优先级为中
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                            //不是缓冲区到缓冲区
    DMA_Init( DMA1_Channel1, &DMA_InitStructure );
    DMA_MuxChannelConfig(DMA_MuxChannel1, 120); 

    /* 不使用传输完成中断 — 裸机轮询即可 */
    DMA_Cmd(DMA1_Channel1, ENABLE); 
    ADC_DMACmd(ADC1, ENABLE);

    //ADC外部触发定时器初始化
    TIM_TimeBaseInitStructure.TIM_Period = arr;
    TIM_TimeBaseInitStructure.TIM_Prescaler = psc;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM8, &TIM_TimeBaseInitStructure);

    TIM_ARRPreloadConfig(TIM8, ENABLE);
    TIM_SelectOutputTrigger(TIM8, TIM_TRGOSource_Update);       //选择定时器更新作为触发源
    ADC_ExternalTrigConvCmd(ADC1, ENABLE);                      //使能ADC外部触发转换功能
    /* 直接置位 PCFR1 的 BIT1 连接 ADC1_ETRGREG 与 TIM8_TRGO */
    /* 注意：不走 GPIO_PinRemapConfig — 它会破坏 SWJ_CFG 导致按键 GPIO 异常 */
    AFIO->PCFR1 |= (uint32_t)0x00000002;

    ADC_RegularChannelConfig(ADC1, channel, 1, ADC_SampleTime_CyclesMode7 ); //设置指定ADC的规则组通道，一个序列
    // TIM_Cmd(TIM8, ENABLE);  //定时器使能后将会开启ADC采集
}