/**
 * @file app_sysinfo.c
 * @brief 系统信息 — 芯片型号、主频、RTT 版本等
 */

#include "app_sysinfo.h"
#include "ch32h417.h"
#include "hal_config.h"

static void app_sysinfo_enter(void)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_remove_style_all(scr);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x16213e), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    app_system_create_topbar(scr, "System Info");

    /* 信息列表 */
    const char *lines[] = {
        "MCU:   CH32H417 (RISC-V)",
        "Freq:  144 MHz (HCLK)",
        "Flash: 512 KB",
        "SRAM:  64 KB",
        "LCD:   480 × 320 (ST7796)",
        "Touch: GT911 (I2C)",
#if HAL_RT_TREAD_ENABLE
        "OS:    RT-Thread",
        "Tick:  1 ms",
#else
        "OS:    Bare Metal",
#endif
        "LVGL:  v8.3.10",
        ""
    };

    lv_obj_t *list = lv_obj_create(scr);
    lv_obj_remove_style_all(list);
    lv_obj_set_size(list, 440, 260);
    lv_obj_align(list, LV_ALIGN_TOP_MID, 0, 48);
    lv_obj_set_style_bg_opa(list, LV_OPA_TRANSP, 0);

    uint8_t i = 0;
    while (lines[i][0] != '\0')
    {
        lv_obj_t *lbl = lv_label_create(list);
        lv_label_set_text(lbl, lines[i]);
        lv_obj_set_style_text_color(lbl, lv_color_hex(0xcccccc), 0);

        /* 交替行背景 */
        if (i % 2 == 0) {
            lv_obj_set_style_bg_color(lbl, lv_color_hex(0x1a1a2e), 0);
            lv_obj_set_style_bg_opa(lbl, LV_OPA_COVER, 0);
        }
        lv_obj_set_size(lbl, 440, 28);
        lv_obj_set_pos(lbl, 0, i * 28);
        i++;
    }

    lv_scr_load_anim(scr, LV_SCR_LOAD_ANIM_FADE_OUT, 300, 0, false);
}

const app_item_t app_sysinfo = {
    .name  = "System",
    .icon  = "ℹ️",
    .color = 0x3498db,
    .enter = app_sysinfo_enter,
    .exit  = NULL,
};
