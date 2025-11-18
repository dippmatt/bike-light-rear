### nPM1100

**Product Specification**

![](_page_0_Picture_3.jpeg)

### <span id="page-1-0"></span>nPM1100

nPM1100 is an integrated Power Management IC (PMIC) with a linear-mode lithium-ion/lithium-polymer battery charger in a compact 2.1x2.1 mm WLCSP or 4.0x4.0 mm QFN package. It has a highly efficient DC/ DC buck regulator with configurable dual mode output.

nPM1100 is an extremely compact PMIC device, created for space constrained applications that have a small lithium-ion or lithium-polymer battery. It is compatible with all nRF52 and nRF53 Series SoCs, supports charging batteries at up to 400 mA through USB, and delivers up to 150 mA of current to power external components with regulated voltage.

A minimum of five passive components are required for operation. It is the perfect companion for nRF52 and nRF53 multiprotocol SoCs in battery powered designs and the device functions without a control interface. Low quiescent current (IQ) extends battery life for shipping and storage with Ship mode, or in operation using auto-controlled hysteretic buck mode for high efficiency down to 1 µA loads. Charge and error indication LED drivers are built in. Charge profile limits are configurable and VBUS current limits can be fixed or auto-controlled with on-chip USB port detection.

- Ultra-high efficiency prolongs battery life or allows for use of smaller and less costly batteries
- Small solution size leaves space for additional features without increasing product size
- No software control
- Automatic USB port detection minimizes development time

![](_page_1_Figure_8.jpeg)

*Figure 1: nPM1100 block diagram*

![](_page_1_Picture_10.jpeg)

### <span id="page-2-0"></span>Feature list

#### **Features:**

- 400 mA linear battery charger
  - Linear charger for lithium-ion/lithium-polymer batteries
  - Adjustable charge current from 20 mA to 400 mA
  - Selectable termination voltage
    - 4.1 V or 4.2 V on the standard VTERM device
    - 4.25 V or 4.35 V on the high VTERM device
  - Automatic trickle, constant current, and constant voltage charging
  - Battery thermal protection
  - Discharge current limitation
  - JEITA compliant
- Li-ion/Li-polymer USB battery charger with a high efficiency buck regulator
- 800 nA Typical quiescent current
- 460 nA Shipping mode quiescent current
- Thermal protection
- Input regulator
  - USB compatible current limit of 100 mA and 500 mA
  - 4.1 V to 6.7 V input voltage range for normal operation
  - 20 V overvoltage protection
  - Reverse current protection
  - 3.0 V to 5.5 V system voltage output
  - USB port detection supporting the following types:
    - SDP
    - CDP/DCP

- 1.8 V to 3.0 V, 150 mA step-down buck regulator
  - Step-down buck regulator with up to 92% efficiency
  - Automatic transition between hysteretic and pulse width modulation (PWM) modes
  - Forced PWM mode for clean power operation
  - Pin-selectable output voltage (1.8 V, 2.1 V, 2.7 V, 3.0 V)
  - Soft start-up
- LED drivers for charger state indication
  - 5 mA low side LED driver for charging indication
  - 5 mA low side LED driver for error indication
- 2.3 V to 4.35 V battery operating input range
- Package options suitable for two layer PCB:
  - 2.1x2.1 mm WLCSP package
  - 4.0x4.0 mm QFN package

#### **Applications:**

- Advanced wearables
  - Health/fitness sensor and monitor devices
- Advanced computer peripherals and I/O devices
  - Mouse
  - Keyboard
  - Multi-touch trackpad

- Interactive entertainment devices
  - Remote controls
  - Gaming controllers

![](_page_2_Picture_48.jpeg)

4445\_367 v1.5 iii

### Contents

|   | nPM1100.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    | ii                                                                                                       |
|---|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|----------------------------------------------------------------------------------------------------------|
|   | Feature list.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               | iii                                                                                                      |
| 1 | Revision history.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           | 6                                                                                                        |
| 2 | About this document.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        | 7                                                                                                        |
|   | 2.1 Document status.<br>2.2 Core component chapters.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        | 7<br>7                                                                                                   |
| 3 | Product overview.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           | 8                                                                                                        |
|   | 3.1 Block diagram.<br>3.1.1 In circuit configurations.<br>3.2 System description.<br>3.3 Power-on reset (POR) and brownout reset (BOR).<br>3.4 DPPM — Dynamic power-path management.<br>3.5 Using Ship mode.<br>3.6 Thermal protection.<br>3.7 Battery considerations.<br>3.8 Charging and error LED drivers.<br>3.9 System electrical parameters.<br>3.10 System efficiency.                                                                                                                                                                                                                                                                                                                                                                                               | 8<br>9<br>9<br>10<br>10<br>10<br>11<br>11<br>11<br>11<br>12                                              |
| 4 | Absolute maximum ratings.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   | 13                                                                                                       |
| 5 | Recommended operating conditions.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           | 15                                                                                                       |
|   | 5.1 Dissipation ratings.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    | 15                                                                                                       |
|   | 5.2 WLCSP light sensitivity.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                | 16                                                                                                       |
| 6 | Core components.<br>6.1 SYSREG — System regulator.<br>6.1.1 USB port detection and VBUS current limiting.<br>6.1.2 SYSREG resistance and output voltage.<br>6.1.3 VBUS overvoltage and undervoltage protection.<br>6.1.4 VBUS disconnect.<br>6.1.5 Electrical specification.<br>6.1.6 Electrical characteristics.<br>6.2 CHARGER — Battery charger.<br>6.2.1 Charging cycle.<br>6.2.2 Termination voltage (VTERMSET).<br>6.2.3 Termination and trickle charge current.<br>6.2.4 Charge current limit (ICHG).<br>6.2.5 Battery thermal protection using NTC thermistor (NTC).<br>6.2.6 Charger thermal regulation.<br>6.2.7 Charger error conditions.<br>6.2.8 Charging indication (CHG) and charging error indication (ERR).<br>6.2.9 DPPM — Dynamic power-path management. | 17<br>17<br>17<br>18<br>18<br>18<br>18<br>19<br>21<br>21<br>22<br>23<br>23<br>23<br>24<br>24<br>25<br>25 |

![](_page_3_Picture_2.jpeg)

4445\_367 v1.5 iv

|   | 6.2.11 Electrical characteristics.                     | 28 |
|---|--------------------------------------------------------|----|
|   | 6.3 BUCK — Buck regulator.                             | 31 |
|   | 6.3.1 Output voltage selection (VOUTBSET0, VOUTBSET1). | 31 |
|   | 6.3.2 BUCK mode selection (MODE).                      | 32 |
|   | 6.3.3 Component selection.                             | 32 |
|   | 6.3.4 Electrical specification.                        | 32 |
|   | 6.3.5 Electrical characteristics.                      | 33 |
| 7 | Application.                                           | 42 |
|   | 7.1 Schematic.                                         | 42 |
|   | 7.2 Supplying from BUCK.                               | 42 |
|   | 7.3 USB port negotiation.                              | 43 |
|   | 7.4 Charging and error states.                         | 43 |
|   | 7.5 Termination voltage and current.                   | 43 |
|   | 7.6 NTC configuration.                                 | 43 |
|   | 7.7 Ship mode.                                         | 43 |
|   | 7.8 Battery monitoring and low battery indication.     | 44 |
| 8 | Hardware and layout.                                   | 45 |
|   | 8.1 Pin assignments.                                   | 45 |
|   | 8.1.1 WLCSP ball assignments.                          | 45 |
|   | 8.1.2 QFN24 pin assignments.                           | 46 |
|   | 8.2 Mechanical specifications.                         | 49 |
|   | 8.2.1 WLCSP 2.075x2.075 mm package.                    | 49 |
|   | 8.2.2 QFN 4.0x4.0 mm package.                          | 49 |
|   | 8.3 Reference circuitry.                               | 50 |
|   | 8.3.1 Configuration 1.                                 | 51 |
|   | 8.3.2 Configuration 2.                                 | 52 |
|   | 8.3.3 Configuration 3.                                 | 53 |
|   | 8.3.4 PCB guidelines.                                  | 55 |
|   | 8.3.5 PCB layout example.                              | 55 |
|   |                                                        |    |
| 9 | Ordering information.                                  | 58 |
|   | 9.1 IC marking.                                        | 58 |
|   | 9.2 Box labels.                                        | 58 |
|   | 9.3 Order code.                                        | 59 |
|   | 9.4 Code ranges and values.                            | 60 |
|   | 9.5 Product options.                                   | 61 |
|   |                                                        |    |

10 [Legal notices](#page-62-0). . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . 63

![](_page_4_Picture_1.jpeg)

4445\_367 v1.5 v

# <span id="page-5-0"></span>1 Revision history

| Date          | Version | Description                                                                                                                                                                                                                                                                                              |
|---------------|---------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| February 2025 | 1.5     | The following has been added or updated:<br>•<br>Using Ship mode on page 10 – added<br>battery charger disable condition<br>•<br>Electrical specification on page 18 – added<br>IBUS quiescent current<br>•<br>Editorial                                                                                 |
| June 2024     | 1.4     | The following has been added or updated:<br>•<br>Mechanical specifications on page 49<br>•<br>Pin assignments on page 45 – pin 3 changed<br>to VOUTBSET0, pin 4 and 25 are connected to<br>AVSS                                                                                                          |
| February 2023 | 1.3     | The following has been added or updated:<br>•<br>Added QFN package variant information to the<br>following chapters:<br>•<br>Mechanical specifications on page 49<br>•<br>Ordering information on page 58<br>•<br>Pin assignments on page 45<br>•<br>CHARGER – added high VTERM option<br>•<br>Editorial |
| October 2022  | 1.2     | The following has been added or updated:<br>•<br>Capacitor on VBAT in the following chapters:<br>•<br>Block diagram on page 8<br>•<br>Reference circuitry on page 50<br>•<br>Schematic on page 42<br>•<br>Absolute Maximum Ratings – MSL value<br>•<br>Editorial                                         |
| June 2022     | 1.1     | The following has been added or updated:<br>•<br>Ordering code for latest revision in Product<br>options on page 61, build code C00 no<br>longer supported<br>•<br>Editorial                                                                                                                             |
| May 2021      | 1.0     | First release                                                                                                                                                                                                                                                                                            |

![](_page_5_Picture_2.jpeg)

# <span id="page-6-0"></span>2 About this document

This document is organized into chapters that are based on the modules available in the IC.

#### <span id="page-6-1"></span>2.1 Document status

The document status reflects the level of maturity of the document.

| Document name                         | Description                                                                                                                                                                                                                                                              |
|---------------------------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| Objective Product Specification (OPS) | Applies to document versions up to 1.0.<br>This document contains target specifications for<br>product development.                                                                                                                                                      |
| Product Specification (PS)            | Applies to document versions 1.0 and higher.<br>This document contains final product<br>specifications. Nordic Semiconductor ASA reserves<br>the right to make changes at any time without<br>notice in order to improve design and supply the<br>best possible product. |

*Table 1: Defined document names*

#### <span id="page-6-2"></span>2.2 Core component chapters

Every core component has a unique capitalized name or an abbreviation of its name, e.g. LED, used for identification and reference. This name is used in chapter headings and references, and it will appear in the C-code header file to identify the component.

The core component instance name, which is different from the core component name, is constructed using the core component name followed by a numbered postfix, starting with 0, for example, LED0. A postfix is normally only used if a core component can be instantiated more than once. The core component instance name is also used in the C-code header file to identify the core component instance.

The chapters describing core components may include the following information:

- A detailed functional description of the core component
- Register configuration for the core component
- Electrical specification tables, containing performance data which apply for the operating conditions described in [Recommended operating conditions](#page-14-2) on page 15.

![](_page_6_Picture_13.jpeg)

# <span id="page-7-0"></span>3 Product overview

nPM1100 is an integrated Power Management IC (PMIC) with a linear-mode lithium-ion/lithium-polymer battery charger in a compact 2.1x2.1 mm WLCSP or 4.0x4.0 mm QFN package. It has a highly efficient DC/ DC buck regulator with configurable dual mode output.

nPM1100 is an extremely compact PMIC device, created for space constrained applications that have a small lithium-ion or lithium-polymer battery. It is compatible with all nRF52 and nRF53 Series SoCs, supports charging batteries at up to 400 mA through USB, and delivers up to 150 mA of current to power external components with regulated voltage.

A minimum of five passive components are required for operation. It is the perfect companion for nRF52 and nRF53 multiprotocol SoCs in battery powered designs and the device functions without a control interface. Low quiescent current (IQ) extends battery life for shipping and storage with Ship mode, or in operation using auto-controlled hysteretic buck mode for high efficiency down to 1 µA loads. Charge and error indication LED drivers are built in. Charge profile limits are configurable and VBUS current limits can be fixed or auto-controlled with on-chip USB port detection.

- Ultra-high efficiency prolongs battery life or allows for use of smaller and less costly batteries
- Small solution size leaves space for additional features without increasing product size
- No software control
- Automatic USB port detection minimizes development time

#### <span id="page-7-1"></span>3.1 Block diagram

The block diagram illustrates the overall system.

![](_page_7_Figure_11.jpeg)

*Figure 2: Block diagram*

![](_page_7_Picture_13.jpeg)

#### <span id="page-8-0"></span>3.1.1 In circuit configurations

The device is configurable for different applications and battery characteristics through input pins.

Static input pins must be configured before power-on reset. Dynamic input pins may be modified during operation under conditions described in references. For the full list of pins, see [Pin assignments](#page-44-1) on page 45.

| Pin         | Function                                      | Input type        | Usage reference                                                     |
|-------------|-----------------------------------------------|-------------------|---------------------------------------------------------------------|
| VTERMSET    | Sets termination voltage<br>Battery dependent | Static (H/L)      | Termination voltage<br>(VTERMSET) on page<br>22                     |
| ICHG        | Charge current limit                          | Static (resistor) | Charge current limit<br>(ICHG) on page 23                           |
| ISET        | VBUS current limit                            | Dynamic (H/L)     | VBUS current limit ISET                                             |
| MODE        | BUCK PWM mode<br>override                     | Dynamic (H/L)     | BUCK mode selection<br>(MODE) on page 32                            |
| VOUTBSET[n] | Two pin VOUTB voltage<br>configuration        | Static (H/L)      | Output voltage<br>selection (VOUTBSET0,<br>VOUTBSET1) on page<br>31 |
| SHPACT      | Enables Ship mode                             | Dynamic (H/L)1    | Using Ship mode on<br>page 10                                       |
| SHPHLD      | Disables Ship mode                            | Dynamic (H/L)1    | Using Ship mode on<br>page 10                                       |

*Table 2: In circuit configurations*

#### <span id="page-8-1"></span>3.2 System description

The device has the following core components that are described in detail in the respective chapters.

- [SYSREG System regulator](#page-16-1) on page 17
- [CHARGER Battery charger](#page-20-0) on page 21
- [BUCK Buck regulator](#page-30-0) on page 31

The system regulator (SYSREG) is a 5 V LDO supplied by **VBUS**. It generates VINT when enabled. VINT is the internal supply for the device and available on an external pin, **VSYS**. SYSREG supports a wide operating voltage range on **VBUS**, tolerates transient voltages up to 20 V, and implements overvoltage protection. SYSREG also implements configurable current limiting from **VBUS**, and USB port detection. When **VBUS** is disconnected, SYSREG ensures the device enters Ultra-Low Power mode to minimize quiescent current. Reverse current protection is enabled when VBUS<VBAT. See [SYSREG — System regulator](#page-16-1) on page 17 for more information and electrical parameters.

The battery charger (CHARGER) is a JEITA compatible linear battery charger for Li-ion/Li-poly batteries. CHARGER controls the charge cycle using a standard Li-ion charge profile. CHARGER implements dynamic power-path management regulating current in and out of the battery, depending on system requirements. Charge current and charge termination voltage can be set with the **ICHG** and **VTERMSET** pins respectively. LED drivers for charging indication and charging error indication are implemented

<span id="page-8-2"></span><sup>1</sup> These pins are level and hold-time controlled.

in CHARGER. See [CHARGER — Battery charger](#page-20-0) on page 21 for more information and electrical parameters.

The buck regulator (BUCK) is a step-down DC/DC regulator with PWM and Hysteretic modes with automatic control for optimum efficiency and manual enable of PWM mode to reduce voltage ripple and inductive interference if needed. The output voltage is pin configurable (through **VOUTSET0** and **VOUTSET1**) for different application circuit requirements. BUCK is supplied by VINT (from SYSREG or the battery). See [BUCK — Buck regulator](#page-30-0) on page 31 for more information and electrical parameters.

The device also features Ship mode, the lowest quiescent current state. It disconnects the battery from the system and reduces the quiescent current of the device to extend battery life when products are in storage. See [Using Ship mode](#page-9-2) on page 10 and [Charging and error LED drivers](#page-10-2) on page 11 for more information.

### <span id="page-9-0"></span>3.3 Power-on reset (POR) and brownout reset (BOR)

When one of the following conditions are met, a power-on reset (POR) occurs.

- **VBUS** voltage rises above [VBUS](#page-17-4)POR
- **VBAT** voltage rises above [VBAT](#page-25-1)POR

When both of the following conditions are met, a brownout reset (BOR) occurs.

- **VBUS** voltage falls below [VBUS](#page-17-4)BOR
- **VBAT** voltage falls below [VBAT](#page-25-1)BOR

BOR may occur if both supply voltages are below the maximum of the parameter range. BOR occurs if both supply voltages are below the minimum of the parameter range.

The device is held in reset, or System OFF, when both supply voltages **VBAT** and **VBUS** are below minimum thresholds.

#### <span id="page-9-1"></span>3.4 DPPM — Dynamic power-path management

Dynamic power-path management (DPPM) is a feature that regulates internal voltage (VINT) as system load (ISYS) changes to maintain supply to the application circuit (supplied by the **VSYS** and **VOUTB** pins).

CHARGER applies DPPM during charging, after charging completes, or when the **VBUS** pin is disconnected, to dynamically control current in and out of the battery. See [DPPM — Dynamic power-path management](#page-24-1) on page 25.

#### <span id="page-9-2"></span>3.5 Using Ship mode

Ship mode isolates the battery, reducing quiescent current.

To enter Ship mode, **SHPACT** must be set high for a minimum period of t[activeToShip](#page-11-1) when **VBUS** is disconnected and **SHPHLD** held high ([V](#page-11-1)IH). **SHPACT** has an internal pull-down resistor. **SHPACT** can be connected to a microcontroller GPIO (using logic levels in the range [V](#page-11-1)IL and [V](#page-11-1)IH) or to a PCB test pin for activation at the end of production. Setting **SHPACT** pin high disables battery charger.

**Note: VBUS** must be discharged to below minimum level VBUSMIN which may require waiting for any capacitive discharge before activating **SHPACT**.

There are two ways to exit Ship mode. Either connect the USB (**VBUS**) or set **SHPHLD** low for a minimum period of t[shipToActive](#page-11-1). The battery supply (**VBAT**) is used to hold **SHPHLD** high through a weak pull-up resistor when Ship mode is enabled. A circuit to pull down **SHPHLD** is optional (see the Button switch

shown in the following figure). If no pull-down circuit is present, Ship mode is exited when **VBUS** is connected.

If Ship mode is not required, then **SHPACT** and **SHPHLD** pins may be tied to **AVSS**.

![](_page_10_Figure_3.jpeg)

*Figure 3: A typical configuration for Ship mode*

#### <span id="page-10-0"></span>3.6 Thermal protection

The device implements thermal regulation based on battery temperature, see [Battery thermal protection](#page-22-2) [using NTC thermistor \(NTC\)](#page-22-2) on page 23 and [Charger thermal regulation](#page-23-0) on page 24.

In addition to battery thermal protection and charger thermal regulation, a global thermal shutdown based on die temperature is implemented when die temperature exceeds the operating temperature range, see [TSD](#page-11-1). All device functions are disabled in thermal shutdown. The device functions are re-enabled when the temperature is sufficiently reduced according to a hysteresis [TSD](#page-11-1)HYST.

#### <span id="page-10-1"></span>3.7 Battery considerations

The charger can only be used with Li-ion/Li-poly rechargeable batteries.

Battery packs connected to the **VBAT** pin must contain the following protection circuitry:

- Overcharge protection
- Undervoltage protection
- Overcurrent discharge fuse
- Thermal fuse to protect from overtemperature (if NTC thermistor is not present)

#### <span id="page-10-2"></span>3.8 Charging and error LED drivers

CHARGER controls the **CHG** and **ERR** pins, which are used to drive LEDs and signal status to an external circuit.

See [Charging indication \(CHG\) and charging error indication \(ERR\)](#page-24-0) on page 25 for more information.

#### <span id="page-10-3"></span>3.9 System electrical parameters

![](_page_10_Picture_19.jpeg)

<span id="page-11-1"></span>

| Symbol        | Description                                                                     | Min. | Typ. | Max. | Unit |
|---------------|---------------------------------------------------------------------------------|------|------|------|------|
| IQSHIP        | Ship mode quiescent current                                                     | -    | 460  | -    | nA   |
| IQBAT         | Quiescent current, battery operation, no load, MODE =<br>LOW, VBUS disconnected | -    | 800  | -    | nA   |
| TSD           | Thermal shutdown threshold                                                      | -    | 120  | -    | °C   |
| TSDHYST       | Thermal shutdown hysteresis                                                     | -    | 10   | -    | °C   |
| VIH           | Input HIGH                                                                      | 1.1  | -    | VINT | V    |
| VIL           | Input LOW                                                                       | 0    | -    | 0.4  | V    |
| RSHPACT       | Internal resistance between SHPACT and AVSS                                     |      | 500  |      | kΩ   |
| tactiveToShip | Duration SHPACT must be held high to enable Ship mode                           | 200  |      |      | ms   |
| tshipToActive | Duration SHPHLD must be held low to disable Ship mode                           | 200  |      |      | ms   |

*Table 3: System electrical parameters*

### <span id="page-11-0"></span>3.10 System efficiency

Described here is the characterization of the power path from the battery supply (**VBAT**) to the BUCK output (**VOUTB**) under different battery voltages, output voltages, and load current conditions.

In the following figure, the load current is swept from 1 µA to 150 mA and back to capture mode change hysteresis.

![](_page_11_Figure_6.jpeg)

*Figure 4: VOUTB = 3.0 V system efficiency, MODE=AUTO*

![](_page_11_Picture_8.jpeg)

# <span id="page-12-1"></span><span id="page-12-0"></span>4 Absolute maximum ratings

Maximum ratings are the extreme limits to which the chip can be exposed for a limited amount of time without permanently damaging it. Exposure to absolute maximum ratings for prolonged periods of time may affect the reliability of the device.

| Pin           | Note                                                                    | Min. | Max.       | Unit |
|---------------|-------------------------------------------------------------------------|------|------------|------|
| VBUS          | Power                                                                   | -0.3 | 20         | V    |
| VBAT          | Power                                                                   | -0.3 | 5.5        | V    |
| VSYS, DEC, SW |                                                                         | -0.3 | 5.5        | V    |
| AVSS, PVSS    | Power                                                                   |      | 0          | V    |
| VANAI/O       | Analog I/O                                                              | -0.3 | VINT + 0.3 | V    |
|               | D-, D+, NTC, ICHG, VOUTB                                                |      |            |      |
| VDIGI/O       | Digital I/O                                                             | -0.3 | VINT + 0.3 | V    |
|               | VOUTBSET0, VOUTBSET1, VTERMSET, SHPHLD,<br>SHPACT, ISET, ERR, CHG, MODE |      |            |      |

*Table 4: Pin voltage*

|                        | Note                       | Min. | Max. | Unit |
|------------------------|----------------------------|------|------|------|
| Storage<br>temperature |                            | -40  | +125 | °C   |
| MSL                    | Moisture Sensitivity Level |      | 1    |      |
| ESD HBM                | Human Body Model Class 2   |      | 2    | kV   |
| ESD CDM                | Charged Device Model       |      | 500  | V    |

*Table 5: Environmental (WLCSP package)*

|                        | Note                       | Min. | Max. | Unit |
|------------------------|----------------------------|------|------|------|
| Storage<br>temperature |                            | -40  | +125 | °C   |
| MSL                    | Moisture Sensitivity Level |      | 2    |      |
| ESD HBM                | Human Body Model Class 2   |      | 2    | kV   |
| ESD CDM                | Charged Device Model       |      | 500  | V    |

*Table 6: Environmental (QFN package)*

![](_page_12_Picture_8.jpeg)

![](_page_13_Picture_1.jpeg)

![](_page_13_Picture_2.jpeg)

# <span id="page-14-2"></span><span id="page-14-0"></span>5 Recommended operating conditions

The operating conditions are the physical parameters that the chip can operate within.

| Symbol | Parameter                | Notes   | Min. | Nom. | Max. | Unit |
|--------|--------------------------|---------|------|------|------|------|
| VBUSOP | Supply voltage           |         | 4.1  | 5    | 6.7  | V    |
| VBATOP | Battery voltage          |         | 2.30 |      | 4.35 | V    |
| TJ     | Junction<br>temperature  |         | -40  |      | +125 | °C   |
| TO     | Operating<br>temperature | Ambient | -40  |      | +85  | °C   |

*Table 7: Recommended operating conditions*

### <span id="page-14-1"></span>5.1 Dissipation ratings

Thermal resistances and thermal characterization parameters as defined by JESD51-7 are shown in the following table.

| Symbol    | Parameter                                    | WLCSP 25 pins | Units |
|-----------|----------------------------------------------|---------------|-------|
| RϴJA      | Junction-to-ambient thermal resistance       | 50.7          | °C/W  |
| RϴJC(top) | Junction-to-case (top) thermal resistance    | 9.2           | °C/W  |
| RϴJB      | Junction-to-board thermal resistance         | 22.6          | °C/W  |
| ΨJT       | Junction-to-top characterization parameter   | 1.05          | °C/W  |
| ΨJB       | Junction-to-board characterization parameter | 23            | °C/W  |

*Table 8: Recommended operating conditions*

| Symbol    | Parameter                                    | QFN 24 pins | Units |
|-----------|----------------------------------------------|-------------|-------|
| RϴJA      | Junction-to-ambient thermal resistance       | 33.5        | °C/W  |
| RϴJC(top) | Junction-to-case (top) thermal resistance    | 16.1        | °C/W  |
| RϴJB      | Junction-to-board thermal resistance         | 14.1        | °C/W  |
| ΨJT       | Junction-to-top characterization parameter   | 0.25        | °C/W  |
| ΨJB       | Junction-to-board characterization parameter | 14.1        | °C/W  |

*Table 9: Recommended operating conditions*

![](_page_14_Picture_11.jpeg)

#### <span id="page-15-0"></span>5.2 WLCSP light sensitivity

WLCSP package is sensitive to visible and near infrared light, which means that a final product design must shield the chip properly.

![](_page_15_Picture_3.jpeg)

# <span id="page-16-0"></span>6 Core components

#### <span id="page-16-1"></span>6.1 SYSREG — System regulator

**VBUS** supplies the input voltage to the system voltage regulator (SYSREG) . **VBUS** voltage is supplied by AC wall adapters or USB ports.

SYSREG is a linear voltage regulator (LDO) that supplies VINT.

Features of SYSREG are the following:

- 5 V linear voltage regulator (LDO) supplying VINT when **VBUS** is connected
- Operating voltage up to 6.7 V
- Overvoltage protection to 20 V
- USB port detection and control pin for setting the current limit on **VBUS**

**Note:** The **VSYS** and **DEC** pins must not be externally supplied.

#### <span id="page-16-2"></span>6.1.1 USB port detection and VBUS current limiting

The device supports automatic detection of USB port type in line with the *Battery Charging Specification* v1.2 found on [usb.org](http://www.usb.org).

Primary detection is performed for Standard Downstream Port (SDP), Dedicated Charging Port (DCP), and Charging Downstream Port (CDP) USB ports. The detection sequence starts once **VBUS** is connected, and completes after T[CONN0](#page-17-4) .

If SDP is detected, the **VBUS** current limit is set to 100 mA. An external microcontroller with a USB interface can negotiate a 500 mA limit with the USB host. It then raises the **VBUS** current limit using a GPIO to control **ISET**. This is referred to as USB port negotiation.

If DCP/CDP is detected, the **VBUS** current limit is set to 500 mA. In this case, **ISET** configuration is ignored.

It is possible to configure the device to set the **VBUS** current limit to either 100 mA or 500 mA using **ISET** and disabling USB port detection.

The following table describes **ISET**, **D+**, and **D-** configurations to fix **VBUS** current limit or set **VBUS** current limit based on either USB port detection or USB port negotiation.

![](_page_16_Picture_17.jpeg)

| Limit set method                                                                  | Pin configuration                                                 | VBUS current limit                                                                                   |
|-----------------------------------------------------------------------------------|-------------------------------------------------------------------|------------------------------------------------------------------------------------------------------|
| Fixed 100 mA                                                                      | ISET = D- = AVSS<br>D+ = NC                                       | 100 mA                                                                                               |
| Fixed 500 mA                                                                      | ISET = VSYS<br>D- = AVSS<br>D+ = NC                               | 500 mA                                                                                               |
| USB port detection                                                                | ISET = AVSS<br>D+ and D- are connected to host                    | 100 mA if SDP detected<br>500 mA if DCP/CDP detected                                                 |
| USB port detection and<br>negotiation (requires a USB<br>enabled microcontroller) | ISET = microcontroller GPIO<br>D+ and D- connected to USB<br>host | 100 mA if SDP detected, ISET =<br>LOW<br>500 mA if SDP detected, ISET =<br>HIGH<br>500 mA if DCP/CDP |

*Table 10: Pin configuration for VBUS current limit*

When a microcontroller uses GPIO to control **ISET** for USB port negotiation, **ISET** must be set LOW on reset and when USB is disconnected. **ISET** is only set HIGH when the USB port is SDP and negotiation for a higher current limit is complete.

See the circuit schematics in the [Reference circuitry](#page-49-0) on page 50 for designs illustrating these configurations.

#### <span id="page-17-0"></span>6.1.2 SYSREG resistance and output voltage

SYSREG regulates the VINT voltage to [VINT](#page-17-4)REG. When the **VBUS** pin voltage is below [VINT](#page-17-4)REG, there is typically [RON](#page-17-4)REG resistance between **VBUS** and VINT.

#### <span id="page-17-1"></span>6.1.3 VBUS overvoltage and undervoltage protection

The overvoltage threshold for **VBUS** is [VBUS](#page-17-4)OVP. The undervoltage threshold for **VBUS** is [VBUS](#page-17-4)MIN.

SYSREG is disabled when **VBUS** voltage is above the overvoltage threshold [VBUS](#page-17-4)OVP, or below the undervoltage threshold [VBUS](#page-17-4)MIN. This isolates **VBUS** and prevents current flowing from VINT to **VBUS**.

#### <span id="page-17-2"></span>6.1.4 VBUS disconnect

SYSREG isolates **VBUS** from VINT when **VBUS** is disconnected and the voltage drops below [VBUS](#page-17-4)MIN.

When **VBUS** reaches [VBUS](#page-17-4)ULP, the device enters an ultra-low power (ULP) operation state. This takes T[DISCONN](#page-17-4), dependent on capacitive load on **VBUS**. The device stays in a ULP state while **VBUS** is under [VBUS](#page-17-4)ULP.

#### <span id="page-17-3"></span>6.1.5 Electrical specification

<span id="page-17-4"></span>

| Symbol   | Description                                           | Min. | Typ. | Max. | Units |
|----------|-------------------------------------------------------|------|------|------|-------|
| IBUSLIM1 | Max VBUS input current, CDP/DCP USB or ISET =<br>HIGH | 450  | -    | 500  | mA    |

![](_page_17_Picture_15.jpeg)

| Symbol   | Description                                                                    | Min. | Typ. | Max. | Units |
|----------|--------------------------------------------------------------------------------|------|------|------|-------|
| IBUSLIM0 | Max VBUS input current, SDP USB and ISET = LOW,<br>25°C                        | 90   | -    | 100  | mA    |
| IBUSQ    | Quiescent current taken from VBUS, no charging, no<br>load on VSYS, VBUS = 5 V |      | 2    |      | mA    |
| VINTREG  | Regulated VINT voltage from SYSREG, VBUS = 6 V                                 |      | 5.2  |      | V     |
| RONREG   | SYSREG on resistance, ISET = HIGH                                              | -    | 440  | 720  | mΩ    |
| VBUSOVP  | Overvoltage protection threshold                                               |      | 6.9  |      | V     |
| VBUSMIN  | Undervoltage threshold                                                         |      | 3.9  |      | V     |
| VBUSULP  | Threshold for entering ULP mode                                                |      | 1.8  |      | V     |
| VBUSPOR  | Power-on reset release voltage for VBUS                                        |      | 3.9  |      | V     |
| VBUSBOR  | Brownout reset trigger voltage for VBUS1                                       |      | 3.8  |      | V     |
| TCONN0   | Time for USB detection, ISET = LOW                                             |      | -    | 700  | ms    |
| TCONN1   | Time for VINT to settle after VBUS connection, ISET =<br>HIGH, no load         | -    | 1.2  |      | ms    |
| TDISCONN | Time for system to reach ULP mode after VBUS<br>disconnect, CVBUS = 10 µF      | -    | 110  |      | ms    |

*Table 11: SYSREG electrical specification*

#### <span id="page-18-0"></span>6.1.6 Electrical characteristics

The following graphs show SYSREG electrical characteristics.

![](_page_18_Figure_6.jpeg)

*Figure 5: VSYS voltage vs. VBUS current, ILIM=500 mA*

![](_page_18_Picture_8.jpeg)

<span id="page-18-1"></span><sup>1</sup> Device enters BOR only if (V(**VBUS**) < VBUSBOR) AND (V(**VBAT**) < [VBAT](#page-25-1)BOR).

![](_page_19_Figure_1.jpeg)

*Figure 6: VSYS voltage vs. VBUS voltage, ILIM=500 mA*

![](_page_19_Figure_3.jpeg)

*Figure 7: VSYS voltage vs. VBUS current, ILIM=100 mA*

![](_page_19_Picture_5.jpeg)

![](_page_20_Figure_1.jpeg)

*Figure 8: VSYS voltage vs. VBUS voltage, ILIM=100 mA*

#### <span id="page-20-0"></span>6.2 CHARGER — Battery charger

The battery charger is suitable for any general purpose applications with lithium-ion/lithium-polymer battery types.

The main features of the battery charger are the following:

- Linear charger for Li-ion/Li-poly battery chemistries
- Configurable charge current with a resistor connected to the **ICHG** pin (from 20 mA to 400 mA)
- Bidirectional power FET for dynamic power-path management
- Active current limitation when **VBAT** supplies VINT
- Selectable termination voltage through the **VTERMSET** pin
  - 4.1 V or 4.2 V on the standard VTERM product
  - 4.25 V or 4.35 V on the high VTERM product
- Automatic trickle, constant current, constant voltage, and end-of-charge/recharge cycle
- JEITA compliant battery thermal protection (NTC) with standard and extended temperature range

#### <span id="page-20-1"></span>6.2.1 Charging cycle

Battery charging starts after a **VBUS** connection and the battery is detected.

If a battery is found, trickle charging begins. Fast charging starts when the battery voltage is above V[TRICKLE\\_FAST](#page-25-1). After the battery voltage reaches V[TERM](#page-21-0), the charger enters constant voltage charging. The battery voltage is maintained while monitoring current flow into the battery. When the current into the battery drops below I[TERM](#page-25-1), charging is complete. The charger waits until the battery voltage is below V[RECHARGE](#page-25-1) before starting a new charging cycle.

To charge the battery, VBUS voltage must be higher than VBAT voltage during the charge cycle. This means VBUS must be VBUS(V) > VBAT(V) + VDROPOUT\_VBUS. If this condition is not met the charge cycle stops.

![](_page_20_Picture_19.jpeg)

![](_page_21_Figure_1.jpeg)

*Figure 9: Charging cycle flow chart*

![](_page_21_Figure_3.jpeg)

*Figure 10: Charging cycle*

#### <span id="page-21-0"></span>6.2.2 Termination voltage (VTERMSET)

The termination voltage, VTERM, is set using **VTERMSET** to support two values of battery charging termination voltage for the chosen product option.

![](_page_21_Picture_7.jpeg)

| Product option | VTERMSET | VTERM threshold |
|----------------|----------|-----------------|
| Standard VTERM | LOW      | 4.1 V           |
| Standard VTERM | HIGH     | 4.2 V           |
| High VTERM     | LOW      | 4.25 V          |
| High VTERM     | HIGH     | 4.35 V          |

*Table 12: VTERMSET*

#### <span id="page-22-0"></span>6.2.3 Termination and trickle charge current

Termination current and trickle charge current are set to a percentage of the charge current limit (I[CHGLIM](#page-22-3)). See [Electrical specification](#page-25-0) on page 26 for the limits.

#### <span id="page-22-1"></span>6.2.4 Charge current limit (ICHG)

The charge current limit is set between 20 mA and 400 mA by connecting the RICHG resistor to the **ICHG** and **AVSS** pins.

The following equation gives the resistance to be connected based on the ICHGLIM.

$$R_{ICHG} = \frac{625}{I_{CHGLIM}} - 1562.5$$

The following apply when the RICHG resistor is between 0 Ω and 30 kΩ.

- ICHGLIM is the fast charge current limit in Amps
- RICHG is the resistance to be connected between the **ICHG** and **AVSS** pins in Ω

Common values are provided in the following table.

<span id="page-22-3"></span>

| RICHG resistor value | Nominal charge current limit,<br>ICHGLIM | Error                   |
|----------------------|------------------------------------------|-------------------------|
| 0 (short to AVSS)    | 400 mA                                   | ± ICHGACC%              |
| 1.5 kΩ               | 200 mA                                   | ± (ICHGACC + RICHGACC)% |
| 4.7 kΩ               | 100 mA                                   | ± (ICHGACC + RICHGACC)% |
| 11 kΩ                | 50 mA                                    | ± (ICHGACC + RICHGACC)% |
| 30 kΩ                | 20 mA                                    | ± (ICHGACC + RICHGACC)% |

*Table 13: Common charge current values*

**Note:** ICHGLIM must be set at or below the safe charge current limit of the battery according to the battery specification.

#### <span id="page-22-2"></span>6.2.5 Battery thermal protection using NTC thermistor (NTC)

Battery thermal protection is implemented in the following two ways.

• Using a battery pack with an integrated NTC thermistor

#### • Connecting a thermistor between the **NTC** pin and the **AVSS** pin

The thermistor needs to have thermal contact with the battery and preferably within the battery pack. Recommended values for the NTC thermistor are found in the following table.

| Parameter                  | Value        | Unit   |
|----------------------------|--------------|--------|
| Nominal resistance at 25°C | 10           | kΩ     |
| Resistance accuracy        | 1            | %      |
| B25/50 constant            | 3380         | Kelvin |
| B25/85 constant            | 3434 to 3435 | Kelvin |
| B constant accuracy        | 1            | %      |

*Table 14: Recommended NTC thermistor values*

If the thermal protection feature is not used, then a 10 kΩ, ≤20% accuracy resistor should be connected between **NTC** and **AVSS** pins.

To provide JEITA compliant thermal protection, the charge current limit and termination voltage are adjusted according to the NTC thermistor measurement.

| Temperature region | Battery temperature | Charging current | Termination voltage |
|--------------------|---------------------|------------------|---------------------|
| Cold               | T < 0°C             | 0 (OFF)          | NA                  |
| Cool               | 0°C < T < 10°C      | IREDUCED         | VTERM               |
| Nominal            | 10°C < T < 45°C     | ICHGLIM          | VTERM               |
| Warm               | 45°C < T < 60°C     | ICHGLIM          | VTERM-VTHIGH_DELTA  |
| Hot                | T > 60°C            | 0 (OFF)          | NA                  |

*Table 15: Battery temperature ranges*

#### <span id="page-23-0"></span>6.2.6 Charger thermal regulation

If the device junction temperature exceeds T[HIGH](#page-25-1) and CHARGER is in Fast Charge mode, the charge current is reduced to I[REDUCED](#page-25-1).

#### <span id="page-23-1"></span>6.2.7 Charger error conditions

A CHARGER error condition occurs when one of the following are present:

- A battery short (**VBAT** to **AVSS**)
- Battery voltage lower than VBATCHARGEMIN after battery detection due to a fault with the battery
- Trickle charge timeout; see [TOUT](#page-25-1)TRICKLE
- Constant voltage charge/fast charge timeout; see [TOUT](#page-25-1)CHARGE
- Device internal error occurs when CHARGER is self-checking

After an error is detected, CHARGER is disabled, the charging error indication is activated, and the charging indication is deactivated. Error conditions are cleared when **VBUS** is disconnected and reconnected again.

**Note:** The constant voltage/fast charge timeout is the combined time spent in both constant voltage charge and fast charge, [TOUT](#page-25-1)CHARGE.

![](_page_23_Picture_20.jpeg)

#### <span id="page-24-0"></span>6.2.8 Charging indication (CHG) and charging error indication (ERR)

The charging indication pin **CHG** and charging error indication pin **ERR** sink 5 mA of current when active. They are high impedance when disabled. This is suitable for driving LEDs or connecting to host GPIOs in a weak pull-up configuration.

![](_page_24_Picture_3.jpeg)

*Figure 11: Configuration for connecting to LEDs*

![](_page_24_Picture_5.jpeg)

*Figure 12: Configuration for connecting to a host*

**Note:** To configure both LED indication and connection to a host, the GPIO input voltage range tolerance must be met, or an external circuit may be required. See [Reference circuitry](#page-49-0) on page 50.

The charging indication pin, **CHG**, is active while the battery is charging.

The charging error indication pin, **ERR**, is activated when an error occurs, see [Charger error conditions](#page-23-1) on page 24.

#### <span id="page-24-1"></span>6.2.9 DPPM — Dynamic power-path management

CHARGER manages battery current flow to maintain VINT voltage.

The system load requirements are prioritized over battery charge current when **VBUS** is connected and the battery is charging. The battery is isolated when **VBUS** is connected and the battery is fully charged. SYSREG supplies the load unless the load exceeds SYSREG limits. When **VBUS** is disconnected, CHARGER switches to battery supply.

During charging, if the combined current load ILOAD on VINT (including BUCK input current) and **VBAT** (ICHG) exceeds the current provided by SYSREG (ILIM), the battery charge current decreases to maintain the VINT voltage. The battery charger reduces the current to maintain the internal voltage: VINT = V(**VBAT** )+

![](_page_24_Picture_14.jpeg)

V[DROPOUT\\_CHARGER](#page-25-1). If more current is required, CHARGER enters Supplement mode, switching to provide current from the battery, up to [IBAT](#page-25-1)LIM.

If a charge cycle ends and ILOAD exceeds ILIM, CHARGER connects the battery and enters Supplement mode to maintain VINT.

When **VBUS** and the battery are connected, the maximum supported load is ILIM + IBATLIM.

When **VBUS** is disconnected, CHARGER sources current for VINT from the battery. In Supplement mode, or when **VBUS** is disconnected, VINT voltage is the same as the battery voltage.

| VBUS<br>connected | Battery<br>connected | Load                                     | CHARGER                    | VINT supply      | VINT voltage                 |
|-------------------|----------------------|------------------------------------------|----------------------------|------------------|------------------------------|
| Yes               | Yes                  | (ILOAD + ICHGLIM) < ILIM                 | Charging                   | VBUS             | V(VBUS)                      |
| Yes               | Yes                  | (ILOAD + ICHGLIM) > ILIM<br>ILOAD < ILIM | Charging<br>(ICHG reduced) | VBUS             | V(VBAT) +<br>VDROPOUTCHARGER |
| Yes               | Yes                  | ILOAD > ILIM                             | Supplement<br>mode         | VBUS and<br>VBAT | 1<br>V(VBAT)                 |
| Yes               | No                   | ILOAD < ILIM                             | N/A                        | VBUS             | V(VBUS)                      |
| No                | Yes                  | ILOAD ≤ IBATLIM                          | N/A                        | VBAT             | 1<br>V(VBAT)                 |

*Table 16: Battery supply*

#### <span id="page-25-0"></span>6.2.10 Electrical specification

<span id="page-25-1"></span>

| Symbol       | Description                                                                       | Min. | Typ. | Max. | Unit         |
|--------------|-----------------------------------------------------------------------------------|------|------|------|--------------|
| ICHGACC      | Fast Charge current accuracy for ICHG ≥ 50 mA,<br>0.1% accuracy external resistor |      | ±10  |      | %            |
| ICHGACC      | Fast Charge current accuracy for ICHG < 50 mA,<br>0.1% accuracy external resistor |      | ±15  |      | %            |
| VTERM0       | Standard termination voltage, VTERMSET = LOW                                      | -    | 4.1  | -    | V            |
| VTERM1       | Standard termination voltage, VTERMSET =<br>HIGH                                  | -    | 4.2  | -    | V            |
| VTERM0       | High termination voltage, VTERMSET = LOW                                          | -    | 4.25 | -    | V            |
| VTERM1       | High termination voltage, VTERMSET = HIGH                                         | -    | 4.35 | -    | V            |
| VTERMACC0    | Termination voltage accuracy                                                      | -1   | -    | +1   | %            |
| VTHIGH_DELTA | VTERM voltage reduction at high temperature                                       |      | 100  |      | mV           |
| ITERM        | Termination current                                                               | 8    | 10   | 12   | % of<br>ICHG |

![](_page_25_Picture_10.jpeg)

<span id="page-25-2"></span><sup>1</sup> CHARGER has a resistance of RON[CHARGER](#page-25-1) between **VBAT** and VINT. The voltage drop from **VBAT** to VINT is IBAT x RONCHARGER, where IBAT is the current being drawn from the battery.

| Symbol           | Description                                                                                                        | Min.  | Typ.  | Max.  | Unit          |
|------------------|--------------------------------------------------------------------------------------------------------------------|-------|-------|-------|---------------|
| ITRICKLE         | Trickle charge current                                                                                             |       | 10    |       | % of<br>ICHG  |
| IREDUCED         | Fast charge current when device junction<br>temperature is above THIGH or battery<br>temperature is below TNTCCOOL | -     | 50    | -     | % of<br>ICHG  |
| THIGH            | High temperature threshold                                                                                         | -     | 100   | -     | °C            |
| THIGHHYST        | High temperature hysteresis                                                                                        | -     | 10    | -     | °C            |
| VTRICKLE_FAST    | Trickle to Fast Charge threshold                                                                                   | -     | 2.9   | -     | V             |
| VRECHARGE        | Recharge threshold                                                                                                 | -     | 97    | -     | % of<br>VTERM |
| VBATCHARGEMIN    | Minimum voltage during charge                                                                                      | -     | 2.1   | -     | V             |
| TOUTTRICKLE      | Trickle charging timeout                                                                                           | -     | 10    | -     | min           |
| TOUTCHARGE       | Timeout for Fast charging and constant current<br>charging                                                         | -     | 7     | -     | hour          |
| VDROPOUT_CHARGER | VINT - VBAT voltage for charging                                                                                   | -     | 50    | -     | mV            |
| VDROPOUT_VBUS    | Minimum VBUS - VBAT voltage for charging                                                                           | -     | 140   | -     | mV            |
| TREDETECT        | Period between detection events                                                                                    | -     | 500   | -     | ms            |
| IBATLIM          | Output current limit from battery in discharge                                                                     | -     | 660   | -     | mA            |
| RONCHARGER       | CHARGER resistance between VBAT and VINT in<br>Discharge, VBAT = 3.7 V                                             | -     | 130   | 230   | mΩ            |
| VBATPOR          | Power-on reset release voltage for VBAT                                                                            | -     | 2.7   | -     | V             |
| VBATBOR          | Brownout reset trigger voltage for VBAT 1                                                                          | -     | 2.5   | -     | V             |
| ISINK            | DC current (CHG and ERR)                                                                                           | -     | 5     | -     | mA            |
| TNTCCOLD         | JEITA cold temperature threshold (Thermistor: 10<br>kΩ, B25/50=3380 K)                                             | -     | 0     | -     | °C            |
| RNTCCOLD_FALLING | Resistance threshold from cool to cold                                                                             | 25.53 | 27.28 | 29.13 | kΩ            |
| RNTCCOLD_RISING  | Resistance threshold from cold to cool                                                                             | 23.10 | 26.00 | 28.20 | kΩ            |
| TNTCCOOL         | JEITA cool temperature threshold (Thermistor: 10<br>kΩ, B25/50=3380 K)                                             | -     | 10    | -     | °C            |
| RNTCCOOL_FALLING | Resistance threshold from nom. to cool                                                                             | 16.80 | 18.00 | 19.20 | kΩ            |
| RNTCCOOL_RISING  | Resistance threshold from cool to nom.                                                                             | 15.50 | 17.10 | 18.60 | kΩ            |
| TNTCWARM         | JEITA warm temperature threshold (Thermistor:<br>10 kΩ, B25/50=3380 K)                                             | -     | 45    | -     | °C            |
| RNTCWARM_FALLING | Resistance threshold from warm to nom.                                                                             | 4.86  | 5.13  | 5.43  | kΩ            |
| RNTCWARM_RISING  | Resistance threshold from nom. to warm                                                                             | 4.68  | 4.92  | 5.17  | kΩ            |
| TNTCHOT          | JEITA hot temperature threshold (Thermistor: 10<br>kΩ, B25/50=3380 K)                                              | -     | 60    | -     | °C            |
| RNTCHOT_FALLING  | Resistance threshold from hot to warm                                                                              | 3.04  | 3.19  | 3.35  | kΩ            |

![](_page_26_Picture_2.jpeg)

| Symbol         | Description                           | Min. | Typ. | Max. | Unit |
|----------------|---------------------------------------|------|------|------|------|
| RNTCHOT_RISING | Resistance threshold from warm to hot | 2.90 | 3.02 | 3.15 | kΩ   |

*Table 17: CHARGER electrical specification*

#### <span id="page-27-0"></span>6.2.11 Electrical characteristics

The following graphs show CHARGER electrical characteristics.

![](_page_27_Figure_6.jpeg)

*Figure 13: CHARGER RDS(ON) vs. VBAT voltage*

![](_page_27_Picture_8.jpeg)

<span id="page-27-1"></span><sup>1</sup> Device enters BOR only if (V(VBUS) < [VBUS](#page-22-3)BOR) AND (V(VBAT) < VBATBOR).

![](_page_28_Figure_1.jpeg)

*Figure 14: Quiescent VBAT current vs. VBAT voltage*

![](_page_28_Figure_3.jpeg)

*Figure 15: CHARGER RDS(ON) vs. temperature*

![](_page_28_Picture_5.jpeg)

![](_page_29_Figure_1.jpeg)

*Figure 16: Quiescent VBAT current vs. temperature*

![](_page_29_Figure_3.jpeg)

*Figure 17: VTERM vs. temperature*

![](_page_29_Picture_5.jpeg)

![](_page_30_Figure_1.jpeg)

*Figure 18: Charge profile with ISET=1*

#### <span id="page-30-0"></span>6.3 BUCK — Buck regulator

BUCK is a step-down DC/DC voltage regulator with the following features:

- High efficiency (low IQ) and low noise operation
- PWM and Hysteretic modes with automatic switching based on load
- **MODE** control pin for forcing PWM mode to minimize output voltage ripple
- Configurable output voltage between 1.8 V and 3.0 V

When VINT is above VINTBUCKMIN, BUCK is enabled and its output voltage is available at VOUTB.

Hysteretic mode offers efficiency for the full range of supported load currents. PWM mode provides a clean supply operation due to a constant switching frequency, FBUCK. This provides optimal coexistence with RF circuits. BUCK can automatically change between Hysteretic and PWM modes. Modes are controlled by the **MODE** pin. The state of the **MODE** pin can be changed at any time.

#### <span id="page-30-1"></span>6.3.1 Output voltage selection (VOUTBSET0, VOUTBSET1)

BUCK output voltage selection pins **VOUTBSET0** and **VOUTBSET1** should be hardwired to **DEC**, **VSYS**, or **AVSS**. Do not toggle these pins during operation.

| VOUTBSET1 | VOUTBSET0 | VOUTB voltage |
|-----------|-----------|---------------|
| LOW       | LOW       | 1.8 V         |
| LOW       | HIGH      | 2.1 V         |
| HIGH      | LOW       | 2.7 V         |
| HIGH      | HIGH      | 3.0 V         |

*Table 18: Output voltage selection*

![](_page_30_Picture_15.jpeg)

For BUCK to supply the desired output voltage, VINT must be VDROPOUT\_BUCK greater than the voltage on **VOUTB**.

When supplied from battery, the following equation gives the VINT, where IBAT is the current being drawn from the battery:

VINT = VBAT – IBAT x RONCHARGER

#### <span id="page-31-0"></span>6.3.2 BUCK mode selection (MODE)

In Automatic mode, BUCK selects Hysteretic mode for low load currents, and PWM mode for high load currents.

This maximizes efficiency over the full range of supported load currents. In PWM mode, BUCK provides a clean supply operation due to constant switching frequency and lower voltage ripple. This allows for optimal coexistence with RF circuits. The **MODE** pin can be changed at any time.

| MODE | BUCK operation mode                                     |
|------|---------------------------------------------------------|
| LOW  | Automatic selection between Hysteretic and PWM<br>modes |
| HIGH | PWM mode                                                |

*Table 19: BUCK mode selection*

#### <span id="page-31-1"></span>6.3.3 Component selection

Recommended values for the inductor are shown in the following table.

| Parameter                 | Value | Units |
|---------------------------|-------|-------|
| Nominal inductance        | 2.2   | μH    |
| Inductor tolerance        | ≤ 20  | %     |
| DC resistance (DCR)       | ≤ 400 | mΩ    |
| Saturation current (lsat) | ≥ 350 | mA    |
| Maximum current (lmax)    | ≥ 350 | mA    |

*Table 20: Inductor selection*

The following table shows the minimum and maximum effective capacitance at VOUTB.

| Recommended nominal capacitor | Min. | Max.  |  |
|-------------------------------|------|-------|--|
| 10 µF                         | 6 µF | 20 µF |  |

*Table 21: Output capacitor selection*

#### <span id="page-31-2"></span>6.3.4 Electrical specification

![](_page_31_Picture_17.jpeg)

| Symbol           | Description                                                                                                           |   | Typ. | Max. | Unit |
|------------------|-----------------------------------------------------------------------------------------------------------------------|---|------|------|------|
| VOUTBACC         | VOUTB accuracy under static conditions; no<br>-2<br>change in supply voltage, load current, or Buck<br>operating mode |   | -    | 8    | %    |
| IOUTBSHORT       | Short circuit current limit                                                                                           | - | -    | 400  | mA   |
| IPWMTHRES        | Load current threshold from Hysteretic to PWM<br>mode (MODE = LOW)                                                    |   | 90   |      | mA   |
| IHYSTTHRES       | Load current threshold from PWM to Hysteretic<br>mode (MODE = LOW)                                                    |   | 40   |      | mA   |
| VOUTBRIPPLE_PWM  | VOUTB ripple, MODE = HIGH or load current<br>-<br>-<br>above IPWMTHRES                                                |   |      | 10   | mVpp |
| VOUTBRIPPLE_HYST | VOUTB ripple, MODE = LOW and load current<br>-<br>-<br>below IPWMTHRES                                                |   |      | 80   | mVpp |
| EFFBUCK          | Efficiency, VOUTBSET = 11 (VOUTB = 3.0 V), VINT<br>-<br>= 3.7 V, IOUTB = 100 mA                                       |   | 93.5 | -    | %    |
| VDROPOUT_BUCK    | Dropout voltage, V(VOUTB) - VINT                                                                                      | - | 0.41 |      | V    |
| FBUCK            | Switching frequency for PWM mode                                                                                      |   | 3.6  | -    | MHz  |
| TPWMMODE         | Hysteretic to PWM mode transition time on MODE<br>pin toggle                                                          |   | -    | 55   | μs   |
| THYSTMODE        | PWM to Hysteretic mode transition time on MODE<br>-<br>pin toggle                                                     |   | -    | 25   | μs   |
| TPWM             | Hysteretic to PWM mode transition time                                                                                |   | -    | 90   | μs   |
| THYST            | PWM to Hysteretic mode transition time                                                                                | - | -    | 35   | μs   |
| TSETTLE          | Settling time to within 1% after load transient of 0<br>A to 100 mA                                                   |   | -    | 20   | μs   |
| VINTBUCKMIN      | Minimum VINT voltage for enabling BUCK                                                                                | - | 2.8  | -    | V    |

*Table 22: BUCK electrical specification*

#### <span id="page-32-0"></span>6.3.5 Electrical characteristics

The following graphs show BUCK electrical characteristics.

![](_page_32_Picture_5.jpeg)

![](_page_33_Figure_1.jpeg)

*Figure 19: VOUTB=3.0 system efficiency, MODE=AUTO*

![](_page_33_Figure_3.jpeg)

*Figure 20: VOUTB=3.0 system efficiency, MODE=PWM*

![](_page_33_Picture_5.jpeg)

![](_page_34_Figure_1.jpeg)

*Figure 21: VOUTB=3.0: VOUTB vs. temperature (VBAT=4.2)*

![](_page_34_Figure_3.jpeg)

*Figure 22: VOUTB=2.7 system efficiency, MODE=AUTO*

![](_page_34_Picture_5.jpeg)

![](_page_35_Figure_1.jpeg)

*Figure 23: VOUTB=2.7 system efficiency, MODE=PWM*

![](_page_35_Figure_3.jpeg)

*Figure 24: VOUTB=2.1 system efficiency, MODE=AUTO*

![](_page_35_Picture_5.jpeg)

![](_page_36_Figure_1.jpeg)

*Figure 25: VOUTB=2.1 system efficiency, MODE=PWM*

![](_page_36_Figure_3.jpeg)

*Figure 26: VOUTB=1.8 system efficiency, MODE=AUTO*

![](_page_36_Picture_5.jpeg)

![](_page_37_Figure_1.jpeg)

*Figure 27: VOUTB=1.8 system efficiency, MODE=PWM*

![](_page_37_Figure_3.jpeg)

*Figure 28: VOUTB=1.8 VOUTB vs. temperature (VBAT=4.2)*

![](_page_37_Picture_5.jpeg)

![](_page_38_Figure_1.jpeg)

*Figure 29: Startup with no load, soft start, Vout=1.8 V, VBAT=3.8 V*

![](_page_38_Figure_3.jpeg)

*Figure 30: BUCK load transition in auto mode (MODE=0), Iout=10 mA → 150 mA → 10 mA (1 µs step), Vout=1.8 V, VBAT=3.8 V*

![](_page_38_Picture_5.jpeg)

![](_page_39_Figure_1.jpeg)

*Figure 31: BUCK Mode transition, MODE pin 0 → 1, Vout=1.8 V Iout=10 mA*

![](_page_39_Figure_3.jpeg)

*Figure 32: BUCK Mode transition, MODE pin 1 → 0, Vout=1.8 V Iout=10 mA*

![](_page_39_Picture_5.jpeg)

![](_page_40_Figure_1.jpeg)

*Figure 33: BUCK load transition in PWM mode (MODE=1), Iout=10 mA → 150 mA → 10 mA (1 µs step), Vout=1.8 V, VBAT=3.8 V*

![](_page_40_Picture_3.jpeg)

### <span id="page-41-0"></span>7 Application

The following application example uses nPM1100 and an nRF5x wireless System on Chip (SoC). Any nRF52 or nRF53 series device with USB can be configured in the same way as this application. When using a device without USB, or for other configurations, see Reference circuitry on page 50.

The example application is for a design with the following configuration and features:

- nPM1100 BUCK regulator supplies the nRF5x device
- USB current limit negotiation
- Charging status monitoring using SoC GPIOs
- ICHG and VTERM configuration
- NTC thermistor in the battery pack
- Ship mode
- Battery monitoring circuit and low battery indication LED (the device must sample the battery voltage)

#### <span id="page-41-1"></span>7.1 Schematic

![](_page_41_Figure_11.jpeg)

Figure 34: Application example

#### <span id="page-41-2"></span>7.2 Supplying from BUCK

nRF5x is supplied by nPM1100 VOUTB at 1.8 V. BUCK mode (MODE) is controlled with a GPIO.

![](_page_41_Picture_15.jpeg)

An application should not be supplied directly from **VBAT** because it can disturb the battery charging process and may cause incorrect behavior from the charger. Instead, **VOUTB** and/or **VSYS** should be used to supply an application.

#### <span id="page-42-0"></span>7.3 USB port negotiation

nRF5x can connect to a USB host.

Port negotiation can be performed after nPM1100 port detection. The nRF5x device and nPM1100 are both connected to USB in the application example. nPM1100 detects SDP or CDP/DCP. If SDP is detected, the USB device can negotiate with the USB host for higher current from **VBUS**.

- **D+** and **D-** pins are connected to both nPM1100 and nRF5x. The nRF5x SoC must wait until nPM1100 completes port detection before enabling its USB port. See [USB port detection and VBUS current](#page-16-2) [limiting](#page-16-2) on page 17 for port detection time after **VBUS** connection.
- An nRF5x GPIO is connected to the **ISET** pin and sets the **VBUS** current limit after negotiation. If CDP or DCP is detected, then a 500 mA limit is automatically set regardless of **ISET** state.
- **VBUS** is supplied to both nPM1100 and nRF5x to supply nPM1100 SYSREG and the nRF5x VBUS regulator.

See [USB port detection and VBUS current limiting](#page-16-2) on page 17 for a detailed description.

### <span id="page-42-1"></span>7.4 Charging and error states

Pins **CHG** and **ERR** indicate charging and error states. See [Charging indication \(CHG\) and charging error](#page-24-0) [indication \(ERR\)](#page-24-0) on page 25 and [Charger error conditions](#page-23-1) on page 24.

#### <span id="page-42-2"></span>7.5 Termination voltage and current

For a product using standard VTERM, the termination voltage is configured to 4.2 V. See [Termination voltage](#page-21-0) [\(VTERMSET\)](#page-21-0) on page 22. The same configuration would provide 4.35 V for a product using high VTERM.

Charge current is configured to 200 mA (±10%) using a 1.5 kΩ (1%) resistor to ground on the **ICHG** pin. See [Charge current limit \(ICHG\)](#page-22-1) on page 23.

#### <span id="page-42-3"></span>7.6 NTC configuration

The **NTC** pin is connected to an external NTC thermistor which should be placed with thermal coupling to the battery pack. See [Battery thermal protection using NTC thermistor \(NTC\)](#page-22-2) on page 23 for more information.

#### <span id="page-42-4"></span>7.7 Ship mode

Ship mode is enabled at production time through an off-board circuit with a probe point on the **SHPACT** pin.

An external button is in the circuit to exit Ship mode. If another circuit is present instead of a button, any signal that is able to pull the **SHPHLD** pin low for the required period can be connected to that net. See [Using Ship mode](#page-9-2) on page 10 for more information.

![](_page_42_Picture_19.jpeg)

#### <span id="page-43-0"></span>7.8 Battery monitoring and low battery indication

The battery monitoring circuit allows the battery voltage to be sampled by the nRF5x ADC.

The transistors enable battery voltage sensing through a resistive divider. When not sampling, the transistors prevent current leakage to ground. The circuit is designed to ensure the voltage range on an analog input pin over the battery voltage is within the limits required by the nRF5x GPIO and ADC. A battery voltage of 2.8 V to 4.2 V is scaled down to 360 mV to 540 mV at **P0.xx** for sampling.

If software on nRF5x determines that the battery on nPM1100 is low, the **Low Bat LED** can be switched on through GPIO. This circuit sources the LED current from **VSYS**. **VSYS** will not be supplied after VBAT drops below [VBAT](#page-25-1)BOR because CHARGER will isolate the battery when a brownout reset occurs. See [Power-on](#page-9-0) [reset \(POR\) and brownout reset \(BOR\)](#page-9-0) on page 10.

![](_page_43_Picture_5.jpeg)

<span id="page-44-0"></span>

# 8 Hardware and layout

#### <span id="page-44-1"></span>8.1 Pin assignments

The pin assignment figures and tables describe the pinouts for the product variants of the chip.

#### <span id="page-44-2"></span>8.1.1 WLCSP ball assignments

The ball assignment figure and table describe the assignments for this variant of the chip.

![](_page_44_Picture_6.jpeg)

*Figure 35: WLCSP ball assignments*

| Pin | Name      | Function     | Description                                                             | Recommended<br>usage                              |
|-----|-----------|--------------|-------------------------------------------------------------------------|---------------------------------------------------|
| A1  | D-        | Analog input | USB D- data line                                                        |                                                   |
| A2  | D+        | Analog input | USB D+ data line                                                        |                                                   |
| A3  | DEC       | Power        | System decoupling capacitor                                             |                                                   |
| A4  | SW        | Power        | BUCK regulator output (to<br>inductor)                                  |                                                   |
| A5  | PVSS      | Power        | Ground (DC/DC)                                                          |                                                   |
| B1  | VBUS      | Power        | Input supply                                                            |                                                   |
| B2  | VBUS      | Power        | Input supply                                                            |                                                   |
| B3  | VOUTBSET1 | Digital I/O  | BUCK regulator output voltage<br>selection                              | Toggle only when the<br>device is in Power<br>OFF |
| B4  | VOUTBSET0 | Digital I/O  | BUCK regulator output voltage<br>selection                              | Toggle only when the<br>device is in Power<br>OFF |
| B5  | VOUTB     | Power        | BUCK regulator output                                                   |                                                   |
| C1  | VSYS      | Power        | System voltage output;<br>automatically enabled after power<br>on reset |                                                   |
| C2  | VSYS      | Power        | System voltage output;<br>automatically enabled after power<br>on reset |                                                   |

![](_page_44_Picture_9.jpeg)

| Pin | Name     | Function     | Description                                                                       | Recommended<br>usage                              |
|-----|----------|--------------|-----------------------------------------------------------------------------------|---------------------------------------------------|
| C3  | VTERMSET | Digital I/O  | Battery charging termination<br>voltage selection                                 | Toggle only when the<br>device is in Power<br>OFF |
| C4  | AVSS     | Power        | Ground                                                                            |                                                   |
| C5  | AVSS     | Power        | Ground                                                                            |                                                   |
| D1  | VBAT     | Power        | Battery                                                                           |                                                   |
| D2  | VBAT     | Power        | Battery                                                                           |                                                   |
| D3  | SHPHLD   | Digital I/O  | Shipping mode hold                                                                |                                                   |
| D4  | SHPACT   | Digital I/O  | Shipping mode activate                                                            |                                                   |
| D5  | ISET     | Digital I/O  | VBUS current limit selection:<br>0 mA to 100 mA (SDP mode only)<br>1 mA to 500 mA |                                                   |
| E1  | NTC      | Analog input | NTC resistor                                                                      |                                                   |
| E2  | ICHG     | Analog input | Charge current limiting resistor                                                  |                                                   |
| E3  | ERR      | Digital OUT  | Open-drain LED driver; enabled<br>when error condition in charging                |                                                   |
| E4  | CHG      | Digital OUT  | Open-drain LED driver; enabled<br>when battery is charging                        |                                                   |
| E5  | MODE     | Digital I/O  | 0 - automatic<br>1 - Forced PWM                                                   |                                                   |

*Table 23: Ball assignments*

**Note: VOUTBSET1** and **VOUTBSET0** balls are located close to **AVSS**, **DEC**, and **VSYS** to allow connection to tracks on the PCB without any via holes.

#### <span id="page-45-0"></span>8.1.2 QFN24 pin assignments

The pin assignment figure and table describe the assignments for this variant of the chip.

![](_page_45_Picture_6.jpeg)

![](_page_46_Figure_1.jpeg)

*Figure 36: QFN pin assignments*

![](_page_46_Picture_3.jpeg)

| Pin         | Name      | Function                                             | Description                                                                       |  |  |  |  |
|-------------|-----------|------------------------------------------------------|-----------------------------------------------------------------------------------|--|--|--|--|
| 1           | VOUTB     | Power                                                | BUCK regulator output                                                             |  |  |  |  |
| 2           | VOUTBSET1 | Digital I/O                                          | BUCK regulator output voltage selection                                           |  |  |  |  |
| 3           | VOUTBSET0 | Digital I/O                                          | BUCK regulator output voltage selection                                           |  |  |  |  |
| 4           | AVSS      | Power                                                |                                                                                   |  |  |  |  |
| 5           | ISET      | Digital I/O                                          | VBUS current limit selection:<br>0 mA to 100 mA (SDP mode only)<br>1 mA to 500 mA |  |  |  |  |
| 6           | SHPACT    | Digital I/O                                          | Shipping mode activate                                                            |  |  |  |  |
| 7           | MODE      | Digital I/O                                          | 0 - automatic<br>1 - Forced PWM                                                   |  |  |  |  |
| 8           | CHG       | Digital OUT                                          | Open-drain LED driver; enabled when battery is<br>charging                        |  |  |  |  |
| 9           | VTERMSET  | Battery charging<br>termination<br>voltage selection | Toggle only when the device is in Power OFF                                       |  |  |  |  |
| 10          | ERR       | Digital OUT                                          | Open-drain LED driver; enabled when error condition<br>in charging                |  |  |  |  |
| 11          | SHPHLD    | Digital I/O                                          | Shipping mode hold                                                                |  |  |  |  |
| 12          | ICHG      | Analog input                                         | Charge current limiting resistor                                                  |  |  |  |  |
| 13          | NC        |                                                      |                                                                                   |  |  |  |  |
| 14          | NTC       | Analog input                                         | NTC resistor                                                                      |  |  |  |  |
| 15          | VBAT      | Power                                                | Battery                                                                           |  |  |  |  |
| 16          | VSYS      | Power                                                | System voltage output; automatically enabled after<br>power-on reset              |  |  |  |  |
| 17          | VBUS      | Power                                                | Input supply                                                                      |  |  |  |  |
| 18          | NC        |                                                      |                                                                                   |  |  |  |  |
| 19          | D-        | Analog input                                         | USB D- data line                                                                  |  |  |  |  |
| 20          | D+        | Analog input                                         | USB D+ data line                                                                  |  |  |  |  |
| 21          | DEC       | Power                                                | System decoupling capacitor                                                       |  |  |  |  |
| 22          | SW        | Power                                                | BUCK regulator output (to inductor)                                               |  |  |  |  |
| 23          | PVSS      | Power                                                | Ground (DC/DC)                                                                    |  |  |  |  |
| 24          | NC        |                                                      |                                                                                   |  |  |  |  |
| Exposed pad | AVSS      | Power                                                |                                                                                   |  |  |  |  |

*Table 24: QFN24 pin assignment*

![](_page_47_Picture_3.jpeg)

#### <span id="page-48-0"></span>8.2 Mechanical specifications

The mechanical specifications for the package shows the dimensions in millimeters.

#### <span id="page-48-1"></span>8.2.1 WLCSP 2.075x2.075 mm package

Dimensions in millimeters for the WLCSP 2.075x2.075 mm package.

![](_page_48_Figure_5.jpeg)

*Figure 37: WLCSP 2.075x2.075 mm package*

|      | A     | A1   | A2    | b     | D     | E     | D2  | E2  | d   | e   | K | L |
|------|-------|------|-------|-------|-------|-------|-----|-----|-----|-----|---|---|
| Min. | 0.406 | 0.14 | 0.266 | 0.195 |       |       |     |     |     |     |   |   |
| Nom. | 0.464 |      | 0.294 |       | 2.075 | 2.075 | 1.6 | 1.6 | 0.4 | 0.4 |   |   |
| Max. | 0.522 | 0.2  | 0.322 | 0.255 |       |       |     |     |     |     |   |   |

*Table 25: WLCSP dimensions in millimeters*

#### <span id="page-48-2"></span>8.2.2 QFN 4.0x4.0 mm package

Dimensions in millimeters for the QFN 4.0x4.0 mm package.

![](_page_48_Picture_11.jpeg)

![](_page_49_Picture_1.jpeg)

*Figure 38: QFN 4.0x4.0 mm package*

|      | A    | A1    | A2   | b    | D   | E   | D2   | E2   | e    | K    | L    |
|------|------|-------|------|------|-----|-----|------|------|------|------|------|
| Min. | 0.80 | 0.00  |      | 0.20 |     |     | 2.60 | 2.60 |      |      | 0.35 |
| Nom. | 0.85 | 0.035 | 0.65 | 0.25 | 4.0 | 4.0 | 2.70 | 2.70 | 0.50 | 0.25 | 0.40 |
| Max. | 0.90 | 0.05  |      | 0.30 |     |     | 2.80 | 2.80 |      |      | 0.45 |

*Table 26: QFN dimensions in millimeters*

#### <span id="page-49-0"></span>8.3 Reference circuitry

Documentation for the different package reference circuits, including Altium Designer files, PCB layout files, and PCB production files can be downloaded from [www.nordicsemi.com](http://www.nordicsemi.com).

The following reference circuits for nPM1100 QFN and WLCSP packages, based on the standard VTERM product, show the schematics and components to support different configurations in a design.

![](_page_49_Picture_8.jpeg)

|                   | Configuration 1                                      | Configuration 2                                      | Configuration 3                         |
|-------------------|------------------------------------------------------|------------------------------------------------------|-----------------------------------------|
| Description       | Minimal configuration Fixed 100 mA <b>VBUS</b> limit | Minimal configuration Fixed 500 mA <b>VBUS</b> limit | Normal configuration USB port detection |
| BUCK              | Not used                                             | Not used                                             | Configured                              |
| Ship mode         | Not used                                             | Not used                                             | Configured                              |
| Battery NTC       | Not used                                             | Not used                                             | Configured                              |
| V <sub>TERM</sub> | VTERMSET = LOW                                       | VTERMSET = LOW                                       | VTERMSET = HIGH                         |
| ISET              | AVSS                                                 | vsys                                                 | AVSS                                    |
| D-                | AVSS                                                 | AVSS                                                 | USB                                     |
| D+                | NC                                                   | NC                                                   | USB                                     |
| ICHG              | 4.7 kΩ                                               | 0 Ω                                                  | 1.5 kΩ                                  |
|                   | 1%                                                   | GND                                                  | GND                                     |
| VOUTB             | -                                                    | -                                                    | 2V1                                     |

Table 27: PCB application configuration

#### <span id="page-50-0"></span>8.3.1 Configuration 1

![](_page_50_Figure_4.jpeg)

Figure 39: WLCSP schematic

![](_page_50_Picture_6.jpeg)

![](_page_51_Figure_1.jpeg)

Figure 40: QFN schematic

| Designator | Value        | Description                                                            | Footprint          |
|------------|--------------|------------------------------------------------------------------------|--------------------|
| C1         | 2.2 μF       | Capacitor, X5R, 25 V, ±20%                                             | 0603               |
| C2         | 22 μF        | Capacitor, X5R, 6.3 V, ± 20%                                           | 0603               |
| С3         | 1.0 μF       | Capacitor, X5R, 10 V, ± 20%                                            | 0201               |
| J1         | Battery pack | Battery pack                                                           | TP_2x1mm_TH        |
| LD1        | L0603R       | LED, SMD, RED                                                          | 0603               |
| LD2        | L0603G       | LED, SMD, GREEN                                                        | 0603               |
| R1         | 10 kΩ        | Resistor, 0.05 W, ±1%                                                  | 0201               |
| R_ICHG     | 4.7 kΩ       | Resistor, 0.05 W, ±1%                                                  | 0201               |
| U1         | nPM1100      | Li-ion/Li-poly USB battery charger with high efficiency buck regulator | WLCSP-25 or<br>QFN |

Table 28: Configuration 1 reference circuitry

#### <span id="page-51-0"></span>8.3.2 Configuration 2

![](_page_51_Picture_6.jpeg)

![](_page_52_Picture_1.jpeg)

Figure 41: WLCSP schematic

![](_page_52_Picture_3.jpeg)

Figure 42: QFN schematic

| Designator | Value        | Description                                                              | Footprint          |
|------------|--------------|--------------------------------------------------------------------------|--------------------|
| C1         | 2.2 μF       | Capacitor, X5R, 25 V, ±20%                                               | 0603               |
| C2         | 22 μF        | Capacitor, X5R, 6.3 V, ±20%                                              | 0603               |
| C3         | 1.0 μF       | Capacitor, X5R, 10 V, ±20%                                               | 0201               |
| J1         | Battery pack | Battery pack                                                             | TP_2x1mm_TH        |
| LD1        | L0603R       | LED, SMD, RED                                                            | 0603               |
| LD2        | L0603G       | LED, SMD, GREEN                                                          | 0603               |
| R1         | 10 kΩ        | Resistor, 0.05 W, ±1%                                                    | 0201               |
| U1         | nPM1100      | Li-ion/Li-poly USB battery charger with a high efficiency buck regulator | WLCSP-25 or<br>QFN |

Table 29: Configuration 2 reference circuitry

#### <span id="page-52-0"></span>8.3.3 Configuration 3

![](_page_52_Picture_8.jpeg)

![](_page_53_Figure_1.jpeg)

Figure 43: WLCSP schematic

![](_page_53_Figure_3.jpeg)

Figure 44: QFN schematic

![](_page_53_Picture_5.jpeg)

| Designator | Value        | Description                                                                 | Footprint          |  |
|------------|--------------|-----------------------------------------------------------------------------|--------------------|--|
| C1         | 2.2 µF       | Capacitor, X5R, 25 V, ±20%                                                  | 0603               |  |
| C2, C3, C4 | 10 µF        | Capacitor, X5R, 6.3 V, ±20%                                                 | 0603               |  |
| C5         | 1.0 µF       | Capacitor, X5R, 10 V, ±20%                                                  | 0201               |  |
| J1         | Battery pack | Battery pack with NTC                                                       | TP_3x1mm_TH        |  |
| L1         | 2.2 µH       | Inductor ±20%                                                               | 0806               |  |
| LD1, LD3   | L0603R       | LED, SMD, RED                                                               | 0603               |  |
| LD2        | L0603G       | LED, SMD, GREEN                                                             | 0603               |  |
| R3         | 1 kΩ         | Resistor, 0.05 W, ±1%                                                       | 0201               |  |
| R_ICHG     | 1.5 kΩ       | Resistor, 0.05 W, ±1%                                                       | 0201               |  |
| U1         | nPM1100      | Li-ion/Li-poly USB battery charger with a high<br>efficiency buck regulator | WLCSP-25 or<br>QFN |  |

*Table 30: Configuration 3 reference circuitry*

#### <span id="page-54-0"></span>8.3.4 PCB guidelines

A well designed PCB is necessary to achieve good performance. A poor layout can lead to loss in performance or functionality.

To ensure functionality, it is essential to follow the schematics and layout references closely.

A PCB with a minimum of two layers, including a ground plane, is recommended for optimal performance.

The DC supply voltage should be decoupled with high performance capacitors as close as possible to the supply pins. See the reference schematic in [Configuration 1](#page-50-0) on page 51 for recommended decoupling capacitor values.

Long power supply lines on the PCB should be avoided. All device grounds, VDD connections, and VDD bypass capacitors must be connected as close as possible to the device.

#### <span id="page-54-1"></span>8.3.5 PCB layout example

The PCB layouts are shown here for WLCSP followed by QFN.

For all available reference layouts, see the Reference Layout section on the Downloads tab for nPM1100 on [www.nordicsemi.com.](http://www.nordicsemi.com)

![](_page_54_Picture_12.jpeg)

*Figure 45: Top silk layer WLCSP*

![](_page_54_Picture_14.jpeg)

![](_page_55_Picture_1.jpeg)

*Figure 46: Top layer WLCSP*

![](_page_55_Picture_3.jpeg)

*Figure 47: Bottom layer WLCSP*

![](_page_55_Picture_5.jpeg)

*Figure 48: Top silk layer QFN*

![](_page_55_Picture_7.jpeg)

![](_page_56_Picture_1.jpeg)

*Figure 49: Top layer QFN*

![](_page_56_Picture_3.jpeg)

*Figure 50: Bottom layer QFN*

**Note:** No components in the bottom layer.

![](_page_56_Picture_6.jpeg)

# <span id="page-57-3"></span><span id="page-57-0"></span>9 Ordering information

This chapter contains information on IC marking, ordering codes, and container sizes.

### <span id="page-57-1"></span>9.1 IC marking

The nPM1100 PMIC package is marked as shown in the following figure.

| N                                                                                                                  | P  | M                                                                            | 1  | 1                                      | 0       | 0 |
|--------------------------------------------------------------------------------------------------------------------|----|------------------------------------------------------------------------------|----|----------------------------------------|---------|---|
| <p< td=""><td>P&gt;</td><td><v< td=""><td>V&gt;</td><td><h></h></td><td><p></p></td><td></td></v<></td></p<>       | P> | <v< td=""><td>V&gt;</td><td><h></h></td><td><p></p></td><td></td></v<>       | V> | <h></h>                                | <p></p> |   |
| <y< td=""><td>Y&gt;</td><td><w< td=""><td>W&gt;</td><td><l< td=""><td>L&gt;</td><td></td></l<></td></w<></td></y<> | Y> | <w< td=""><td>W&gt;</td><td><l< td=""><td>L&gt;</td><td></td></l<></td></w<> | W> | <l< td=""><td>L&gt;</td><td></td></l<> | L>      |   |

*Figure 51: IC marking*

#### <span id="page-57-2"></span>9.2 Box labels

The following figures define the box labels used for the nPM1100 device.

![](_page_57_Figure_8.jpeg)

*Figure 52: Inner box label*

![](_page_57_Picture_10.jpeg)

![](_page_58_Picture_1.jpeg)

Figure 53: Outer box label

#### <span id="page-58-0"></span>9.3 Order code

The following tables define the nPM1100 order codes and definitions.

| n | Р | М | 1 | 1 | 0 | 0 | - | <p< th=""><th>P&gt;</th><th><v< th=""><th>V&gt;</th><th>-</th><th><c< th=""><th>C&gt;</th></c<></th></v<></th></p<> | P> | <v< th=""><th>V&gt;</th><th>-</th><th><c< th=""><th>C&gt;</th></c<></th></v<> | V> | - | <c< th=""><th>C&gt;</th></c<> | C> |
|---|---|---|---|---|---|---|---|---------------------------------------------------------------------------------------------------------------------|----|-------------------------------------------------------------------------------|----|---|-------------------------------|----|
|---|---|---|---|---|---|---|---|---------------------------------------------------------------------------------------------------------------------|----|-------------------------------------------------------------------------------|----|---|-------------------------------|----|

Figure 54: Order code

![](_page_58_Picture_7.jpeg)

| Abbreviation                | Definition and implemented codes                                                                                                                                             |
|-----------------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| N11/nPM11                   | nPM11 series product                                                                                                                                                         |
| 00                          | Part code                                                                                                                                                                    |
| <pp></pp>                   | Package variant code                                                                                                                                                         |
| <vv></vv>                   | Function variant code                                                                                                                                                        |
| <h><p><f></f></p></h>       | Build code<br>H - Hardware version code<br>P - Production configuration code (production site, etc.)<br>F - Firmware version code (only visible on shipping container label) |
| <yy><ww><ll></ll></ww></yy> | Tracking code<br>YY - Year code<br>WW - Assembly week number<br>LL - Wafer lot code                                                                                          |
| <cc></cc>                   | Container code                                                                                                                                                               |
| eX                          | nd Level Interconnect Symbol where value of X is based on J-STD-609<br>2                                                                                                     |

*Table 31: Abbreviations*

#### <span id="page-59-0"></span>9.4 Code ranges and values

The following tables define the nPM1100 code ranges and values.

| <pp></pp> | Package | Size (mm)   | Pin/Ball count | Pitch (mm) |
|-----------|---------|-------------|----------------|------------|
| CA        | WLCSP   | 2.075x2.075 | 25             | 0.4        |
| QD        | QFN     | 4.0x4.0     | 24             | 0.5        |

*Table 32: Package variant codes*

| <vv></vv> | Flash (kB) | VTERM    |  |  |
|-----------|------------|----------|--|--|
| AA        | n/a        | Standard |  |  |
| AB        | n/a        | High     |  |  |

*Table 33: Function variant codes*

| <h></h> | Description                                        |
|---------|----------------------------------------------------|
| [A Z]   | Hardware version/revision identifier (incremental) |

*Table 34: Hardware version codes*

![](_page_59_Picture_11.jpeg)

| <p></p> | Description                                 |
|---------|---------------------------------------------|
| [0 9]   | Production device identifier (incremental)  |
| [A Z]   | Engineering device identifier (incremental) |

*Table 35: Production configuration codes*

| <f></f>    | Description                              |
|------------|------------------------------------------|
| [A N, P Z] | Version of preprogrammed firmware        |
| [0]        | Delivered without preprogrammed firmware |

*Table 36: Production version codes*

| <yy></yy> | Description                   |
|-----------|-------------------------------|
| [16 99]   | Production year: 2016 to 2099 |

*Table 37: Year codes*

| <ww></ww> | Description        |
|-----------|--------------------|
| [1 52]    | Week of production |

*Table 38: Week codes*

| <ll></ll> | Description                     |
|-----------|---------------------------------|
| [AA ZZ]   | Wafer production lot identifier |

*Table 39: Lot codes*

| <cc></cc> | Description |
|-----------|-------------|
| R7        | 7" Reel     |
| R         | 13" Reel    |

*Table 40: Container codes*

#### <span id="page-60-0"></span>9.5 Product options

The following tables define the nPM1100 product options.

![](_page_60_Picture_15.jpeg)

| Order code        | MOQ1     | Comment      |
|-------------------|----------|--------------|
| nPM1100-CAAA-R    | N/A      | Discontinued |
| nPM1100-CAAA-R7   | N/A      | Discontinued |
| nPM1100-CAAA-E-R  | 7000 pcs |              |
| nPM1100-CAAA-E-R7 | 1500 pcs |              |
| nPM1100-CAAB-R    | 4000 pcs |              |
| nPM1100-CAAB-R7   | 1500 pcs |              |
| nPM1100-QDAA-R    | 4000 pcs |              |
| nPM1100-QDAA-R7   | 1500 pcs |              |
| nPM1100-QDAB-R    | 4000 pcs |              |
| nPM1100-QDAB-R7   | 1500 pcs |              |

*Table 41: nPM1100 order codes*

| Order code   | Description                   |  |
|--------------|-------------------------------|--|
| nPM1100-EK   | Standard VTERM evaluation kit |  |
| nPM1100-EKHV | High VTERM evaluation kit     |  |

*Table 42: Development tools order code*

![](_page_61_Picture_6.jpeg)

<span id="page-61-0"></span><sup>1</sup> Minimum Ordering Quantity

# <span id="page-62-0"></span>10 Legal notices

By using this documentation you agree to our terms and conditions of use. Nordic Semiconductor may change these terms and conditions at any time without notice.

#### **Liability disclaimer**

Nordic Semiconductor ASA reserves the right to make changes without further notice to the product to improve reliability, function, or design. Nordic Semiconductor ASA does not assume any liability arising out of the application or use of any product or circuits described herein.

Nordic Semiconductor ASA does not give any representations or warranties, expressed or implied, as to the accuracy or completeness of such information and shall have no liability for the consequences of use of such information. If there are any discrepancies, ambiguities or conflicts in Nordic Semiconductor's documentation, the Product Specification prevails.

Nordic Semiconductor ASA reserves the right to make corrections, enhancements, and other changes to this document without notice.

Customer represents that, with respect to its applications, it has all the necessary expertise to create and implement safeguards that anticipate dangerous consequences of failures, monitor failures and their consequences, and lessen the likelihood of failures that might cause harm, and to take appropriate remedial actions.

Nordic Semiconductor ASA assumes no liability for applications assistance or the design of customers' products. Customers are solely responsible for the design, validation, and testing of its applications as well as for compliance with all legal, regulatory, and safety-related requirements concerning its applications.

Nordic Semiconductor ASA's products are not designed for use in life-critical medical equipment, support appliances, devices, or systems where malfunction of Nordic Semiconductor ASA's products can reasonably be expected to result in personal injury. Customer may not use any Nordic Semiconductor ASA's products in life-critical medical equipment unless adequate design and operating safeguards by customer's authorized officers have been made. Customer agrees that prior to using or distributing any life-critical medical equipment that include Nordic Semiconductor ASA's products, customer will thoroughly test such systems and the functionality of such products as used in such systems.

Customer will fully indemnify Nordic Semiconductor ASA and its representatives against any damages, costs, losses, and/or liabilities arising out of customer's non-compliance with this section.

#### **RoHS and REACH statement**

Refer to [www.nordicsemi.com](https://www.nordicsemi.com) for complete hazardous substance reports, material composition reports, and latest version of Nordic's RoHS and REACH statements.

#### **Trademarks**

All trademarks, service marks, trade names, product names, and logos appearing in this documentation are the property of their respective owners.

#### **Copyright notice**

© 2025 Nordic Semiconductor ASA. All rights are reserved. Reproduction in whole or in part is prohibited without the prior written permission of the copyright holder.

![](_page_62_Picture_16.jpeg)

![](_page_63_Picture_1.jpeg)

![](_page_63_Picture_2.jpeg)