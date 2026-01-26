cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(rear_light_peripheral)

# Option to enable debug features (I2C scanner and debug prints)
option(DEBUG "Enable debug features (I2C scanner and debug prints)" ON)

# Option to enable Bluetooth
option(BLUETOOTH "Enable Bluetooth" ON)
# Option to enable BLE master mode
option(BLE_MASTER "Enable BLE master mode" ON)

target_sources(app PRIVATE 
    src/main.c
    src/sensor_data_collector.c
    src/button.c
    src/light_modes.c
    src/main_state_machine.c
    src/power_management.c
    src/bluetooth.c
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

# Conditionally enable Bluetooth
if(BLUETOOTH)
    target_compile_definitions(app PRIVATE 
        BLUETOOTH=1
    )
endif()

# Conditionally enable BLE master mode
if(BLE_MASTER)
    target_compile_definitions(app PRIVATE 
        BLE_MASTER=1
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