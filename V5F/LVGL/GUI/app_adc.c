/**
 * @file app_adc.c
 * @brief ADC 监测 — 实时显示 ADC 采样值和电压
 */

#include "app_adc.h"
#include "adc.h"

static lv_obj_t *value_label;
static lv_obj_t *volt_label;
static lv_timer_t *refresh_timer = NULL;

/* 前向声明 */
static void app_adc_cleanup(void);

static void refresh_cb(lv_timer_t *t)
{
    LV_UNUSED(t);
    uint16_t val = adc1_single_convert_average_value_get(ADC_Channel_4, 10, ADC_SampleTime_CyclesMode7);
    uint32_t mv = adc1_voltage_get(val);

    lv_label_set_text_fmt(value_label, "Raw: %d (12-bit)", val);
    lv_label_set_text_fmt(volt_label, "Voltage: %d.%03d V", mv / 1000, mv % 1000);
}

static void app_adc_enter(void)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_remove_style_all(scr);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x16213e), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    app_system_create_topbar(scr, "ADC Monitor");

    /* ADC 值显示 */
    value_label = lv_label_create(scr);
    lv_label_set_text(value_label, "Raw: ---");
    lv_obj_set_style_text_color(value_label, lv_color_hex(0x00d2ff), 0);
    lv_obj_set_style_text_font(value_label, &lv_font_montserrat_24, 0);
    lv_obj_align(value_label, LV_ALIGN_CENTER, 0, -30);

    /* 电压显示 */
    volt_label = lv_label_create(scr);
    lv_label_set_text(volt_label, "Voltage: ---");
    lv_obj_set_style_text_color(volt_label, lv_color_hex(0x2ecc71), 0);
    lv_obj_set_style_text_font(volt_label, &lv_font_montserrat_24, 0);
    lv_obj_align(volt_label, LV_ALIGN_CENTER, 0, 20);

    /* 提示 */
    lv_obj_t *hint = lv_label_create(scr);
    lv_label_set_text(hint, "Channel: PC4 (ADC1_IN4)");
    lv_obj_set_style_text_color(hint, lv_color_hex(0x888888), 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -30);

    /* 启动定时器，每 200ms 刷新一次 */
    if (refresh_timer) lv_timer_del(refresh_timer);
    refresh_timer = lv_timer_create(refresh_cb, 200, NULL);
    lv_timer_set_repeat_count(refresh_timer, -1); /* 无限重复 */

    /* 立即刷新一次 */
    refresh_cb(NULL);

    lv_scr_load_anim(scr, LV_SCR_LOAD_ANIM_FADE_OUT, 300, 0, false);
}

const app_item_t app_adc = {
    .name  = "ADC",
    .icon  = "📊",
    .color = 0x1abc9c,
    .enter = app_adc_enter,
    .exit  = app_adc_cleanup,
};

void app_adc_init(void)
{
    adc1_single_convert_init();
}

void app_adc_cleanup(void)
{
    if (refresh_timer) {
        lv_timer_del(refresh_timer);
        refresh_timer = NULL;
    }
}
