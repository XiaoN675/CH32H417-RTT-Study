/**
 * @file app_mic.c
 * @brief 录音回放 — 按下"Record"开始录音，点击"Play"回放
 */

#include "app_mic.h"
#include "mic.h"
#include "speaker.h"
#include "adc.h"

/* 录音缓冲区：8KHz × 16bit × 2秒 */
#define RECORD_BUF_LEN      16000
static uint16_t record_buf[RECORD_BUF_LEN] = {0};
static volatile uint8_t is_recording = 0;

static lv_obj_t *status_label;
static lv_obj_t *record_btn;
static lv_obj_t *play_btn;
static lv_timer_t *play_status_timer = NULL;

/* 前向声明 */
static void app_mic_cleanup(void);

static void update_ui_state(void)
{
    if (is_recording) {
        lv_label_set_text(status_label, "Status: Recording...");
        lv_obj_set_style_bg_color(record_btn, lv_color_hex(0xe74c3c), 0);
    } else {
        lv_label_set_text(status_label, "Status: Ready");
        lv_obj_set_style_bg_color(record_btn, lv_color_hex(0x2ecc71), 0);
    }
}

static void record_btn_cb(lv_event_t *e)
{
    LV_UNUSED(e);
    if (is_recording) {
        /* 停止录音 */
        mic_adc_collection_set(0);
        is_recording = 0;
    } else {
        /* 开始录音 — 重新初始化 ADC 为连续模式（防止被 ADC App 覆盖） */
        adc1_continue_convert_init(record_buf, RECORD_BUF_LEN,
                                   MIC_ADC_CHANNEL,
                                   MIC_CONTROL_TIM_PERIOD,
                                   MIC_CONTROL_TIM_PRESCALER);
        mic_adc_reset_recording(record_buf, RECORD_BUF_LEN);
        mic_adc_collection_set(1);
        is_recording = 1;
    }
    update_ui_state();
}


static void play_finish_cb(lv_timer_t *t)
{
    LV_UNUSED(t);
    lv_label_set_text(status_label, "Status: Ready");
    lv_timer_del(t);
    play_status_timer = NULL;
}

/* 录音时显示 ADC 值（调试用） */
static lv_timer_t *monitor_timer = NULL;
static lv_obj_t *monitor_label = NULL;

static void monitor_cb(lv_timer_t *t)
{
    LV_UNUSED(t);
    if (!monitor_label) return;

    if (is_recording) {
        /* 读缓冲区开头位置（DMA 最先写这里） */
        uint32_t sum = 0;
        for (uint16_t i = 0; i < 32; i++) {
            sum += record_buf[i];
        }
        uint16_t avg = sum / 32;
        lv_label_set_text_fmt(monitor_label, "ADC: %d", avg);
    } else {
        lv_label_set_text(monitor_label, "ADC: ---");
    }
}

static void play_btn_cb_fixed(lv_event_t *e)
{
    LV_UNUSED(e);
    if (is_recording) return;

    /* 对录音数据进行简单滤波处理 */
    mic_audio_filter(record_buf, RECORD_BUF_LEN);

    lv_label_set_text(status_label, "Status: Playing...");

    /* 启动 DAC 播放 */
    speaker_dac_play_recording(record_buf, RECORD_BUF_LEN);

    /* 估算播放时长后恢复状态 */
    uint32_t play_ms = (RECORD_BUF_LEN * 1000) / 8000 + 500;
    if (play_status_timer) lv_timer_del(play_status_timer);
    play_status_timer = lv_timer_create(play_finish_cb, play_ms, NULL);
    lv_timer_set_repeat_count(play_status_timer, 1);
}

/* 生成 1KHz 测试音（方波）并播放 */
static void test_tone_cb(lv_event_t *e)
{
    LV_UNUSED(e);
    if (is_recording) return;

    /* 用 1KHz 方波填充播放缓冲区 */
    for (uint32_t i = 0; i < RECORD_BUF_LEN; i++) {
        /* 8KHz采样率，1KHz方波 = 每8个采样一个周期 */
        record_buf[i] = (i / 4) & 1 ? 3072 : 1024;  /* 半幅值，减少电源波动 */
    }

    lv_label_set_text(status_label, "Status: Test Tone (1KHz)...");
    speaker_dac_play_recording(record_buf, RECORD_BUF_LEN);

    uint32_t play_ms = 1000;
    if (play_status_timer) lv_timer_del(play_status_timer);
    play_status_timer = lv_timer_create(play_finish_cb, play_ms, NULL);
    lv_timer_set_repeat_count(play_status_timer, 1);
}

static void app_mic_enter(void)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_remove_style_all(scr);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x16213e), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    app_system_create_topbar(scr, "Audio");

    /* 状态提示 */
    status_label = lv_label_create(scr);
    lv_label_set_text(status_label, "Status: Ready");
    lv_obj_set_style_text_color(status_label, lv_color_hex(0x00d2ff), 0);
    lv_obj_align(status_label, LV_ALIGN_TOP_MID, 0, 55);

    /* Record 按钮 */
    record_btn = lv_btn_create(scr);
    lv_obj_set_size(record_btn, 200, 70);
    lv_obj_align(record_btn, LV_ALIGN_CENTER, -60, -40);
    lv_obj_set_style_bg_color(record_btn, lv_color_hex(0x2ecc71), 0);
    lv_obj_set_style_radius(record_btn, 35, 0);
    lv_obj_add_event_cb(record_btn, record_btn_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *rec_lbl = lv_label_create(record_btn);
    lv_label_set_text(rec_lbl, "⏺ Record");
    lv_obj_set_style_text_color(rec_lbl, lv_color_hex(0xffffff), 0);
    lv_obj_center(rec_lbl);

    /* Play 按钮 */
    play_btn = lv_btn_create(scr);
    lv_obj_set_size(play_btn, 200, 70);
    lv_obj_align(play_btn, LV_ALIGN_CENTER, -60, 50);
    lv_obj_set_style_bg_color(play_btn, lv_color_hex(0x3498db), 0);
    lv_obj_set_style_radius(play_btn, 35, 0);
    lv_obj_add_event_cb(play_btn, play_btn_cb_fixed, LV_EVENT_CLICKED, NULL);

    lv_obj_t *play_lbl = lv_label_create(play_btn);
    lv_label_set_text(play_lbl, "▶ Play");
    lv_obj_set_style_text_color(play_lbl, lv_color_hex(0xffffff), 0);
    lv_obj_center(play_lbl);

    /* 测试音按钮（产生 1KHz 方波） */
    lv_obj_t *test_btn = lv_btn_create(scr);
    lv_obj_set_size(test_btn, 200, 70);
    lv_obj_align(test_btn, LV_ALIGN_CENTER, -60, 130);
    lv_obj_set_style_bg_color(test_btn, lv_color_hex(0xe67e22), 0);
    lv_obj_set_style_radius(test_btn, 35, 0);
    lv_obj_add_event_cb(test_btn, test_tone_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *test_lbl = lv_label_create(test_btn);
    lv_label_set_text(test_lbl, "♪ Test Tone");
    lv_obj_set_style_text_color(test_lbl, lv_color_hex(0xffffff), 0);
    lv_obj_center(test_lbl);

    /* 提示 */
    lv_obj_t *info = lv_label_create(scr);
    lv_label_set_text(info, "8KHz · 16bit · ~2s");
    lv_obj_set_style_text_color(info, lv_color_hex(0x888888), 0);
    lv_obj_align(info, LV_ALIGN_BOTTOM_MID, 0, -40);

    /* ADC 监视标签 */
    monitor_label = lv_label_create(scr);
    lv_label_set_text(monitor_label, "ADC: ---");
    lv_obj_set_style_text_color(monitor_label, lv_color_hex(0xf1c40f), 0);
    lv_obj_align(monitor_label, LV_ALIGN_BOTTOM_MID, 0, -15);

    /* 启动监视器（每 200ms 刷新一次 ADC 值） */
    monitor_timer = lv_timer_create(monitor_cb, 200, NULL);
    lv_timer_set_repeat_count(monitor_timer, -1);

    lv_scr_load_anim(scr, LV_SCR_LOAD_ANIM_FADE_OUT, 300, 0, false);
}

const app_item_t app_mic = {
    .name  = "Audio",
    .icon  = "🎤",
    .color = 0x2ecc71,
    .enter = app_mic_enter,
    .exit  = app_mic_cleanup,
};

void app_mic_init(void)
{
    /* 初始化麦克风运放和 ADC（使用 record_buf） */
    mic_opa_init();
    mic_adc_init(record_buf, RECORD_BUF_LEN);

    /* 初始化扬声器 DAC */
    speaker_dac_init();
}

void app_mic_cleanup(void)
{
    /* 停止录音 */
    if (is_recording) {
        mic_adc_collection_set(0);
        is_recording = 0;
    }
    /* 停止播放定时器 */
    if (play_status_timer) {
        lv_timer_del(play_status_timer);
        play_status_timer = NULL;
    }
    /* 停止监视器 */
    if (monitor_timer) {
        lv_timer_del(monitor_timer);
        monitor_timer = NULL;
    }
    monitor_label = NULL;
}
