cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(nrf_connect_prj)

# Option to enable debug features (I2C scanner and debug prints)
option(DEBUG "Enable debug features (I2C scanner and debug prints)" ON)

target_sources(app PRIVATE 
    src/main.c
    src/sensor_data_collector.c
    src/button.c
    src/light_modes.c
    src/main_state_machine.c
    src/power_management.c
)

# Conditionally enable debug features
if(DEBUG)
    target_sources(app PRIVATE 
        src/i2c_scanner.c
    )
    target_compile_definitions(app PRIVATE 
        DEBUG=1
    )
endif()

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