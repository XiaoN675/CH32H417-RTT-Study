/**
 * @file lv_port_disp.c
 * @brief LVGL 显示驱动适配层 —— 对接 WCH LCD 驱动
 *
 * 基于 V5F/HAL/LCD/lcd.c 中的底层接口：
 *   - LCD_Address_Set(x1, y1, x2, y2)  设置窗口
 *   - LCD_WR_DATA(color)                写入一个像素 (RGB565)
 */

#include "lvgl.h"
#include "lcd.h"

/*********************
 * 显示缓冲区
 *********************/

/**
 * 定义显示缓冲区。
 * LVGL 先在这里绘制，然后通过 disp_flush 一次性刷到屏幕。
 *
 * 缓冲区大小 = 一行 × 40 行 = 480 × 40 × 2 字节 ≈ 38KB
 * 如果芯片内存紧张，可以改小（如 480×20），但刷屏会慢一些。
 */
#define DISP_BUF_ROWS   40
static lv_disp_draw_buf_t disp_buf;
static lv_color_t buf_1[480 * DISP_BUF_ROWS];

/*********************
 * 刷屏回调函数
 *********************/

/**
 * LVGL 在需要更新屏幕显示时调用此函数。
 *
 * @param drv     显示驱动指针
 * @param area    需要刷新的矩形区域
 * @param color_p 像素颜色数据
 */
static void disp_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p)
{
    int32_t x1 = area->x1;
    int32_t y1 = area->y1;
    int32_t x2 = area->x2;
    int32_t y2 = area->y2;

    /* 设置 LCD 窗口范围 */
    LCD_Address_Set(x1, y1, x2, y2);

    /* 逐像素写入颜色数据 */
    uint32_t pixels = (x2 - x1 + 1) * (y2 - y1 + 1);
    for (uint32_t i = 0; i < pixels; i++)
    {
        LCD_WR_DATA(color_p->full);
        color_p++;
    }

    /* 通知 LVGL 刷新完成 */
    lv_disp_flush_ready(drv);
}

/*********************
 * 初始化显示驱动
 *********************/

void lv_port_disp_init(void)
{
    /* 1. 初始化显示缓冲区 */
    lv_disp_draw_buf_init(&disp_buf, buf_1, NULL, 480 * DISP_BUF_ROWS);

    /* 2. 初始化显示驱动 */
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);

    /* 绑定刷屏回调 */
    disp_drv.flush_cb = disp_flush;

    /* 绑定显示缓冲区 */
    disp_drv.draw_buf = &disp_buf;

    /* 设置屏幕分辨率 */
    disp_drv.hor_res = 480;
    disp_drv.ver_res = 320;

    /* 注册显示驱动到 LVGL */
    lv_disp_drv_register(&disp_drv);
}
