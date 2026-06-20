/**
 * @file lv_port_indev.c
 * @brief LVGL 触摸输入驱动适配层 — 直接轮询 GT911 寄存器
 *
 * 直接使用 GT911.h 的 read/write 寄存器函数读取触摸状态，
 * 不依赖 touch_menu.c 的中断或外部变量。
 */

#include "lvgl.h"
#include "GT911.h"
#include "GT911_IIC.h"

/* ========== 直接轮询读取 GT911 ========== */
static void gt911_poll(Touch_data_t *data)
{
    uint8_t temp = 0;

    /* 读状态寄存器 */
    GT911_read_register(GT911_BUFFER_STATUS, &temp, 1);

    data->TouchState  = temp & 0x80;
    data->TouchNumber = temp & 0x0F;

    if (data->TouchNumber > GT911_TOUCH_MAX_NUM)
    {
        data->TouchState  = 0;
        data->TouchNumber = 0;
        temp = 0;
        GT911_write_register(GT911_BUFFER_STATUS, &temp, 1);
        return;
    }

    if (data->TouchState == 0x80)
    {
        /* 清除状态（允许 GT911 继续上报下一笔数据） */
        temp = 0;
        GT911_write_register(GT911_BUFFER_STATUS, &temp, 1);

        /* 读取所有触摸点坐标 */
        for (uint8_t i = 0; i < data->TouchNumber; i++)
        {
            uint16_t reg = GT911_BUFFER_START + i * 8;

            GT911_read_register(reg + 0, &temp, 1);
            data->Touch_xy[i].PointY  = temp;
            GT911_read_register(reg + 1, &temp, 1);
            data->Touch_xy[i].PointY |= (temp << 8);

            GT911_read_register(reg + 2, &temp, 1);
            data->Touch_xy[i].PointX  = temp;
            GT911_read_register(reg + 3, &temp, 1);
            data->Touch_xy[i].PointX |= (temp << 8);
            data->Touch_xy[i].PointX = lcddev.width - data->Touch_xy[i].PointX - 1;

            GT911_read_register(reg + 4, &temp, 1);
            data->Touch_xy[i].PointS  = temp;
            GT911_read_register(reg + 5, &temp, 1);
            data->Touch_xy[i].PointS |= (temp << 8);
        }
    }
}

/* ========== LVGL 触摸读取回调 ========== */
static void touchpad_read(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    /* 轮询读取 GT911 */
    gt911_poll(&touchData);

    if (touchData.TouchState & 0x80)
    {
        data->state = LV_INDEV_STATE_PR;
        data->point.x = touchData.Touch_xy[0].PointX;
        data->point.y = touchData.Touch_xy[0].PointY;
    }
    else
    {
        data->state = LV_INDEV_STATE_REL;
    }
}

/* ========== 初始化 ========== */
void lv_port_indev_init(void)
{
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);

    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touchpad_read;
    lv_indev_drv_register(&indev_drv);
}
