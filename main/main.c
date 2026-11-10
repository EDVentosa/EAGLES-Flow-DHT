#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"

#include "esp_err.h"
#include "esp_log.h"

#include "ui/ui.h"

#include <inttypes.h>
#include "esp_timer.h"

#include "dht.h"

#define DHT22_GPIO GPIO_NUM_26

#define FLOW_SENSOR_GPIO GPIO_NUM_27
#define FLOW_PULSES_PER_LITER 450.0f
#define FLOW_HZ_PER_LPM 7.5f

static const char *TAG = "FLOW";

static volatile uint32_t pulse_count = 0;

static portMUX_TYPE pulse_mux = portMUX_INITIALIZER_UNLOCKED;


// Flow sensor ISR
static void IRAM_ATTR flow_sensor_isr(void *arg)
{
    portENTER_CRITICAL_ISR(&pulse_mux);
    pulse_count++;
    portEXIT_CRITICAL_ISR(&pulse_mux);
}


// GPIO initialization
static void flow_sensor_init(void)
{
    gpio_config_t config = {
        .pin_bit_mask = 1ULL << FLOW_SENSOR_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_POSEDGE,
    };

    ESP_ERROR_CHECK(gpio_config(&config));

    ESP_ERROR_CHECK(
        gpio_install_isr_service(ESP_INTR_FLAG_IRAM)
    );

    ESP_ERROR_CHECK(
        gpio_isr_handler_add(
            FLOW_SENSOR_GPIO,
            flow_sensor_isr,
            NULL
        )
    );
}


// Read and reset pulse counter
static uint32_t get_pulse_count(void)
{
    uint32_t count;

    portENTER_CRITICAL(&pulse_mux);

    count = pulse_count;
    pulse_count = 0;

    portEXIT_CRITICAL(&pulse_mux);

    return count;
}


// Flow measurement task
static void flow_monitor_task(void *arg)
{
    int64_t last_time = esp_timer_get_time();

    TickType_t last_wake = xTaskGetTickCount();

    while (1) {

        // Wait for next measurement interval
        vTaskDelayUntil(
            &last_wake,
            pdMS_TO_TICKS(1000)
        );

        // Get pulse count
        uint32_t pulses = get_pulse_count();

        // Calculate actual elapsed time
        int64_t current_time = esp_timer_get_time();

        float elapsed_seconds =
            (current_time - last_time) / 1000000.0f;

        last_time = current_time;

        // Calculate frequency
        float frequency = pulses / elapsed_seconds;

        // Convert frequency to L/hr
        float flow_rate_lph =
            (frequency / FLOW_HZ_PER_LPM) * 60.0f;

        ESP_LOGI(
            TAG,
            "Pulses: %" PRIu32 " | Frequency: %.2f Hz | Flow: %.1f L/hr",
            pulses,
            frequency,
            flow_rate_lph
        );

        // Update display
        ui_set_flow_rate(flow_rate_lph);
    }
}

// Read DHT data
static esp_err_t read_dht22(float *temperature, float *humidity)
{
    return dht_read_float_data(
        DHT_TYPE_AM2301,
        DHT22_GPIO,
        humidity,
        temperature
    );
}

// DHT Task
static void dht22_task(void *arg)
{
    float temperature;
    float humidity;

    while (1) {
        esp_err_t err = read_dht22(&temperature, &humidity);

        if (err == ESP_OK) {
            ESP_LOGI(TAG,
                     "Temperature: %.1f C | Humidity: %.1f %%",
                     temperature,
                     humidity);
            ui_set_environment(temperature, humidity);
        } else {
            ESP_LOGW(TAG, "DHT22 read failed");
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}


void app_main(void)
{
    ESP_ERROR_CHECK(ui_init());

    flow_sensor_init();

    xTaskCreate(
        flow_monitor_task,
        "flow_monitor",
        3072,
        NULL,
        5,
        NULL
    );

    xTaskCreate(
        dht22_task,
        "dht22_task",
        4096,
        NULL,
        5,
        NULL
    );
}