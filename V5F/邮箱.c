/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Description        : RT-Thread Mailbox example
 *********************************************************************************
 * Copyright (c) 2025 Nanjing Qinheng Microelectronics Co., Ltd.
 *******************************************************************************/

/*
 * ════════════════════════════════════════════════════════
 *  RTT 学习 —— 第三步：邮箱（Mailbox）
 *
 *  学习目标：
 *    1. 掌握 rt_mb_create / send / recv
 *    2. 理解"邮箱传的是指针大小的值（4字节）"
 *    3. 用邮箱在线程间发命令
 *
 *  硬件（你的板子）：
 *    RGB LED — PC7(蓝) PC8(绿) PC9(红)，低电平点亮
 *    按键    — PA8(KEY1) PA9(KEY2) PA10(KEY3) PA15(KEY4)
 * ════════════════════════════════════════════════════════
 */

#include <rtthread.h>
#include "ch32h417.h"

/*===============================
 *  定义命令枚举
 *  这些值通过邮箱在线程间传递
 *===============================*/
typedef enum
{
    CMD_RED    = 1,
    CMD_GREEN  = 2,
    CMD_BLUE   = 3,
    CMD_CYCLE  = 4,     /* 自动循环切换颜色 */
    CMD_OFF    = 5
} led_cmd_t;

/* 邮箱句柄 */
static rt_mailbox_t led_mb = RT_NULL;

/*===============================
 *  线程 1：LED 控制线程
 *  从邮箱收命令，控制 RGB 灯
 *===============================*/
static void led_ctrl_thread_entry(void *parameter)
{
    (void)parameter;

    /* 初始化三路 RGB 灯 GPIO（PC7/8/9，低电平点亮） */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_HB2PeriphClockCmd(RCC_HB2Periph_GPIOC, ENABLE);

    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_Very_High;
    GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* 默认全灭 */
    GPIO_SetBits(GPIOC, GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9);

    /* 用来接收邮箱消息的变量 */
    rt_uint32_t cmd;
    rt_uint32_t cycle_color = 0;   /* 0=R, 1=G, 2=B, 用于循环模式 */

    while (1)
    {
        /* ─── 等待邮箱消息（阻塞直到收到消息） ─── */
        if (rt_mb_recv(led_mb, &cmd, RT_WAITING_FOREVER) == RT_EOK)
        {
            switch ((led_cmd_t)cmd)
            {
            case CMD_RED:
                GPIO_SetBits(GPIOC, GPIO_Pin_7 | GPIO_Pin_8);  /* 关蓝绿 */
                GPIO_ResetBits(GPIOC, GPIO_Pin_9);             /* 开红 */
                rt_kprintf("[LED] >>> RED\r\n");
                cycle_color = 0;
                break;

            case CMD_GREEN:
                GPIO_SetBits(GPIOC, GPIO_Pin_7 | GPIO_Pin_9);  /* 关蓝红 */
                GPIO_ResetBits(GPIOC, GPIO_Pin_8);             /* 开绿 */
                rt_kprintf("[LED] >>> GREEN\r\n");
                cycle_color = 1;
                break;

            case CMD_BLUE:
                GPIO_SetBits(GPIOC, GPIO_Pin_8 | GPIO_Pin_9);  /* 关绿红 */
                GPIO_ResetBits(GPIOC, GPIO_Pin_7);             /* 开蓝 */
                rt_kprintf("[LED] >>> BLUE\r\n");
                cycle_color = 2;
                break;

            case CMD_CYCLE:
                rt_kprintf("[LED] >>> CYCLE mode start\r\n");
                /* 循环切换 5 次 */
                for (int i = 0; i < 5; i++)
                {
                    /* 全灭 */
                    GPIO_SetBits(GPIOC, GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9);
                    rt_thread_mdelay(300);

                    switch (cycle_color % 3)
                    {
                    case 0: /* 红 */
                        GPIO_ResetBits(GPIOC, GPIO_Pin_9);
                        rt_kprintf("[LED]   RED\r\n");
                        break;
                    case 1: /* 绿 */
                        GPIO_ResetBits(GPIOC, GPIO_Pin_8);
                        rt_kprintf("[LED]   GREEN\r\n");
                        break;
                    case 2: /* 蓝 */
                        GPIO_ResetBits(GPIOC, GPIO_Pin_7);
                        rt_kprintf("[LED]   BLUE\r\n");
                        break;
                    }
                    rt_thread_mdelay(500);
                    cycle_color++;
                }
                /* 循环结束，停在最后颜色 */
                rt_kprintf("[LED] >>> CYCLE done\r\n");
                break;

            case CMD_OFF:
            default:
                GPIO_SetBits(GPIOC, GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9);
                rt_kprintf("[LED] >>> OFF\r\n");
                break;
            }
        }
    }
}

/*===============================
 *  线程 2：按键扫描线程
 *  检测按键 → 通过邮箱发命令
 *===============================*/
static void key_scan_thread_entry(void *parameter)
{
    (void)parameter;

    /* 初始化4个按键 GPIO（PA8/9/10/15 上拉输入） */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_HB2PeriphClockCmd(RCC_HB2Periph_GPIOA, ENABLE);
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 |
                               GPIO_Pin_10 | GPIO_Pin_15;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    while (1)
    {
        uint32_t cmd = 0;

        /* 检测 KEY1 (PA8) → 红灯 */
        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8) == RESET)
        {
            cmd = CMD_RED;
            while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8) == RESET)
                rt_thread_mdelay(10);
        }
        /* KEY2 (PA9) → 绿灯 */
        else if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_9) == RESET)
        {
            cmd = CMD_GREEN;
            while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_9) == RESET)
                rt_thread_mdelay(10);
        }
        /* KEY3 (PA10) → 蓝灯 */
        else if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_10) == RESET)
        {
            cmd = CMD_BLUE;
            while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_10) == RESET)
                rt_thread_mdelay(10);
        }
        /* KEY4 (PA15) → 自动循环 */
        else if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_15) == RESET)
        {
            cmd = CMD_CYCLE;
            while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_15) == RESET)
                rt_thread_mdelay(10);
        }

        if (cmd != 0)
        {
            rt_kprintf("[KEY] send cmd=%d to mailbox\r\n", cmd);
            /* ─── 通过邮箱发送命令 ─── */
            rt_mb_send(led_mb, cmd);
        }

        rt_thread_mdelay(50);
    }
}

/*===============================
 *  初始化
 *===============================*/
static int rt_mailbox_example_init(void)
{
    rt_kprintf("═══════════════════════════════════════\r\n");
    rt_kprintf("  邮箱（Mailbox）示例\r\n");
    rt_kprintf("  KEY1 → 红灯  | KEY2 → 绿灯\r\n");
    rt_kprintf("  KEY3 → 蓝灯  | KEY4 → 循环变色\r\n");
    rt_kprintf("═══════════════════════════════════════\r\n");

    /* 1. 创建邮箱
     *    rt_mb_create(名字, 容量, 标志)
     *    容量 = 最多缓存几条消息（这里 4 条足够了）  */
    led_mb = rt_mb_create("led_mb", 4, RT_IPC_FLAG_FIFO);
    if (led_mb == RT_NULL)
    {
        rt_kprintf("Failed to create mailbox!\r\n");
        return -1;
    }

    /* 2. LED 控制线程（优先级 12） */
    rt_thread_t tid;
    tid = rt_thread_create("led_ctrl",
                           led_ctrl_thread_entry,
                           RT_NULL,
                           1024,
                           12, 10);
    if (tid) rt_thread_startup(tid);

    /* 3. 按键扫描线程（优先级 14，比 LED 低，不影响 LED 响应） */
    tid = rt_thread_create("key_scan",
                           key_scan_thread_entry,
                           RT_NULL,
                           512,
                           14, 10);
    if (tid) rt_thread_startup(tid);

    return 0;
}

INIT_APP_EXPORT(rt_mailbox_example_init);

int main(void)
{
    return 0;
}
