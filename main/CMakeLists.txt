idf_component_register(
    SRCS
        "main.c"
        "ui/ui.c"

    INCLUDE_DIRS
        "."
        "ui"

    REQUIRES
        esp_lcd
        esp_lvgl_port
        esp_timer
        driver
        dht
)