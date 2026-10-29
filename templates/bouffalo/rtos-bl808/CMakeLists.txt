cmake_minimum_required(VERSION 3.15)

# legacy SDK (bl808) is configured by proj.conf; SDK v2 (bl616/bl702) by defconfig/Kconfig
if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/proj.conf)
include(proj.conf)
endif()

find_package(bouffalo_sdk REQUIRED HINTS $ENV{BL_SDK_BASE})


sdk_add_include_directories(.)   # FreeRTOSConfig.h
sdk_set_main_file(main.c)

project({{NAME}})
