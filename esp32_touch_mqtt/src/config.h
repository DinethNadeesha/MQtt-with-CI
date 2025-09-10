#pragma once

// ----------------- DRD CONFIG -----------------
#define DRD_TIMEOUT   8       // seconds window for double reset
#define DRD_ADDRESS   0x00    // NVS slot used by ESP_DoubleResetDetector

// ----------------- TOUCH CONFIG -----------------
#define TOUCH_PIN        T9
#define TOUCH_THR_INIT   30   // editable over API at runtime

// ----------- SERVICE ENTRY VIA TOUCH DOUBLE-TAP (GPIO14 = T6) -----------
#define SVC_TOUCH_PIN    T6    // GPIO14 (capacitive touch)
#define SVC_TOUCH_THR    25    // tune after testing: idle > touched

// ----------------- DEFAULT WIFI CONFIG (fallback) ------------------
#define DEFAULT_STA_SSID      "iPhone"
#define DEFAULT_STA_PASSWORD  "dineth123"

// ----------------- CUSTOMER AP (end-user) ------------------
#define AP_SSID_CUSTOMER      "ESP32_TouchCounter"
#define AP_PASS_CUSTOMER      "touch1234"   // >= 8 chars

// ----------------- COMPANY AP (service/engineering) ------------------
#define AP_PASS_COMPANY       "svc12345"    // different from customer
// SSID will be built at runtime: "ESP32_TouchCounter-SVC-XXXX"

// ----------------- MQTT CONFIG (customer mode only) ------------------
#define MQTT_HOST        "broker.hivemq.com"
#define MQTT_PORT        1883
#define MQTT_TOPIC_COUNT "esp32/touch/count"
#define MQTT_TOPIC_STAT  "esp32/touch/status"
#define MQTT_BIRTH_PAY   "online"
#define MQTT_WILL_PAY    "offline"

// ----------------- UI / Detection tuning ------------------
#define SVC_DOUBLE_TAP_WINDOW_MS  1200
