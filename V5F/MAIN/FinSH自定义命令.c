/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Description        : RT-Thread FinSH custom commands
 *********************************************************************************
 * Copyright (c) 2025 Nanjing Qinheng Microelectronics Co., Ltd.
 *******************************************************************************/

/*
 * ════════════════════════════════════════════════════════
 *  RTT 学习 —— FinSH 自定义命令
 *
 *  学习目标：
 *    1. 掌握 MSH_CMD_EXPORT 添加命令
 *    2. 理解命令函数的 argc / argv 参数
 *    3. 在串口输入命令控制板子
 *
 *  硬件：
 *    RGB LED — PC7(蓝) PC8(绿) PC9(红)
 * ════════════════════════════════════════════════════════
 */

#include <rtthread.h>
#include "ch32h417.h"
#include "led.h"

/* PWM 周期值（由 pwm_init 命令设置） */
static uint16_t pwm_period = 0;

/*===============================
 *  工具函数：字符串转整数
 *  RT-Thread 没有提供 atoi，自己写一个
 *===============================*/
static int str2int(const char *s)
{
    int n = 0;
    while (*s >= '0' && *s <= '9')
    {
        n = n * 10 + (*s - '0');//字符 '0' 的码值是 48，字符 '5' 的码值是 53。单片机不认识字符 '5'，但如果你用 '5' - '0'（即 $53 - 48$）
        s++;
    }
    return n;
}

/*===============================
 *  GPIO 初始化（只做一次）
 *===============================*/
static void led_gpio_init(void)
{
    static int inited = 0;
    if (inited) return;
    inited = 1;

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_HB2PeriphClockCmd(RCC_HB2Periph_GPIOC, ENABLE);
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_Very_High;
    GPIO_Init(GPIOC, &GPIO_InitStruct);
    GPIO_SetBits(GPIOC, GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9);
}

/*===============================
 *  命令 1：led  <red|green|blue|off>
 *  用法：led red     → 红灯亮
 *        led off     → 关灯
 *===============================*/
static void led(int argc, char **argv)
{
    if (argc < 2)
    {
        rt_kprintf("Usage: led <red|green|blue|off>\r\n");
        return;
    }

    led_gpio_init();
    GPIO_SetBits(GPIOC, GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9);

    if (rt_strcmp(argv[1], "red") == 0)
    {
        GPIO_ResetBits(GPIOC, GPIO_Pin_9);
        rt_kprintf("LED -> RED\r\n");
    }
    else if (rt_strcmp(argv[1], "green") == 0)
    {
        GPIO_ResetBits(GPIOC, GPIO_Pin_8);
        rt_kprintf("LED -> GREEN\r\n");
    }
    else if (rt_strcmp(argv[1], "blue") == 0)
    {
        GPIO_ResetBits(GPIOC, GPIO_Pin_7);
        rt_kprintf("LED -> BLUE\r\n");
    }
    else if (rt_strcmp(argv[1], "off") == 0)
    {
        rt_kprintf("LED -> OFF\r\n");
    }
    else
    {
        rt_kprintf("Unknown color: %s\r\n", argv[1]);
    }
}
MSH_CMD_EXPORT(led, "set LED color: led <red|green|blue|off>");

/*===============================
 *  命令 2：blink <次数> <间隔ms>
 *  用法：blink 5 200  → 闪烁5次，每次200ms
 *===============================*/
static void blink(int argc, char **argv)
{
    int count = 3;      /* 默认 3 次 */
    int interval = 300; /* 默认 300ms */

    if (argc >= 2)
        count = str2int(argv[1]);
    if (argc >= 3)
        interval = str2int(argv[2]);

    if (count < 1 || count > 100)
    {
        rt_kprintf("count must be 1-100\r\n");
        return;
    }
    if (interval < 50 || interval > 5000)
    {
        rt_kprintf("interval must be 50-5000ms\r\n");
        return;
    }

    led_gpio_init();

    for (int i = 0; i < count; i++)
    {
        GPIO_ResetBits(GPIOC, GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9);
        rt_thread_mdelay(interval);
        GPIO_SetBits(GPIOC, GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9);
        rt_thread_mdelay(interval);
    }

    rt_kprintf("blink %d times, interval %dms\r\n", count, interval);
}
MSH_CMD_EXPORT(blink, "blink LED: blink [count] [interval_ms]");

/*===============================
 *  命令 3：pwm_init  — 初始化 PWM
 *===============================*/
static void pwm_init(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    pwm_period = LED_CONTROL_TIM_PERIOD_1MS;
    led_pwm_init(pwm_period, LED_CONTROL_TIM_PRESCALER, 0);

    rt_kprintf("PWM initialized (period=%d, prescaler=%d)\r\n",
               pwm_period, LED_CONTROL_TIM_PRESCALER);
}
MSH_CMD_EXPORT(pwm_init, "initialize TIM3 PWM for RGB LED");

/*===============================
 *  命令 4：pwm <red|green|blue|all> <0-100>
 *  用法：pwm red 50  → 红灯 50% 亮度
 *  注意：必须先执行 pwm_init
 *===============================*/
static void pwm(int argc, char **argv)
{
    if (argc < 3)
    {
        rt_kprintf("Usage: pwm <red|green|blue|all> <0-100>\r\n");
        return;
    }

    if (pwm_period == 0)
    {
        rt_kprintf("PWM not initialized, run 'pwm_init' first\r\n");
        return;
    }

    int percent = str2int(argv[2]);
    if (percent < 0 || percent > 100)
    {
        rt_kprintf("percent must be 0-100\r\n");
        return;
    }

    if (rt_strcmp(argv[1], "red") == 0)
        led_pwm_control(kRED_LED,   pwm_period, percent);
    else if (rt_strcmp(argv[1], "green") == 0)
        led_pwm_control(kGREEN_LED, pwm_period, percent);
    else if (rt_strcmp(argv[1], "blue") == 0)
        led_pwm_control(kBLUE_LED,  pwm_period, percent);
    else if (rt_strcmp(argv[1], "all") == 0)
        led_pwm_control(kALL_LED,   pwm_period, percent);
    else
    {
        rt_kprintf("Unknown: %s\r\n", argv[1]);
        return;
    }

    rt_kprintf("PWM %s = %d%%\r\n", argv[1], percent);
}
MSH_CMD_EXPORT(pwm, "set PWM brightness: pwm <red|green|blue|all> <0-100>");

/*===============================
 *  命令 5：info  — 打印系统信息
 *===============================*/
static void info(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    rt_kprintf("════════════════════════════\r\n");
    rt_kprintf("  System Info\r\n");
    rt_kprintf("  Tick: %d Hz\r\n", RT_TICK_PER_SECOND);
    rt_kprintf("  Priority levels: %d\r\n", RT_THREAD_PRIORITY_MAX);
    rt_kprintf("  Heap size: %d bytes\r\n", 15 * 1024);
    rt_kprintf("════════════════════════════\r\n");
    rt_kprintf("Available commands:\r\n");
    rt_kprintf("  led       - led <red|green|blue|off>\r\n");
    rt_kprintf("  blink     - blink [count] [interval_ms]\r\n");
    rt_kprintf("  pwm_init  - init PWM timer\r\n");
    rt_kprintf("  pwm       - pwm <red|green|blue|all> <0-100>\r\n");
    rt_kprintf("  info      - this info\r\n");
}
MSH_CMD_EXPORT(info, "print system info and command list");

/*===============================
 *  main()
 *===============================*/
int main(void)
{
    rt_kprintf("════════════════════════════════════\r\n");
    rt_kprintf("  FinSH 自定义命令示例\r\n");
    rt_kprintf("  在串口输入 help 查看所有命令\r\n");
    rt_kprintf("  或直接输入命令名使用：\r\n");
    rt_kprintf("    led red     → 红灯\r\n");
    rt_kprintf("    blink 5 200 → 闪烁5次\r\n");
    rt_kprintf("    pwm_init    → 初始化PWM\r\n");
    rt_kprintf("    pwm red 50  → 红灯50%%\r\n");
    rt_kprintf("    info        → 查看命令列表\r\n");
    rt_kprintf("════════════════════════════════════\r\n");

    return 0;
}
