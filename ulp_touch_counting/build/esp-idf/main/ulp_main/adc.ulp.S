       
       
       
       
.set adc_channel, 6
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
    REG_RD ((((0x3ff48000 + 0xc0)) - 0x3ff48000) / 4), ((19) + ((((((((0x1) >> (0)) & 1) + (((0x1) >> (0 + 1)) & 1)) + ((((0x1) >> (0 + 2)) & 1) + (((0x1) >> (0 + 2 + 1)) & 1))) + (((((0x1) >> (0 + 4)) & 1) + (((0x1) >> (0 + 4 + 1)) & 1)) + ((((0x1) >> (0 + 4 + 2)) & 1) + (((0x1) >> (0 + 4 + 2 + 1)) & 1)))) + ((((((0x1) >> (0 + 8)) & 1) + (((0x1) >> (0 + 8 + 1)) & 1)) + ((((0x1) >> (0 + 8 + 2)) & 1) + (((0x1) >> (0 + 8 + 2 + 1)) & 1))) + (((((0x1) >> (0 + 8 + 4)) & 1) + (((0x1) >> (0 + 8 + 4 + 1)) & 1)) + ((((0x1) >> (0 + 8 + 4 + 2)) & 1) + (((0x1) >> (0 + 8 + 4 + 2 + 1)) & 1)))))) - 1), (19)
    and r0, r0, 1
    jump exit, eq
    wake
    REG_WR ((((0x3ff48000 + 0x18)) - 0x3ff48000) / 4), ((24) + (((((((0x1) >> (0)) & 1) + (((0x1) >> (0 + 1)) & 1)) + ((((0x1) >> (0 + 2)) & 1) + (((0x1) >> (0 + 2 + 1)) & 1))) + (((((0x1) >> (0 + 4)) & 1) + (((0x1) >> (0 + 4 + 1)) & 1)) + ((((0x1) >> (0 + 4 + 2)) & 1) + (((0x1) >> (0 + 4 + 2 + 1)) & 1))))) - 1), (24), ((((0) & 0x1)) & 0xff)
exit:
    halt
