idf_component_register(
    SRCS "ulp_adc_example_main.c"
         "${IDF_PATH}/components/ulp/ulp_fsm/ulp.c"
    INCLUDE_DIRS "." "ulp"      # ✅ add "ulp" here for example_config.h
    REQUIRES ulp
)

# Include ULP header directories
target_include_directories(${COMPONENT_LIB} PRIVATE
   "${IDF_PATH}/components/ulp/ulp_fsm/include"
    "${IDF_PATH}/components/ulp/ulp_fsm/include/esp32"
    "${IDF_PATH}/components/ulp/ulp_common/include"
)

# ULP configuration
set(ulp_app_name ulp_${COMPONENT_NAME})
set(ulp_s_sources "ulp/adc.S")
set(ulp_exp_dep_srcs "ulp_adc_example_main.c")
ulp_embed_binary(${ulp_app_name} "${ulp_s_sources}" "${ulp_exp_dep_srcs}")
