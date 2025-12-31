# bike-light-rear

High-visibility rear bicycle light with Bluetooth Low Energy control, featuring smart motion detection, ambient light sensing, and efficient single-cell Li-ion power management. Designed for hybrid assembly (pick-and-place for 0402/0603 passives, hand-soldering for connectors and high-value components).

## Hardware architecture

### Compute & wireless
- **BL653 module** (453-00039R) – Nordic nRF52833 SoC with integrated PCB trace antenna, Bluetooth 5.1, and 64 MHz Arm Cortex-M4F
- **Programming** – 2×3 1.27mm SWD header for firmware flashing and debugging
- **System power** – 2.7V from NPM1100 buck regulator optimized for 90-95% battery capacity utilization (3.11V cutoff)

### Sensors (I2C bus @ 2.7V)
- **LIS3DH accelerometer** – 3-axis motion detection (braking / fall detection) for brake/acceleration events, configurable thresholds, interrupt-driven
- **OPT3001 light sensor** – 0.01-83k lux range with human-eye spectral matching for automatic brightness control and day/night mode switching

### High-power LED driver
- **Cree XP-E2** (XPEBRD-L1-0000-00801) – 625nm Red emitter driven at 500mA via PAM2804 step-down constant-current driver
- **Control** – PWM dimming via BL653 GPIO with 100kΩ pull-down for safe default-off state

### Power subsystem
- **Battery** – Single 18350 Li-ion cell (Vapcell 1100mAh, unprotected), 3.11-4.2V operating range
- **Charging** – USB-C PD sink (5.1kΩ CC resistors), NPM1100 linear charger @ 400mA (0Ω ICHG resistor), 4.2V termination
- **Protection** – DW01A IC with dual AO3400A N-channel MOSFETs (drain-to-drain configuration) for over-voltage (4.3V), under-voltage (2.5V), and over-current (>2A) protection
- **Buck regulator** – NPM1100 internal 2.7V @ 150mA output
- **Status monitoring** – CHG and ERR signals routed to BL653 GPIO for firmware-based charge status indication
- **NTC disable** – Thermal monitoring disabled via 10kΩ resistor from NTC pin to AVSS (GND)

### PCB specifications
- **Design tool** – KiCad 9.0 (schema version `20250114`)
- **Test points** – Critical nets labeled for bring-up and debugging (+BATT, +VSYS, +VDD_nRF, GND, I2C_SDA, I2C_SCL)

## Repository layout
- `hardware/` – KiCad project, fabrication-toolkit config, custom libs (`kicad_symbols/`, `kicad_footprints/`, `kicad_3d_models/`), datasheets (markdown format), and generated BOM
- `software/` *(planned)* – source for the nRF52833 firmware once the control stack is published
- `docs/` *(planned)* – rendered schematics, mechanical notes, test plans, and user-facing manuals
- `LICENSE` – GPLv3 with NonCommercial clause

## Hardware workflow
1. Install KiCad 9.0 or newer.
2. Install the [JLCPCB Fabrication Toolkit](https://github.com/bennymeg/Fabrication-Toolkit) to be able to export the design in the correct format for JLCPCBs [Design For Manufacturing (DFM) Tool](https://jlcdfm.com/).
3. Open `hardware/kicad_project/RearLight.kicad_pro` to access the schematic and PCB.
4. Run ERC/DRC before layout edits; generated artifacts (Gerbers, pick-and-place, STEP export) go to `hardware/kicad_project/production`, which is gitignored and intentionally kept out of releases.
5. Note: Components marked DNP (Do Not Populate) in fabrication files are intended for hand-soldering to minimize assembly costs.

## Firmware

The bike light firmware implements intelligent lighting control with multiple operating modes:

### Features
- **Four Operating Modes**: OFF, 50% continuous, 50/80 flash, and Smart Mode
- **Smart Mode**: Adaptive brightness based on braking detection and ambient light
- **Braking Detection**: Automatic 80% brightness boost during deceleration
- **Ambient Light Sensing**: Adjusts brightness for day/night conditions
- **Auto-Off**: Power-saving mode when stationary for 2.5+ minutes
- **Sensor Buffering**: 3-minute circular buffer for motion and environmental data

For complete operating instructions and technical details, see the **[User Manual](software/nrf_connect_prj/MANUAL.md)**.

### Firmware environment setup
1. Install the [nRF Command Line Tools](https://www.nordicsemi.com/Products/Development-tools/nrf-command-line-tools) for access to `nrfjprog` and programming utilities.
2. Install [nRF Connect for VS Code](https://www.nordicsemi.com/Products/Development-tools/nrf-connect-for-vs-code) 
3. Within its extension packs, install the **nRF Connect Bare Metal SDK v0.9.0**.
4. Use the nRF Connect for VS Code Toolchain Manager to install the **nRF Connect SDK toolchain v3.1.1** so the project can target the BL653/nRF52833 with the expected compilers and CMake presets.
5. (Optional) Through the nrf Connect Tab in VS Code, install the nrf Kconfig and nrf Devicetree extentions

Note 1: Device flashing using WSL2
If you are not using WSL, 
To connect a J-Link debugger to to a wsl instance take the following steps:
- Open a Windows Powershell CLI and run..
- `usbipd list`
- `usbipd bind --busid <J-Link BusID>`
- `usbipd attach --wsl --busid <J-Link BusID>`

Note 2: Device does not show up under "Connected Devices" in nRF Connect VSCode Extention
- Install [nRF-Util](https://www.nordicsemi.com/Products/Development-tools/nRF-Util)
- Make nrfutil executable using `chmod +x nrfutil`
- Move the nrfutil executable to a directory in the system $PATH (e.g. /usr/bin)
- Install the _device_ package for nrfutil using `nrfutil install device`
- Restart VSCode. Your device should now show up under _Connected Devices_ in nRF Connect  


## License
This hardware is released under **GPLv3 + NonCommercial**. You may copy, modify, and share for non-commercial use as long as derivatives retain the same license and attribution. See `LICENSE` for full terms.
