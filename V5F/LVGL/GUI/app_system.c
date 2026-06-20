/**
 * @file app_system.c
 * @brief 系统主框架 — 翻页卡片菜单 + 滑动翻页 + 点击进入
 *
 * 每页显示 3 个卡片，左右滑动翻页，底部圆点指示当前页。
 * 滑动由 LVGL 内置手势识别（LV_EVENT_GESTURE）处理，
 * 与卡片点击（LV_EVENT_CLICKED）互斥，不会冲突。
 */

#include "app_system.h"
#include "GT911.h"
#include <stdint.h>
#include <string.h>

/* 调试：尝试重新初始化 GT911 并读状态 */
static lv_obj_t *touch_debug_label = NULL;
static uint8_t touch_init_attempted = 0;

static void touch_debug_update(lv_timer_t *t)
{
    LV_UNUSED(t);
    if (!touch_debug_label) return;

    uint8_t status = 0;
    uint8_t id[5] = {0};

    /* 尝试读 GT911 状态和 ID */
    GT911_read_register(GT911_BUFFER_STATUS, &status, 1);
    GT911_read_register(GT911_ID_ADDRESS, id, 4);

    if (status == 0xFF && !touch_init_attempted) {
        /* 第一次读到 0xFF，尝试重新初始化 GT911 */
        touch_init_attempted = 1;
        GT911_iic_gpio_init();
        GT911_reset();
        GT911_set_iic_address();
        /* 再读一次 */
        GT911_read_register(GT911_BUFFER_STATUS, &status, 1);
        GT911_read_register(GT911_ID_ADDRESS, id, 4);
    }

    lv_label_set_text_fmt(touch_debug_label, "S:0x%02X ID:%c%c%c",
        status,
        id[0] >= 0x20 ? id[0] : '.',
        id[1] >= 0x20 ? id[1] : '.',
        id[2] >= 0x20 ? id[2] : '.');
}

/* ============================================================
 *  布局常量
 * ============================================================ */
#define CARD_W              420
#define CARD_H              72
#define CARD_GAP            14
#define ITEMS_PER_PAGE      3

/* 滑动阈值（由 LVGL 内部管理，这里只用来判断方向） */
#define SWIPE_THRESHOLD     30

/* ============================================================
 *  静态变量
 * ============================================================ */
#define MAX_APPS            12
static const app_item_t    *app_list[MAX_APPS];
static uint8_t              app_count = 0;

static lv_obj_t            *menu_scr = NULL;
static lv_obj_t            *card_container = NULL;   /* 卡片容器 */
static lv_obj_t            *dot_container = NULL;    /* 页面指示点容器 */
static uint8_t              current_page = 0;
static uint8_t              total_pages = 0;

static const app_item_t    *current_app = NULL;

/* ============================================================
 *  内部函数声明
 * ============================================================ */
static void show_page(uint8_t page);

/* ============================================================
 *  App 注册
 * ============================================================ */
void app_system_register(const app_item_t *app)
{
    if (app_count >= MAX_APPS) return;
    app_list[app_count++] = app;
}

uint32_t app_system_get_app_count(void)
{
    return app_count;
}

/* ============================================================
 *  导航：打开 App / 返回菜单
 * ============================================================ */
void app_system_open(const app_item_t *app)
{
    if (!app) return;
    current_app = app;
    if (app->enter) app->enter();
}

void app_system_back_to_menu(void)
{
    /* 通知当前 App 退出，清理资源 */
    if (current_app && current_app->exit) {
        current_app->exit();
    }
    current_app = NULL;
    lv_scr_load_anim(menu_scr, LV_SCR_LOAD_ANIM_FADE_OUT, 300, 0, true);
}

/* ============================================================
 *  卡片点击回调
 * ============================================================ */
static void card_click_cb(lv_event_t *e)
{
    const app_item_t *app = (const app_item_t *)lv_event_get_user_data(e);
    if (app) app_system_open(app);
}

/* ============================================================
 *  滑动翻页 — 手势事件回调
 * ============================================================ */
static void swipe_gesture_cb(lv_event_t *e)
{
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());

    if (dir == LV_DIR_LEFT && current_page < total_pages - 1)
    {
        show_page(current_page + 1);
    }
    else if (dir == LV_DIR_RIGHT && current_page > 0)
    {
        show_page(current_page - 1);
    }
}

/* ============================================================
 *  更新页面指示圆点
 * ============================================================ */
static void update_dots(void)
{
    if (!dot_container) return;

    /* 清除旧的指示点 */
    lv_obj_clean(dot_container);

    for (uint8_t i = 0; i < total_pages; i++)
    {
        lv_obj_t *dot = lv_obj_create(dot_container);
        lv_obj_remove_style_all(dot);
        lv_obj_set_size(dot, 10, 10);
        lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);

        if (i == current_page) {
            lv_obj_set_style_bg_color(dot, lv_color_hex(0x00d2ff), 0);
            lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);
        } else {
            lv_obj_set_style_bg_color(dot, lv_color_hex(0x555555), 0);
            lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);
        }
    }
}

/* ============================================================
 *  显示指定页面
 * ============================================================ */
static void show_page(uint8_t page)
{
    if (page >= total_pages) return;
    current_page = page;

    /* 清空卡片容器 */
    lv_obj_clean(card_container);

    /* 计算当前页的起始索引 */
    uint8_t start = page * ITEMS_PER_PAGE;
    uint8_t end = start + ITEMS_PER_PAGE;
    if (end > app_count) end = app_count;

    /* 创建该页的卡片 */
    for (uint8_t i = start; i < end; i++)
    {
        const app_item_t *app = app_list[i];

        lv_obj_t *card = lv_obj_create(card_container);
        lv_obj_remove_style_all(card);
        lv_obj_set_size(card, CARD_W, CARD_H);
        lv_obj_set_style_bg_color(card, lv_color_hex(app->color), 0);
        lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(card, 12, 0);
        lv_obj_set_style_shadow_width(card, 6, 0);
        lv_obj_set_style_shadow_color(card, lv_color_hex(0x000000), 0);
        lv_obj_set_style_shadow_opa(card, 50, 0);

        /* 点击进入 */
        lv_obj_add_event_cb(card, card_click_cb, LV_EVENT_CLICKED, (void *)app);
        /* 滑动翻页（添加到卡片上，事件冒泡到容器也能处理） */
        lv_obj_add_event_cb(card, swipe_gesture_cb, LV_EVENT_GESTURE, NULL);

        /* 图标（左侧大号 emoji） */
        lv_obj_t *icon = lv_label_create(card);
        lv_label_set_text(icon, app->icon);
        lv_obj_set_style_text_color(icon, lv_color_hex(0xffffff), 0);
        lv_obj_align(icon, LV_ALIGN_LEFT_MID, 18, 0);

        /* 名称（居中） */
        lv_obj_t *name = lv_label_create(card);
        lv_label_set_text(name, app->name);
        lv_obj_set_style_text_color(name, lv_color_hex(0xffffff), 0);
        lv_obj_align(name, LV_ALIGN_CENTER, 20, 0);

        /* 右箭头提示 */
        lv_obj_t *arrow = lv_label_create(card);
        lv_label_set_text(arrow, ">");
        lv_obj_set_style_text_color(arrow, lv_color_hex(0xffffff), 0);
        lv_obj_set_style_text_opa(arrow, LV_OPA_50, 0);
        lv_obj_align(arrow, LV_ALIGN_RIGHT_MID, -15, 0);
    }

    /* 更新指示点 */
    update_dots();
}

/* ============================================================
 *  创建通用的 "返回按钮" — 供各 App 使用
 * ============================================================ */
static void back_btn_cb(lv_event_t *e)
{
    LV_UNUSED(e);
    app_system_back_to_menu();
}

void app_system_create_topbar(lv_obj_t *scr, const char *title)
{
    /* 顶栏背景 */
    lv_obj_t *header = lv_obj_create(scr);
    lv_obj_remove_style_all(header);
    lv_obj_set_size(header, 480, 40);
    lv_obj_set_pos(header, 0, 0);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x1a1a2e), 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);

    /* 返回按钮 */
    lv_obj_t *btn = lv_btn_create(header);
    lv_obj_set_size(btn, 60, 30);
    lv_obj_align(btn, LV_ALIGN_LEFT_MID, 8, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x333355), 0);
    lv_obj_set_style_radius(btn, 4, 0);
    lv_obj_add_event_cb(btn, back_btn_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *lbl = lv_label_create(btn);
    lv_label_set_text(lbl, "←");
    lv_obj_center(lbl);
    lv_obj_set_style_text_color(lbl, lv_color_hex(0xcccccc), 0);

    /* 标题 */
    lv_obj_t *title_lbl = lv_label_create(header);
    lv_label_set_text(title_lbl, title);
    lv_obj_set_style_text_color(title_lbl, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_text_font(title_lbl, &lv_font_montserrat_24, 0);
    lv_obj_align(title_lbl, LV_ALIGN_CENTER, 0, 0);
}

/* ============================================================
 *  创建主菜单界面
 * ============================================================ */
void app_system_start(void)
{
    if (app_count == 0) return;

    /* 计算总页数 */
    total_pages = (app_count + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE;
    current_page = 0;

    /* ---- 创建主 Screen ---- */
    menu_scr = lv_obj_create(NULL);
    lv_obj_remove_style_all(menu_scr);
    lv_obj_set_style_bg_color(menu_scr, lv_color_hex(0x16213e), 0);
    lv_obj_set_style_bg_opa(menu_scr, LV_OPA_COVER, 0);

    /* ---- 顶部标题 ---- */
    lv_obj_t *title = lv_label_create(menu_scr);
    lv_label_set_text(title, "Main Menu");
    lv_obj_set_style_text_color(title, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 15);

    /* 分隔线 */
    lv_obj_t *line = lv_obj_create(menu_scr);
    lv_obj_remove_style_all(line);
    lv_obj_set_size(line, 460, 2);
    lv_obj_align(line, LV_ALIGN_TOP_MID, 0, 44);
    lv_obj_set_style_bg_color(line, lv_color_hex(0x0f3460), 0);
    lv_obj_set_style_bg_opa(line, LV_OPA_COVER, 0);

    /* ---- 卡片容器（垂直布局，3 张卡片） ---- */
    card_container = lv_obj_create(menu_scr);
    lv_obj_remove_style_all(card_container);
    lv_obj_set_size(card_container, CARD_W, CARD_H * 3 + CARD_GAP * 2);
    lv_obj_align(card_container, LV_ALIGN_TOP_MID, 0, 55);
    lv_obj_set_flex_flow(card_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card_container, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(card_container, LV_OPA_TRANSP, 0);

    /* ---- 页面指示圆点 ---- */
    dot_container = lv_obj_create(menu_scr);
    lv_obj_remove_style_all(dot_container);
    lv_obj_align(dot_container, LV_ALIGN_BOTTOM_MID, 0, -18);
    lv_obj_set_flex_flow(dot_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(dot_container, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(dot_container, 10, 0);
    lv_obj_set_style_bg_opa(dot_container, LV_OPA_TRANSP, 0);

    /* 显示第 0 页 */
    show_page(0);

    /* ---- 添加屏幕级手势检测（滑空白区域也能翻页） ---- */
    lv_obj_add_event_cb(menu_scr, swipe_gesture_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_add_event_cb(card_container, swipe_gesture_cb, LV_EVENT_GESTURE, NULL);

    /* ---- 触摸调试：右上角显示坐标 ---- */
    touch_debug_label = lv_label_create(menu_scr);
    lv_label_set_text(touch_debug_label, "");
    lv_obj_set_style_text_color(touch_debug_label, lv_color_hex(0x00ff00), 0);
    lv_obj_align(touch_debug_label, LV_ALIGN_TOP_RIGHT, -5, 50);
    lv_timer_t *td_timer = lv_timer_create(touch_debug_update, 100, NULL);
    lv_timer_set_repeat_count(td_timer, -1);

    /* ---- 加载菜单 Screen（从 boot_screen 切换过来） ---- */
    lv_scr_load_anim(menu_scr, LV_SCR_LOAD_ANIM_FADE_OUT, 400, 0, true);
}
