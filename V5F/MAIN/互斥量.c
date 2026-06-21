/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : WCH
 * Version            : V1.0.0
 * Description        : RT-Thread Mutex example
 *********************************************************************************
 * Copyright (c) 2025 Nanjing Qinheng Microelectronics Co., Ltd.
 *******************************************************************************/

/*
 * ════════════════════════════════════════════════════════
 *  RTT 学习 —— 第二步：互斥量（Mutex）
 *
 *  学习目标：
 *    1. 理解共享资源需要保护的原因
 *    2. 掌握 rt_mutex_create / take / release
 *    3. 用互斥量保护 LED 亮度变量不被并发破坏
 *
 *  硬件（你的板子）：
 *    RGB LED — PC7(蓝) PC8(绿) PC9(红)，低电平点亮
 *    按键    — PA8(KEY1) PA9(KEY2)，低电平按下
 * ════════════════════════════════════════════════════════
 */

#include <rtthread.h>
#include "ch32h417.h"

/*===============================
 *  共享资源 —— LED 亮度值
 *  两个线程都会读写它
 *===============================*/
static uint8_t led_brightness = 50;     /* 共享资源：亮度百分比 0-100 */
static rt_mutex_t brightness_mutex;     /* 保护它的互斥量 */

/*===============================
 *  线程 1：按键调节亮度
 *  KEY1 按一次 +10，KEY2 按一次 -10
 *===============================*/
static void key_control_thread_entry(void *parameter)
{
    (void)parameter;

    /* 初始化按键 GPIO（PA8=KEY1, PA9=KEY2） */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_HB2PeriphClockCmd(RCC_HB2Periph_GPIOA, ENABLE);
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    while (1)
    {
        uint8_t changed = 0;

        /* KEY1（PA8）按一次亮度 +10 */
        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8) == RESET)
        {
            /* ─── 加锁 ─── */
            rt_mutex_take(brightness_mutex, RT_WAITING_FOREVER);

            if (led_brightness < 100)
                led_brightness += 10;
            if (led_brightness > 100)
                led_brightness = 100;
            changed = 1;

            /* ─── 解锁 ─── */
            rt_mutex_release(brightness_mutex);

            rt_kprintf("[KEY1] brightness +10 -> %d%%\r\n", led_brightness);
            while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8) == RESET)
                rt_thread_mdelay(10); /* 等待松开 */
        }

        /* KEY2（PA9）按一次亮度 -10 */
        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_9) == RESET)
        {
            rt_mutex_take(brightness_mutex, RT_WAITING_FOREVER);

            if (led_brightness > 0)
                led_brightness -= 10;
            if (led_brightness > 100)   /* 处理无符号溢出 */
                led_brightness = 0;
            changed = 1;

            rt_mutex_release(brightness_mutex);

            rt_kprintf("[KEY2] brightness -10 -> %d%%\r\n", led_brightness);
            while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_9) == RESET)
                rt_thread_mdelay(10);
        }

        if (!changed)
            rt_thread_mdelay(50);
    }
}

/*===============================
 *  线程 2：定时打印当前亮度
 *  模拟"另一个线程也在读共享资源"
 *===============================*/
static void monitor_thread_entry(void *parameter)
{
    (void)parameter;

    while (1)
    {
        uint8_t brightness_copy;

        /* ─── 加锁读取（确保读到完整正确的值） ─── */
        rt_mutex_take(brightness_mutex, RT_WAITING_FOREVER);
        brightness_copy = led_brightness;
        rt_mutex_release(brightness_mutex);

        rt_kprintf("[monitor] current brightness = %d%%\r\n", brightness_copy);

        rt_thread_mdelay(3000);
    }
}

/*===============================
 *  线程 3：模拟 LED 呼吸效果
 *  根据共享亮度值输出（用串口模拟）
 *===============================*/
static void led_sim_thread_entry(void *parameter)
{
    (void)parameter;

    /* 初始化板载红灯（PC9）做简单展示 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_HB2PeriphClockCmd(RCC_HB2Periph_GPIOC, ENABLE);
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_Very_High;
    GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* 默认关灯（红灯低电平点亮，所以 SetBits = 关） */
    GPIO_SetBits(GPIOC, GPIO_Pin_9);

    while (1)
    {
        uint8_t brightness;

        /* ─── 加锁读取当前亮度值 ─── */
        rt_mutex_take(brightness_mutex, RT_WAITING_FOREVER);
        brightness = led_brightness;
        rt_mutex_release(brightness_mutex);

        /* 模拟呼吸：占空比 = brightness% */
        /* on 时间 = brightness * 2 ms，off 时间 = (100 - brightness) * 2 ms */
        /* 周期固定 200ms */
        if (brightness > 0)
        {
            GPIO_ResetBits(GPIOC, GPIO_Pin_9);  /* 亮 */
            rt_thread_mdelay(brightness * 2);
        }

        if (brightness < 100)
        {
            GPIO_SetBits(GPIOC, GPIO_Pin_9);    /* 灭 */
            rt_thread_mdelay((100 - brightness) * 2);
        }
    }
}

/*===============================
 *  验证线程：故意不用锁，看数据乱不乱
 *  把这个线程打开，对比加锁/不加锁的区别
 *===============================*/
#if 0   /* ← 把 0 改成 1 来启用不加锁的"破坏"线程 */
static void evil_thread_entry(void *parameter)
{
    (void)parameter;
    while (1)
    {
        /* 不加锁直接改 —— 会导致 monitor 读到中间值 */
        led_brightness = 30;
        rt_thread_mdelay(1);
        led_brightness = 80;
        rt_thread_mdelay(1);
    }
}
#endif

/*===============================
 *  初始化所有线程和互斥量
 *===============================*/
static int rt_mutex_example_init(void)
{
    rt_kprintf("═══════════════════════════════════════\r\n");
    rt_kprintf("  互斥量（Mutex）示例\r\n");
    rt_kprintf("  KEY1(PA8) = brightness +10%%\r\n");
    rt_kprintf("  KEY2(PA9) = brightness -10%%\r\n");
    rt_kprintf("  红灯(PC9) 按亮度呼吸\r\n");
    rt_kprintf("═══════════════════════════════════════\r\n");

    /* 1. 创建互斥量 */
    brightness_mutex = rt_mutex_create("bright_mtx", RT_IPC_FLAG_FIFO);
    if (brightness_mutex == RT_NULL)
    {
        rt_kprintf("Failed to create mutex!\r\n");
        return -1;
    }

    /* 2. 创建按键控制线程 */
    rt_thread_t tid;
    tid = rt_thread_create("key_ctrl",
                           key_control_thread_entry,
                           RT_NULL,
                           1024,
                           12, 10);
    if (tid) rt_thread_startup(tid);

    /* 3. 创建监控线程 */
    tid = rt_thread_create("monitor",
                           monitor_thread_entry,
                           RT_NULL,
                           512,
                           13, 10);
    if (tid) rt_thread_startup(tid);

    /* 4. 创建 LED 模拟线程 */
    tid = rt_thread_create("led_sim",
                           led_sim_thread_entry,
                           RT_NULL,
                           512,
                           14, 10);
    if (tid) rt_thread_startup(tid);

#if 0   /* 改为 1 可启用"破坏线程"，观察不加锁的危害 */
    tid = rt_thread_create("evil",
                           evil_thread_entry,
                           RT_NULL,
                           256,
                           15, 10);
    if (tid) rt_thread_startup(tid);
    rt_kprintf("[WARN] evil thread enabled — data race may occur!\r\n");
#endif

    return 0;
}

INIT_APP_EXPORT(rt_mutex_example_init);

int main(void)
{
    return 0;
}
