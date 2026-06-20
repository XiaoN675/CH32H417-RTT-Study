/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Description        : RT-Thread Software Timer example
 *********************************************************************************
 * Copyright (c) 2025 Nanjing Qinheng Microelectronics Co., Ltd.
 *******************************************************************************/

/*
 * ════════════════════════════════════════════════════════
 *  RTT 学习 —— 软件定时器（Software Timer）
 *
 *  学习目标：
 *    1. 掌握 rt_timer_create / start / stop / control
 *    2. 理解"定时器回调不占线程栈"
 *    3. 用定时器做周期任务，按键动态调频率
 *
 *  硬件：
 *    RGB LED — PC7(蓝) PC8(绿) PC9(红)
 *    按键    — PA8(KEY1) PA9(KEY2)
 * ════════════════════════════════════════════════════════
 */

#include <rtthread.h>
#include "ch32h417.h"

/*===============================
 *  软件定时器句柄
 *===============================*/
static rt_timer_t led_timer = RT_NULL;

/*===============================
 *  定时器回调函数
 *  每 tick 一次，翻转 LED
 *===============================*/
static void led_toggle_callback(void *parameter)
{
    (void)parameter;

    /* 翻转红灯（PC9 低电平点亮）
     * 读取当前输出状态，取反 */
    if (GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_9) == RESET)
        GPIO_SetBits(GPIOC, GPIO_Pin_9);    /* 灭 */
    else
        GPIO_ResetBits(GPIOC, GPIO_Pin_9);  /* 亮 */
}

/*===============================
 *  按键扫描线程
 *  动态修改定时器周期
 *===============================*/
static void key_scan_thread_entry(void *parameter)
{
    (void)parameter;

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_HB2PeriphClockCmd(RCC_HB2Periph_GPIOA, ENABLE);
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    rt_tick_t period_tick;
    const char *msg;

    while (1)
    {
        /* KEY1 → 加快闪烁 */
        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8) == RESET)
        {
            period_tick = rt_tick_from_millisecond(100);  /* 100ms *///rt_tick_from_millisecond（100）自动把100ms换算成tick
            msg = "FAST (100ms)";
            while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8) == RESET)
                rt_thread_mdelay(10);

            /* 重新设置定时器周期 */
            rt_timer_control(led_timer, RT_TIMER_CTRL_SET_TIME, &period_tick);//就相当于把上面的100ms给他了
            rt_kprintf("[KEY1] period -> %s\r\n", msg);
        }
        /* KEY2 → 减慢闪烁 */
        else if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_9) == RESET)
        {
            period_tick = rt_tick_from_millisecond(500);  /* 500ms */
            msg = "SLOW (500ms)";
            while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_9) == RESET)
                rt_thread_mdelay(10);

            rt_timer_control(led_timer, RT_TIMER_CTRL_SET_TIME, &period_tick);
            rt_kprintf("[KEY2] period -> %s\r\n", msg);
        }

        rt_thread_mdelay(50);
    }
}

/*===============================
 *  自动初始化
 *===============================*/
static int rt_app_init(void)
{
    /* 初始化 LED GPIO */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_HB2PeriphClockCmd(RCC_HB2Periph_GPIOC, ENABLE);
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;          /* 红灯 */
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_Very_High;
    GPIO_Init(GPIOC, &GPIO_InitStruct);
    GPIO_SetBits(GPIOC, GPIO_Pin_9);                /* 初始灭 */

    /* 1. 创建软件定时器
     *    rt_timer_create(名字, 回调, 参数,
     *                    周期(tick), 标志)
     *
     *    标志：
     *      RT_TIMER_FLAG_PERIODIC  — 周期性
     *      RT_TIMER_FLAG_ONE_SHOT  — 单次
     *      RT_TIMER_FLAG_SOFT_TIMER — 软定时器线程中执行（不在中断上下文）
     */
    led_timer = rt_timer_create("led_tmr",
                                led_toggle_callback,
                                RT_NULL,
                                rt_tick_from_millisecond(500),
                                RT_TIMER_FLAG_PERIODIC | RT_TIMER_FLAG_ONE_SHOT);
    if (led_timer == RT_NULL)
    {
        rt_kprintf("rt_timer_create failed!\r\n");
        return -1;
    }

    /* 2. 启动定时器 */
    rt_timer_start(led_timer);

    rt_kprintf("════════════════════════════════════\r\n");
    rt_kprintf("  软件定时器示例\r\n");
    rt_kprintf("  红灯每 500ms 闪烁\r\n");
    rt_kprintf("  KEY1 → 加快(100ms)\r\n");
    rt_kprintf("  KEY2 → 减慢(500ms)\r\n");
    rt_kprintf("════════════════════════════════════\r\n");

    /* 3. 创建按键扫描线程 */
    rt_thread_t tid;
    tid = rt_thread_create("key_scan", key_scan_thread_entry,
                           RT_NULL, 512, 14, 10);
    if (tid) rt_thread_startup(tid);

    return 0;
}

INIT_APP_EXPORT(rt_app_init);

int main(void){
    return 0;
}
