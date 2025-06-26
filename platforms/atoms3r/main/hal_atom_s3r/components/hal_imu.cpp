/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "../hal_atom_s3r.h"
#include "../hal_config.h"
#include "../utils/bmi270/src/bmi270.h"
#include <cstdint>
#include <cstdio>
#include <mooncake.h>
#include <driver/gpio.h>
#include "esp32-hal.h"
#include <smooth_ui_toolkit.h>
#include <cmath>
#include <cfloat>
#include <nvs_flash.h>
#include <nvs.h>
// https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bmi270-ds000.pdf
// https://github.com/boschsensortec/BMI270_SensorAPI
// https://github.com/arduino-libraries/Arduino_BMI270_BMM150

using namespace SmoothUIToolKit;

static BMI270_Class* _imu = nullptr;
static bool _is_bmm150_ok = false;

bool loadMagCalibrationFromNvs();

void HAL_AtomS3R::imu_init()
{
    spdlog::info("imu init");

    assert(_imu == nullptr);
    _imu = new BMI270_Class();
    if (!_imu->init()) {
        delete _imu;
        _imu = nullptr;
        // popFatalError("imu init failed");
        spdlog::warn("bmi270 init failed");
    } else {
        spdlog::info("bmi270 init ok");
    }

    if (!_imu->initAuxBmm150()) {
        delete _imu;
        _imu = nullptr;
        // popFatalError("imu init bmm150 failed");
        _is_bmm150_ok = false;
        spdlog::warn("bmm150 init failed");
    } else {
        _is_bmm150_ok = true;
        spdlog::info("bmm150 init ok");
    }

    // Interrupt
    if (_imu != nullptr) {
        // _imu->setWristWearWakeup();
        if (!_imu->enableAnyMotionInterrupt()) {
            // popFatalError("imu enable any motion intterrupt failed");
            spdlog::warn("imu enable any motion intterrupt failed");
        } else {
            spdlog::info(" enable any motion intterrupt");
        }

        // Pin
        gpio_reset_pin((gpio_num_t)HAL_PIN_IMU_INT);
        gpio_set_direction((gpio_num_t)HAL_PIN_IMU_INT, GPIO_MODE_INPUT);
        gpio_set_pull_mode((gpio_num_t)HAL_PIN_IMU_INT, GPIO_FLOATING);
    }

    loadMagCalibrationFromNvs();

    // /* -------------------------------------------------------------------------- */
    // /*                                    Test                                    */
    // /* -------------------------------------------------------------------------- */
    // _imu_test();
    // _imu_keep_sending_data();
}

void HAL_AtomS3R::updateImuData()
{
    // _imu->readAcceleration(_data.imu_data.accelX, _data.imu_data.accelY, _data.imu_data.accelZ);
    _imu->readAcceleration(_data.imu_data.accelY, _data.imu_data.accelX, _data.imu_data.accelZ);
    _imu->readGyroscope(_data.imu_data.gyroX, _data.imu_data.gyroY, _data.imu_data.gyroZ);
    _imu->readMagneticField(_data.imu_data.magX, _data.imu_data.magY, _data.imu_data.magZ);

    // Reverse
    _data.imu_data.magX = -_data.imu_data.magX;
    _data.imu_data.magZ = -_data.imu_data.magZ;
}

bool HAL_AtomS3R::getImuInterruptState()
{
    return gpio_get_level((gpio_num_t)HAL_PIN_IMU_INT) == 0;
}

bool HAL_AtomS3R::isImuAvailable()
{
    return _imu != nullptr;
}

bool HAL_AtomS3R::isImuMagAvailable()
{
    return _is_bmm150_ok;
}

void HAL_AtomS3R::updateImuTiltBallOffset()
{
    // spdlog::info("{} {} {}", _data.imu_data.accelX, _data.imu_data.accelY, _data.imu_data.accelZ);

    static float value_limit        = 0.7;
    static int offset_limit         = 12;
    static float tilt_offset_factor = (float)offset_limit / value_limit;

    _data.imu_data.tiltBallOffsetX = _data.imu_data.accelX * tilt_offset_factor;
    _data.imu_data.tiltBallOffsetY = _data.imu_data.accelY * tilt_offset_factor;

    // Limit
    _data.imu_data.tiltBallOffsetX =
        SmoothUIToolKit::Clamp(_data.imu_data.tiltBallOffsetX, {-offset_limit, offset_limit});
    _data.imu_data.tiltBallOffsetY =
        SmoothUIToolKit::Clamp(_data.imu_data.tiltBallOffsetY, {-offset_limit, offset_limit});

    // spdlog::info("{} {}", _data.imu_data.tiltBallOffsetX, _data.imu_data.tiltBallOffsetY);
}

struct MagCalibrationData {
    float offsetX = 0.0f;
    float offsetY = 0.0f;
};

MagCalibrationData magCalib;

const char* NVS_NAMESPACE = "calib";
const char* NVS_KEY_X     = "mag_offset_x";
const char* NVS_KEY_Y     = "mag_offset_y";

bool saveMagCalibrationToNvs()
{
    spdlog::info("saveMagCalibrationToNvs");

    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        spdlog::error("nvs_open failed");
        return false;
    }

    nvs_set_blob(handle, NVS_KEY_X, &magCalib.offsetX, sizeof(float));
    nvs_set_blob(handle, NVS_KEY_Y, &magCalib.offsetY, sizeof(float));
    nvs_commit(handle);
    nvs_close(handle);

    return true;
}

bool loadMagCalibrationFromNvs()
{
    spdlog::info("loadMagCalibrationFromNvs");

    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
    if (err != ESP_OK) {
        spdlog::error("nvs_open failed");
        return false;
    }

    size_t required_size = sizeof(float);
    float ox = 0.0f, oy = 0.0f;

    err = nvs_get_blob(handle, NVS_KEY_X, &ox, &required_size);
    if (err != ESP_OK) {
        nvs_close(handle);
        return false;
    }

    err = nvs_get_blob(handle, NVS_KEY_Y, &oy, &required_size);
    if (err != ESP_OK) {
        nvs_close(handle);
        return false;
    }

    nvs_close(handle);

    magCalib.offsetX = ox;
    magCalib.offsetY = oy;

    spdlog::info("loadMagCalibrationFromNvs ok, offsetX: {}, offsetY: {}", ox, oy);

    return true;
}

void HAL_AtomS3R::calibrateMagnetometer(std::function<void()> onUpdate)
{
    float magX_min = FLT_MAX, magX_max = -FLT_MAX;
    float magY_min = FLT_MAX, magY_max = -FLT_MAX;

    constexpr int samples  = 500;
    constexpr int delay_ms = 10;

    spdlog::info("Start magnetometer calibration...");
    spdlog::info("Please slowly rotate the device in all directions for a few seconds.");

    for (int i = 0; i < samples; ++i) {
        updateImuData();
        feedTheDog();
        onUpdate();

        float magX = _data.imu_data.magX;
        float magY = _data.imu_data.magY;

        if (magX < magX_min) magX_min = magX;
        if (magX > magX_max) magX_max = magX;
        if (magY < magY_min) magY_min = magY;
        if (magY > magY_max) magY_max = magY;

        delay(delay_ms);  // 延迟以采样更多方向
    }

    // 计算 offset（硬铁干扰）
    magCalib.offsetX = (magX_max + magX_min) / 2.0f;
    magCalib.offsetY = (magY_max + magY_min) / 2.0f;

    spdlog::info("Calibration complete.");
    spdlog::info("Offset X: {}, Offset Y: {}", magCalib.offsetX, magCalib.offsetY);

    saveMagCalibrationToNvs();
}

// 计算磁力计 yaw 角（单位：度）
static float _calculate_compass_yaw(float magX_raw, float magY_raw)
{
    float magX = magX_raw;
    float magY = magY_raw;

    magX -= magCalib.offsetX;
    magY -= magCalib.offsetY;

    float yaw_rad = atan2(-magX, -magY);
    float yaw_deg = yaw_rad * (180.0f / M_PI);

    yaw_deg -= 180.0f;
    if (yaw_deg < 0) {
        yaw_deg += 360.0f;
    }

    return yaw_deg;
}

void HAL_AtomS3R::updateImuDialAngle()
{
    float yaw                = _calculate_compass_yaw(_data.imu_data.magX, _data.imu_data.magY);
    _data.imu_data.dialAngle = static_cast<int32_t>(yaw * 10);
}

void HAL_AtomS3R::imu_test()
{
    // float ax, ay, az, gx, gy, gz, mx, my, mz;
    int hit = 1;
    while (1) {
        feedTheDog();
        // delay(100);
        delay(20);

        hit = gpio_get_level((gpio_num_t)HAL_PIN_IMU_INT);

        updateImuData();
        spdlog::info("{} | {:.1f} {:.1f} {:.1f} | {:.1f} {:.1f} {:.1f} | {:.1f} {:.1f} {:.1f}", hit,
                     getImuData().accelX, getImuData().accelY, getImuData().accelZ, getImuData().gyroX,
                     getImuData().gyroX, getImuData().gyroZ, getImuData().magX, getImuData().magY, getImuData().magZ);

        if (hit == 0) {
            spdlog::info("hit");
            delay(500);
        }
    }
}

#include <ArduinoJson.h>

void HAL_AtomS3R::imu_keep_sending_data()
{
    spdlog::info("start sending imu msg");

    JsonDocument doc;
    std::string json_buffer;

    while (1) {
        delay(5);
        feedTheDog();

        updateImuData();

        // Encode json
        doc["ax"] = getImuData().accelX;
        doc["ay"] = getImuData().accelY;
        doc["az"] = getImuData().accelZ;
        doc["gx"] = getImuData().gyroX;
        doc["gy"] = getImuData().gyroY;
        doc["gz"] = getImuData().gyroZ;
        doc["mx"] = getImuData().magX;
        doc["my"] = getImuData().magY;
        doc["mz"] = getImuData().magZ;

        serializeJson(doc, json_buffer);

        printf("%s\n", json_buffer.c_str());
    }
}
