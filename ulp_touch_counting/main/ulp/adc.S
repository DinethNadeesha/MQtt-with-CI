#include "soc/rtc_cntl_reg.h"
#include "soc/soc_ulp.h"
#include "example_config.h"

.set adc_channel, EXAMPLE_ADC_CHANNEL
.set adc_oversampling_factor_log, 2
.set adc_oversampling_factor, (1 << adc_oversampling_factor_log)

.bss
.global ulp_low_thr
ulp_low_thr:
    .long 0

.global ulp_high_thr
ulp_high_thr:
    .long 0

.global ulp_sample_counter
ulp_sample_counter:
    .long 0

.global ulp_last_result
ulp_last_result:
    .long 0

.global ulp_touch_counter
ulp_touch_counter:
    .long 0

.text
.global entry
entry:
    move r3, ulp_sample_counter
    ld r2, r3, 0
    add r2, r2, 1
    st r2, r3, 0

    move r0, 0
    stage_rst
measure:
    adc r1, 0, adc_channel
    add r0, r0, r1
    stage_inc 1
    jumps measure, adc_oversampling_factor, lt

    rsh r0, r0, adc_oversampling_factor_log
    move r3, ulp_last_result
    st r0, r3, 0

    move r3, ulp_low_thr
    ld r3, r3, 0
    sub r3, r0, r3
    jump wake_up_and_count, ov

    move r3, ulp_high_thr
    ld r3, r3, 0
    sub r3, r3, r0
    jump wake_up_and_count, ov

    halt

.global wake_up_and_count
wake_up_and_count:
    move r3, ulp_touch_counter
    ld r2, r3, 0
    add r2, r2, 1
    st r2, r3, 0

    READ_RTC_FIELD(RTC_CNTL_LOW_POWER_ST_REG, RTC_CNTL_RDY_FOR_WAKEUP)
    and r0, r0, 1
    jump exit, eq

    wake
    WRITE_RTC_FIELD(RTC_CNTL_STATE0_REG, RTC_CNTL_ULP_CP_SLP_TIMER_EN, 0)

exit:
    halt
