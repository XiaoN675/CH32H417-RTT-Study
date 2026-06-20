/**
 * @file boot_screen.h
 * @brief 开机上电动画 — 展示 Logo + 进度条 + 淡入转场
 */

#ifndef _BOOT_SCREEN_H_
#define _BOOT_SCREEN_H_

#include "lvgl.h"

/** 开机动画完成时的回调函数类型 */
typedef void (*boot_complete_cb_t)(void);

/**
 * @brief 创建并启动开机上电动画
 *
 * 动画流程：
 *   1. Logo 由小变大 + 淡入 (0~600ms)
 *   2. 标题文字淡入 (400~700ms)
 *   3. 进度条淡入 (700~900ms)
 *   4. 进度条 0%→100% 平滑填充 (900~2600ms)
 *   5. 进度到达 100% 后调用 on_complete 回调
 *
 * @param on_complete 动画完成后调用的回调（用于切换到主界面）
 */
void boot_screen_start(boot_complete_cb_t on_complete);

#endif /* _BOOT_SCREEN_H_ */
