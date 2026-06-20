/**
 * @file app_system.h
 * @brief 系统主框架 — 菜单导航 + App 注册管理
 */
#ifndef _APP_SYSTEM_H_
#define _APP_SYSTEM_H_

#include "lvgl.h"

/* 每个 App 的描述符 */
typedef struct {
    const char *name;       /* 显示名称 */
    const char *icon;       /* 图标字符 */
    uint32_t    color;      /* 卡片颜色 (RGB hex) */
    void      (*enter)(void);  /* 进入该 App */
    void      (*exit)(void);   /* 退出该 App（清理定时器等） */
} app_item_t;

/**
 * @brief 注册一个 App 到系统菜单
 * @param app  App 描述符指针（必须保持有效）
 */
void app_system_register(const app_item_t *app);

/**
 * @brief 启动系统（创建主菜单并加载）
 *        在 LVGL 初始化完成后调用
 */
void app_system_start(void);

/**
 * @brief 切换到指定 App
 * @param app  App 描述符指针
 */
void app_system_open(const app_item_t *app);

/**
 * @brief 返回主菜单
 */
void app_system_back_to_menu(void);

/**
 * @brief 在 App 页面上创建顶栏（含返回按钮和标题）
 * @param scr    App 的 screen 对象
 * @param title  标题文字
 */
void app_system_create_topbar(lv_obj_t *scr, const char *title);

/**
 * @brief 获取当前 activity 数量（用于系统信息）
 */
uint32_t app_system_get_app_count(void);

#endif /* _APP_SYSTEM_H_ */
