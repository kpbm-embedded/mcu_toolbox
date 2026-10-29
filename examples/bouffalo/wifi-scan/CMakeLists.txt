cmake_minimum_required(VERSION 3.15)

# legacy SDK (bl808) is configured by proj.conf; SDK v2 (bl616/bl702) by defconfig/Kconfig
if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/proj.conf)
include(proj.conf)
endif()

find_package(bouffalo_sdk REQUIRED HINTS $ENV{BL_SDK_BASE})

# The prebuilt Wi-Fi MAC library (components/wireless/macsw/lib/libmacsw_*.a) carries LTO bytecode from a newer GCC than the
# installed Xuantie 10.4.0 (\"LTO version 9.0 instead of 9.4\"); its objects are fat, so link without LTO and use their machine code.
sdk_add_link_options(-fno-lto)

sdk_add_include_directories(.)
sdk_set_main_file(main.c)

project(app)
