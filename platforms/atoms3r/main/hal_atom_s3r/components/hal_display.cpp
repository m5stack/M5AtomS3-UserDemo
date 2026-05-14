/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
// Ref:  https://github.com/lovyan03/LovyanGFX/blob/master/examples/HowToUse/2_user_setting/2_user_setting.ino
// Panel autodetect aligned with M5GFX M5AtomS3R (ESP32-S3 LGA56): ST7735S vs GC9107
#include "../hal_atom_s3r.h"
#include "../hal_config.h"
#include <mooncake.h>
#include <lgfx/v1/panel/Panel_ST7735.hpp>
#include <lgfx/v1/panel/Panel_GC9A01.hpp>
#include "../utils/lp5562/lp5562.h"
#include "spdlog/spdlog.h"
#include <M5UnitOLED.h>

#include <driver/spi_master.h>
#include <memory>

/* -------------------------------------------------------------------------- */
/*                                  Backlight                                 */
/* -------------------------------------------------------------------------- */
LP5562_Class* __get_led_controller();

class Light_AtomS3R : public lgfx::ILight {
public:
    bool init(uint8_t brightness) override
    {
        spdlog::info("lcd backlight init");
        return true;
    }

    void setBrightness(uint8_t brightness) override
    {
        if (__get_led_controller() == nullptr) {
            spdlog::warn("led controller is not available");
            return;
        }
        __get_led_controller()->setBrightness(3, brightness);
    }
};

namespace {

void pin_level(int_fast16_t pin, bool level)
{
    lgfx::pinMode(pin, lgfx::pin_mode_t::output);
    if (level) {
        lgfx::gpio_hi(pin);
    } else {
        lgfx::gpio_lo(pin);
    }
}

void pin_reset(int_fast16_t pin, bool use_reset)
{
    lgfx::gpio_hi(pin);
    lgfx::pinMode(pin, lgfx::pin_mode_t::output);
    lgfx::delay(1);
    if (!use_reset) {
        return;
    }
    lgfx::gpio_lo(pin);
    lgfx::delay(2);
    lgfx::gpio_hi(pin);
    lgfx::delay(10);
}

uint32_t read_panel_id(lgfx::Bus_SPI* bus, int_fast16_t pin_cs, uint32_t cmd = 0x04, uint8_t dummy_read_bit = 1)
{
    bus->beginTransaction();
    pin_level(pin_cs, true);
    bus->writeCommand(0, 8);
    bus->wait();
    pin_level(pin_cs, false);
    bus->writeCommand(cmd, 8);
    bus->beginRead(dummy_read_bit);
    uint32_t res = bus->readData(32);
    bus->endTransaction();
    pin_level(pin_cs, true);
    spdlog::debug("LCD panel ID (cmd 0x{:02x}): 0x{:08x}", (unsigned)cmd, (unsigned)res);
    return res;
}

}  // namespace

/* -------------------------------------------------------------------------- */
/*                                    Panel                                   */
/* -------------------------------------------------------------------------- */

class LGFX_AtomS3R : public lgfx::LGFX_Device {
    lgfx::Bus_SPI _bus_instance;
    Light_AtomS3R _light_instance;
    std::unique_ptr<lgfx::Panel_Device> _panel_owned;

public:
    LGFX_AtomS3R(void)
    {
        pin_reset(HAL_PIN_LCD_RST, true);

        auto bus_cfg        = _bus_instance.config();
        bus_cfg.pin_mosi    = HAL_PIN_LCD_MOSI;
        bus_cfg.pin_miso    = (gpio_num_t)HAL_PIN_LCD_MISO;
        bus_cfg.pin_sclk    = HAL_PIN_LCD_SCLK;
        bus_cfg.pin_dc      = HAL_PIN_LCD_DC;
        bus_cfg.spi_mode    = 0;
        bus_cfg.spi_3wire   = true;
        bus_cfg.freq_write  = 8000000;
        bus_cfg.freq_read   = 8000000;
        bus_cfg.spi_host    = SPI2_HOST;
        bus_cfg.dma_channel = SPI_DMA_CH_AUTO;
        bus_cfg.use_lock    = true;
        _bus_instance.config(bus_cfg);
        _bus_instance.init();

        const uint32_t id    = read_panel_id(&_bus_instance, HAL_PIN_LCD_CS);
        const bool is_st7735 = ((id & 0xFFFF) == 0x7683 || (id & 0xFFFF) == 0x897C);
        const bool is_gc9107 = (id & 0xFFFFFF) == 0x079100;

        if (!is_st7735 && !is_gc9107) {
            spdlog::error("AtomS3R LCD: unknown panel ID 0x{:08x} (expected ST7735S or GC9107)", (unsigned)id);
            _bus_instance.release();
            setPanel(nullptr);
            return;
        }

        _bus_instance.release();
        bus_cfg.spi_host   = SPI3_HOST;
        bus_cfg.freq_write = 40000000;
        bus_cfg.freq_read  = 16000000;
        bus_cfg.spi_3wire  = true;
        _bus_instance.config(bus_cfg);
        _bus_instance.init();

        if (is_st7735) {
            spdlog::info("AtomS3R LCD: ST7735S");
            auto* p = new lgfx::Panel_ST7735S();
            p->bus(&_bus_instance);
            auto cfg            = p->config();
            cfg.pin_cs          = HAL_PIN_LCD_CS;
            cfg.pin_rst         = HAL_PIN_LCD_RST;
            cfg.panel_width     = 128;
            cfg.panel_height    = 128;
            cfg.offset_x        = 2;
            cfg.offset_y        = 31;
            cfg.offset_rotation = 2;
            cfg.readable        = true;
            cfg.bus_shared      = false;
            cfg.invert          = true;
            p->config(cfg);
            _panel_owned.reset(p);
        } else {
            spdlog::info("AtomS3R LCD: GC9107");
            auto* p = new lgfx::Panel_GC9107();
            p->bus(&_bus_instance);
            auto cfg         = p->config();
            cfg.pin_cs       = HAL_PIN_LCD_CS;
            cfg.pin_rst      = HAL_PIN_LCD_RST;
            cfg.panel_width  = 128;
            cfg.panel_height = 128;
            cfg.offset_y     = 32;
            cfg.readable     = false;
            cfg.bus_shared   = false;
            p->config(cfg);
            _panel_owned.reset(p);
        }

        _panel_owned->setLight(&_light_instance);
        setPanel(_panel_owned.get());
    }

    ~LGFX_AtomS3R() override
    {
        setPanel(nullptr);
    }
};

void HAL_AtomS3R::disp_init()
{
    spdlog::info("display init");

    assert(_data.display == nullptr);
    _data.display = new LGFX_AtomS3R;
    if (!_data.display->init()) {
        delete (_data.display);
        _data.display = nullptr;
        spdlog::warn("display init failed");
    }

    assert(_data.canvas == nullptr);
    if (_data.display != nullptr) {
        _data.canvas = new LGFX_SpriteFx(_data.display);
        _data.canvas->createSprite(_data.display->width(), _data.display->height());
    }

    assert(_data.unit_oled == nullptr);
    _data.unit_oled = new M5UnitOLED(HAL_PIN_I2C_EXTER_SDA, HAL_PIN_I2C_EXTER_SCL, 400000, 1);
    if (!_data.unit_oled->init()) {
        delete _data.unit_oled;
        _data.unit_oled = nullptr;
        spdlog::warn("unit oled init failed");
    } else {
        _data.unit_oled->setRotation(1);
        _data.unit_oled->fillScreen(TFT_WHITE);
    }
}

void HAL_AtomS3R::disp_test()
{
    while (1) {
        feedTheDog();

        spdlog::info("rrr");
        _data.display->fillScreen(TFT_RED);
        delay(500);

        spdlog::info("ggg");
        _data.display->fillScreen(TFT_GREEN);
        delay(500);

        spdlog::info("bbb");
        _data.display->fillScreen(TFT_BLUE);
        delay(500);

        spdlog::info("fff");
        _data.display->fillScreen(TFT_WHITE);
        _data.display->drawRect(0, 0, _data.display->width(), _data.display->height(), TFT_RED);
        delay(3000);
    }
}
