/**
 * @file app_screentest.c
 * @brief 屏幕测试 — 全屏纯色轮播
 */

#include "app_screentest.h"
#include "lcd.h"
#include "app_system.h"

static const struct {
    uint16_t color;     /* LCD 颜色宏 */
    uint32_t lv_hex;    /* LVGL hex 颜色 */
    const char *name;
} test_colors[] = {
    { RED,   0xe74c3c, "Red"    },
    { GREEN, 0x2ecc71, "Green"  },
    { BLUE,  0x3498db, "Blue"   },
    { BLACK, 0x000000, "Black"  },
    { WHITE, 0xffffff, "White"  },
};
#define TEST_COLOR_COUNT (sizeof(test_colors)/sizeof(test_colors[0]))

static uint8_t test_index = 0;
static lv_obj_t *name_label = NULL;

static void next_color(void)
{
    test_index = (test_index + 1) % TEST_COLOR_COUNT;
    LCD_Fill(0, 0, 480, 320, test_colors[test_index].color);
    if (name_label) {
        lv_label_set_text(name_label, test_colors[test_index].name);
    }
}

static void prev_color(void)
{
    test_index = (test_index == 0) ? TEST_COLOR_COUNT - 1 : test_index - 1;
    LCD_Fill(0, 0, 480, 320, test_colors[test_index].color);
    if (name_label) {
        lv_label_set_text(name_label, test_colors[test_index].name);
    }
}

static void back_btn_cb(lv_event_t *e)
{
    LV_UNUSED(e);
    app_system_back_to_menu();
}

static void prev_btn_cb(lv_event_t *e)
{
    LV_UNUSED(e);
    prev_color();
}

static void next_btn_cb(lv_event_t *e)
{
    LV_UNUSED(e);
    next_color();
}

static void app_screentest_enter(void)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_remove_style_all(scr);

    /* "Back to Menu" 按钮固定在左上角 */
    lv_obj_t *back_btn = lv_btn_create(scr);
    lv_obj_set_size(back_btn, 50, 30);
    lv_obj_align(back_btn, LV_ALIGN_TOP_LEFT, 5, 5);
    lv_obj_set_style_bg_color(back_btn, lv_color_hex(0x333333), 0);
    lv_obj_set_style_opa(back_btn, 180, 0);
    lv_obj_add_event_cb(back_btn, back_btn_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *blbl = lv_label_create(back_btn);
    lv_label_set_text(blbl, "←");
    lv_obj_center(blbl);
    lv_obj_set_style_text_color(blbl, lv_color_hex(0xffffff), 0);

    /* 颜色名称居中显示 */
    name_label = lv_label_create(scr);
    lv_obj_set_style_text_color(name_label, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_text_font(name_label, &lv_font_montserrat_24, 0);
    lv_obj_align(name_label, LV_ALIGN_CENTER, 0, -20);

    /* 上/下翻页按钮 */
    lv_obj_t *prev_btn = lv_btn_create(scr);
    lv_obj_set_size(prev_btn, 120, 50);
    lv_obj_align(prev_btn, LV_ALIGN_BOTTOM_MID, -70, -15);
    lv_obj_set_style_bg_color(prev_btn, lv_color_hex(0x555555), 0);
    lv_obj_set_style_opa(prev_btn, 180, 0);
    lv_obj_add_event_cb(prev_btn, prev_btn_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *plbl = lv_label_create(prev_btn);
    lv_label_set_text(plbl, "◀ Prev");
    lv_obj_center(plbl);

    lv_obj_t *next_btn = lv_btn_create(scr);
    lv_obj_set_size(next_btn, 120, 50);
    lv_obj_align(next_btn, LV_ALIGN_BOTTOM_MID, 70, -15);
    lv_obj_set_style_bg_color(next_btn, lv_color_hex(0x555555), 0);
    lv_obj_set_style_opa(next_btn, 180, 0);
    lv_obj_add_event_cb(next_btn, next_btn_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *nlbl = lv_label_create(next_btn);
    lv_label_set_text(nlbl, "Next ▶");
    lv_obj_center(nlbl);

    /* 显示第一个颜色 */
    test_index = 0;
    LCD_Fill(0, 0, 480, 320, test_colors[0].color);
    lv_label_set_text(name_label, test_colors[0].name);

    lv_scr_load_anim(scr, LV_SCR_LOAD_ANIM_FADE_OUT, 300, 0, false);
}

const app_item_t app_screentest = {
    .name  = "Screen",
    .icon  = "🖥️",
    .color = 0x9b59b6,
    .enter = app_screentest_enter,
    .exit  = NULL,
};
