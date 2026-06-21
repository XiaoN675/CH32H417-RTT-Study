/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Description        : RT-Thread Event example
 *********************************************************************************
 * Copyright (c) 2025 Nanjing Qinheng Microelectronics Co., Ltd.
 *******************************************************************************/

/*
 * ════════════════════════════════════════════════════════
 *  RTT 学习 —— 事件（Event）
 *
 *  学习目标：
 *    1. 掌握 rt_event_create / send / recv
 *    2. 理解"等任意一个" vs "等全部" 两种模式
 *    3. 用一个线程同时响应按键和定时器
 *
 *  硬件：
 *    RGB LED — PC7(蓝) PC8(绿) PC9(红)
 *    按键    — PA8(KEY1) PA9(KEY2) PA15(KEY4)
 * ════════════════════════════════════════════════════════
 */

#include <rtthread.h>
#include "ch32h417.h"

/*===============================
 *  定义事件标志位（一个 bit 代表一种事件）
 *================================
 *  事件是一个 rt_uint32_t，最多 32 个独立的标志位
 *  每个 bit 可以独立置位和等待
 *===============================*/
#define EVENT_GREEN     (1 << 0)    /* bit 0：亮绿灯 */
#define EVENT_RED       (1 << 1)    /* bit 1：亮红灯 */
#define EVENT_OFF       (1 << 2)    /* bit 2：关灯 */
#define EVENT_BLINK     (1 << 3)    /* bit 3：闪烁一次 */

static rt_event_t led_event = RT_NULL;

/*===============================
 *  LED 线程：等事件，处理事件
 *===============================*/
static void led_thread_entry(void *parameter)
{
    (void)parameter;

    /* 初始化 RGB LED */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_HB2PeriphClockCmd(RCC_HB2Periph_GPIOC, ENABLE);
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_Very_High;
    GPIO_Init(GPIOC, &GPIO_InitStruct);
    GPIO_SetBits(GPIOC, GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9);

    rt_uint32_t recv_set;

    while (1)
    {
        /*
         * 等待事件——三种事件中的"任意一个"发生
         *
         * rt_event_recv(事件,  要等的标志,  模式,      超时,     &收到的标志)
         *
         * 模式：
         *   RT_EVENT_FLAG_AND  — 等全部指定的 bit 都置位才返回
         *   RT_EVENT_FLAG_OR   — 等任意一个指定的 bit 置位就返回
         *
         * 可以加 CLEAR 标志让内核自动清零（不用手动清）
         */
        if (rt_event_recv(led_event,
                          EVENT_GREEN | EVENT_RED | EVENT_OFF | EVENT_BLINK,
                          RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
                          RT_WAITING_FOREVER,
                          &recv_set) == RT_EOK)
        {
            /* 先全灭 */
            GPIO_SetBits(GPIOC, GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9);

            if (recv_set & EVENT_GREEN)
            {
                GPIO_ResetBits(GPIOC, GPIO_Pin_8);
                rt_kprintf("[LED] GREEN (event bit 0)\r\n");
            }
            if (recv_set & EVENT_RED)
            {
                GPIO_ResetBits(GPIOC, GPIO_Pin_9);
                rt_kprintf("[LED] RED (event bit 1)\r\n");
            }
            if (recv_set & EVENT_OFF)
            {
                rt_kprintf("[LED] OFF (event bit 2)\r\n");
            }
            if (recv_set & EVENT_BLINK)
            {
                /* 闪烁一次：亮→等→灭 */
                rt_kprintf("[LED] BLINK (event bit 3)\r\n");
                for (int i = 0; i < 3; i++)
                {
                    GPIO_ResetBits(GPIOC, GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9);
                    rt_thread_mdelay(100);
                    GPIO_SetBits(GPIOC, GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9);
                    rt_thread_mdelay(100);
                }
            }
        }
    }
}

/*===============================
 *  按键扫描线程：按不同键发不同事件
 *===============================*/
static void key_scan_thread_entry(void *parameter)
{
    (void)parameter;

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_HB2PeriphClockCmd(RCC_HB2Periph_GPIOA, ENABLE);
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_15;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    while (1)
    {
        rt_uint32_t event_set = 0;

        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8) == RESET)      /* KEY1 */
        {
            event_set = EVENT_GREEN;
            while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8) == RESET)
                rt_thread_mdelay(10);
        }
        else if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_9) == RESET)  /* KEY2 */
        {
            event_set = EVENT_RED;
            while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_9) == RESET)
                rt_thread_mdelay(10);
        }
        else if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_15) == RESET) /* KEY4 */
        {
            event_set = EVENT_BLINK;
            while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_15) == RESET)
                rt_thread_mdelay(10);
        }

        if (event_set)
        {
            /*
             * 发送事件：可以同时发多个 bit
             * rt_event_send(事件, 标志位)
             */
            rt_kprintf("[KEY] send event 0x%x\r\n", event_set);
            rt_event_send(led_event, event_set);
        }

        rt_thread_mdelay(50);
    }
}

/*===============================
 *  自动初始化
 *===============================*/
static int rt_app_init(void)
{
    /* 1. 创建事件
     *    rt_event_create(名字, 标志) */
    led_event = rt_event_create("led_evt", RT_IPC_FLAG_FIFO);
    if (led_event == RT_NULL)
    {
        rt_kprintf("rt_event_create failed!\r\n");
        return -1;
    }

    /* 2. 创建 LED 线程 */
    rt_thread_t tid;
    tid = rt_thread_create("led", led_thread_entry,
                           RT_NULL, 1024, 12, 10);
    if (tid) rt_thread_startup(tid);

    /* 3. 创建按键扫描线程 */
    tid = rt_thread_create("key_scan", key_scan_thread_entry,
                           RT_NULL, 512, 14, 10);
    if (tid) rt_thread_startup(tid);

    rt_kprintf("═══════════════════════════════════════\r\n");
    rt_kprintf("  事件（Event）示例\r\n");
    rt_kprintf("  KEY1 → 发 EVENT_GREEN (bit 0)\r\n");
    rt_kprintf("  KEY2 → 发 EVENT_RED   (bit 1)\r\n");
    rt_kprintf("  KEY4 → 发 EVENT_BLINK (bit 3)\r\n");
    rt_kprintf("  LED 线程等上面任意事件，收到就执行\r\n");
    rt_kprintf("═══════════════════════════════════════\r\n");

    return 0;
}

INIT_APP_EXPORT(rt_app_init);

int main(void)
{
    return 0;
}
