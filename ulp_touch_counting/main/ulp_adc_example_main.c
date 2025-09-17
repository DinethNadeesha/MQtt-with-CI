#include "ulp_fsm_common.h" 
#include "ulp_adc.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "driver/adc.h"
#include "esp32/ulp.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "ulp_main.h"
#include "ulp_common.h"
#include "example_config.h"

extern const uint8_t ulp_main_bin_start[] asm("_binary_ulp_main_bin_start");
extern const uint8_t ulp_main_bin_end[]   asm("_binary_ulp_main_bin_end");

static const char *TAG = "ULP_TOUCH_COUNTER";

static void init_ulp_program(void);
static void start_ulp_program(void);

void app_main(void)
{
    vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for Serial Monitor

    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();

    if (cause != ESP_SLEEP_WAKEUP_ULP) {
        ESP_LOGI(TAG, "Power-on or reset, initializing ULP");
        init_ulp_program();
    } else {
        ESP_LOGI(TAG, "Woke up from ULP");
    }

    start_ulp_program();
    ESP_ERROR_CHECK(esp_sleep_enable_ulp_wakeup());

    while (1) {
        ESP_LOGI(TAG, "ULP did %" PRIu32 " measurements", ulp_ulp_sample_counter & UINT16_MAX);
        ESP_LOGI(TAG, "Touch count: %" PRIu32, ulp_ulp_touch_counter & UINT16_MAX);
        ESP_LOGI(TAG, "Last ADC value: %" PRIu32, ulp_ulp_last_result & UINT16_MAX);
        ESP_LOGI(TAG, "Thresholds: LOW=%" PRIu32 ", HIGH=%" PRIu32, 
                 ulp_ulp_low_thr & UINT16_MAX, ulp_ulp_high_thr & UINT16_MAX);

        if ((ulp_ulp_last_result & UINT16_MAX) < (ulp_ulp_low_thr & UINT16_MAX)) {
            ESP_LOGI(TAG, "Status: Below threshold");
        } else if ((ulp_ulp_last_result & UINT16_MAX) > (ulp_ulp_high_thr & UINT16_MAX)) {
            ESP_LOGI(TAG, "Status: Above threshold");
        } else {
            ESP_LOGI(TAG, "Status: Within threshold");
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); // Wait 1 second
    }
}

static void init_ulp_program(void)
{
    // Load ULP binary
    ESP_ERROR_CHECK(ulp_load_binary(0, ulp_main_bin_start,
        (ulp_main_bin_end - ulp_main_bin_start) / sizeof(uint32_t)));

    // Configure ADC1_CH6 (GPIO34)
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);

    // Initialize ULP variables
    ulp_ulp_low_thr        = EXAMPLE_ADC_LOW_TRESHOLD;
    ulp_ulp_high_thr       = EXAMPLE_ADC_HIGH_TRESHOLD;
    ulp_ulp_touch_counter  = 0;
    ulp_ulp_sample_counter = 0;
    ulp_ulp_last_result    = 0;

    // Set wakeup period (1 second)
    ESP_ERROR_CHECK(ulp_set_wakeup_period(0, 1000000)); // 1 second in microseconds
}

static void start_ulp_program(void)
{
    ESP_ERROR_CHECK(ulp_run(&ulp_entry - RTC_SLOW_MEM));
}
