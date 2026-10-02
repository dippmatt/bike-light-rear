cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(rear_light_peripheral)

# Option to enable debug features (I2C scanner and debug prints)
option(DEBUG "Enable debug features (I2C scanner and debug prints)" ON)

target_sources(app PRIVATE
    src/main.c
    src/app_events.c
    src/state_machine.c
    src/light.c
    src/status_led.c
    src/button.c
    src/sensors.c
    src/battery.c
    src/ble.c
    src/power.c
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


target_compile_options(app PRIVATE
    -Wall
    -Wextra
    -Werror
)