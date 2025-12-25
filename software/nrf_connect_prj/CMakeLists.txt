cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(nrf_connect_prj)

target_sources(app PRIVATE 
    src/main.c
    src/sensor_data_collector.c
)

target_include_directories(app PRIVATE
    inc/
)

target_link_libraries(app PRIVATE
    -lm
)

target_compile_options(app PRIVATE
    -Wall
    -Wextra
    -Werror
)