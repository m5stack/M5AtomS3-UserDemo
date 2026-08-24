/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "../view.h"
#include <mooncake.h>
#include <hal/hal.h>
#include <assets/assets.h>
#include <shared/shared.h>
#include "lgfx/v1/misc/enum.hpp"
#include "spdlog/spdlog.h"
#include <lvgl.h>
#include <src/widgets/image/lv_image.h>

using namespace VIEW;
using namespace SmoothUIToolKit;

extern lv_image_dsc_t ui_img_imu_cross_mark_png;
extern lv_image_dsc_t ui_img_imu_dial_png;
extern lv_image_dsc_t ui_img_imu_tilt_ball_png;

static lv_obj_t* _lv_img_tilt_ball = nullptr;
static lv_obj_t* _lv_img_dial      = nullptr;

void func_imu_t::start()
{
    spdlog::info("start imu demo");

    // Clear
    lv_obj_clean(lv_screen_active());

    // Background
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_black(), 0);
    lv_obj_set_scrollbar_mode(lv_screen_active(), LV_SCROLLBAR_MODE_OFF);

    lv_obj_t* img_buffer = nullptr;

    // Dial
    ui_img_imu_dial_png.data      = AssetPool::GetImage().AppUserDemo.imu_dial;
    ui_img_imu_dial_png.data_size = sizeof(AssetPool::GetImage().AppUserDemo.imu_dial);

    img_buffer   = lv_image_create(lv_screen_active());
    _lv_img_dial = img_buffer;
    lv_image_set_src(img_buffer, &ui_img_imu_dial_png);
    lv_obj_align(img_buffer, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_width(img_buffer, LV_SIZE_CONTENT);
    lv_obj_set_height(img_buffer, LV_SIZE_CONTENT);
    lv_obj_add_flag(img_buffer, LV_OBJ_FLAG_ADV_HITTEST);
    lv_obj_remove_flag(img_buffer, LV_OBJ_FLAG_SCROLLABLE);

    // BMM150 is optional on newer AtomS3R hardware. Keep the level UI but hide
    // the compass dial when the magnetometer is not detected.
    if (!HAL::IsImuMagAvailable()) {
        lv_obj_add_flag(_lv_img_dial, LV_OBJ_FLAG_HIDDEN);
    }

    // Tilt ball
    ui_img_imu_tilt_ball_png.data      = AssetPool::GetImage().AppUserDemo.imu_tilt_ball;
    ui_img_imu_tilt_ball_png.data_size = sizeof(AssetPool::GetImage().AppUserDemo.imu_tilt_ball);

    img_buffer        = lv_image_create(lv_screen_active());
    _lv_img_tilt_ball = img_buffer;
    lv_image_set_src(img_buffer, &ui_img_imu_tilt_ball_png);
    lv_obj_align(img_buffer, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_width(img_buffer, LV_SIZE_CONTENT);
    lv_obj_set_height(img_buffer, LV_SIZE_CONTENT);
    lv_obj_add_flag(img_buffer, LV_OBJ_FLAG_ADV_HITTEST);
    lv_obj_remove_flag(img_buffer, LV_OBJ_FLAG_SCROLLABLE);

    // Cross mark
    ui_img_imu_cross_mark_png.data      = AssetPool::GetImage().AppUserDemo.imu_cross_mark;
    ui_img_imu_cross_mark_png.data_size = sizeof(AssetPool::GetImage().AppUserDemo.imu_cross_mark);

    img_buffer = lv_image_create(lv_screen_active());
    lv_image_set_src(img_buffer, &ui_img_imu_cross_mark_png);
    lv_obj_align(img_buffer, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_width(img_buffer, LV_SIZE_CONTENT);
    lv_obj_set_height(img_buffer, LV_SIZE_CONTENT);
    lv_obj_add_flag(img_buffer, LV_OBJ_FLAG_ADV_HITTEST);
    lv_obj_remove_flag(img_buffer, LV_OBJ_FLAG_SCROLLABLE);
}

void func_imu_t::update(bool btn_click)
{
    const bool mag_available = HAL::IsImuMagAvailable();

    if (btn_click && mag_available) {
        auto panel = lv_obj_create(lv_screen_active());
        lv_obj_set_style_bg_color(panel, lv_color_black(), 0);
        lv_obj_set_size(panel, 128, 128);
        auto label = lv_label_create(panel);
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
        lv_label_set_text(label, "Calibrating...\n\nRotate slowly\nin 8-shape");
        lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_text_color(label, lv_color_white(), 0);

        HAL::CalibrateMagnetometer([&]() { HAL::LvglTimerHandler(); });

        lv_obj_delete(panel);
    }

    HAL::UpdateImuData();
    HAL::UpdateImuTiltBallOffset();

    /* -------------------------------- Directly -------------------------------- */
    // Tilt ball
    lv_obj_align(_lv_img_tilt_ball, LV_ALIGN_CENTER, HAL::GetImuData().tiltBallOffsetX,
                 HAL::GetImuData().tiltBallOffsetY);
    // Dial (requires BMM150)
    if (mag_available) {
        HAL::UpdateImuDialAngle();
        lv_image_set_rotation(_lv_img_dial, HAL::GetImuData().dialAngle);
    }

    // Update and render
    HAL::LvglTimerHandler();
}

void func_imu_t::stop()
{
    spdlog::info("quit imu demo");
    lv_obj_clean(lv_screen_active());
}
