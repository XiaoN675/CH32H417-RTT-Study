/**
 * @file app_led.c
 * @brief LED 控制 — 点击色块切换 RGB 颜色
 */

#include "app_led.h"
#include "led.h"

/* 颜色表 */
static const struct {
    uint32_t    hex;        /* LVGL 颜色值 */
    led_color_t led;        /* LED 驱动颜色枚举 */
    const char *name;       /* 显示名称 */
} colors[] = {
    { 0xe74c3c, kRed,     "Red"     },
    { 0x2ecc71, kGreen,   "Green"   },
    { 0x3498db, kBlue,    "Blue"    },
    { 0xf1c40f, kYellow,  "Yellow"  },
    { 0x9b59b6, kMagenta, "Magenta" },
    { 0x1abc9c, kCyan,    "Cyan"    },
    { 0x000000, kBlack,   "Off"     },
};
#define COLOR_COUNT  (sizeof(colors)/sizeof(colors[0]))

static lv_obj_t *status_label;

/* 色块点击回调 */
static void color_btn_cb(lv_event_t *e)
{
    uint8_t idx = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
    if (idx >= COLOR_COUNT) return;

    led_color_change(colors[idx].led);
    if (status_label) {
        lv_label_set_text_fmt(status_label, "Current: %s", colors[idx].name);
    }
}

static void app_led_enter(void)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_remove_style_all(scr);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x16213e), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    app_system_create_topbar(scr, "LED Control");

    /* 提示文字 */
    lv_obj_t *hint = lv_label_create(scr);
    lv_label_set_text(hint, "Tap a color:");
    lv_obj_set_style_text_color(hint, lv_color_hex(0xcccccc), 0);
    lv_obj_align(hint, LV_ALIGN_TOP_MID, 0, 50);

    /* 颜色色块网格 — 3列 */
    lv_obj_t *grid = lv_obj_create(scr);
    lv_obj_remove_style_all(grid);
    lv_obj_set_size(grid, 440, 200);
    lv_obj_align(grid, LV_ALIGN_TOP_MID, 0, 75);
    lv_obj_set_flex_flow(grid, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(grid, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_SPACE_EVENLY);
    lv_obj_set_style_pad_row(grid, 10, 0);
    lv_obj_set_style_pad_column(grid, 10, 0);
    lv_obj_set_style_bg_opa(grid, LV_OPA_TRANSP, 0);

    for (uint8_t i = 0; i < COLOR_COUNT; i++)
    {
        lv_obj_t *btn = lv_btn_create(grid);
        lv_obj_set_size(btn, 130, 45);
        lv_obj_set_style_bg_color(btn, lv_color_hex(colors[i].hex), 0);
        lv_obj_set_style_radius(btn, 8, 0);
        lv_obj_add_event_cb(btn, color_btn_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)i);

        lv_obj_t *lbl = lv_label_create(btn);
        lv_label_set_text(lbl, colors[i].name);
        lv_obj_set_style_text_color(lbl, lv_color_hex(0xffffff), 0);
        /* 深色背景用白色文字，亮色背景用黑色 */
        if (i == 3 || i == 5) { /* Yellow, Cyan */
            lv_obj_set_style_text_color(lbl, lv_color_hex(0x333333), 0);
        }
        lv_obj_center(lbl);
    }

    /* 当前状态 */
    status_label = lv_label_create(scr);
    lv_label_set_text(status_label, "Current: Off");
    lv_obj_set_style_text_color(status_label, lv_color_hex(0x00d2ff), 0);
    lv_obj_align(status_label, LV_ALIGN_BOTTOM_MID, 0, -20);

    lv_scr_load_anim(scr, LV_SCR_LOAD_ANIM_FADE_OUT, 300, 0, false);
}

const app_item_t app_led = {
    .name  = "LED",
    .icon  = "💡",
    .color = 0xe74c3c,
    .enter = app_led_enter,
    .exit  = NULL,
};

void app_led_init(void)
{
    led_init();
}
