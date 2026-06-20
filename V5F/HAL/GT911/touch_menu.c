/******************************************************************************
 * touch_menu.c - 触摸菜单手势处理实现
 *
 * 本模块从 main.c 中提取而出，通过 extern 引用 main.c 中的菜单全局变量
 * 和绘图函数，为菜单界面提供触摸上滑/下滑/单击进入功能。
 ******************************************************************************/
#include "touch_menu.h"
#include "GT911.h"

/* ========== GT911 低频轮询读取（不用中断，每 50ms 调用一次即可） ========== */
void GT911_PollTouch(Touch_data_t *data)
{
    uint8_t temp = 0;

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
        temp = 0;
        GT911_write_register(GT911_BUFFER_STATUS, &temp, 1);

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

/* ========== 引用 main.c 中的全局变量 ========== */
extern uint8_t  menu_index;
extern uint8_t  menu_scroll;
extern uint8_t  music_note_index;
extern uint32_t mode_tick;
extern uint8_t  current_page;   /* PageState 枚举，当作 uint8_t 使用 */

/* ========== 引用 main.c 中的函数 ========== */
extern void Draw_Menu_Page(uint8_t active_idx);
extern void Draw_Mode_Page(uint8_t mode_idx);

/* PageState 枚举值（与 main.c 保持一致） */
#define PAGE_MODE  2

/* ****************************************************************************
 * Process_Touch_Menu  -  菜单触摸手势处理
 *
 * 三态状态机：
 *   (1) 触摸按下 (touch_down=1, prev=0)  → 记录起始 X/Y
 *   (2) 触摸滑动中 (touch_down=1, prev=1) → 检测 Y 偏移，判定上滑/下滑
 *   (3) 触摸释放 (touch_down=0, prev=1)   → 判定单击（同时检查 X/Y 是否在按钮矩形区域内），选中/进入模式
 *
 * X 轴约束：释放时须 tx ∈ [TOUCH_BUTTON_X_MIN, TOUCH_BUTTON_X_MAX) 才视为有效单击，
 *           避免屏幕边缘误触触发菜单操作。
 ******************************************************************************/
void Process_Touch_Menu(uint16_t tx, uint16_t ty, uint8_t touch_down)
{
    static uint16_t touch_start_x = 0;
    static uint16_t touch_start_y = 0;
    static uint8_t  touch_prev = 0;
    static uint8_t  gesture_used = 0;

    /* ---- 状态 1：触摸按下 ---- */
    if (touch_down && !touch_prev)
    {
        touch_start_x = tx;
        touch_start_y = ty;
        gesture_used = 0;
    }
    /* ---- 状态 2：触摸滑动中 — 检测滑动手势 ---- */
    else if (touch_down && touch_prev && !gesture_used)
    {
        int16_t dy = (int16_t)ty - (int16_t)touch_start_y;
        if (dy > TOUCH_SWIPE_THRESHOLD)
        {
            /* 下滑 → 菜单上移（反向，内容跟随手指） */
            if (menu_index > 0) menu_index--;
            else menu_index = MENU_ITEM_COUNT - 1;
            Draw_Menu_Page(menu_index);
            gesture_used = 1;
        }
        else if (dy < -TOUCH_SWIPE_THRESHOLD)
        {
            /* 上滑 → 菜单下移（反向，内容跟随手指） */
            if (menu_index < MENU_ITEM_COUNT - 1) menu_index++;
            else menu_index = 0;
            Draw_Menu_Page(menu_index);
            gesture_used = 1;
        }
    }
    /* ---- 状态 3：触摸释放（单击）— 判定点击行 ---- */
    else if (!touch_down && touch_prev && !gesture_used)
    {
        /* 释放时同时检查 X 和 Y：只有点击在按钮矩形区域内才生效 */
        if (tx >= TOUCH_BUTTON_X_MIN && tx < TOUCH_BUTTON_X_MAX &&
            ty >= 120 && ty < 270)
        {
            uint8_t row = (ty - 120) / 50;  /* 0/1/2，对应三个可视行 */
            uint8_t idx = menu_scroll + row;
            if (idx < MENU_ITEM_COUNT)
            {
                if (idx == menu_index)
                {
                    /* 点击已选中项 → 进入模式 (同 KEY3) */
                    current_page = PAGE_MODE;
                    Draw_Mode_Page(menu_index);
                    mode_tick = 0;
                    music_note_index = 0;
                }
                else
                {
                    /* 点击不同项 → 选中它 */
                    menu_index = idx;
                    Draw_Menu_Page(menu_index);
                }
            }
        }
    }

    touch_prev = touch_down;
}
