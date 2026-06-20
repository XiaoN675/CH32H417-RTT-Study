/******************************************************************************
 * touch_menu.h - 触摸菜单手势处理模块
 *
 * 基于 GT911 触摸驱动，为菜单界面提供上滑/下滑/单击进入功能。
 * 与 main.c 中的菜单全局变量和绘图函数配合使用。
 ******************************************************************************/
#ifndef _TOUCH_MENU_H_
#define _TOUCH_MENU_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "GT911.h"

/* 触摸滑动判定阈值 (像素) — 手指移动超过此值视为滑动手势 */
#define TOUCH_SWIPE_THRESHOLD   40

/* 菜单按钮触摸区域 X 范围 — 触摸点的 X 坐标必须在此区间内才视为有效的按钮点击 */
/* 屏幕宽 480px，留出左右边缘各 50px 防误触 */
#define TOUCH_BUTTON_X_MIN      100
#define TOUCH_BUTTON_X_MAX      300

/* 菜单项总数 & 每页显示行数 (与 main.c 保持一致) */
#define MENU_ITEM_COUNT  6
#define MENU_SHOW_COUNT  3

/*
 * Process_Touch_Menu  -  菜单触摸手势处理
 *
 * 参数:
 *   tx         - 触摸点 X 坐标 (0~479)
 *   ty         - 触摸点 Y 坐标 (0~319)
 *   touch_down - 1 = 手指按下/持续按住, 0 = 手指释放
 *
 * 行为:
 *   按下时记录起始位置；滑动中检测 Y 偏移量并滚动菜单；
 *   释放时若偏移量小于阈值则视为单击，判断点击行并选中/进入。
 *
 * 需由外部提供的全局变量/函数 (均在 main.c 中定义):
 *   menu_index, menu_scroll, music_note_index, mode_tick, current_page
 *   Draw_Menu_Page(), Draw_Mode_Page()
 */
void Process_Touch_Menu(uint16_t tx, uint16_t ty, uint8_t touch_down);

/* 低频轮询读取 GT911（不依赖中断，每 50ms 调用一次即可） */
void GT911_PollTouch(Touch_data_t *data);

#ifdef __cplusplus
}
#endif

#endif /* _TOUCH_MENU_H_ */
