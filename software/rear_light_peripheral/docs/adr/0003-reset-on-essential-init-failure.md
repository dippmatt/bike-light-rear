# Essential initialization failure resets the device

If the main LED, status LED or button fails to initialize, `main()` performs a software reset instead of returning. Returning leaves a dead device with no light and no way to wake it, while a reset gives a transient fault another chance and the boot policy then turns the light on (fail operational). The status LED is deliberately included to keep the rule simple, although it is not essential. A permanent hardware fault now causes a reset loop with no usable light, which is accepted. Battery, sensor and BLE failures do not reset.
