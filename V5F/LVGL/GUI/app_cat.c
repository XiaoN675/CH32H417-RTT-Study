/**
 * @file app_cat.c
 * @brief 小猫 GIF 播放
 */

#include "app_cat.h"

extern const lv_img_dsc_t gif_image;

static void gif_restart_cb(lv_timer_t *t)
{
    lv_obj_t *gif = (lv_obj_t *)t->user_data;
    if (gif) lv_gif_restart(gif);
}

static void app_cat_enter(void)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_remove_style_all(scr);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    /* 顶栏带返回 */
    app_system_create_topbar(scr, "🐱 Cat GIF");

    /* GIF 播放器 */
    lv_obj_t *gif = lv_gif_create(scr);
    lv_gif_set_src(gif, &gif_image);
    lv_obj_center(gif);
    lv_obj_set_y(gif, 5);

    /* 延迟重启 GIF 动画（确保屏幕加载完成后开始播放） */
    lv_timer_t *t = lv_timer_create(gif_restart_cb, 100, gif);
    lv_timer_set_repeat_count(t, 1);

    lv_scr_load_anim(scr, LV_SCR_LOAD_ANIM_FADE_OUT, 300, 0, false);
}

const app_item_t app_cat = {
    .name  = "Cat GIF",
    .icon  = "🐱",
    .color = 0xe91e63,
    .enter = app_cat_enter,
    .exit  = NULL,
};
