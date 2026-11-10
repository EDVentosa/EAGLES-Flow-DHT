#include <stdio.h>

#include "ui.h"

#include "driver/spi_master.h"
#include "driver/gpio.h"

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"

#include "esp_lvgl_port.h"
#include "lvgl.h"
#include <stdlib.h>

#define LCD_HOST       SPI2_HOST

#define LCD_MOSI       19
#define LCD_SCLK       18
#define LCD_CS         5
#define LCD_DC         16
#define LCD_RST        23
#define LCD_BL         4

#define LCD_H_RES      135
#define LCD_V_RES      240

static lv_obj_t *flow_rate_label;
static lv_obj_t *temperature_label;
static lv_obj_t *humidity_label;

// Icons
static void create_thermometer_icon(lv_obj_t *parent, lv_coord_t x, lv_coord_t y)
{
    lv_color_t color = lv_color_hex(0x008FBC);

    // Thermometer stem
    lv_obj_t *stem = lv_obj_create(parent);
    lv_obj_set_size(stem, 6, 17);
    lv_obj_set_pos(stem, x + 6, y);
    lv_obj_set_style_radius(stem, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(stem, color, 0);
    lv_obj_set_style_border_width(stem, 0, 0);
    lv_obj_set_style_pad_all(stem, 0, 0);

    // Thermometer bulb
    lv_obj_t *bulb = lv_obj_create(parent);
    lv_obj_set_size(bulb, 13, 13);
    lv_obj_set_pos(bulb, x + 2, y + 13);
    lv_obj_set_style_radius(bulb, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(bulb, color, 0);
    lv_obj_set_style_border_width(bulb, 0, 0);
    lv_obj_set_style_pad_all(bulb, 0, 0);
}
static void create_droplet_icon(lv_obj_t *parent, lv_coord_t x, lv_coord_t y)
{
    static const lv_point_precise_t points[] = {
        {10, 0},
        {16, 9},
        {19, 15},
        {17, 21},
        {13, 24},
        {7, 24},
        {3, 21},
        {1, 15},
        {4, 9},
        {10, 0}
    };

    lv_obj_t *drop = lv_line_create(parent);

    lv_line_set_points(drop, points, 10);

    lv_obj_set_pos(drop, x, y);
    lv_obj_set_style_line_color(drop, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_line_width(drop, 2, 0);
    lv_obj_set_style_line_rounded(drop, true, 0);
}

static void create_ui(void)
{
    lv_obj_t *screen = lv_screen_active();

    lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), 0);

    // -------------------------
    // FLOW RATE SECTION
    // -------------------------

    lv_obj_t *flow_title = lv_label_create(screen);
    lv_label_set_text(flow_title, "FLOW RATE");
    lv_obj_set_style_text_color(flow_title,
                                lv_color_hex(0x90A4AE), 0);
    lv_obj_align(flow_title, LV_ALIGN_TOP_MID, 0, 15);

    flow_rate_label = lv_label_create(screen);
    lv_label_set_text(flow_rate_label, "0.0");
    lv_obj_set_style_text_color(flow_rate_label,
                                lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(flow_rate_label,
                               &lv_font_montserrat_40, 0);
    lv_obj_set_width(flow_rate_label, 135);
    lv_label_set_long_mode(flow_rate_label, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_align(flow_rate_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(flow_rate_label, LV_ALIGN_TOP_MID, 0, 35);

    lv_obj_t *flow_unit = lv_label_create(screen);
    lv_label_set_text(flow_unit, "L/hr");
    lv_obj_set_style_text_color(flow_unit,
                                lv_color_hex(0x29B6F6), 0);
    lv_obj_set_style_text_font(flow_unit,
                               &lv_font_montserrat_20, 0);
    lv_obj_align(flow_unit, LV_ALIGN_TOP_MID, 0, 83);

    // -------------------------
    // TEMPERATURE PANEL
    // -------------------------

    lv_obj_t *temp_panel = lv_obj_create(screen);
    lv_obj_set_size(temp_panel, 62, 92);
    lv_obj_align(temp_panel, LV_ALIGN_BOTTOM_LEFT, 3, -5);

    lv_obj_set_style_bg_color(temp_panel,
                              lv_color_hex(0xE3D5CA), 0);
    lv_obj_set_style_border_width(temp_panel, 0, 0);
    lv_obj_set_style_radius(temp_panel, 8, 0);
    lv_obj_set_style_pad_all(temp_panel, 0, 0);

    create_thermometer_icon(temp_panel, 23, 8);

    lv_obj_t *temp_title = lv_label_create(temp_panel);
    lv_label_set_text(temp_title, "TEMP");
    lv_obj_set_style_text_color(temp_title,
                                lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(temp_title, LV_ALIGN_TOP_MID, 0, 35);

    temperature_label = lv_label_create(temp_panel);
    lv_label_set_text(temperature_label, "--.- C");
    lv_obj_set_style_text_color(temperature_label,
                                lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(temperature_label,
                               &lv_font_montserrat_14, 0);
    lv_obj_set_width(temperature_label, 62);
    lv_label_set_long_mode(temperature_label, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_align(temperature_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(temperature_label, LV_ALIGN_BOTTOM_MID, 0, -9);

    // -------------------------
    // HUMIDITY PANEL
    // -------------------------

    lv_obj_t *humidity_panel = lv_obj_create(screen);
    lv_obj_set_size(humidity_panel, 62, 92);
    lv_obj_align(humidity_panel, LV_ALIGN_BOTTOM_RIGHT, -3, -5);

    lv_obj_set_style_bg_color(humidity_panel,
                              lv_color_hex(0xE3D5CA), 0);
    lv_obj_set_style_border_width(humidity_panel, 0, 0);
    lv_obj_set_style_radius(humidity_panel, 8, 0);
    lv_obj_set_style_pad_all(humidity_panel, 0, 0);

    create_droplet_icon(humidity_panel, 21, 5);

    lv_obj_t *humidity_title = lv_label_create(humidity_panel);
    lv_label_set_text(humidity_title, "HUMIDITY");
    lv_obj_set_style_text_color(humidity_title,
                                lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(humidity_title,
                               &lv_font_montserrat_12, 0);
    lv_obj_set_width(humidity_label, 62);
    lv_label_set_long_mode(humidity_label, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_align(humidity_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(humidity_title, LV_ALIGN_TOP_MID, 0, 35);

    humidity_label = lv_label_create(humidity_panel);
    lv_label_set_text(humidity_label, "--.- %");
    lv_obj_set_style_text_color(humidity_label,
                                lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(humidity_label,
                               &lv_font_montserrat_14, 0);
    lv_obj_align(humidity_label, LV_ALIGN_BOTTOM_MID, 0, -9);
}

esp_err_t ui_init(void)
{
    // Backlight
    gpio_config_t bl_config = {
        .pin_bit_mask = 1ULL << LCD_BL,
        .mode = GPIO_MODE_OUTPUT,
    };

    ESP_ERROR_CHECK(gpio_config(&bl_config));
    gpio_set_level(LCD_BL, 1);

    // SPI bus
    spi_bus_config_t bus_config = {
        .sclk_io_num = LCD_SCLK,
        .mosi_io_num = LCD_MOSI,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_H_RES * 40 * sizeof(uint16_t),
    };

    ESP_ERROR_CHECK(
        spi_bus_initialize(LCD_HOST, &bus_config, SPI_DMA_CH_AUTO)
    );

    // LCD SPI interface
    esp_lcd_panel_io_handle_t io_handle = NULL;

    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = LCD_DC,
        .cs_gpio_num = LCD_CS,
        .pclk_hz = 40 * 1000 * 1000,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };

    ESP_ERROR_CHECK(
        esp_lcd_new_panel_io_spi(
            LCD_HOST,
            &io_config,
            &io_handle
        )
    );

    // ST7789 panel
    esp_lcd_panel_handle_t panel_handle = NULL;

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = LCD_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
        .bits_per_pixel = 16,
    };

    ESP_ERROR_CHECK(
        esp_lcd_new_panel_st7789(
            io_handle,
            &panel_config,
            &panel_handle
        )
    );

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

    ESP_ERROR_CHECK(
        esp_lcd_panel_invert_color(panel_handle, true)
    );

    ESP_ERROR_CHECK(
        esp_lcd_panel_set_gap(panel_handle, 52, 40)
    );

    // LVGL initialization
    lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();

    ESP_ERROR_CHECK(lvgl_port_init(&lvgl_cfg));

    const lvgl_port_display_cfg_t display_config = {
        .io_handle = io_handle,
        .panel_handle = panel_handle,
        .buffer_size = 135 * 40,
        .double_buffer = true,
        .hres = LCD_H_RES,
        .vres = LCD_V_RES,
        .monochrome = false,
        .color_format = LV_COLOR_FORMAT_RGB565,
        .rotation = {
            .swap_xy = false,
            .mirror_x = false,
            .mirror_y = false,
        },
        .flags = {
            .buff_dma = true,
            .buff_spiram = false,
            .sw_rotate = false,
        },
    };

    lv_display_t *display = lvgl_port_add_disp(&display_config);

    if (display == NULL) {
        return ESP_FAIL;
    }

    ESP_ERROR_CHECK(
        esp_lcd_panel_disp_on_off(panel_handle, true)
    );

    // Create UI
    if (!lvgl_port_lock(1000)) {
        return ESP_ERR_TIMEOUT;
    }

    create_ui();

    lvgl_port_unlock();

    return ESP_OK;
}


// Update flow rate from main.c
void ui_set_flow_rate(float flow_rate)
{
    if (flow_rate_label == NULL) {
        return;
    }

    int value10 = (int)(flow_rate * 10.0f +
                        (flow_rate >= 0 ? 0.5f : -0.5f));

    int whole = value10 / 10;
    int decimal = abs(value10 % 10);

    if (lvgl_port_lock(0)) {
        lv_label_set_text_fmt(
            flow_rate_label,
            "%d.%d",
            whole,
            decimal
        );

        lvgl_port_unlock();
    }
}

// Update DHT
void ui_set_environment(float temperature, float humidity)
{
    if (temperature_label == NULL || humidity_label == NULL) {
        return;
    }

    int temp10 = (int)(temperature * 10.0f +
                       (temperature >= 0 ? 0.5f : -0.5f));

    int humidity10 = (int)(humidity * 10.0f + 0.5f);

    int temp_whole = temp10 / 10;
    int temp_decimal = abs(temp10 % 10);

    int humidity_whole = humidity10 / 10;
    int humidity_decimal = abs(humidity10 % 10);

    if (lvgl_port_lock(0)) {
        lv_label_set_text_fmt(
            temperature_label,
            "%d.%d C",
            temp_whole,
            temp_decimal
        );

        lv_label_set_text_fmt(
            humidity_label,
            "%d.%d %%",
            humidity_whole,
            humidity_decimal
        );

        lvgl_port_unlock();
    }
}