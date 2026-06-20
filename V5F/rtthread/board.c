/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-05-24                  the first version
 */

#include <rthw.h>
#include <rtthread.h>
#include "ch32h417.h"

#if defined(RT_USING_USER_MAIN) && defined(RT_USING_HEAP)
/*
 * Please modify RT_HEAP_SIZE if you enable RT_USING_HEAP
 * the RT_HEAP_SIZE max value = (sram size - ZI size), 1024 means 1024 bytes
 */
#define RT_HEAP_SIZE (15*1024)
static rt_uint8_t rt_heap[RT_HEAP_SIZE];

RT_WEAK void *rt_heap_begin_get(void)
{
    return rt_heap;
}

RT_WEAK void *rt_heap_end_get(void)
{
    return rt_heap + RT_HEAP_SIZE;
}
#endif

void rt_hw_us_delay(rt_uint32_t us)
{
    rt_uint32_t ticks;
    rt_uint32_t told, tnow, tcnt = 0;
    rt_uint32_t reload = SysTick0->CMP + 1;

    /* 获得延时经过的 tick 数 */
    ticks = us * reload / (1000000 / RT_TICK_PER_SECOND);
    /* 获得当前时间 */
    told = SysTick0->CNT;
    while (1)
    {
        /* 循环获得当前时间，直到达到指定的时间后退出循环 */
        tnow = SysTick0->CNT;
        if (tnow != told)
        {
            if (tnow < told)
            {
                // 取决于 SysTick->CNT 递增或递减，这里是递增的
                // 通过一个 tcnt 变量将当前计数值 tnow 与上一时刻的计数值 told 的差值进行累加（注意 SysTick->VAL 为递减还是递增计数器）
                // ，当累加值 tcnt≥延时节拍 ticks 时跳出循环，而 tcnt 最大值为 0xffff ffff，不可能出现死循环的情况
//                tcnt += told - tnow;
                tcnt += reload - told + tnow;
            }
            else
            {
//                tcnt += reload - tnow + told;
                tcnt += tnow - told;
            }
            told = tnow;
            if (tcnt >= ticks)
            {
                break;
            }
        }
    }
}

static rt_uint32_t SysTick_Config(rt_uint32_t ticks)
{
    NVIC_SetPriority(SysTick0_IRQn, 0xF0);
    NVIC_SetPriority(Software_IRQn, 0xF0);
    SysTick0->CTLR = 0;
    SysTick0->ISR = 0;
    SysTick0->CNT = 0;
    SysTick0->CMP = ticks - 1;
    SysTick0->CTLR |= 0x0F;   
    NVIC_EnableIRQ(SysTick0_IRQn);
    NVIC_EnableIRQ(Software_IRQn);
    return 0;
}

/**
 * This function will initial your board.
 */
void rt_hw_board_init(void)
{
    /* Shut down the internal 1.8V LDO */
    PWR_VIO18ModeCfg(PWR_VIO18CFGMODE_SW);
	PWR_VIO18LevelCfg(PWR_VIO18Level_MODE5);

    /* Update SystemClock and CoreClock variable */
    SystemAndCoreClockUpdate();

    /* OS Tick Configuration */
    SysTick_Config(HCLKClock / RT_TICK_PER_SECOND);

    /* Call components board initial (use INIT_BOARD_EXPORT()) */
#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif

#if defined(RT_USING_USER_MAIN) && defined(RT_USING_HEAP)
    rt_system_heap_init(rt_heap_begin_get(), rt_heap_end_get());
#endif

#ifdef RT_USING_CONSOLE
    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
#endif
}

void SysTick0_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void SysTick0_Handler(void)
{
    GET_INT_SP();
    /* enter interrupt */
    rt_interrupt_enter();
    SysTick0->ISR = 0;
    rt_tick_increase();
    /* leave interrupt */
    rt_interrupt_leave();
    FREE_INT_SP();
}