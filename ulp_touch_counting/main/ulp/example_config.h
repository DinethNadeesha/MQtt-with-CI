#pragma once

// ADC Configuration for GPIO34 (ADC1 Channel 6)
#define EXAMPLE_ADC_UNIT      1               // ADC unit 1
#define EXAMPLE_ADC_CHANNEL   6               // ADC1_CH6 = GPIO34
#define EXAMPLE_ADC_WIDTH     12              // 12-bit resolution
#define EXAMPLE_ADC_ATTEN     3               // ADC_ATTEN_DB_11 (0–3.6V input range)

// Very Sensitive Thresholds (for debugging with dry finger/metal touch)
#define EXAMPLE_ADC_LOW_TRESHOLD   500
#define EXAMPLE_ADC_HIGH_TRESHOLD  3000
