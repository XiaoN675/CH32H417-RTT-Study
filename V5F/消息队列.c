/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Description        : RT-Thread Message Queue + PWM brightness control
 *********************************************************************************
 * Copyright (c) 2025 Nanjing Qinheng Microelectronics Co., Ltd.
 *******************************************************************************/

/*
 * ════════════════════════════════════════════════════════
 *  RTT 学习 —— 消息队列 + PWM 亮度控制
 *
 *  通过 INIT_APP_EXPORT 自动初始化。
 *
 *  硬件：
 *    RGB LED — PC7(蓝) PC8(绿) PC9(红)，TIM3 PWM
 *    按键    — PA8(KEY1) PA9(KEY2) PA10(KEY3) PA15(KEY4)
 * ════════════════════════════════════════════════════════
 */

#include <rtthread.h>
#include "ch32h417.h"
#include "led.h"

/*===============================
 *  消息结构体
 *===============================*/
typedef enum
{
    CMD_RED,
    CMD_GREEN,
    CMD_BLUE,
    CMD_OFF
} led_cmd_e;

typedef struct
{
    led_cmd_e   color;
    uint8_t     brightness;     /* 0-100，PWM 占空比 */
    uint16_t    duration_ms;    /* 0 = 一直保持 */
} led_msg_t;

static rt_mq_t led_mq = RT_NULL;
static uint16_t pwm_period = 0;

/*===============================
 *  线程 1：LED 控制（PWM）
 *===============================*/
static void led_ctrl_thread_entry(void *parameter)
{
    (void)parameter;

    led_msg_t msg;

    while (1)
    {
        if (rt_mq_recv(led_mq, &msg, sizeof(msg), RT_WAITING_FOREVER) == RT_EOK)
        {
            rt_kprintf("[LED] color=%d bright=%d%% dur=%dms\r\n",
                       msg.color, msg.brightness, msg.duration_ms);

            if (msg.color == CMD_OFF || msg.brightness == 0)
            {
                led_pwm_control(kALL_LED, pwm_period, 0);
                rt_kprintf("[LED] >>> OFF\r\n");
                continue;
            }

            switch (msg.color)
            {
            case CMD_RED:
                led_pwm_control(kRED_LED,   pwm_period, msg.brightness);
                led_pwm_control(kGREEN_LED, pwm_period, 0);
                led_pwm_control(kBLUE_LED,  pwm_period, 0);
                break;
            case CMD_GREEN:
                led_pwm_control(kRED_LED,   pwm_period, 0);
                led_pwm_control(kGREEN_LED, pwm_period, msg.brightness);
                led_pwm_control(kBLUE_LED,  pwm_period, 0);
                break;
            case CMD_BLUE:
                led_pwm_control(kRED_LED,   pwm_period, 0);
                led_pwm_control(kGREEN_LED, pwm_period, 0);
                led_pwm_control(kBLUE_LED,  pwm_period, msg.brightness);
                break;
            default:
                break;
            }

            if (msg.duration_ms > 0)
            {
                rt_thread_mdelay(msg.duration_ms);
                led_pwm_control(kALL_LED, pwm_period, 0);
                rt_kprintf("[LED] %dms passed, OFF\r\n", msg.duration_ms);
            }
        }
    }
}

/*===============================
 *  线程 2：按键扫描
 *===============================*/
static void key_scan_thread_entry(void *parameter)
{
    (void)parameter;

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_HB2PeriphClockCmd(RCC_HB2Periph_GPIOA, ENABLE);
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 |
                               GPIO_Pin_10 | GPIO_Pin_15;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    uint8_t bright_table[4] = { 20, 50, 80, 100 };

    while (1)
    {
        led_msg_t msg = {0};
        int has_msg = 0;

        if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8) == RESET)
        {
            msg.color = CMD_RED;
            msg.brightness = bright_table[0];
            msg.duration_ms = 2000;
            has_msg = 1;
            while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8) == RESET)
                rt_thread_mdelay(10);
        }
        else if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_9) == RESET)
        {
            msg.color = CMD_GREEN;
            msg.brightness = bright_table[1];
            msg.duration_ms = 2000;
            has_msg = 1;
            while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_9) == RESET)
                rt_thread_mdelay(10);
        }
        else if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_10) == RESET)
        {
            msg.color = CMD_BLUE;
            msg.brightness = bright_table[2];
            msg.duration_ms = 2000;
            has_msg = 1;
            while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_10) == RESET)
                rt_thread_mdelay(10);
        }
        else if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_15) == RESET)
        {
            msg.color = CMD_OFF;
            msg.brightness = 0;
            msg.duration_ms = 0;
            has_msg = 1;
            while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_15) == RESET)
                rt_thread_mdelay(10);
        }

        if (has_msg)
        {
            rt_kprintf("[KEY] send: color=%d bright=%d%%\r\n",
                       msg.color, msg.brightness);
            rt_mq_send(led_mq, &msg, sizeof(msg));
        }

        rt_thread_mdelay(50);
    }
}

/*===============================
 *  自动初始化函数
 *===============================*/
static int rt_app_init(void)
{
    rt_kprintf("════════════════════════════════════\r\n");
    rt_kprintf("  消息队列 + PWM 亮度控制\r\n");
    rt_kprintf("  KEY1 → 红 20%%  | KEY2 → 绿 50%%\r\n");
    rt_kprintf("  KEY3 → 蓝 80%%  | KEY4 → 关灯\r\n");
    rt_kprintf("  按键时长保持 2s 后自动灭\r\n");
    rt_kprintf("════════════════════════════════════\r\n");

    /* 1. 初始化 TIM3 PWM */
    pwm_period = LED_CONTROL_TIM_PERIOD_1MS;
    led_pwm_init(pwm_period, LED_CONTROL_TIM_PRESCALER, 0);

    /* 2. 创建消息队列 */
    led_mq = rt_mq_create("led_mq", sizeof(led_msg_t), 4, RT_IPC_FLAG_FIFO);
    if (led_mq == RT_NULL)
    {
        rt_kprintf("rt_mq_create failed!\r\n");
        return -1;
    }

    /* 3. 创建线程 */
    rt_thread_t tid;

    tid = rt_thread_create("led_ctrl", led_ctrl_thread_entry,
                           RT_NULL, 1024, 12, 10);
    if (tid) rt_thread_startup(tid);

    tid = rt_thread_create("key_scan", key_scan_thread_entry,
                           RT_NULL, 512, 14, 10);
    if (tid) rt_thread_startup(tid);

    return 0;
}

INIT_APP_EXPORT(rt_app_init);

/*===============================
 *  main() — 空
 *===============================*/
int main(void)
{
    return 0;
}
