# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Espressif/frameworks/esp-idf-v5.3.1/components/ulp/cmake"
  "C:/Users/dineth/workspace/ulp_touch_counting/build/esp-idf/main/ulp_main"
  "C:/Users/dineth/workspace/ulp_touch_counting/build/esp-idf/main/ulp_main-prefix"
  "C:/Users/dineth/workspace/ulp_touch_counting/build/esp-idf/main/ulp_main-prefix/tmp"
  "C:/Users/dineth/workspace/ulp_touch_counting/build/esp-idf/main/ulp_main-prefix/src/ulp_main-stamp"
  "C:/Users/dineth/workspace/ulp_touch_counting/build/esp-idf/main/ulp_main-prefix/src"
  "C:/Users/dineth/workspace/ulp_touch_counting/build/esp-idf/main/ulp_main-prefix/src/ulp_main-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/dineth/workspace/ulp_touch_counting/build/esp-idf/main/ulp_main-prefix/src/ulp_main-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/dineth/workspace/ulp_touch_counting/build/esp-idf/main/ulp_main-prefix/src/ulp_main-stamp${cfgdir}") # cfgdir has leading slash
endif()
