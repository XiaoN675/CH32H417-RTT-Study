/**
 * @file boot_screen.c
 * @brief 开机上电动画实现 — LVGL v8.3.x
 *
 * 动画流程：
 *   ┌────────────────────────────────┐
 *   │         [ Penguin Logo ]       │  ← 由小变大 + 淡入 (0~600ms)
 *   │       System Starting...       │  ← 淡入 (300~700ms)
 *   │   ┌────────────────────────┐   │
 *   │   │ ████████████████████░░ │   │  ← 进度条淡入 (600~900ms)
 *   │   └────────────────────────┘   │
 *   │            75%                 │  ← 百分比数字
 *   │     © 2024 WCH RISC-V         │  ← 页脚
 *   └────────────────────────────────┘
 *   进度 0%→100% 填充 (900~2600ms) → 回调通知完成
 */

#include "boot_screen.h"
#include <stdint.h>

/* 布局常量 */
#define PROGRESS_BAR_W      300
#define PROGRESS_BAR_H      14

/* ---- 静态变量（仅在动画期间使用） ---- */
static lv_obj_t *progress_label;      /* 百分比文字（回调中需要） */
static boot_complete_cb_t complete_cb;/* 完成回调 */

/* 外部引用的资源 */
extern const lv_img_dsc_t img_penguin;

/* ============================================================
 *  内部回调函数
 * ============================================================ */

/**
 * @brief 进度条动画的执行回调 — 更新进度条 + 百分比文字
 */
static void progress_anim_exec_cb(void *bar, int32_t v)
{
    lv_bar_set_value((lv_obj_t *)bar, v, LV_ANIM_ON);
    if (progress_label) {
        lv_label_set_text_fmt(progress_label, "%d%%", v);
    }
}

/**
 * @brief 进度条动画完成 — 调用用户回调切换屏幕
 */
static void progress_anim_ready_cb(lv_anim_t *a)
{
    LV_UNUSED(a);

    /* 确保显示 100% */
    if (progress_label) {
        lv_label_set_text(progress_label, "100%");
    }

    /* 调用用户回调（由用户负责切换屏幕） */
    if (complete_cb) {
        complete_cb();
    }

    /* 清理静态引用 */
    progress_label = NULL;
    complete_cb = NULL;
}

/**
 * @brief 安全的透明度动画包装 — 避免函数指针类型不匹配
 */
static void opa_anim_cb(void *obj, int32_t v)
{
    lv_obj_set_style_opa((lv_obj_t *)obj, (lv_opa_t)v, 0);
}

/**
 * @brief Logo 动画合并执行回调 — 同时处理缩放和淡入
 *        v 从 0→1000，按比例映射到缩放值和透明度
 *
 * 注意：LVGL 8.3 使用 transform_zoom 而非 transform_scale。
 *       zoom 范围 0~1024，256=1.0x。
 */
static void logo_anim_exec_cb(void *obj, int32_t v)
{
    /* 缩放值：25 (≈0.1x) → 256 (1.0x) */
    lv_obj_set_style_transform_zoom((lv_obj_t *)obj,
                                    25 + (v * 231) / 1000, 0);

    /* 透明度：前 600/1000 的过程中 0→255 */
    lv_opa_t opa = (lv_opa_t)((v * 255) / 600);
    if (opa > 255) opa = LV_OPA_COVER;
    lv_obj_set_style_opa((lv_obj_t *)obj, opa, 0);
}

/* ============================================================
 *  公共 API
 * ============================================================ */

void boot_screen_start(boot_complete_cb_t on_complete)
{
    complete_cb = on_complete;

    /* ---- 1. 创建开机 Screen（深蓝色背景） ---- */
    lv_obj_t *boot_scr = lv_obj_create(NULL);
    lv_obj_remove_style_all(boot_scr);            /* 清除默认样式 */
    lv_obj_set_style_bg_color(boot_scr, lv_color_hex(0x1a1a2e), 0);
    lv_obj_set_style_bg_opa(boot_scr, LV_OPA_COVER, 0);

    /* ---- 2. Logo（企鹅图片，80×80） ---- */
    lv_obj_t *logo = lv_img_create(boot_scr);
    lv_img_set_src(logo, &img_penguin);
    lv_obj_center(logo);
    lv_obj_set_y(logo, -55);
    /* 设置变换锚点到图片中心，使缩放从中心展开 */
    lv_obj_set_style_transform_pivot_x(logo, 40, 0);
    lv_obj_set_style_transform_pivot_y(logo, 40, 0);
    /* 初始状态：极小 + 透明 */
    lv_obj_set_style_transform_zoom(logo, 25, 0);
    lv_obj_set_style_opa(logo, LV_OPA_0, 0);

    /* ---- 3. 标题文字（使用 Montserrat 24） ---- */
    lv_obj_t *title = lv_label_create(boot_scr);
    lv_label_set_text(title, "Will uhsaudaidsda...");
    lv_obj_set_style_text_color(title, lv_color_hex(0xcccccc), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_align_to(title, logo, LV_ALIGN_OUT_BOTTOM_MID, 0, 30);
    lv_obj_set_style_opa(title, LV_OPA_0, 0);

    /* ---- 4. 进度条 ---- */
    lv_obj_t *bar = lv_bar_create(boot_scr);
    lv_obj_remove_style_all(bar);
    lv_obj_set_size(bar, PROGRESS_BAR_W, PROGRESS_BAR_H);
    lv_obj_align_to(bar, title, LV_ALIGN_OUT_BOTTOM_MID, 0, 30);
    lv_bar_set_range(bar, 0, 100);
    lv_bar_set_value(bar, 0, LV_ANIM_OFF);

    /* 进度条背景样式 */
    lv_obj_set_style_bg_color(bar, lv_color_hex(0x16213e), 0);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(bar, 7, 0);
    lv_obj_set_style_border_color(bar, lv_color_hex(0x0f3460), 0);
    lv_obj_set_style_border_width(bar, 1, 0);
    lv_obj_set_style_opa(bar, LV_OPA_0, 0);

    /* 进度条指示器（填充部分）样式 */
    lv_obj_set_style_bg_color(bar, lv_color_hex(0x00d2ff), LV_PART_INDICATOR);
    lv_obj_set_style_radius(bar, 7, LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, LV_PART_INDICATOR);

    /* ---- 5. 百分比文字 ---- */
    progress_label = lv_label_create(boot_scr);
    lv_label_set_text(progress_label, "0%");
    lv_obj_set_style_text_color(progress_label, lv_color_hex(0x00d2ff), 0);
    lv_obj_align_to(progress_label, bar, LV_ALIGN_OUT_BOTTOM_MID, 0, 8);
    lv_obj_set_style_opa(progress_label, LV_OPA_0, 0);

    /* ---- 6. 页脚文字 ---- */
    lv_obj_t *footer = lv_label_create(boot_scr);
    lv_label_set_text(footer, "");
    lv_obj_set_style_text_color(footer, lv_color_hex(0x555555), 0);
    lv_obj_align(footer, LV_ALIGN_BOTTOM_MID, 0, -12);

    /* ---- 加载开机 Screen ---- */
    lv_scr_load(boot_scr);

    /* ============================================================
     *  启动动画序列（所有动画同时 start，靠 delay 错开）
     * ============================================================ */

    /* --- 动画 A: Logo 缩放 + 淡入 (0→600ms) --- */
    lv_anim_t a_logo;
    lv_anim_init(&a_logo);
    lv_anim_set_var(&a_logo, logo);
    lv_anim_set_exec_cb(&a_logo, logo_anim_exec_cb);
    lv_anim_set_values(&a_logo, 0, 1000);
    lv_anim_set_time(&a_logo, 600);
    lv_anim_set_path_cb(&a_logo, lv_anim_path_ease_out);
    lv_anim_start(&a_logo);

    /* --- 动画 B: 标题淡入 (300→700ms) --- */
    lv_anim_t a_title;
    lv_anim_init(&a_title);
    lv_anim_set_var(&a_title, title);
    lv_anim_set_exec_cb(&a_title, opa_anim_cb);
    lv_anim_set_values(&a_title, LV_OPA_0, LV_OPA_COVER);
    lv_anim_set_time(&a_title, 400);
    lv_anim_set_delay(&a_title, 300);
    lv_anim_set_path_cb(&a_title, lv_anim_path_ease_out);
    lv_anim_start(&a_title);

    /* --- 动画 C: 进度条 + 百分比淡入 (600→900ms) --- */
    lv_anim_t a_bar_fade;
    lv_anim_init(&a_bar_fade);
    lv_anim_set_var(&a_bar_fade, bar);
    lv_anim_set_exec_cb(&a_bar_fade, opa_anim_cb);
    lv_anim_set_values(&a_bar_fade, LV_OPA_0, LV_OPA_COVER);
    lv_anim_set_time(&a_bar_fade, 300);
    lv_anim_set_delay(&a_bar_fade, 600);
    lv_anim_set_path_cb(&a_bar_fade, lv_anim_path_ease_out);
    lv_anim_start(&a_bar_fade);

    lv_anim_t a_pct_fade;
    lv_anim_init(&a_pct_fade);
    lv_anim_set_var(&a_pct_fade, progress_label);
    lv_anim_set_exec_cb(&a_pct_fade, opa_anim_cb);
    lv_anim_set_values(&a_pct_fade, LV_OPA_0, LV_OPA_COVER);
    lv_anim_set_time(&a_pct_fade, 300);
    lv_anim_set_delay(&a_pct_fade, 600);
    lv_anim_set_path_cb(&a_pct_fade, lv_anim_path_ease_out);
    lv_anim_start(&a_pct_fade);

    /* --- 动画 D: 进度条填充 (900→2600ms) --- */
    lv_anim_t a_progress;
    lv_anim_init(&a_progress);
    lv_anim_set_var(&a_progress, bar);
    lv_anim_set_exec_cb(&a_progress, progress_anim_exec_cb);
    lv_anim_set_values(&a_progress, 0, 100);
    lv_anim_set_time(&a_progress, 1700);
    lv_anim_set_delay(&a_progress, 900);
    lv_anim_set_path_cb(&a_progress, lv_anim_path_ease_in_out);
    lv_anim_set_ready_cb(&a_progress, progress_anim_ready_cb);
    lv_anim_start(&a_progress);
}
