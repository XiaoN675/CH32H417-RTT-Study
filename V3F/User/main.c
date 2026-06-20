/********************************** (C) COPYRIGHT *******************************
 * File Name          : main_v3f.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2025/03/01
 * Description        : Main program body for V3F.
 *********************************************************************************
 * Copyright (c) 2025 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#include "debug.h"

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
    SystemInit();
    SystemAndCoreClockUpdate();
    // Delay_Init();
    // USART_Printf_Init(115200);
    // printf("V3F SystemCoreClock: %d\r\n", SystemCoreClock);
    // printf("HCLKClock: %d\r\n", HCLKClock);
    
    NVIC_WakeUp_V5F(Core_V5F_StartAddr); //Wake up V5
    PWR_EnterSTOPMode(PWR_Regulator_ON, PWR_STOPEntry_WFE);

    while(1);
}
