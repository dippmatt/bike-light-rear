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
- **Cree XP-E2** (XPEBRD-L1-0000-00801) – Far-red emitter driven at 500mA via PAM2804 step-down constant-current driver
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
1. Install KiCad 9.0 (schema version `20250114`) or newer.
2. Install the [JLCPCB Fabrication Toolkit](https://github.com/bennymeg/Fabrication-Toolkit) to be able to export the design in the correct format for JLCPCBs [Design For Manufacturing (DFM) Tool](https://jlcdfm.com/).
3. Open `hardware/kicad_project/RearLight.kicad_pro` to access the schematic and PCB.
4. Run ERC/DRC before layout edits; generated artifacts (Gerbers, pick-and-place, STEP export) go to `hardware/kicad_project/production`, which is gitignored and intentionally kept out of releases.
5. Note: Components marked DNP (Do Not Populate) in fabrication files are intended for hand-soldering to minimize assembly costs.

## Key design decisions
- **2.7V system voltage** – Optimized for nRF52833 efficiency while allowing 90-95% battery utilization before undervoltage cutoff (3.11V minimum)
- **400mA charge current** – Conservative charging rate extends 18350 cell lifespan; full charge in ~3 hours from 0%
- **500mA LED drive** – Balances brightness requirements with thermal constraints; typically operated at lower duty cycles via PWM
- **Unprotected cell + DW01A** – Cost optimization by using commodity unprotected cells with board-level protection rather than protected battery packs

## License
This hardware is released under **GPLv3 + NonCommercial**. You may copy, modify, and share for non-commercial use as long as derivatives retain the same license and attribution. See `LICENSE` for full terms.
