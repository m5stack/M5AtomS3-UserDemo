/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>
#include <M5GFX.h>
#include "utils/lgfx_fx/lgfx_fx.h"
#include "utils/serial/serial_base.h"
#include "types.h"

/**
 * @brief Provide a dingleton to abstract hardware methods
 * 1) Inherit and override methods to create a specific hal
 * 2) Use HAL::Inject() to inject your hal
 * 3) Use HAL:Get() to get this hal wherever you want
 */
class HAL {
private:
    static HAL* _hal;

public:
    /**
     * @brief Get HAL instance
     *
     * @return HAL*
     */
    static HAL* Get();

    /**
     * @brief Check if HAL is valid
     *
     * @return true
     * @return false
     */
    static bool Check();

    /**
     * @brief HAL injection, init() will be called here
     *
     * @param hal
     * @return true
     * @return false
     */
    static bool Inject(HAL* hal);

    /**
     * @brief Destroy HAL instance
     *
     */
    static void Destroy();

    /**
     * @brief Base class
     *
     */
public:
    HAL()
    {
    }
    virtual ~HAL()
    {
    }

    static std::string Type()
    {
        return Get()->type();
    }
    virtual std::string type()
    {
        return "Base";
    }

    static std::string CompileDate()
    {
        return Get()->compileDate();
    }
    virtual std::string compileDate()
    {
        return __DATE__;
    }

    virtual void init()
    {
    }

    /**
     * @brief Components
     *
     */
protected:
    struct Data_t {
        LGFX_Device* display   = nullptr;
        LGFX_SpriteFx* canvas  = nullptr;
        LGFX_Device* unit_oled = nullptr;
        time_t time_buffer;
        CONFIG::SystemConfig_t config;
        IMU::ImuData_t imu_data;
    };
    Data_t _data;

    /* -------------------------------------------------------------------------- */
    /*                                   Display                                  */
    /* -------------------------------------------------------------------------- */
public:
    /**
     * @brief Display device
     *
     * @return LGFX_Device*
     */
    static LGFX_Device* GetDisplay()
    {
        return Get()->_data.display;
    }

    /**
     * @brief Full screen canvas (sprite)
     *
     * @return LGFX_SpriteFx*
     */
    static LGFX_SpriteFx* GetCanvas()
    {
        return Get()->_data.canvas;
    }

    static LGFX_Device* GetUnitOled()
    {
        return Get()->_data.unit_oled;
    }

    /**
     * @brief Push framebuffer
     *
     */
    static void CanvasUpdate()
    {
        Get()->canvasUpdate();
    }
    virtual void canvasUpdate()
    {
        GetCanvas()->pushSprite(0, 0);
    }

    /**
     * @brief Render fps panel
     *
     */
    static void RenderFpsPanel()
    {
        Get()->renderFpsPanel();
    }
    virtual void renderFpsPanel();

    /**
     * @brief Pop error message and wait reboot
     *
     * @param msg
     */
    static void PopFatalError(std::string msg)
    {
        Get()->popFatalError(msg);
    }
    virtual void popFatalError(std::string msg);

    /**
     * @brief Pop warning message and wait continue
     *
     * @param msg
     */
    static void PopWarning(std::string msg)
    {
        Get()->popWarning(msg);
    }
    virtual void popWarning(std::string msg);

    /**
     * @brief Pop success message and wait continue
     *
     * @param msg
     */
    static void PopSuccess(std::string msg, bool showSuccessLabel = true)
    {
        Get()->popSuccess(msg, showSuccessLabel);
    }
    virtual void popSuccess(std::string msg, bool showSuccessLabel = true);

    /* -------------------------------------------------------------------------- */
    /*                                    Lvgl                                    */
    /* -------------------------------------------------------------------------- */
public:
    static bool LvglInit()
    {
        return Get()->lvglInit();
    }
    virtual bool lvglInit()
    {
        return false;
    }

    static bool LvglDeinit()
    {
        return Get()->lvglDeinit();
    }
    virtual bool lvglDeinit()
    {
        return false;
    }

    static void LvglTimerHandler()
    {
        Get()->lvglTimerHandler();
    }
    virtual void lvglTimerHandler()
    {
    }

    /* -------------------------------------------------------------------------- */
    /*                                   System                                   */
    /* -------------------------------------------------------------------------- */
public:
    /**
     * @brief Delay(ms)
     *
     * @param milliseconds
     */
    static void Delay(unsigned long milliseconds)
    {
        Get()->delay(milliseconds);
    }
    virtual void delay(unsigned long milliseconds)
    {
        lgfx::delay(milliseconds);
    }

    /**
     * @brief Get the number of milliseconds passed since boot
     *
     * @return unsigned long
     */
    static unsigned long Millis()
    {
        return Get()->millis();
    }
    virtual unsigned long millis()
    {
        return lgfx::millis();
    }

    /**
     * @brief Power off
     *
     */
    static void PowerOff()
    {
        Get()->powerOff();
    }
    virtual void powerOff()
    {
    }

    /**
     * @brief Reboot
     *
     */
    static void Reboot()
    {
        Get()->reboot();
    }
    virtual void reboot()
    {
    }

    /**
     * @brief Set RTC time
     *
     * @param dateTime
     */
    static void SetSystemTime(tm dateTime)
    {
        return Get()->setSystemTime(dateTime);
    }
    virtual void setSystemTime(tm dateTime)
    {
    }

    /**
     * @brief Get local time(wrap of localtime())
     *
     * @return tm*
     */
    static tm* GetLocalTime()
    {
        return Get()->getLocalTime();
    }
    virtual tm* getLocalTime();

    /**
     * @brief Reset system watch dog
     *
     */
    static void FeedTheDog()
    {
        Get()->feedTheDog();
    }
    virtual void feedTheDog()
    {
    }

    /* -------------------------------------------------------------------------- */
    /*                                     MSC                                    */
    /* -------------------------------------------------------------------------- */
public:
    /**
     * @brief Start MSC mode to expose fs as a USB disk
     *
     */
    static void StartMscMode()
    {
        Get()->startMscMode();
    }
    virtual void startMscMode()
    {
    }

    /**
     * @brief Stop MSC mode
     *
     */
    static void StopMscMode()
    {
        Get()->stopMscMode();
    }
    virtual void stopMscMode()
    {
    }

    /* -------------------------------------------------------------------------- */
    /*                                System config                               */
    /* -------------------------------------------------------------------------- */
public:
    /**
     * @brief Load system config from fs
     *
     */
    static void LoadSystemConfig()
    {
        Get()->loadSystemConfig();
    }
    virtual void loadSystemConfig()
    {
    }

    /**
     * @brief Save system config to fs
     *
     */
    static void SaveSystemConfig()
    {
        Get()->saveSystemConfig();
    }
    virtual void saveSystemConfig()
    {
    }

    /**
     * @brief Get system config
     *
     * @return CONFIG::SystemConfig_t&
     */
    static CONFIG::SystemConfig_t& GetSystemConfig()
    {
        return Get()->_data.config;
    }

    /**
     * @brief Set system config
     *
     * @param cfg
     */
    static void SetSystemConfig(CONFIG::SystemConfig_t cfg)
    {
        Get()->_data.config = cfg;
    }

    /**
     * @brief Apply system config to device
     *
     */
    static void ApplySystemConfig()
    {
        Get()->applySystemConfig();
    }
    virtual void applySystemConfig()
    {
    }

    /* -------------------------------------------------------------------------- */
    /*                                   Buzzer                                   */
    /* -------------------------------------------------------------------------- */
public:
    /**
     * @brief Buzzer beep
     *
     * @param frequency
     * @param duration
     */
    static void Beep(float frequency, uint32_t duration = 4294967295U)
    {
        Get()->beep(frequency, duration);
    }
    virtual void beep(float frequency, uint32_t duration)
    {
    }

    /**
     * @brief Stop buzzer beep
     *
     */
    static void BeepStop()
    {
        Get()->beepStop();
    }
    virtual void beepStop()
    {
    }

    /* -------------------------------------------------------------------------- */
    /*                                   Gamepad                                  */
    /* -------------------------------------------------------------------------- */
public:
    /**
     * @brief Get button state, 获取按键状态
     *
     * @param button
     * @return true Pressing, 按下
     * @return false Released, 松开
     */
    static bool GetButton(GAMEPAD::GamePadButton_t button)
    {
        return Get()->getButton(button);
    }
    virtual bool getButton(GAMEPAD::GamePadButton_t button)
    {
        return false;
    }

    /**
     * @brief Get any button state, 获取任意按键状态
     *
     * @return true Pressing, 按下
     * @return false Released, 松开
     */
    static bool GetAnyButton()
    {
        return Get()->getAnyButton();
    }
    virtual bool getAnyButton();

    /* -------------------------------------------------------------------------- */
    /*                                   Encoder                                  */
    /* -------------------------------------------------------------------------- */
public:
    /**
     * @brief Update encoder count
     *
     */
    static void UpdateEncoderCount()
    {
        Get()->updateEncoderCount();
    }
    virtual void updateEncoderCount()
    {
    }

    /**
     * @brief Get encoder count
     *
     * @return int
     */
    static int GetEncoderCount()
    {
        return Get()->getEncoderCount();
    }
    virtual int getEncoderCount()
    {
        return 0;
    }

    /**
     * @brief Reset encoder count
     *
     */
    static void ResetEncoderCount(int value = 0)
    {
        Get()->resetEncoderCount(value);
    }
    virtual void resetEncoderCount(int value)
    {
    }

    /* -------------------------------------------------------------------------- */
    /*                                  Touchpad                                  */
    /* -------------------------------------------------------------------------- */
public:
    /**
     * @brief Update touch
     *
     */
    static void UpdateTouch()
    {
        Get()->updateTouch();
    }
    virtual void updateTouch()
    {
    }

    /**
     * @brief Is touching
     *
     * @return int
     */
    static int IsTouching()
    {
        return Get()->isTouching();
    }
    virtual bool isTouching()
    {
        return false;
    }

    /**
     * @brief Get touch point
     *
     * @return TOUCH::Point_t
     */
    static TOUCH::Point_t GetTouchPoint()
    {
        return Get()->getTouchPoint();
    }
    virtual TOUCH::Point_t getTouchPoint()
    {
        return {-1, -1};
    }

    /* -------------------------------------------------------------------------- */
    /*                                     IMU                                    */
    /* -------------------------------------------------------------------------- */
public:
    static bool IsImuAvailable()
    {
        return Get()->isImuAvailable();
    }
    virtual bool isImuAvailable()
    {
        return false;
    }

    static bool IsImuMagAvailable()
    {
        return Get()->isImuMagAvailable();
    }
    virtual bool isImuMagAvailable()
    {
        return false;
    }

    /**
     * @brief Update IMU data, 刷新IMU数据
     *
     */
    static void UpdateImuData()
    {
        Get()->updateImuData();
    }
    virtual void updateImuData()
    {
    }

    /**
     * @brief Get the Imu Data, 获取IMU数据
     *
     * @return const IMU::ImuData_t&
     */
    static const IMU::ImuData_t& GetImuData()
    {
        return Get()->getImuData();
    }
    const IMU::ImuData_t& getImuData()
    {
        return _data.imu_data;
    }

    /**
     * @brief Get the Imu Interrupt State object
     *
     * @return true triggered
     * @return false
     */
    static bool GetImuInterruptState()
    {
        return Get()->getImuInterruptState();
    }
    virtual bool getImuInterruptState()
    {
        return false;
    }

    static void UpdateImuTiltBallOffset()
    {
        Get()->updateImuTiltBallOffset();
    }
    virtual void updateImuTiltBallOffset()
    {
    }

    static void UpdateImuDialAngle()
    {
        Get()->updateImuDialAngle();
    }
    virtual void updateImuDialAngle()
    {
    }

    static void StartImuMagCalibration(uint32_t duration = 10000)
    {
        Get()->startImuMagCalibration(duration);
    }
    virtual void startImuMagCalibration(uint32_t duration = 10000)
    {
    }

    static void CalibrateMagnetometer(std::function<void()> onUpdate)
    {
        Get()->calibrateMagnetometer(onUpdate);
    }
    virtual void calibrateMagnetometer(std::function<void()> onUpdate)
    {
        for (int i = 0; i < 5000; i += 10) {
            this->delay(10);
            onUpdate();
        }
    }

    /* -------------------------------------------------------------------------- */
    /*                                   Network                                  */
    /* -------------------------------------------------------------------------- */
public:
    static bool CheckWifiConfig()
    {
        return Get()->checkWifiConfig();
    }
    virtual bool checkWifiConfig();

    static bool ConnectWifi(OnLogPageRenderCallback_t onLogPageRender, bool reconnect = false)
    {
        return Get()->connectWifi(onLogPageRender, reconnect);
    }
    virtual bool connectWifi(OnLogPageRenderCallback_t onLogPageRender, bool reconnect)
    {
        return false;
    }

    /* -------------------------------------------------------------------------- */
    /*                                     OTA                                    */
    /* -------------------------------------------------------------------------- */
public:
    static OTA_UPGRADE::OtaInfo_t GetLatestFirmwareInfoViaOta(OnLogPageRenderCallback_t onLogPageRender)
    {
        return Get()->getLatestFirmwareInfoViaOta(onLogPageRender);
    }
    virtual OTA_UPGRADE::OtaInfo_t getLatestFirmwareInfoViaOta(OnLogPageRenderCallback_t onLogPageRender)
    {
        return OTA_UPGRADE::OtaInfo_t();
    }

    static bool UpgradeFirmwareViaOta(OnLogPageRenderCallback_t onLogPageRender, const std::string& firmwareUrl)
    {
        return Get()->upgradeFirmwareViaOta(onLogPageRender, firmwareUrl);
    }
    virtual bool upgradeFirmwareViaOta(OnLogPageRenderCallback_t onLogPageRender, const std::string& firmwareUrl)
    {
        return false;
    }

    /* -------------------------------------------------------------------------- */
    /*                                     IR                                     */
    /* -------------------------------------------------------------------------- */
public:
    static void IrSendNecMsg(uint16_t addr, uint16_t command)
    {
        Get()->irSendNecMsg(addr, command);
    }
    virtual void irSendNecMsg(uint16_t addr, uint16_t command)
    {
    }

    /* -------------------------------------------------------------------------- */
    /*                                    Psram                                   */
    /* -------------------------------------------------------------------------- */
public:
    static size_t GetPsramSize()
    {
        return Get()->getPsramSize();
    }
    // virtual size_t getPsramSize() { return 0; }
    virtual size_t getPsramSize()
    {
        return 0;
    }

    /* -------------------------------------------------------------------------- */
    /*                                Factory Test                                */
    /* -------------------------------------------------------------------------- */
public:
    static void StartWifiFactoryTestDaemon()
    {
        Get()->startWifiFactoryTestDaemon();
    }
    virtual void startWifiFactoryTestDaemon()
    {
    }

    static FACTORY_TEST::WifiTestResult_t* GetWifiFactoryTestResult()
    {
        return Get()->getWifiFactoryTestResult();
    }
    virtual FACTORY_TEST::WifiTestResult_t* getWifiFactoryTestResult()
    {
        return (FACTORY_TEST::WifiTestResult_t*)nullptr;
    }

    static void StartBleFactoryTestDaemon()
    {
        Get()->startBleFactoryTestDaemon();
    }
    virtual void startBleFactoryTestDaemon()
    {
    }

    static void StartMbusFactoryTestDaemon()
    {
        Get()->startMbusFactoryTestDaemon();
    }
    virtual void startMbusFactoryTestDaemon()
    {
    }

    static bool IsLedControllerAvailable()
    {
        return Get()->isLedControllerAvailable();
    }
    virtual bool isLedControllerAvailable()
    {
        return true;
    }

    /* -------------------------------------------------------------------------- */
    /*                                    Misc                                    */
    /* -------------------------------------------------------------------------- */
public:
    static int RandomInt(int min, int max)
    {
        return Get()->randomInt(min, max);
    }
    virtual int randomInt(int min, int max);

    /* -------------------------------------------------------------------------- */
    /*                                  User demo                                 */
    /* -------------------------------------------------------------------------- */
public:
    /* ---------------------------------- Wifi ---------------------------------- */
    static bool StartWifiScan()
    {
        return Get()->startWifiScan();
    }
    virtual bool startWifiScan()
    {
        return false;
    }

    static bool IsWifiScanFinish()
    {
        return Get()->isWifiScanFinish();
    }
    virtual bool isWifiScanFinish()
    {
        return false;
    }

    static std::vector<NETWORK::WifiScanResult_t>* GetWifiScanResult()
    {
        return Get()->getWifiScanResult();
    }
    virtual std::vector<NETWORK::WifiScanResult_t>* getWifiScanResult()
    {
        return nullptr;
    }

    static void FreeWifiScanResult()
    {
        Get()->freeWifiScanResult();
    }
    virtual void freeWifiScanResult()
    {
    }

    /* ----------------------------------- I2C ---------------------------------- */
    static bool StartI2CScan()
    {
        return Get()->startI2CScan();
    }
    virtual bool startI2CScan()
    {
        return false;
    }

    static std::vector<uint8_t>* GetI2cScanResult()
    {
        return Get()->getI2cScanResult();
    }
    virtual std::vector<uint8_t>* getI2cScanResult()
    {
        return nullptr;
    }

    static void FreeI2cScanResult()
    {
        Get()->freeI2cScanResult();
    }
    virtual void freeI2cScanResult()
    {
    }

    /* ---------------------------------- Uart ---------------------------------- */
    static void StartUartPassThrough(uint32_t baudrate, int pinRx, int pinTx)
    {
        Get()->startUartPassThrough(baudrate, pinRx, pinTx);
    }
    virtual void startUartPassThrough(uint32_t baudrate, int pinRx, int pinTx)
    {
    }

    static void UpdateUartPassThrough(std::function<void(uint8_t)> onGrove2Usb,
                                      std::function<void(uint8_t)> onUsb2Grove)
    {
        Get()->updateUartPassThrough(onGrove2Usb, onUsb2Grove);
    }
    virtual void updateUartPassThrough(std::function<void(uint8_t)> onGrove2Usb,
                                       std::function<void(uint8_t)> onUsb2Grove)
    {
    }

    static void StopUartPassThrough()
    {
        Get()->stopUartPassThrough();
    }
    virtual void stopUartPassThrough()
    {
    }

    /* ----------------------------------- PWM ---------------------------------- */
    static void StartPwm()
    {
        Get()->startPwm();
    }
    virtual void startPwm()
    {
    }

    static void ChangePwmDuty(uint8_t duty)
    {
        Get()->changePwmDuty(duty);
    }
    virtual void changePwmDuty(uint8_t duty)
    {
    }

    static void StopPwm()
    {
        Get()->stopPwm();
    }
    virtual void stopPwm()
    {
    }

    /* ----------------------------------- ADC ---------------------------------- */
    static int GetAdcValue(uint8_t port)
    {
        return Get()->getAdcValue(port);
    }
    virtual int getAdcValue(uint8_t port)
    {
        return 114154;
    }

    /* ----------------------------------- IR ----------------------------------- */
    static void StartIr()
    {
        Get()->startIr();
    }
    virtual void startIr()
    {
    }

    static bool GetIrCmdFromSerial(uint8_t& cmd)
    {
        return Get()->getIrCmdFromSerial(cmd);
    }
    virtual bool getIrCmdFromSerial(uint8_t& cmd)
    {
        return false;
    }

    static bool IrSendCmd(uint8_t cmd)
    {
        return Get()->irSendCmd(cmd);
    }
    virtual bool irSendCmd(uint8_t cmd)
    {
        return false;
    }

    static void StopIr()
    {
        Get()->stopIr();
    }
    virtual void stopIr()
    {
    }
};
