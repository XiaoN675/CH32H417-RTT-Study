/**
 * @file app_beep.c
 * @brief 蜂鸣器 — 7 个音阶按钮，点击播放对应音调
 */

#include "app_beep.h"
#include "beep.h"

/* 7 音阶（6 度音，约 1~2KHz，对蜂鸣器更友好） */
static const char *note_names[] = {"Do", "Re", "Mi", "Fa", "Sol", "La", "Si"};
/* ARR = 100000 / freq - 1 */
static const uint16_t note_periods[] = {
    94,  /* Do  (C6) 1047Hz */
    84,  /* Re  (D6) 1175Hz */
    74,  /* Mi  (E6) 1319Hz */
    70,  /* Fa  (F6) 1397Hz */
    62,  /* Sol (G6) 1568Hz */
    55,  /* La  (A6) 1760Hz */
    49   /* Si  (B6) 1976Hz */
};
#define NOTE_COUNT 7

/* 蜂鸣器停止回调（一次性定时器） */
static void beep_stop_timer_cb(lv_timer_t *t)
{
    /* 占空比设 0 = 无声，保留当前频率不变 */
    TIM9_12_SetCompare2(TIM9, 0);
    lv_timer_del(t);
}

/* 音阶按钮点击回调 */
static void note_btn_cb(lv_event_t *e)
{
    uint8_t idx = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
    if (idx >= NOTE_COUNT) return;

    /* 播放该音符（50% 占空比） */
    beep_pwm_control(note_periods[idx], 50);

    /* 250ms 后自动停止 */
    lv_timer_t *t = lv_timer_create(beep_stop_timer_cb, 250, NULL);
    lv_timer_set_repeat_count(t, 1);
}

static void app_beep_enter(void)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_remove_style_all(scr);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x16213e), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    app_system_create_topbar(scr, "Buzzer");

    lv_obj_t *hint = lv_label_create(scr);
    lv_label_set_text(hint, "Tap a note:");
    lv_obj_set_style_text_color(hint, lv_color_hex(0xcccccc), 0);
    lv_obj_align(hint, LV_ALIGN_TOP_MID, 0, 50);

    /* 音阶按钮 — 3 列网格 */
    lv_obj_t *grid = lv_obj_create(scr);
    lv_obj_remove_style_all(grid);
    lv_obj_set_size(grid, 440, 260);
    lv_obj_align(grid, LV_ALIGN_TOP_MID, 0, 75);
    lv_obj_set_flex_flow(grid, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(grid, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_SPACE_EVENLY);
    lv_obj_set_style_pad_row(grid, 12, 0);
    lv_obj_set_style_pad_column(grid, 12, 0);
    lv_obj_set_style_bg_opa(grid, LV_OPA_TRANSP, 0);

    for (uint8_t i = 0; i < NOTE_COUNT; i++)
    {
        lv_obj_t *btn = lv_btn_create(grid);
        lv_obj_set_size(btn, 130, 60);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x3498db), 0);
        lv_obj_set_style_radius(btn, 10, 0);
        lv_obj_add_event_cb(btn, note_btn_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)i);

        lv_obj_t *lbl = lv_label_create(btn);
        lv_label_set_text_fmt(lbl, "%s\n%dHz", note_names[i],
                              100000 / (note_periods[i] + 1));
        lv_obj_set_style_text_color(lbl, lv_color_hex(0xffffff), 0);
        lv_obj_center(lbl);
    }

    lv_scr_load_anim(scr, LV_SCR_LOAD_ANIM_FADE_OUT, 300, 0, false);
}

const app_item_t app_beep = {
    .name  = "Buzzer",
    .icon  = "🔊",
    .color = 0xf39c12,
    .enter = app_beep_enter,
    .exit  = NULL,
};

void app_beep_init(void)
{
    beep_pwm_init(BEEP_CONTROL_TIM_PERIOD_1MS,
                  BEEP_CONTROL_TIM_PRESCALER, 0);
    beep_pwm_control(0, 0); /* 初始无声 */
}
