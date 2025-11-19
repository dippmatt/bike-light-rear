# bike-light-rear

High-visibility rear bicycle light built around the Laird BL653 (Nordic nRF52833) wireless module, a Cree XP-E2 LED, and smart sensing (LIS3DH accelerometer and OPT3001 ambient light). Power comes from a single-cell Li-ion battery with USB-C charging (NPM1100 PMIC) and DW01A protection so the cell does not need pack protection itself.

## Hardware architecture
- **EDA Software** – KiCad 9.0 project (`hardware/kicad_project/RearLight.kicad_pro`) with curated symbols, footprints, and 3D assets stored in `hardware/` for reproducible ECAD setups.
- **Compute & radio** – Laird BL653 (Nordic nRF52833) executes BLE control logic and exposes SWD/GPIO via a 2×3 1.27 mm header for bring-up, logging, and firmware flashing.
- **Sensing** – LIS3DH accelerometer provides motion/brake cues, while OPT3001 ambient light readings drive automatic LED brightness adjustments and future adaptive flashing profiles.
- **Power LED** – PAM2804 boost LED driver feeds a Cree XP-E2 emitter, enabling firmware-defined current limits and flash patterns for traffic visibility.
- **Power subsystem** – Single-cell Li-ion path starts at the USB-C port protected by USBLC6-2SC6, charges through the NPM1100 PMIC, and is safeguarded by a DW01A + dual FET pack so unprotected cells can be used confidently.
- **Mechanical & test** – Fiducials, mounting holes, USB-C placement, and labelled test pads support enclosure alignment, pick-and-place, and ATE probing.

## Repository layout
- `hardware/` – KiCad project, fabrication-toolkit config, custom libs (`kicad_symbols/`, `kicad_footprints/`, `kicad_3d_models/`), datasheets, and generated BOM
- `software/` *(planned)* – source for the nRF52833 firmware once the control stack is published
- `docs/` *(planned)* – rendered schematics, mechanical notes, test plans, and user-facing manuals
- `LICENSE` – GPLv3 with NonCommercial clause

## Hardware workflow
1. Install KiCad 9.0 (schema version `20250114`) or newer.
2. Install the [JLCPCB Fabrication Toolkit ](https://github.com/bennymeg/Fabrication-Toolkit) to be able to export the design in the correct format for JLCPCBs [Design For Manufacturing (DFM) Tool](https://jlcdfm.com/).
3. Open `hardware/kicad_project/RearLight.kicad_pro` to access the schematic and PCB.
4. Run ERC/DRC before layout edits; generated artifacts (Gerbers, pick-and-place, STEP export) go to `hardware/kicad_project/production`, which is gitignored and intentionally kept out of releases.

## License
This hardware is released under **GPLv3 + NonCommercial**. You may copy, modify, and share for non-commercial use as long as derivatives retain the same license and attribution. See `LICENSE` for full terms.