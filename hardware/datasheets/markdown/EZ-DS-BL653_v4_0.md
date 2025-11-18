![](_page_0_Picture_0.jpeg)

# **Datasheet**

**BL653** 

Version 4.0

![](_page_1_Picture_1.jpeg)

# **Revision History**

|                                                    | /            |                                                                                                                                                                       |                               |                |
|----------------------------------------------------|--------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------|-------------------------------|----------------|
| Version                                            | Date         | Notes                                                                                                                                                                 | Contributor(s)                | Approver       |
| 1.0                                                | 04 Jun 2020  | Initial version                                                                                                                                                       | Raj Khatri                    | Jonathan Kaye  |
| 1.1                                                | 04 Aug 2020  | Added dielectric constant of prepreg and solder mask in section 6.4 Figure 12 BL653 development board PCB stack-up and L1 to L2 50-Ohms Grounded CPW RF trace design. | Raj Khatri                    | Jonathan Kaye  |
| 1.2                                                | 13 Oct 2020  | Added Bluetooth Qualification process                                                                                                                                 | Raj Khatri                    | Jonathan Kaye  |
| 1.2                                                | 13 001 2020  | Added Blactooth addinication process                                                                                                                                  | Najikilatii                   | Johathamaye    |
| 2.0 14 Dec 2020 Updated all regulatory information |              | Updated all regulatory information                                                                                                                                    | Maggie Teng<br>Ryan Urness    | Jonathan Kaye  |
| 2.1                                                | 22 Jan 2021  | Transferred all regulatory information to a separate document                                                                                                         | Maggie Teng                   | Jonathan Kaye  |
| 2.2                                                | 18 Feb 2021  | Fixed equation in 5.5.2 NFC Antenna Coil Tuning Capacitors                                                                                                            | Raj Khatri                    | Dave Drogowsk  |
| 2.3                                                | 26 July 2021 | Added board image to polar plot in section 6.1 453-00039 On-board PCB Antenna Characteristics                                                                         | Raj Khatri                    | Dave Drogowski |
| 2.4                                                | 22 Dec 2021  | Updated Mechanical Details                                                                                                                                            | Dave Drogowski                | Andrew Chen    |
| 2.5 10 Aug 2022                                    |              | Removed PCB printed antenna from External Antenna<br>Integration with 453-00041                                                                                       | Raj Khatri                    | Jonathan Kaye  |
| 2.6                                                | 16 Aug 2023  | Updated Tape and Reel Package Information section                                                                                                                     | Robert Gosewehr               | Dave Drogowsk  |
| 2.7                                                | 29 Sept 2023 | Corrected Table 1: Specifications Summary SPI max speed from 4Mbps to 8Mbps.                                                                                          | Raj Khatri, Dave<br>Drogowski | Jonathan Kaye  |
| 2.8                                                | 11 Dec 2023  | Updated On-board PCB Antenna Characteristics                                                                                                                          | Adam Engelbrecht              | Dave Drogowski |
| 2.9                                                | 22 Apr 2024  | Updated to BT 5.4. Updated QDID in Bluetooth  Qualification process                                                                                                   | Dave Drogowski                | Jonathan Kaye  |
| 3.0                                                | 30 Oct 2024  | Updates to Bluetooth Qualification process                                                                                                                            | Dave Drogowski                | Jonathan Kaye  |
| 4.0                                                | 23 Jan 2025  | Updated to Ezurio branding.                                                                                                                                           | Sue White                     | Dave Drogowsk  |
|                                                    |              |                                                                                                                                                                       |                               |                |

![](_page_2_Picture_1.jpeg)

# Contents

| 1   | Over   | riew and Key Features                             | 6                       |
|-----|--------|---------------------------------------------------|-------------------------|
|     | 1.1    | Features and Benefits                             | 6                       |
|     | 1.2    | Application Areas                                 | 6                       |
| 2   | Spec   | fication                                          | 7                       |
|     | 2.1    | Specification Summary                             | 7                       |
| 3   | Hard   | vare Specifications                               | 11                      |
|     | 3.1    | Block Diagram and Pin-out                         | 11                      |
|     | 3.2    | Pin Definitions                                   | 12                      |
|     | 3.3    | Electrical Specifications                         | 17                      |
|     | 3.3.1  | Absolute Maximum Ratings                          | 17                      |
|     | 3.3.2  | Recommended Operating Parameters                  | 18                      |
|     | 3.4    | Programmability                                   | 20                      |
|     | 3.4.1  | BL653 Default Firmware                            | 20                      |
|     | 3.4.2  | BL653 Special Function Pins in <i>smart</i> BASIC | 20                      |
| 4   | Powe   | r Consumption                                     | 21                      |
|     | 4.1    | Power Consumption                                 | 21                      |
|     | 4.2    | Peripheral Block Current Consumption              | 22                      |
| 5   | Funct  | ional Description                                 | 23                      |
|     | 5.1    | Power Management                                  | 23                      |
|     | 5.2    | BL653 Power Supply Options                        | 24                      |
|     | 5.3    | Clocks and Timers                                 | 25                      |
|     | 5.3.1  | Clocks                                            | 25                      |
|     | 5.3.2  | Timers                                            | 25                      |
|     | 5.4    | Radio Frequency (RF)                              | 26                      |
|     | 5.5    | NFC                                               | 26                      |
|     | 5.5.1  | Use Cases                                         | 26                      |
|     | 5.5.2  | NFC Antenna Coil Tuning Capacitors                | 26                      |
|     | 5.6    | UART Interface                                    | 27                      |
|     | 5.7    | USB Interface                                     | 28                      |
|     | 5.8    | SPI Bus                                           | 28                      |
|     | 5.9    | I2C Interface                                     | 29                      |
|     | 5.10   | General Purpose I/O, ADC, PWM, and FREQ           | 29                      |
|     | 5.10.1 | GPIO                                              | 29                      |
|     | 5.10.2 | ADC                                               | 29                      |
|     | 5.10.3 | PWM Signal Output on Up to 16 SIO Pins            | 29                      |
|     | 5.10.4 |                                                   |                         |
|     | 5.11   | nreset Pin                                        |                         |
|     | 5.12   | Two-Wire Interface SWD (JTAG)                     |                         |
| htt |        | v.ezurio.com/ 3                                   | © Copyright 2025 Ezurio |

![](_page_3_Picture_1.jpeg)

|    | 5.13   | BL653 Wakeup                                                                                          | 31 |
|----|--------|-------------------------------------------------------------------------------------------------------|----|
|    | 5.13.1 | Waking Up BL653 From Host                                                                             | 31 |
|    | 5.14   | Low Power Modes                                                                                       | 31 |
|    | 5.15   | Temperature Sensor                                                                                    | 31 |
|    | 5.16   | Security/Privacy                                                                                      | 32 |
|    | 5.16.1 | Random Number Generator                                                                               | 32 |
|    | 5.16.2 | AES Encryption/Decryption                                                                             | 32 |
|    | 5.16.3 | Readback Protection                                                                                   | 32 |
|    | 5.16.4 | Elliptic Curve Cryptography                                                                           | 32 |
| 6  | Optio  | nal External 32.768 kHz Crystal                                                                       | 32 |
|    | 6.1    | 453-00039 On-board PCB Antenna Characteristics                                                        | 33 |
|    | 6.1.1  | 2402MHz Radiated Performance                                                                          | 34 |
|    | 6.1.2  | 2440MHz Radiated Performance                                                                          | 35 |
|    | 6.1.3  | 2480MHz Radiated Performance                                                                          | 36 |
|    | 6.1.4  | 451-00001 Return Loss Measurement                                                                     | 37 |
| 7  | Hardy  | vare Integration Suggestions                                                                          | 37 |
|    | 7.1    | Circuit                                                                                               |    |
|    | 7.2    | PCB Layout on Host PCB - General                                                                      | 39 |
|    | 7.3    | PCB Layout on Host PCB for the 453-00039                                                              | 39 |
|    | 1.1.1  | Antenna Keep-Out on Host PCB                                                                          | 39 |
|    | 7.3.1  | Antenna Keep-Out and Proximity to Metal or Plastic                                                    | 41 |
|    | 7.4    | 50-Ohms RF Trace and RF Match Series 2nH RF Inductor on Host PCB for BL653 RF Pad Variant (453-00041) | 41 |
|    | 7.5    | External Antenna Integration with 453-00041                                                           | 44 |
| 8  | Mech   | anical Details                                                                                        |    |
|    | 8.1    | BL653 Mechanical Details                                                                              | 44 |
|    | 8.2    | Host PCB Land Pattern and Antenna Keep-out for the 453-00039                                          |    |
| 9  | Applio | cation Note for Surface Mount Modules                                                                 |    |
|    | 9.1    | Introduction                                                                                          |    |
|    | 9.2    | Shipping                                                                                              |    |
|    | 9.2.1  | Tape and Reel Package Information                                                                     |    |
|    | 9.2.2  | Carton Contents                                                                                       |    |
|    | 9.2.3  | Module Orientation in Cavity                                                                          |    |
|    | 9.2.4  | Labeling                                                                                              |    |
|    | 9.3    | Reflow Parameters                                                                                     |    |
| 10 |        | atory                                                                                                 |    |
| 11 | 0      | ing Information                                                                                       |    |
| 12 |        | ooth Qualification process                                                                            |    |
|    | 12.1   | Overview                                                                                              |    |
|    | 12.2   | Scope                                                                                                 |    |

![](_page_4_Picture_1.jpeg)

| 12 | 2.3                      | Qualification Steps When Referencing a single existing design, (unmodified) – Option 1 in the QPRDv3 | . 55 |
|----|--------------------------|------------------------------------------------------------------------------------------------------|------|
| 12 | 2.4                      | Example Designs for reference                                                                        | .56  |
| 12 | 5                        | Qualify More Products                                                                                | .56  |
| 13 | Reliab                   | pility Tests                                                                                         | . 57 |
| 14 | 4 Additional Information |                                                                                                      |      |

![](_page_5_Picture_1.jpeg)

# 1 Overview and Key Features

Every BL653 Series module is designed to simplify OEMs enablement of Bluetooth Low Energy (BLE) v5.4 and Thread (802.15.4) to small, portable, power-conscious devices. The BL653 provides engineers with considerable design flexibility in both hardware and software programming capabilities. Based on the world-leading Nordic Semiconductor nRF52833 chipset, the BL653 modules provide ultra-low power consumption with outstanding wireless range via +8 dBm of transmit power and the Long Range (CODED PHY) Bluetooth 5 feature. The BL653 is programmable via Ezurio's *smart*BASIC language, AT command set, Zephyr RTOS or Nordic's software development kit (SDK).

![](_page_5_Picture_4.jpeg)

![](_page_5_Picture_5.jpeg)

smartBASIC is an event-driven programming language that is highly optimized for memory-constrained systems such as embedded modules. It was designed to make BLE development quicker and simpler, vastly cutting down time to market.

The Nordic SDK, on the other hand, offers developers source code (in C) and precompiled libraries containing BLE and ANT+ device profiles, wireless communication, as well as application examples.

**Note:** BL653 hardware provides all functionality of the nRF52833 chipset used in the module design. This is a hardware datasheet only – it does not cover the software aspects of the BL653.

For customers using *smart*BASIC, refer to the *smart*BASIC extensions guide (available from the <u>BL653</u> product page of the Ezurio website. For customers using the Nordic SDK, refer to <u>www.nordicsemi.com</u>.

#### 1.1 Features and Benefits

- Bluetooth v5.4 Single mode
- NFC
- 802.15.4 (Thread) radio support
- External or internal antennas
- Multiple programming options
  - smartBASIC
  - AT command set
  - Nordic SDK in C
  - Zephyr RTOS
- Compact footprint
- Programmable Tx power +8 dBm to -20 dBm, -40 dBm
- Rx sensitivity -96 dBm (1 Mbps), -103 dBm (125 kbps)
- Ultra-low power consumption
- Tx 4.9 mA peak (at 0 dBm, DCDC on) (See Note 1 in the *Power Consumption* section)
- Rx: 4.6 mA peak (DCDC on)
  (See Note 1 in the Power Consumption section)

- Standby Doze 2.6 uA typical
- Deep Sleep 0.6 uA (See Note 4 in the Power Consumption section)
- UART, GPIO, ADC, PWM, FREQ output, timers, I2C, SPI, I2S. PDM, and USB interfaces
- Fast time-to-market
- FCC, EU, ISED, RCM, Japan, and KC certified
- Full Bluetooth Declaration ID
- Other regulatory certifications on request
- No external components required
- Extended Industrial temperature range (-40° C to +105° C)

## 1.2 Application Areas

- Medical devices
- IoT Sensors
- Appcessories

- Fitness sensors
- Location awareness
- Home automation

![](_page_6_Picture_1.jpeg)

# 2 Specification

### 2.1 Specification Summary

Table 1: Specifications Summary

| <i>able 1: Specifications Summary</i> Categories/Feature | Implement                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            | ation                                                                                                 |                                                   |  |
|----------------------------------------------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|-------------------------------------------------------------------------------------------------------|---------------------------------------------------|--|
| Wireless Specification                                   |                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      |                                                                                                       |                                                   |  |
| Bluetooth®                                               | <ul> <li>BT 5.4 - Single mode</li> <li>Angle-of-arrival (AoA) and angle-of-departure (AoD) - BT 5.1</li> <li>4x Range (CODED PHY support) - BT 5.0</li> <li>2x Speed (2M PHY support) - BT 5.0</li> <li>LE Advertising Extensions - BT 5.0</li> <li>Concurrent master, slave</li> <li>BLE Mesh capabilities</li> <li>Diffie-Hellman based pairing (LE Secure Connections) - BT 4.2</li> <li>Data Packet Length Extension - BT 4.2</li> <li>Link Layer Privacy (LE Privacy 1.2) - BT 4.2</li> <li>LE Dual Mode Topology - BT 4.1</li> <li>LE Ping - BT 4.1</li> </ul> |                                                                                                       |                                                   |  |
| Frequency                                                | 2.402 - 2.480 GHz                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    |                                                                                                       |                                                   |  |
| Raw Data Rates                                           | 2 Mbps BLE<br>125 kbps BL                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            | Mbps BLE (over-the-air) Mbps BLE (over-the-air) 25 kbps BLE (over-the-air) 00 kbps BLE (over-the-air) |                                                   |  |
| Maximum Transmit Power Setting                           | +8 dBm                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               | Conducted 453-00039 (Integrated antenna)                                                              |                                                   |  |
|                                                          | +8 dBm                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               | Conducted 453-                                                                                        | -00041 (Trace pin-connecting to External antenna) |  |
| Minimum Transmit Power Setting                           | -40 dBm, -20 dBm (in 4 dB steps) -16 dBm, -12 dBm, - 8 dBm, - 4 dBm, 0 dBm, 2 dBm, 4 dBm, 5 dBm, 6 dBm, 7 dBm,                                                                                                                                                                                                                                                                                                                                                                                                                                                       |                                                                                                       | ' '                                               |  |
| Receive Sensitivity                                      | BLE 1 Mbps                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           | (BER=1E-3)                                                                                            | -96 dBm typical                                   |  |
| (≤ 37-byte packet)                                       | BLE 2 Mbps                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           |                                                                                                       | -92 dBm typical                                   |  |
|                                                          | BLE 125 kbps                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         |                                                                                                       | -103 dBm typical                                  |  |
|                                                          | BLE 500 kbps                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         |                                                                                                       | -99 dBm typical                                   |  |
| Link Budget (conducted)                                  | 104 dB                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               |                                                                                                       | @ BLE1Mbps                                        |  |
|                                                          | 111 dB                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               |                                                                                                       | @ BLE 125 kbps                                    |  |
| Link Budget (conducted)                                  | BLE 500 kbp                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |                                                                                                       | -99 dBm typical  @ BLE 1 Mbps                     |  |

![](_page_7_Picture_1.jpeg)

| Categories/Feature              | Implementation                                                                                                                                                                                                 |
|---------------------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| NFC                             |                                                                                                                                                                                                                |
| NFC-A Listen mode compliant     | Based on NFC forum specification  13.56 MHz  Date rate 106 kbps  NFC Type 2 and Type 4 emulation  Modes of Operation:  Disable                                                                                 |
|                                 | <ul> <li>Sense</li> <li>Activated</li> <li>Use Cases:</li> <li>Touch-to-Pair with NFC</li> <li>NFC enabled Out-of-Band Pairing</li> </ul>                                                                      |
| System Wake-On-Field function   | Proximity Detection                                                                                                                                                                                            |
| Host Interfaces and Peripherals |                                                                                                                                                                                                                |
| Total                           | 42 x multifunction I/O lines                                                                                                                                                                                   |
| UART                            | 2 UARTs Tx, Rx, CTS, RTS DCD, RI, DTR, DSR (See Note 1 in the Module Specification Notes) Default 115200, n, 8, 1 From 1,200 bps to 1 Mbps                                                                     |
| USB                             | USB 2.0 FS (Full Speed, 12 Mbps). CDC driver/Virtual UART (baud rate TBD) Other USB drivers available via Nordic SDK                                                                                           |
| GPIO                            | Up to 42, with configurable:  I/O direction  O/P drive strength (standard 0.5 mA or high 3mA/5 mA)  Pull-up /pull-down  Input buffer disconnect                                                                |
| ADC                             | Eight 8/10/12-bit channels 0.6 V internal reference Configurable 4, 2, 1, 1/2, 1/3, 1/4, 1/5 1/6 (default) pre-scaling Configurable acquisition time 3uS, 5uS, 10uS (default), 15uS, 20uS, 40uS. One-shot mode |
| PWM Output                      | PWM outputs on 16 GPIO output pins  • PWM output duty cycle: 0%-100%  • PWM output frequency: Up to 500 kHz                                                                                                    |
| FREQ Output                     | FREQ outputs on 16 GPIO output pins • FREQ output frequency: 0 MHz-4 MHz (50% duty cycle)                                                                                                                      |
| 12C                             | Two I2C interface (up to 400 kbps) – See Note 2 in the Module Specification Notes                                                                                                                              |
| SPI                             | Four SPI Master Slave interface (up to 8 Mbps)                                                                                                                                                                 |
| Temperature Sensor              | <ul> <li>One temperature sensor</li> <li>Temperature range equal to the operating temperature range</li> <li>Resolution 0.25 degrees</li> </ul>                                                                |
| RSSI Detector                   | <ul> <li>One RF received signal strength indicator</li> <li>±2 dB accuracy (valid over -90 to -20 dBm)</li> </ul>                                                                                              |

![](_page_8_Picture_1.jpeg)

| Categories/Feature                                                   | Implementation One of Proceeds the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the control of the c |  |  |  |  |  |  |
|----------------------------------------------------------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|--|--|--|--|--|--|
|                                                                      | One dB resolution                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              |  |  |  |  |  |  |
| I2S                                                                  | One inter-IC sound interface                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   |  |  |  |  |  |  |
| PDM                                                                  | One pulse density modulation interface                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         |  |  |  |  |  |  |
| Optional (External to the BL653 modul                                | e)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             |  |  |  |  |  |  |
| External 32.768 kHz crystal                                          | For customer use, connect +/-20 ppm accuracy crystal for more accurate protocol timing.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        |  |  |  |  |  |  |
| Profiles                                                             |                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                |  |  |  |  |  |  |
| Supported Services                                                   | Central mode                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   |  |  |  |  |  |  |
|                                                                      | Peripheral mode                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                |  |  |  |  |  |  |
|                                                                      | Mesh (with custom models)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      |  |  |  |  |  |  |
|                                                                      | Custom and adopted profiles                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    |  |  |  |  |  |  |
| Programmability                                                      |                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                |  |  |  |  |  |  |
| <i>smart</i> BASIC                                                   | FW upgrade via JTAG or UART                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    |  |  |  |  |  |  |
|                                                                      | <ul> <li>Application download via UART or Via Over-the-Air (if SIO_02 pin is pulled high externally)</li> </ul>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                |  |  |  |  |  |  |
| Nordic SDK                                                           | Via JTAG                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       |  |  |  |  |  |  |
| Operating Modes                                                      |                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                |  |  |  |  |  |  |
| <i>smart</i> BASIC                                                   | Self-contained Run mode                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        |  |  |  |  |  |  |
|                                                                      | Selected by nAutoRun pin status: LOW (0V).                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     |  |  |  |  |  |  |
|                                                                      | Then runs \$autorun\$ (smartBASIC application script) if it exists.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            |  |  |  |  |  |  |
|                                                                      | Interactive/Development mode                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   |  |  |  |  |  |  |
|                                                                      | HIGH (VDD).                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    |  |  |  |  |  |  |
|                                                                      | Then runs via at+run (and file name of smartBASIC application script).                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         |  |  |  |  |  |  |
| AT Command set                                                       | Comprehensive Hayes-style AT command set                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       |  |  |  |  |  |  |
| Nordic SDK                                                           | As per Nordic SDK                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              |  |  |  |  |  |  |
| Zephyr RTOS                                                          | As per www.zephyrproject.org                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   |  |  |  |  |  |  |
| Supply Voltage                                                       |                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                |  |  |  |  |  |  |
| Supply (VDD or VDD_HV) options                                       | <ul> <li>Normal voltage mode VDD 1.7- 3.6 V - Internal DCDC converter or LDO<br/>(See Note 3 in the Module Specification Notes)</li> </ul>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     |  |  |  |  |  |  |
|                                                                      | OR  High voltage mode V/DD, HV 2 5V-5 5V Internal I DO                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         |  |  |  |  |  |  |
|                                                                      | <ul> <li>High voltage mode VDD_HV 2.5V-5.5V Internal LDO     (See Note 3 and Note 4 in the Module Specification Notes)</li> </ul>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              |  |  |  |  |  |  |
| Power Consumption                                                    |                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                |  |  |  |  |  |  |
| Active Modes Peak Current (for maximum Tx power +8 dBm) - Radio only | 14.2 mA peak Tx (with DCDC)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    |  |  |  |  |  |  |
| Active Modes Peak Current (for Tx power -40 dBm) – Radio only        | 2.3 mA peak Tx (with DCDC)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     |  |  |  |  |  |  |
| Active Modes Average Current                                         | Depends on many factors, see <i>Power Consumption section</i>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  |  |  |  |  |  |  |
| Ultra-low Power Modes                                                | Standby Doze 2.6 uA typical                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    |  |  |  |  |  |  |
|                                                                      | Deep Sleep 0.6 uA                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              |  |  |  |  |  |  |
| Internal                                                             | Printed PCB monopole antenna – on-board 453-00039 variant                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      |  |  |  |  |  |  |
| External                                                             | <ul> <li>Dipole antenna (with IPEX connector)</li> <li>Dipole PCB antenna (with IPEX connector)</li> </ul>                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     |  |  |  |  |  |  |
|                                                                      |                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                |  |  |  |  |  |  |

![](_page_9_Picture_1.jpeg)

| Catagorias/Footura     | Implementation                                                                              |  |  |  |  |  |
|------------------------|---------------------------------------------------------------------------------------------|--|--|--|--|--|
| Categories/Feature     | Implementation  • Connection via RF connector (IPEX MH4) – 453-00041 variant (RF trace pin) |  |  |  |  |  |
|                        | See the Antenna Information sections for FCC, ISED, MIC, RCM, KC, and EU.                   |  |  |  |  |  |
| Physical               |                                                                                             |  |  |  |  |  |
| Dimensions             | 15.0 mm x 10 mm x 2.2 mm                                                                    |  |  |  |  |  |
|                        | Pad Pitch – 0.8 mm                                                                          |  |  |  |  |  |
|                        | Pad Type - Two rows of pads                                                                 |  |  |  |  |  |
| Weight                 | <1 gram                                                                                     |  |  |  |  |  |
|                        |                                                                                             |  |  |  |  |  |
| Operating              | -40 °C to +105 °C                                                                           |  |  |  |  |  |
| Storage                | -40 °C to +85 °C                                                                            |  |  |  |  |  |
| Miscellaneous          |                                                                                             |  |  |  |  |  |
| Lead Free              | Lead-free and RoHS-compliant                                                                |  |  |  |  |  |
| Warranty               | One-year warranty                                                                           |  |  |  |  |  |
| Development Tools      |                                                                                             |  |  |  |  |  |
| Development Kit        | Development kit per module SKU (453-00039-K1 and 453-00041-K1) and free software tools      |  |  |  |  |  |
| Approvals              |                                                                                             |  |  |  |  |  |
| Bluetooth®             | Full Bluetooth SIG Declaration ID                                                           |  |  |  |  |  |
| FCC/ISED/EU/MIC/RCM/KC | All BL653 Series                                                                            |  |  |  |  |  |
|                        |                                                                                             |  |  |  |  |  |

#### Module Specification Notes:

| Note 1 | DSR, DTR, RI, and DCD can be implemented in the <i>smart</i> BASIC application or through the Nordic SDK.                      |
|--------|--------------------------------------------------------------------------------------------------------------------------------|
| Note 2 | With I2C interface selected, pull-up resistors on I2C SDA and I2C SCL <i>must</i> be connected externally as per I2C standard. |
| Note 3 | Use of the internal DCDC (REG1) convertor or LDO (REG1) is decided by the underlying BLE stack.                                |
| Note 4 | In High Voltage mode (VDD_HV), no external current draw (from VDD pin) is allowed (limitation of nRF52833 chipset).            |

![](_page_10_Picture_1.jpeg)

# 3 Hardware Specifications

### 3.1 Block Diagram and Pin-out

![](_page_10_Figure_4.jpeg)

Figure 1: BL653 block diagram

![](_page_10_Figure_6.jpeg)

Figure 2: Functional HW and SW block diagram for BL653 series BLE module

![](_page_11_Picture_1.jpeg)

![](_page_11_Figure_2.jpeg)

Figure 3: BL653 module pin-out (top view). Outer row pads (long red line) and inner row pads (short red line) shown

#### 3.2 Pin Definitions

Table 2: Pin definitions

| Pin# | Pin Name            | Default<br>Function | Alternate<br>Function | In/ Out | Pull-<br>Up/<br>Down | nRF52833<br>QFN Pin | nRF52833 QFN<br>Name | Comment                                                        |
|------|---------------------|---------------------|-----------------------|---------|----------------------|---------------------|----------------------|----------------------------------------------------------------|
| 0    | GND                 | -                   | -                     | -       | -                    | -                   | -                    | -                                                              |
| 1    | SWDIO               | SWDIO               | -                     | ln      | Pull-up              | AC24                | SWDIO                | -                                                              |
| 2    | SIO_36              | SIO_36              | -                     | In      | Pull-up              | U24                 | P1.04                | -                                                              |
| 3    | SWDCLK              | SWDCLK              | -                     | ln      | Pull-<br>down        | AA24                | SWDCLK               | -                                                              |
| 4    | SIO_34              | SIO_34              | -                     | -       | Pull-up              | W24                 | P1.02                | -                                                              |
| 5    | SIO_35/<br>nAutoRUN | nAutoRUN            | SIO_35                | ln      | Pull-<br>down        | Y23                 | P1.01                | Ezurio Devkit:<br>FTDI USB_DTR via<br>jumper on J12<br>pin 1-2 |
| 6    | NC                  | -                   | -                     | -       | -                    | -                   | -                    | -                                                              |
| 7    | SIO_32              | SIO_32              | -                     | In      | Pull-up              | AD22                | P1.00/<br>TRACEDATA0 | -                                                              |
| 8    | SIO_25              | SIO_25              | -                     | ln      | Pull-up              | AD18                | P0.22                | Ezurio Devkit:<br>BUTTON4                                      |

![](_page_12_Picture_1.jpeg)

| D: # | P: N                | Default  | Alternate |         | Pull-       | nRF52833 | nRF52833 QFN         |                                                                                                                                                        |
|------|---------------------|----------|-----------|---------|-------------|----------|----------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------|
| Pin# | Pin Name            | Function | Function  | In/ Out | Up/<br>Down | QFN Pin  | Name                 | Comment                                                                                                                                                |
| 9    | NC                  | -        | -         | -       | -           | -        | -                    | -                                                                                                                                                      |
| 10   | SIO_24              | SIO_24   |           | ln      | Pull-up     | AD20     | P0.24                | <b>Ezurio Devkit</b> :<br>BUTTON3                                                                                                                      |
| 11   | NC                  | -        | -         | -       | -           | -        | -                    | -                                                                                                                                                      |
| 12   | SIO_21              | SIO_21   | -         | In      | Pull-up     | AC17     | P0.21                |                                                                                                                                                        |
| 13   | SIO_20              | SIO_20   | -         | In      | Pull-up     | AD16     | P0.20                | -                                                                                                                                                      |
| 14   | NC                  | -        | -         | -       | -           | -        | -                    |                                                                                                                                                        |
| 15   | D+                  | D+       | -         | In      |             | AD6      | D+                   | -                                                                                                                                                      |
| 16   | SIO_17              | SIO_17   | -         | In      | Pull-up     | AD12     | P0.17                | -                                                                                                                                                      |
| 17   | D-                  | D-       | -         | In      |             | AD4      | D-                   | -                                                                                                                                                      |
| 18   | SIO_15              | SIO_15   | =         | ln      | Pull-up     | AD10     | P0.15                | Ezurio Devkit: LED3                                                                                                                                    |
| 19   | nRESET              | nRESET   | SIO_18    | ln      | Pull-up     | AC13     | P0.18/<br>nRESET     | System Reset (Active<br>Low)                                                                                                                           |
| 20   | SIO_13              | SIO_13   | -         | In      | Pull-up     | AD8      | P0.13                | Ezurio Devkit: LED1                                                                                                                                    |
| 21   | SIO_16              | SIO_16   | -         | ln      | Pull-up     | AC11     | P0.16                | Ezurio Devkit: LED4                                                                                                                                    |
| 22   | SIO_14              | SIO_14   | -         | In      | Pull-up     | AC9      | P0.14                | Ezurio Devkit: LED2                                                                                                                                    |
| 23   | GND                 | -        | -         | -       | -           | -        | -                    | -                                                                                                                                                      |
| 24   | VBUS                | -        | -         | -       | -           | AD2      | VBUS                 | 4.35V - 5.5V                                                                                                                                           |
| 25   | VDD_HV              | -        | -         | -       | -           | Y2       | VDDH                 | 2.5V to 5.5V                                                                                                                                           |
| 26   | GND                 | _        | -         | -       | _           | -        | _                    | -                                                                                                                                                      |
| 27   | SIO_11              | SIO_11   | -         | ln      | Pull-up     | T2       | P0.11/<br>TRACEDATA2 | Ezurio Devkit:<br>BUTTON1                                                                                                                              |
| 28   | SIO_12              | SIO_12   | -         | ln      | Pull-up     | U1       | P0.12/<br>TRACEDATA1 | -                                                                                                                                                      |
| 29   | SIO_08/<br>UART_RX  | SIO_08   | UART_RX   | ln      | Pull-up     | N1       | P0.08                | UARTCLOSE() selects DIO functionality UARTOPEN() selects UART COMMS behavior                                                                           |
| 30   | SIO_41/<br>SPI_CLK  | SIO_41   | SPI_CLK   | In      | Pull-up     | R1       | P1.09/<br>TRACEDATA3 | Ezurio Devkit:  SPI EEPROM.  SPI_Eeprom_CLK, Output:  SPIOPEN() in smartBASIC selects  SPI function, MOSI and CLK are outputs when in SPI master mode. |
| 31   | VDD                 | -        | -         | -       | -           | AD14     | VDD                  | 1.7V to 3.6V                                                                                                                                           |
| 32   | SIO_40/<br>SPI_MOSI | SIO_40   | SPI_MOSI  | ln      | Pull-up     | P2       | P1.08                | Ezurio Devkit:  SPI EEPROM.  SPI_Eeprom_MOSI,  Output  SPIOPEN() in  smartBASIC selects                                                                |

![](_page_13_Picture_1.jpeg)

| Pin# | Pin Name                     | Default<br>Function | Alternate<br>Function | In/Out | Pull-<br>Up <i>l</i><br>Down | nRF52833<br>QFN Pin | nRF52833 QFN<br>Name | Comment                                                                                                                                          |
|------|------------------------------|---------------------|-----------------------|--------|------------------------------|---------------------|----------------------|--------------------------------------------------------------------------------------------------------------------------------------------------|
|      |                              |                     |                       |        |                              |                     |                      | SPI function, MOSI and<br>CLK are outputs in SPI<br>master.                                                                                      |
| 33   | GND                          | -                   | -                     | -      | -                            | -                   | -                    | -                                                                                                                                                |
| 34   | SIO_04/<br>AIN2/<br>SPI_MISO | SIO_04              | AIN2/<br>SPI_MISO     | In     | Pull-up                      | J1                  | P0.04/AIN2           | Ezurio Devkit: SPI EEPROM. SPI_Eeprom_MISO, Input SPIOPEN() in smartBASIC selects SPI function; MOSI and CLK are outputs when in SPI master mode |
| 35   | SIO_06/<br>UART_TX           | SIO_06              | UART_TX               | Out    | Set<br>High in<br>FW         | L1                  | P0.06                | UARTCLOSE() selects DIO functionality UARTOPEN() selects UART COMMS behaviour                                                                    |
| 36   | SIO_26/<br>I2C_SDA           | SIO_26              | I2C_SDA               | ln     | Pull-up                      | G1                  | P0.26                | <b>Ezurio Devkit:</b><br>I2C RTC chip<br>I2C data line                                                                                           |
| 37   | SIO_07/<br>UART_CTS          | SIO_07              | UART_CTS              | IN     | Pull-<br>down                | M2                  | P0.07/<br>TRACECLK   | UARTCLOSE() selects DIO functionality UARTOPEN() selects UART COMMS behaviour                                                                    |
| 38   | SIO_27/<br>I2C_SCL           | SIO_27              | I2C_SCL               | ln     | Pull-up                      | H2                  | P0.27                | Ezurio Devkit:<br>I2C RTC chip<br>I2C clock line                                                                                                 |
| 39   | SIO_05/<br>UART_RTS/<br>AIN3 | SIO_05              | UART_RTS/<br>AIN3     | Out    | Set<br>Low in<br>FW          | K2                  | P0.05/AIN3           | UARTCLOSE() selects DIO functionality UARTOPEN() selects UART COMMS behavior                                                                     |
| 40   | GND                          | -                   | -                     | -      | -                            | -                   | -                    | -                                                                                                                                                |
| 41   | SIO_01/<br>XL2               | SIO_01              | XL2                   | ln     | Pull-up                      | F2                  | P0.01/XL2            | Ezurio Devkit: Optional 32.768kHz crystal pad XL2 and associated load capacitor                                                                  |
| 42   | SIO_00/<br>XL1               | SIO_00              | XL1                   | In     | Pull-up                      | D2                  | P0.00/XL1            | Ezurio Devkit:<br>Optional 32.768kHz<br>crystal pad XL1 and<br>associated load<br>capacitor                                                      |
| 43   | GND                          | -                   | -                     | -      | -                            | -                   | -                    | -                                                                                                                                                |
| 44   | SIO_31/<br>AIN7              | SIO_31              | AIN7                  | ln     | Pull-up                      | A8                  | P0.31/AIN7           | -                                                                                                                                                |

![](_page_14_Picture_1.jpeg)

| Pin# | Pin Name        | Default<br>Function | Alternate<br>Function | In/Out | Pull-<br>Up/<br>Down | nRF52833<br>QFN Pin | nRF52833 QFN<br>Name | Comment                                                                |
|------|-----------------|---------------------|-----------------------|--------|----------------------|---------------------|----------------------|------------------------------------------------------------------------|
| 45   | SIO_30/<br>AIN6 | SIO_30              | AIN6                  | ln     | Pull-up              | В9                  | P0.30/AIN6           | -                                                                      |
| 46   | SIO_28/<br>AIN4 | SIO_28              | AIN4                  | In     | Pull-up              | B11                 | P0.28/AIN4           | -                                                                      |
| 47   | GND             | -                   | -                     | -      | -                    | -                   | -                    | -                                                                      |
| 48   | SIO_29/<br>AIN5 | SIO_29              | AIN5                  | ln     | Pull-up              | A10                 | P0.29/AIN5           | -                                                                      |
| 49   | SIO_03/<br>AIN1 | SIO_03              | AIN1                  | ln     | Pull-up              | B13                 | P0.03/AIN1           | Ezurio Devkit: Temp<br>Sens Analog                                     |
| 50   | SIO_02/<br>AIN0 | SIO_02              | AIN0                  | IN     | Pull-<br>down        | A12                 | P0.02/AIN0           | Pull High externally to<br>enter VSP (Virtual<br>Serial Port) Service. |
| 51   | SIO_46          | SIO_46              | -                     | ln     | Pull-up              | B15                 | P1.03                | -                                                                      |
| 52   | GND             | -                   | -                     | -      | -                    | -                   | -                    | -                                                                      |
| 53   | SIO_47          | SIO_47              | -                     | ln     | Pull-up              | A14                 | P0.19                | -                                                                      |
| 54   | SIO_44          | SIO_44              | -                     | ln     | Pull-up              | B17                 | P0.23                | Ezurio Devkit: SPI EEPROM SPI_Eeprom_CS, Input                         |
| 55   | GND             | -                   | -                     | -      | -                    | -                   | -                    | -                                                                      |
| 56   | SIO_45          | SIO_45              | -                     | In     | Pull-up              | A16                 | P1.05                | -                                                                      |
| 57   | NFC2/<br>SIO_10 | NFC2                | SIO_10                | ln     | -                    | J24                 | P0.10/NFC2           | -                                                                      |
| 58   | GND             | -                   | -                     | -      | -                    | -                   | -                    | -                                                                      |
| 59   | NFC1/<br>SIO_09 | NFC1                | SIO_09                | ln     | -                    | L24                 | P0.09/NFC1           | -                                                                      |
| 60   | NC              | -                   | -                     | -      | -                    | -                   | -                    | -                                                                      |
| 61   | NC              | -                   | -                     | -      | -                    | -                   | -                    | -                                                                      |
| 62   | SIO_42          | SIO_42              | -                     | ln     | Pull-up              | A20                 | P0.25                | -                                                                      |
| 63   | SIO_38          | N/C                 | -                     | ln     | Pull-up              | R24                 | P1.06                | Reserved for future use. Do not connect.                               |
| 64   | SIO_39          | SIO_39              | -                     | In     | Pull-up              | P23                 | P1.07                |                                                                        |
| 65   | GND             | -                   | -                     | -      | -                    | -                   | -                    | -                                                                      |
| 66   | GND             | -                   | -                     | -      | -                    | -                   | -                    | -                                                                      |
| 67   | GND             | -                   | -                     | -      | -                    | -                   | -                    | -                                                                      |
| 68   | GND             | -                   | -                     | -      | -                    | -                   | -                    | -                                                                      |
| 69   | GND             | -                   | -                     | -      | -                    | -                   | -                    | -                                                                      |
| 70   | GND             | -                   | -                     | -      | -                    | -                   | _                    | -                                                                      |
| 71   | GND             | -                   | -                     | -      | -                    | -                   | -                    | -                                                                      |
| 72   | RF pad          | -                   | -                     | -      | -                    | -                   | -                    | Active on BL653 RF<br>pad variant (453-<br>00041). Note11              |

![](_page_15_Picture_1.jpeg)

| Note 1  | SIO = Signal Input or Output. Secondary function is selectable in <i>smart</i> BASIC application or via Nordic SDK. I/O voltage level tracks VDD. AIN = Analog Input.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  |
|---------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| Note 2  | At reset, all SIO lines are configured as the defaults shown above.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    |
|         | SIO lines can be configured through the <i>smart</i> BASIC application script to be either inputs or outputs with pull-ups or pull-downs. When an alternative SIO function is selected (such as I2C or SPI), the firmware does not allow the setup of internal pull-up/pull-down. Therefore, when I2C interface is selected, pull-up resistors on I2C SDA and I2C SCL <i>must</i> be connected externally as per I2C standard.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         |
| Note 3  | JTAG (two-wire SWD interface), pin 1 (SWDIO) and pin 3 (SWDCLK).                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       |
|         | JTAG is required because Nordic SDK applications can only be loaded using JTAG ( <i>smart</i> BASIC firmware can be loaded using the JTAG as well as UART). We recommend that you use JTAG (2-wire interface) to handle future BL653 module <i>smart</i> BASIC firmware upgrades. You MUST wire out the JTAG (2-wire interface) on your host design (see Figure 7, where four lines (SWDIO, SWDCLK, GND and VDD) should be wired out. <i>smart</i> BASIC firmware upgrades can still be performed over the BL653 UART interface, but this is slower (60 seconds using UART vs. 10 seconds when using JTAG) than using the BL653 JTAG (2-wire interface).                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               |
|         | Upgrading <i>smart</i> BASIC firmware or loading the <i>smart</i> BASIC applications is done using the UART interface.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 |
| Note 4  | Pull the nRESET pin (pin 19) low for minimum 100 milliseconds to reset the BL653.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      |
| Note 5  | The SIO_02 pin (pin 50) must be pulled high externally to enable VSP (Virtual Serial Port) which would allow OTA (over-the-air) <i>smart</i> BASIC application download. Refer to the latest firmware release documentation for details.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               |
| Note 6  | Ensure that SIO_02 (pin 50) and AutoRUN (pin 5) are <i>not both high</i> (externally), in that state, the UART is bridged to Virtue Serial Port service; the BL653 module does not respond to AT commands and cannot load <i>smart</i> BASIC application scripts.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      |
| Note 7  | Pin 5 (nAutoRUN) is an input, with active low logic. In the development kit it is connected so that the state is driven by the host's DTR output line. The nAutoRUN pin must be externally held high or low to select between the following two BL653 operating modes:  • Self-contained Run mode (nAutoRUN pin held at 0V -this is the default (internal pull-down enabled))  • Interactive/Development mode (nAutoRUN pin held at VDD)  The <i>smart</i> BASIC firmware checks for the status of nAutoRUN during power-up or reset. If it is low and if there is a <i>smart</i> BASIC application script named <b>\$autorun\$</b> , then the <i>smart</i> BASIC firmware executes the application script automatically; hence the name S <i>elf-contained Run Mode</i> .                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             |
| Note 8  | The <i>smartBASIC</i> firmware has SIO pins as Digital (Default Function) INPUT pins, which are set PULL-UP by default. This avoids floating inputs (which can cause current consumption to drive with time in low power modes (such as Standby Doze). You can disable the PULL-UP through your <i>smart</i> BASIC application.  All of the SIO pins (with a default function of DIO) are inputs (apart from SIO_05 and SIO_06, which are outputs):  SIO_06 (alternative function UART_TX) is an output, set High (in the firmware).  SIO_05 (alternative function UART_RTS) is an output, set Low (in the firmware).  SIO_08 (alternative function UART_RX) is an input, set with internal pull-up (in the firmware).  SIO_07 (alternative function UART_CTS) is an input, set with internal pull-down (in the firmware).  SIO_02 is an input set with internal pull-down (in the firmware). It is used for OTA downloading of <i>smart</i> BASIC applications. Refer to the latest firmware extension documentation for details.  UART_RX, UART_TX, and UART_CTS are 3.3 V level logic (if VDD is 3.3 V; such as SIO pin I/O levels track VDD). For example, when Rx and Tx are idle, they sit at 3.3 V (if VDD is 3.3 V). Conversely, handshaking pins CTS and RTS at 0V are treated as assertions. |
| Note 9  | BL653 also allows as an option to connect an external higher accuracy (±20 ppm) 32.768 kHz crystal to the BL653 pins SIO_01/XL2 (pin 41) and SIO_00/XL1 (pin 42). This provides higher accuracy protocol timing and helps with radio power consumption in the system standby doze/deep sleep modes by reducing the time that the Rx window must be open.                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               |
| Note 10 | Not required for BL653 module normal operation. The on-chip 32.768kHz LFRC oscillator provides the standard accuracy of ±500 ppm, with calibration required every 8seconds (default) to stay within ±500 ppm.  BL653 power supply options:                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             |

![](_page_16_Picture_1.jpeg)

• Option 1 - Normal voltage power supply mode entered when the external supply voltage is connected to both the VDD and VDD\_HV pins (so that VDD equals VDD\_HV). Connect external supply within range 1.7V to 3.6V range to BL653 VDD and VDD\_HV pins.

OR

Option 2 - High voltage mode power supply mode (using BL653 VDD\_HV pin) entered when the external supply voltage in ONLY connected to the VDDH pin and the VDD pin is not connected to any external voltage supply. Connect external supply within range 2.5V to 5.5V range to BL653 VDD\_HV pin. BL653 VDD pin left unconnected.
 No external current draw (from VDD pin) is allowed when using High Voltage mode (VDD\_HV) (limitation of nRF52833 chipset).

For either option, if you use USB interface then the BL653 VBUS pin must be connected to external supply within the range 4.35V to 5.5V. When using the BL653 VBUS pin, you MUST externally fit a 4.7uF to ground.

Note 11

RF\_pad (pin72) is for the BL653 RF pad variant (453-00041) module only. If using the BL653 module RF pad variant (453-00041), customer MUST copy the 50-Ohms GCPW RF track design, MUST add series 2nH RF inductor (Murata LQG15HN2N0B02# or Murata LQG15HS2N0B02) and RF connector IPEX MHF4 Receptacle (MPN: 20449-001E) Detailed in the following section: 50-Ohms RF Trace and RF Match Series 2nH RF Inductor on Host PCB for BL653 RF Pad Variant (453-00041).

#### 3.3 Electrical Specifications

#### 3.3.1 Absolute Maximum Ratings

Absolute maximum ratings for supply voltage and voltages on digital and analogue pins of the module are listed in the following table; exceeding these values causes permanent damage.

Table 3: Maximum current ratings

| Parameter                                                                | Min  | Max           | Unit               |
|--------------------------------------------------------------------------|------|---------------|--------------------|
| Voltage at VDD pin                                                       | -0.3 | +3.9 (Note 1) | V                  |
| Voltage at VDD_HV pin                                                    | -0.3 | +5.8          | V                  |
| VBUS                                                                     | -0.3 | +5.8          | V                  |
| Voltage at GND pin                                                       |      | 0             | V                  |
| Voltage at SIO pin (at VDD≤3.6V)                                         | -0.3 | VDD +0.3      | V                  |
| Voltage at SIO pin (at VDD≥3.6V)                                         | -0.3 | 3.9           | V                  |
| NFC antenna pin current (NFC1/2)                                         | -    | 80            | mA                 |
| Radio RF input level                                                     | -    | 10            | dBm                |
| Environmental                                                            |      |               |                    |
| Storage temperature                                                      | -40  | +85           | °C                 |
| MSL (Moisture Sensitivity Level)                                         | -    | 4             | -                  |
| ESD (as per EN301-489)                                                   |      |               |                    |
| Conductive                                                               |      | 4             | KV                 |
| Air Coupling                                                             |      | 8             | KV                 |
| Flash Memory (Endurance) (Note 2)                                        | -    | 10000         | Write/erase cycles |
| Flash Memory (Retention) at 85°C                                         | 10   | -             | years              |
| Flash Memory (Retention) at 105°C                                        | 3    |               | years              |
| (Limited to 1000 write /read cycles)                                     |      |               |                    |
| Flash Memory (Retention) at 105°C to 85°C, execution split               | 6.7  |               | years              |
| (Limited to 1000 write /read cycles, 75% execution time at 85°C or less) |      |               |                    |
| Thermal characteristics (Junction temperature)                           |      | 110           | °C                 |

![](_page_17_Picture_1.jpeg)

#### Maximum Ratings Notes:

**Note 1** The absolute maximum rating for VDD\_nRF pin (max) is 3.9V for the BL653.

Note 2 Wear levelling is used in file system.

#### 3.3.2 Recommended Operating Parameters

Table 4: Power supply operating parameters

| Parameter                                                      | Min  | Тур | Max  | Unit |
|----------------------------------------------------------------|------|-----|------|------|
| VDD (independent of DCDC enable) <sup>1,4</sup> supply range   | 1.7  | 3.3 | 3.6  | V    |
| VDD <sub>POR</sub> supply voltage needed during power on reset | 1.75 |     |      | V    |
| VDD_HV supply range <sup>4</sup>                               | 2.5  | 3.7 | 5.5  | V    |
| VBUS USB supply range                                          | 4.35 | 5   | 5.5  | V    |
| VDD Maximum ripple or noise <sup>2</sup>                       | -    | -   | 10   | mV   |
| VDD supply rise time (0V to 1.7V) <sup>3</sup>                 | -    | -   | 60   | mS   |
| VDD_HV supply rise time (0V to 3.7V) <sup>3</sup>              |      |     | 100  | mS   |
| Operating temperature range                                    | -40  | -   | +85  | ۰C   |
| Extended operating temperature range <sup>5</sup>              | +85  |     | +105 | °C   |

#### **Recommended Operating Parameters Notes:**

| Note   | 4.7 uF internal to module on VDD. The VDD internal DCDC (REG1) convertor or LDO (REG1) is decided by the underlying BLE stack depending on the load current. Through a smartBASIC command can also tell softdevice to use DCDC always or LDO always. |
|--------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| Note 2 | This is the maximum VDD or VDD_HV ripple or noise (at any frequency) that does not disturb the radio.                                                                                                                                                |
| Note:  | The on-board power-on reset circuitry may not function properly for rise times longer than the specified maximum.                                                                                                                                    |

#### Note 4

BL653 power supply options:

• Option 1 - Normal voltage power supply mode entered when the external supply voltage is connected to both the VDD and VDD\_HV pins (so that VDD equals VDD\_HV). Connect external supply within range 1.7V to 3.6V range to BL653 VDD and VDD\_HV pins.

OR

• Option 2 - High voltage mode power supply mode (using BL653 VDD\_HV pin) entered when the external supply voltage in ONLY connected to the VDD\_HV pin and the VDD pin is not connected to any external voltage supply. Connect external supply within range 2.5V to 5.5V range to BL653 VDD\_HV pin. BL653 VDD pin left unconnected. No external current draw (from VDD pin) is allowed when using High Voltage mode (VDD\_HV), limitation of the nRF52833 chip.

For either option, if you use USB interface then the BL653 VBUS pin must be connected to external supply within the range 4.35V to 5.5V. When using the BL653 VBUS pin, you MUST externally fit a 4.7uF to ground.

Note 5

To avoid surpassing the maximum die temperature (110 °C), it is important to minimize current consumption when operating in the extended operating temperature conditions (+85 °C to +105 °C). It is therefore recommended to use the module device on Normal Voltage mode with DCDC (REG1) enabled.

Table 5: Signal levels for interface, SIO

| rabio of eighar for old for internace, ele |          |     |           |      |
|--------------------------------------------|----------|-----|-----------|------|
| Parameter                                  | Min      | Тур | Max       | Unit |
| V <sub>IH</sub> Input high voltage         | 0.7 VDD  |     | VDD       | V    |
| V <sub>IL</sub> Input low voltage          | VSS      |     | 0.3 x VDD | V    |
| V <sub>OH</sub> Output high voltage        |          |     |           |      |
| (std. drive, 0.5mA) (Note 1)               | VDD -0.4 |     | VDD       | V    |
| (high-drive, 3mA) (Note 1)                 | VDD -0.4 |     | VDD       | V    |

![](_page_18_Picture_1.jpeg)

| Parameter                                           | Min      | Тур | Max     | Unit |
|-----------------------------------------------------|----------|-----|---------|------|
| (high-drive, 5mA) (Note 2)                          | VDD -0.4 |     | VDD     |      |
| V <sub>OL</sub> Output low voltage                  |          |     |         |      |
| (std. drive, 0.5mA) (Note 1)                        | VSS      |     | VSS+0.4 | V    |
| (high-drive, 3mA) (Note 1)                          | VSS      |     | VSS+0.4 | V    |
| (high-drive, 5mA) (Note 2)                          | VSS      |     | VSS+0.4 |      |
| V <sub>OL</sub> Current at VSS+0.4V, Output set low |          |     |         |      |
| (std. drive, 0.5mA) (Note 1)                        | 1        | 2   | 4       | mA   |
| (high-drive, 3mA) (Note 1)                          | 3        | -   | -       | mA   |
| (high-drive, 5mA) (Note 2)                          | 6        | 10  | 15      | mA   |
| V <sub>OL</sub> Current at VDD -0.4, Output set low |          |     |         |      |
| (std. drive, 0.5mA) (Note 1)                        | 1        | 2   | 4       | mA   |
| (high-drive, 3mA) (Note 1)                          | 3        | -   | -       | mA   |
| (high-drive, 5mA) (Note 2)                          | 6        | 9   | 14      | mA   |
| Pull up resistance                                  | 11       | 13  | 16      | kΩ   |
| Pull down resistance                                | 11       | 13  | 16      | kΩ   |
| Pad capacitance                                     |          | 3   |         | pF   |
| Pad capacitance at NFC pads                         |          | 4   |         | pF   |

#### Signal Levels Notes:

| Note 1 | For VDD≥1.7V. The firmware supports high drive (3 mA, as well as standard drive). |
|--------|-----------------------------------------------------------------------------------|
|        |                                                                                   |

Note 2

For VDD  $\geq$  2.7V. The firmware supports high drive (5 mA (since VDD  $\geq$  2.7V), as well as standard drive).

The GPIO (SIO) high reference voltage always equals the level on the VDD pin.

- Normal voltage mode The GPIO high level equals the voltage supplied to the VDD pin
- High voltage mode The GPIO high level equals the level specified (is configurable to 1.8V, 2.1V, 2.4V, 2.7V, 3.0V, and 3.3V. The default voltage is 1.8V). In High voltage mode, the VDD pin becomes an output voltage pin. The VDD output voltage and hence the GPIO is configurable from 1.8V to 3.3V with possible settings of 1.8V, 2.1V, 2.4V, 2.7V, 3.0V, and 3.3V.

Table 6: SIO pin alternative function AIN (ADC) specification

| Parameter                                                                   | Min        | Тур                                | Max         | Unit    |
|-----------------------------------------------------------------------------|------------|------------------------------------|-------------|---------|
| ADC Internal reference voltage                                              | -1.5%      | 0.6 V                              | +1.5%       | %       |
| ADC pin input internal selectable scaling                                   |            | 4, 2, 1, 1/2, 1/3,<br>1/4, 1/5 1/6 |             | scaling |
| ADC input pin (AIN) voltage maximum without damaging ADC w.r.t (see Note 1) |            |                                    |             |         |
| VCC Prescaling                                                              |            |                                    |             |         |
| 0V-VDD 4, 2, 1, ½, 1/3, ¼, 1/5, 1/6                                         |            | VDD+0.3                            |             | V       |
| Configurable                                                                | 8-bit mode | 10-bit mode                        | 12-bit mode | bits    |
| Resolution                                                                  |            |                                    |             |         |
| Configurable (see Note 2)                                                   |            |                                    |             |         |
| Acquisition Time, source resistance ≤10kΩ Acquisition                       |            | 3                                  |             | uS      |
| Time, source resistance $\leq 40 k\Omega$                                   |            | 5                                  |             | uS      |
| Acquisition Time, source resistance $\leq 100 k\Omega$                      |            | 10                                 |             | uS      |
| Acquisition Time, source resistance ≤200kΩ                                  |            | 15                                 |             | uS      |

All Rights Reserved

![](_page_19_Picture_1.jpeg)

| Parameter                                              | Min | Тур | Max | Unit |
|--------------------------------------------------------|-----|-----|-----|------|
| Acquisition Time, source resistance ≤400kΩ             |     | 20  |     | uS   |
| Acquisition Time, source resistance $\leq 800 k\Omega$ |     | 40  |     | uS   |
| Conversion Time (see Note 3)                           |     | <2  |     | uS   |
| ADC input impedance (during operation) (see Note 3)    |     |     |     |      |
| Input Resistance                                       |     | >1  |     | MOhm |
| Sample and hold capacitance at maximum gain            |     | 2.5 |     | pF   |

| Recommen | ded Operating Parameters Notes:                                                                                                                                                                                                                                                                                                                                                                 |
|----------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| Note 1   | Stay within internal 0.6 V reference voltage with given pre-scaling on AIN pin and do not violate ADC maximum input voltage (for damage) for a given VCC, e.g. If VDD is 3.6V, you can only expose AIN pin to VDD+0.3 V. Default pre-scaling is 1/6 which configurable via smartBASIC.                                                                                                          |
| Note 2   | Firmware allows configurable resolution (8-bit, 10-bit or 12-bit mode) and acquisition time. BL653 ADC is a Successive Approximation type ADC (SSADC), as a result no external capacitor is needed for ADC operation. Configure the acquisition time according to the source resistance that customer has.                                                                                      |
|          | The sampling frequency is limited by the sum of sampling time and acquisition time. The maximum sampling time is 2us. For acquisition time of 3us the total conversion time is therefore 5us, which makes maximum sampling frequency of 1/5us = 200kHz. Similarly, if acquisition time of 40us chosen, then the conversion time is 42us and the maximum sampling frequency is 1/42us = 23.8kHz. |
| Note 3   | ADC input impedance is estimated mean impedance of the ADC (AIN) pins.                                                                                                                                                                                                                                                                                                                          |

### 3.4 Programmability

#### 3.4.1 BL653 Default Firmware

The BL653 module comes loaded with *smart*BASIC firmware but does not come loaded with any *smart*BASIC application script (as that is dependent on customer-end application or use). Ezurio provides many sample *smart*BASIC application scripts via a sample application folder on GitHub – https://github.com/LairdCP/BL653-Applications

Therefore, it boots into AT command mode by default.

#### 3.4.2 BL653 Special Function Pins in *smart*BASIC

Refer to the *smart*BASIC extension manual for details of functionality connected to this:

- nAutoRUN pin (SIO\_35), see Table 7 for default
- VSP pin (SIO\_02), see Table 8 for default
- SIO\_38 Reserved for future use. Do not connect. See Table 9

#### Table 7: nAutoRUN pin

| Signal Name       | Pin# | I/O | Comments                                                                    |
|-------------------|------|-----|-----------------------------------------------------------------------------|
| nAutoRUN/(SIO_35) | 5    | 1   | Input with active low logic. Internal pull down (default).                  |
|                   |      |     | Operating mode selected by nAutoRun pin status:                             |
|                   |      |     | <ul> <li>Self-contained Run mode (nAutoRUN pin held at 0V)</li> </ul>       |
|                   |      |     | <ul> <li>If Low (0V), runs \$autorun\$ if it exists</li> </ul>              |
|                   |      |     | <ul> <li>Interactive/Development mode (nAutoRUN pin held at VCC)</li> </ul> |
|                   |      |     | • If High (VCC), runs via AT+rRUN (and file name of application)            |

In the development board nAutoRUN pin is connected so that the state is driven by the host's DTR output line.

#### Table 8: VSP mode

| Signal Name | Pin# | 1/0 | Comments                      |
|-------------|------|-----|-------------------------------|
| SIO_02      | 50   | 1   | Internal pull down (default). |

![](_page_20_Picture_1.jpeg)

| Signal Name     | Pin# | I/O | Comments                                                                |  |  |  |  |
|-----------------|------|-----|-------------------------------------------------------------------------|--|--|--|--|
|                 |      |     | VSP mode selected by externally pulling-up SIO_02 pin:                  |  |  |  |  |
|                 |      |     | High (VCC), then OTA smartBASIC application download is possible.       |  |  |  |  |
| Table 9: SIO_38 |      |     |                                                                         |  |  |  |  |
| Signal Name     | Pin# | 1/0 | Comments                                                                |  |  |  |  |
| SIO_38          | 63   | 1   | Internal pull up (default).                                             |  |  |  |  |
|                 |      |     | Reserved for future use. Do not connect if using <i>smart</i> BASIC FW. |  |  |  |  |

# 4 Power Consumption

Data at VDD of 3.3 V with internal (to chipset) LDO(REG1) ON or with internal (to chipset) DCDC(REG1) ON (see Power Consumption Note 1) and 25°C.

### 4.1 Power Consumption

Table 10: Power consumption

| Parameter   |                                                                                                                                                                                                                                                                                                                                                                      | Min Typ              | Max Unit |  |  |  |  |
|-------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|----------------------|----------|--|--|--|--|
| Active mod  | de 'peak' current (Note 1)                                                                                                                                                                                                                                                                                                                                           | With DCDC [with LDO] |          |  |  |  |  |
| (Advertisin | ng or Connection)                                                                                                                                                                                                                                                                                                                                                    |                      |          |  |  |  |  |
| Tx only     | run peak current @ Txpwr = +8 dBm                                                                                                                                                                                                                                                                                                                                    | 14.2 [30.4]          | mA       |  |  |  |  |
| Tx only     | run peak current @ Txpwr = +4 dBm                                                                                                                                                                                                                                                                                                                                    | 9.6 [20.7]           | mA       |  |  |  |  |
| Tx only     | run peak current @ Txpwr = 0 dBm                                                                                                                                                                                                                                                                                                                                     | 4.9 [10.3]           |          |  |  |  |  |
| Tx only     | run peak current @ Txpwr = -4 dBm                                                                                                                                                                                                                                                                                                                                    | 3.8 [8.0]            |          |  |  |  |  |
| Tx only     | run peak current @ Txpwr = -8 dBm                                                                                                                                                                                                                                                                                                                                    | 3.4 [7.1]            |          |  |  |  |  |
| Tx only     | run peak current @ Txpwr = -12 dBm                                                                                                                                                                                                                                                                                                                                   | 3.1 [6.4]            |          |  |  |  |  |
| Tx only     | run peak current @ Txpwr = -16 dBm                                                                                                                                                                                                                                                                                                                                   | 2.9 [5.9]            |          |  |  |  |  |
| Tx only     | run peak current @ Txpwr = -20 dBm                                                                                                                                                                                                                                                                                                                                   | 2.7 [5.5]            |          |  |  |  |  |
| Tx only     | run peak current @ Txpwr = -40 dBm                                                                                                                                                                                                                                                                                                                                   | 2.3 [4.5]            |          |  |  |  |  |
| Active Mode | e                                                                                                                                                                                                                                                                                                                                                                    |                      |          |  |  |  |  |
|             | 'peak' current, BLE 1Mbps (Note 1)                                                                                                                                                                                                                                                                                                                                   | 4.6 [9.6]            | mA       |  |  |  |  |
| Rx only     | 'peak' current, BLE 2Mbps (Note 2)                                                                                                                                                                                                                                                                                                                                   | 5.2 [10.7]           | mA       |  |  |  |  |
| Ultra-Low F | Power Mode 1 (Note 2)                                                                                                                                                                                                                                                                                                                                                | 2.6                  | uA       |  |  |  |  |
| Standb      | y Doze, 256k RAM retention                                                                                                                                                                                                                                                                                                                                           |                      |          |  |  |  |  |
| Ultra-Low F | Power Mode 2 (Note 3)                                                                                                                                                                                                                                                                                                                                                | 0.6                  | uA       |  |  |  |  |
| Deep SI     | leep (no RAM retention)                                                                                                                                                                                                                                                                                                                                              |                      |          |  |  |  |  |
| Active Mod  | de Average current (Note 4)                                                                                                                                                                                                                                                                                                                                          |                      |          |  |  |  |  |
| Advertisino | g Average Current draw                                                                                                                                                                                                                                                                                                                                               |                      |          |  |  |  |  |
| Max, with a | advertising interval (min) 20 mS                                                                                                                                                                                                                                                                                                                                     | Note 4               | uA       |  |  |  |  |
| Min, with a | dvertising interval (max) 10240 mS                                                                                                                                                                                                                                                                                                                                   | Note 4               | uA       |  |  |  |  |
| Connection  | n Average Current draw                                                                                                                                                                                                                                                                                                                                               |                      |          |  |  |  |  |
| Max, with o | connection interval (min) 7.5 mS                                                                                                                                                                                                                                                                                                                                     | Note 4               | uA       |  |  |  |  |
| Min, with c | onnection interval (max) 4000 mS                                                                                                                                                                                                                                                                                                                                     | Note4                | uA       |  |  |  |  |
| Power Cons  | sumption Notes:                                                                                                                                                                                                                                                                                                                                                      |                      |          |  |  |  |  |
| Note 1      | This is for Peak Radio Current only, but there is additional current due to the MCU. The internal DCDC convertor (REG1) or LDO (REG1) is decided by the underlying BLE stack.                                                                                                                                                                                        |                      |          |  |  |  |  |
| Note 2      | BL653 modules Standby Doze is 2.6 uA typical. When using smartBASIC firmware, Standby Doze is entered automatically (when a waitevent statement is encountered within a smartBASIC application script). In Standby Doze, all peripherals that are enabled stay on and may re-awaken the chip. Depending on active peripherals, current consumption ranges from 2.6 µ |                      |          |  |  |  |  |

![](_page_21_Picture_1.jpeg)

to 370 uA (when UART is ON). See individual peripherals current consumption data in the Peripheral Block Current Consumption section. smartBASIC firmware has functionality to detect GPIO change with no current consumption cost, it is possible to close the UART and get to the 2.6 uA current consumption regime and still be able to detect for incoming data and be woken up so that the UART can be re-opened at expense of losing that first character.

The BL653 Standby Doze current (at 25°C) consists of the below nRF52833 blocks:

- nRF52 System ON IDLE current (no RAM retention) (1.1 uA) This is the base current of the CPU
- LFRC (0.7 uA) and RTC (0.1uA or near 0uA) running as well as 128k RAM retention (0.8 uA) This adds to the total of 2.7 uA typical. The RAM retention is 20nA per 4k block, but this can vary to 30nA per 4k block which would make the total 2.55uA to 2.86uA.

#### Note 3

In Deep Sleep, everything is disabled and the only wake-up sources (including NFC to wakeup) are reset and changes on SIO or NFC pins on which sense is enabled. The current consumption seen is  $\sim$ 0.6 uA typical in BL653 modules.

• Coming out from Deep Sleep to Standby Doze through the reset vector.

#### Note 4

Average current consumption depends on several factors (including Tx power, VCC, accuracy of 32MHz and 32.768 kHz). With these factors fixed, the largest variable is the advertising or connection interval set.

Advertising Interval range:

• 20 milliseconds to 10240 mS (10485759.375 mS in BT5.1) in multiples of 0.625 milliseconds.

For an advertising event:

- The minimum average current consumption is when the advertising interval is large 10240 mS (10485759.375 mS (in BT5.0) although this may cause long discover times (for the advertising event) by scanners
- The maximum average current consumption is when the advertising interval is small 20 mS
- Other factors that are also related to average current consumption include the advertising payload bytes in each
  advertising packet and whether it's continuously advertising or periodically advertising.

Connection Interval range (for a peripheral):

• 7.5 milliseconds to 4000 milliseconds in multiples of 1.25 milliseconds.

For a connection event (for a peripheral device):

- The minimum average current consumption is when the connection interval is large 4000 milliseconds
- The maximum average current consumption is with the shortest connection interval of 7.5 ms; no slave latency.

Other factors that are also related to average current consumption include:

- Number packets per connection interval with each packet payload size
- An inaccurate 32.768 kHz master clock accuracy would increase the average current consumption.

Connection Interval range (for a central device):

• 2.5 milliseconds to 40959375 milliseconds in multiples of 1.25 milliseconds.

### 4.2 Peripheral Block Current Consumption

The values below are calculated for a typical operating voltage of 3V.

Table 11: UART power consumption

|                                     |     | Ту                  |                    |      |      |
|-------------------------------------|-----|---------------------|--------------------|------|------|
| Parameter                           | Min | WITH<br>DCDC (REG1) | WITH LDO<br>(REG1) | Max  | Unit |
| UART Run current @ 115200 bps       | -   | 450                 | 721                | -    | uA   |
| UART Run current @ 1200 bps         | -   | 450                 | 721                | -    | uA   |
| Idle current for UART (no activity) | -   | 2.9                 | 2.9                | -    | uA   |
| UART Baud rate                      | 1.2 | -                   |                    | 1000 | kbps |

Table 12: SPI power consumption

|                                 |     | Тур                 | )                  |     |      |
|---------------------------------|-----|---------------------|--------------------|-----|------|
| Parameter                       | Min | WITH<br>DCDC (REG1) | WITH LDO<br>(REG1) | Тур | Unit |
| SPI Master Run current @ 2 Mbps | -   | 536                 | 803                | -   | uA   |

![](_page_22_Picture_1.jpeg)

|                                    | Тур |                     |                    |     |      |
|------------------------------------|-----|---------------------|--------------------|-----|------|
| Parameter                          | Min | WITH<br>DCDC (REG1) | WITH LDO<br>(REG1) | Тур | Unit |
| SPI Master Run current @ 8 Mbps    | -   | 536                 | 803                | -   | uA   |
| Idle current for SPI (no activity) | -   | <1                  | <1                 | -   | uA   |
| SPI bit rate                       | -   | -                   |                    | 8   | Mbps |

Table 13: I2C power consumption

|                                    | Тур |                     |                    |     |      |
|------------------------------------|-----|---------------------|--------------------|-----|------|
| Parameter                          | Min | WITH<br>DCDC (REG1) | WITH LDO<br>(REG1) | Тур | Unit |
| I2C Run current @ 100 kbps         | -   | 734                 | 994                | -   | uA   |
| I2C Run current @ 400 kbps         | -   | 734                 | 994                | -   | uA   |
| Idle current for I2C (no activity) | -   | 2.5                 | 2.5                | -   | uA   |
| I2C Bit rate                       | 100 | -                   |                    | 400 | kbps |

Table 14: ADC power consumption

|                               |     | Ту                 |                    |     |      |
|-------------------------------|-----|--------------------|--------------------|-----|------|
| Parameter                     | Min | WITH<br>DCDC(REG1) | WITH LDO<br>(REG1) | Max | Unit |
| ADC current during conversion | -   | 1900               | 1350               | -   | uA   |
| Idle current                  | -   | 0                  | 0                  | -   | uA   |

The above current consumption is for the given peripheral including the internal blocks that are needed for that peripheral for both the case when DCDC (REG1) is on and LDO (REG1) is on. The peripheral idle current is when the peripheral is enabled but not running (not sending data or being used) and must be added to the StandByDoze current (Nordic System ON Idle current). In all cases radio is not turned on.

For asynchronous interface, like the UART (asynchronous as the other end can communicate at any time), the UART on the BL653 must be kept open (by a command in *smart*BASIC application script), resulting in the base current consumption penalty.

For a synchronous interface like the I2C or SPI (since BL653 side is the master), the interface can be closed and opened (by a command in *smart*BASIC application script) only when needed, resulting in current saving (no base current consumption penalty). There's a similar argument for ADC (open ADC when needed).

## 5 Functional Description

To provide the widest scope for integration, a variety of physical host interfaces/sensors are provided. The major BL653 series module functional blocks described below

### 5.1 Power Management

Power management features:

- System Standby Doze and Deep Sleep modes
- Open/Close peripherals (UART, SPI, I2C, SIO's, ADC, NFC). Peripherals consume current when open; each peripheral can be individually closed to save power consumption
- Use of the internal DCDC (REG1) convertor or LDO (REG1) is decided by the underlying BLE stack
- smartBASIC command allows the supply voltage to be read (through the internal ADC)
- Pin wake-up system from deep sleep (including from NFC pins)

Power supply features:

- Supervisor hardware to manage power during reset, brownout, or power fail.
- 1.7V to 3.6V supply range for normal power supply (VDD pin) using internal DCDC convertor (REG1) or LDO(REG1) decided by the underlying BLE stack.
- 2.5V to 5.5 supply range for High voltage power supply (VDD\_HV pin) using internal LDO(REG0).

![](_page_23_Picture_1.jpeg)

• 4.35V to 5.5V supply range for powering USB (VBUS pin) portion of BL653 only. The remainder of the BL653 module circuitry must still be powered through the VDD (or VDD\_HV) pin.

### 5.2 BL653 Power Supply Options

The BL653 module power supply internally contains the following two main supply regulator stages (Figure 4):

- REG0 Connected to the VDD\_HV pin
- REG1 Connected to the VDD pin

The USB power supply is separate (connected to the VBUS pin).

![](_page_23_Figure_8.jpeg)

Figure 4: BL653 power supply block diagram (adapted from the following resource: http://infocenter.nordicsemi.com/pdf/nRF52833\_PS\_v1.0.pdf

The BL653 power supply system enters one of two supply voltage modes, normal or high voltage mode, depending on how the external supply voltage is connected to these pins.

BL653 power supply options:

• Option 1 - Normal voltage power supply mode entered when the external supply voltage is connected to both the VDD and VDD\_HV pins (so that VDD equals VDD\_HV). Connect external supply within range 1.7V to 3.6V range to BL653 VDD and VDD\_HV pins.

OR

- Option 2 High voltage mode power supply mode (using BL653 VDD\_HV pin) entered when the external supply voltage in ONLY connected to the VDD\_HV pin and the VDD pin is not connected to any external voltage supply. Connect external supply within range 2.5V to 5.5V range to BL653 VDD\_HV pin. BL653 VDD pin left unconnected.
  - No external current draw (from VDD pin) is allowed when using High Voltage mode (VDD\_HV), limitation of the nRF52833 chip.

Note: To avoid surpassing the maximum die temperature (110 °C), it is important to minimize current consumption when operating in the extended operating temperature conditions (+85 °C to +105 °C). It is therefore recommended to use the module device on Normal Voltage mode with DCDC (REG1) enabled.

For either option, if you use USB interface then the BL653 VBUS pin must be connected to external supply within the range 4.35V to 5.5V. When using the BL653 VBUS pin, you **MUST** externally fit a 4.7uF to ground.

Table 15 summarizes these power supply options.

![](_page_24_Picture_1.jpeg)

| Power Supply Pins<br>and Operating<br>Voltage Range | OPTION 1<br>Normal voltage mode<br>operation connect? | OPTION 2<br>High voltage mode<br>operation connect? | OPTION 1 with USB<br>peripheral,<br>and normal voltage mode<br>operation connect? | OPTION 2 with USB<br>peripheral, and high<br>voltage operation<br>connect? |
|-----------------------------------------------------|-------------------------------------------------------|-----------------------------------------------------|-----------------------------------------------------------------------------------|----------------------------------------------------------------------------|
| VDD (pin31)                                         | Yes                                                   | No                                                  | Yes                                                                               | No                                                                         |
| 1.7V to 3.6V                                        | (Note 1)                                              | (Note 2)                                            |                                                                                   | (Note 2)                                                                   |
| VDD_HV (pin25)                                      | No                                                    | Yes                                                 | No                                                                                | Yes                                                                        |
| 2.5V to 5.5V                                        |                                                       |                                                     |                                                                                   | (Note 5)                                                                   |
| VBUS (pin24)                                        | No                                                    | (Note 3)                                            | Yes                                                                               | Yes                                                                        |
| 4.35V to 5.5V                                       |                                                       |                                                     | (Note 4)                                                                          | (Note 4)                                                                   |

| Power Supp | oly Option Notes:                                                                                                                                                                                                                                                                                                                                         |  |  |  |  |  |  |
|------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|--|--|--|--|--|--|
| Note 1     | <b>Option 1 –</b> External supply voltage is connected to BOTH the VDD and VDD_HV pins (so that VDD equals VDD_HV). Connect external supply within range 1.7V to 3.6V range to BOTH BL653 VDD and VDD_HV pins.                                                                                                                                            |  |  |  |  |  |  |
| Note 2     | Option 2 - External supply within range 2.5V to 5.5V range to the BL653 VDD_HV pin ONLY. BL653 VDD pin left unconnected.                                                                                                                                                                                                                                  |  |  |  |  |  |  |
|            | In High voltage mode, the VDD pin becomes an output voltage pin but no current can be taken from VDD pin (limitation of the nRF52833 chip). It Additionally, the VDD output voltage is configurable from 1.8V to 3.3V with possible settings of 1.8V 2.1V, 2.4V, 2.7V, 3.0V, and 3.3V. The default voltage is 1.8V.                                       |  |  |  |  |  |  |
|            | The supported BL653 VDD pin output voltage range depends on the supply voltage provided on the BL653 VDD_HV pin. The minimum difference between voltage supplied on the VDD_HV pin and the voltage output on the VDD pin is 0.3 V. The maximum output voltage of the VDD pin is VDDH – 0.3V.                                                              |  |  |  |  |  |  |
| Note 3     | Depends on whether USB operation is required                                                                                                                                                                                                                                                                                                              |  |  |  |  |  |  |
| Note 4     | When using the BL653 <b>VBUS</b> pin, you <b>must</b> externally fit a 4.7uF capacitor to ground.                                                                                                                                                                                                                                                         |  |  |  |  |  |  |
| Note 5     | <ol> <li>To use the BL653 USB peripheral:</li> <li>Connect the BL653 VBUS pin to the external supply within the range 4.35V to 5.5V. When using the BL653 VBUS pin, you MUST externally fit a 4.7uF to ground.</li> <li>Connect the external supply to either the VDD (Option 1) or VDD_HV (Option 2) pin to operate the rest of BL653 module.</li> </ol> |  |  |  |  |  |  |
|            | When using the BL653 USB peripheral, the VBUS pin can be supplied from same source as VDD_HV (within the                                                                                                                                                                                                                                                  |  |  |  |  |  |  |

#### 5.3 Clocks and Timers

#### 5.3.1 Clocks

The integrated high accuracy 32 MHz (±10 ppm) crystal oscillator helps with radio operation and reducing power consumption in the active modes.

The integrated on-chip 32.768 kHz LFRC oscillator (±500 ppm) provides protocol timing and helps with radio power consumption in the system StandByDoze and Deep Sleep modes by reducing the time that the RX window needs to be open.

operating voltage range of the VBUS pin and VDD\_HV pin).

To keep the on-chip  $32.768\,\text{kHz}$  LFRC oscillator within  $\pm 500\,\text{ppm}$  (which is needed to run the BLE stack) accuracy, RC oscillator needs to be calibrated (which takes  $33\,\text{mS}$ ) regularly. The default calibration interval is eight seconds which is enough to keep within  $\pm 500\,\text{ppm}$ . The calibration interval ranges from  $0.25\,\text{seconds}$  to  $31.75\,\text{seconds}$  (in multiples of  $0.25\,\text{seconds}$ ) and configurable via firmware

#### 5.3.2 Timers

When using smartBASIC, the timer subsystem enables applications to be written which allow future events to be generated based on timeouts.

- Regular Timer There are eight built-in timers (regular timers) derived from a single RTC clock which are controlled solely by smart BASIC functions. The resolution of the regular timer is 976 microseconds.
- Tick Timer A 31-bit free running counter that increments every (1) millisecond. The resolution of this counter is 488 microseconds.

Refer to the *smartBASIC User Guide* available from the Ezurio BL653 product page. For timer utilization when using the Nordic SDK, refer to <a href="http://infocenter.nordicsemi.com/index.jsp">http://infocenter.nordicsemi.com/index.jsp</a>.

![](_page_25_Picture_1.jpeg)

### 5.4 Radio Frequency (RF)

- 2402–2480 MHz Bluetooth Low Energy radio BT 5.4 1 Mbps, 2 Mbps, and long-range (125 kbps and 500 kbps) over-the-air data rate.
- Tx output power of +8 dBm programmable down to 7 dBm, 6 dBm, 5 dBm, 4 dBm, 2 dBm, 0 dBm and further down to -20 dBm in steps of 4 dB and final TX power level of -40 dBm.
- Receiver (with integrated channel filters) to achieve maximum sensitivity -96 dBm @ 1 Mbps BLE, -92 dBm @2 Mbps, -103 dBm @ 125 kbps long-range and -99 dBm @500kbps long-range).
- RF conducted interface available in the following two ways:
  - 453-00039: RF connected to on-board PCB trace antenna
  - 453-00041: RF connected to RF pad on BL653
- Antenna options:
  - Integrated PCB trace antenna on the 453-00039
  - External dipole antenna connected to IPEX MH4 RF connector on host PCB. If using the BL653 module RF pad variant (453-00041), customer MUST copy the 50-Ohms GCPW RF track design, MUST add series 2nH RF inductor (Murata LQG15HN2N0B02# or Murata LQG15HS2N0B02) and RF connector IPEX MHF4 Receptacle (MPN: 20449-001E) Detailed in the following section: 50-Ohms RF Trace and RF Match Series 2nH RF Inductor on Host PCB for BL653 RF Pad Variant (453-00041).
- Received Signal Strength Indicator (RSSI)
  - RSSI accuracy (valid range -90 to -20 dBm) is ±2 dB typical
  - RSSI resolution 1 dB typical

#### 5.5 NFC

#### NFC support:

- Based on the NFC forum specification
  - 13.56 MHz
  - Date rate 106 kbps
  - NFC Type2 and Type4 tag emulation
- Modes of operation:
  - Disable
  - Sense
  - Activated

#### 5.5.1 Use Cases

- Touch-to Pair with NFC
- Launch a smartphone app (on Android)
- NFC enabled Out-of-Band Pairing
- System Wake-On-Field function
  - Proximity Detection

#### Table 16: NFC interface

| Signal Name | Pin No | 1/0 | Comments                                                                            |
|-------------|--------|-----|-------------------------------------------------------------------------------------|
| NFC1/SIO_09 | 59     | I/O | The NFC pins are by default NFC pins and an alternate function on each pin is GPIO. |
| NFC2/SIO_10 | 57     | I/O | Refer to the <i>smart</i> BASIC. User manual.                                       |

#### 5.5.2 NFC Antenna Coil Tuning Capacitors

From Nordic's nRF52833Objective Product Specification v1.0: http://infocenter.nordicsemi.com/pdf/nRF52833\_PS\_v1.0.pdf

The NFC antenna coil must be the connected differential between the NFC1 and NFC2 pins of the BL653. Two external capacitors should be used to tune the resonance of the antenna circuit to 13.56 MHz (Figure 5).

![](_page_26_Picture_1.jpeg)

![](_page_26_Picture_2.jpeg)

Figure 5: NFC antenna coil tuning capacitors

The required external tuning capacitor value is given by the following equation:

$$C_{tune} = \frac{2}{(2\pi \cdot 13.56 \text{ MHz})^2 \cdot L_{ant}} - C_p - C_{int}$$

An antenna inductance of Lant = 0.72 uH provides tuning capacitors in the range of 300 pF on each pin. The total capacitance on NFC1 and NFC2 must be matched. Cint and Cp are small usually (Cint is 4pF), so can be omitted from calculation.

**Battery Protection Note:** If the NFC coil antenna is exposed to a strong NFC field, the supply current may flow in the opposite direction due to parasitic diodes and ESD structures.

If the used battery does not tolerate a return current, a series diode must be placed between the battery and the BL653 to protect the battery.

#### 5.6 UART Interface

Note: The BL653 has two UARTs.

The Universal Asynchronous Receiver/Transmitter (UART) offers fast, full-duplex, asynchronous serial communication with built-in flow control support (UART\_CTS, UART\_RTS) in HW up to one Mbps baud. Parity checking and generation for the ninth data bit are supported.

UART\_TX, UART\_RTS, and UART\_CTS form a conventional asynchronous serial data port with handshaking. The interface is designed to operate correctly when connected to other UART devices such as the 16550A. The signaling levels are nominal 0 V and 3.3 V (tracks VDD) and are inverted with respect to the signaling on an RS232 cable.

Two-way hardware flow control is implemented by UART\_RTS and UART\_CTS. UART\_RTS is an output and UART\_CTS is an input. Both are active low.

These signals operate according to normal industry convention. UART\_RX, UART\_TX, UART\_CTS, UART\_RTS are all 3.3 V level logic (tracks VDD). For example, when RX and TX are idle they sit at 3.3 V. Conversely for handshaking pins CTS, RTS at 0 V is treated as an assertion.

27

The module communicates with the customer application using the following signals:

- Port/TxD of the application sends data to the module's UART\_RX signal line
- Port/RxD of the application receives data from the module's UART\_TX signal line

![](_page_26_Picture_18.jpeg)

Figure 6: UART signals

![](_page_27_Picture_1.jpeg)

Note:

The BL653 serial module output is at 3.3V CMOS logic levels (tracks VDD). Level conversion must be added to interface with an RS-232 level compliant interface.

Some serial implementations link CTS and RTS to remove the need for handshaking. We do not recommend linking CTS and RTS other than for testing and prototyping. If these pins are linked and the host sends data at the point that the BL653 deasserts its RTS signal, there is significant risk that internal receive buffers will overflow, which could lead to an internal processor crash. This will drop the connection and may require a power cycle to reset the module. We recommend that the correct CTS/RTS handshaking protocol be adhered to for proper operation.

Table 17: UART interface

| Signal Name       | Pin No. | I/O | Comments                                                                                       |
|-------------------|---------|-----|------------------------------------------------------------------------------------------------|
| SIO_06 / UART_Tx  | 35      | 0   | SIO_06 (alternative function UART_Tx) is an output, set high (in firmware).                    |
| SIO_08 / UART_Rx  | 29      | I   | SIO_08 (alternative function UART_Rx) is an input, set with internal pull-up (in firmware).    |
| SIO_05 / UART_RTS | 39      | 0   | SIO_05 (alternative function UART_RTS) is an output, set low (in firmware).                    |
| SIO_07 / UART_CTS | 37      | I   | SIO_07 (alternative function UART_CTS) is an input, set with internal pull-down (in firmware). |

The UART interface is also used to load customer developed *smart*BASIC application script.

#### 5.7 USB Interface

BL653 has USB 2.0 FS (Full Speed, 12 Mbps) hardware capability. There is a CDC driver/Virtual UART as well as other USB drivers available via Nordic SDK – such as: usb\_audio, usb\_hid, usb\_generic, usb\_msc (mass storage device).

Table 18: USB interface

| Signal Name | Pin No | 1/0 | Comments                                                                                                                                     |  |  |
|-------------|--------|-----|----------------------------------------------------------------------------------------------------------------------------------------------|--|--|
| D-          | 17     | I/O |                                                                                                                                              |  |  |
| D+          | 15     | I/O |                                                                                                                                              |  |  |
| VBUS        | 24     |     | When using the BL653 VBUS pin (which is mandatory when USB interface is used), Customer MUST connect externally a 4.7uF capacitor to ground. |  |  |
|             |        |     | <b>Note:</b> You MUST power the rest of BL653 module circuitry through the VDD pin (OPTION1) or VDD_HV pin (OPTION2).                        |  |  |

### 5.8 SPI Bus

The SPI interface is an alternate function on SIO pins.

The module is a master device that uses terminals SPI\_MOSI, SPI\_MISO, and SPI\_CLK. SPI\_CS is implemented using any spare SIO digital output pins to allow for multi-dropping.

The SPI interface enables full duplex synchronous communication between devices. It supports a three-wire (SPI\_MOSI, SPI\_MISO, SPI\_SCK,) bidirectional bus with fast data transfers to and from multiple slaves. Individual chip select signals are necessary for each of the slave devices attached to a bus, but control of these is left to the application through use of SIO signals. I/O data is double buffered.

The SPI peripheral supports SPI mode 0, 1, 2, and 3.

Table 19: SPI interfaces

| Table 15: 61 Tiffice Table 5 |    |     |                                                                                                                                              |  |  |
|------------------------------|----|-----|----------------------------------------------------------------------------------------------------------------------------------------------|--|--|
| Signal Name Pin No I/O       |    | 1/0 | Comments                                                                                                                                     |  |  |
| SIO_40/SPI_MOSI              | 32 | 0   | This interface is an alternate function configurable by                                                                                      |  |  |
| SIO_04/AIN2/SPI_MISO         | 34 | I   | smartBASIC. Default in the FW pin 56 and 53 are SIO inputs. SPIOPEN() in                                                                     |  |  |
| SIO_41/SPI_CLK               | 30 | 0   | smartBASIC selects SPI function and changes pin 56 and 53 to outputs (when in SPI master mode).                                              |  |  |
| Any_SIO/SPI_CS               | 54 | I   | SPI_CS is implemented using any spare SIO digital output pins to allow for multi-dropping. On Ezurio devboard SIO_44 (pin54) used as SPI_CS. |  |  |

![](_page_28_Picture_1.jpeg)

#### 5.9 I2C Interface

The I2C interface is an alternate function on SIO pins.

The two-wire interface can interface a bi-directional wired-OR bus with two lines (SCL, SDA) and has master /slave topology. The interface is capable of clock stretching. Data rates of 100 kbps and 400 kbps are supported.

An I2C interface allows multiple masters and slaves to communicate over a shared wired-OR type bus consisting of two lines which normally sit at VDD. The SCL is the clock line which is always sourced by the master and SDA is a bi-directional data line which can be driven by any device on the bus.

**IMPORTANT:** 

It is essential to remember that pull-up resistors on both SCL and SDA lines are not provided in the module and MUST be provided external to the module.

#### Table 20: I2C interface

| Signal Name    |    | 1/0 | Comments                                                                                 |
|----------------|----|-----|------------------------------------------------------------------------------------------|
| SIO_26/I2C_SDA | 36 | I/O | This interface is an alternate function on each pin, configurable by <i>smart</i> BASIC. |
| SIO_27/I2C_SCL | 38 | I/O | I2COPEN() in <i>smart</i> BASIC selects I2C function.                                    |

### 5.10 General Purpose I/O, ADC, PWM, and FREQ

#### 5.10.1 GPIO

The 42 SIO pins are configurable by *smart*BASIC application script or Nordic SDK. They can be accessed individually. Each has the following user configured features:

- Input/output direction
- Output drive strength (standard drive 0.5 mA or high drive 5mA)
- Internal pull-up and pull-down resistors (13 K Ohms typical) or no pull-up/down or input buffer disconnect
- Wake-up from high or low-level triggers on all pins including NFC pins

#### 5.10.2 ADC

The ADC is an alternate function on SIO pins, configurable by *smart*BASIC or Nordic SDK.

The BL653 provides access to 8-channel 8/10/12-bit successive approximation ADC in one-shot mode. This enables sampling up to 8 external signals through a front-end MUX. The ADC has configurable input and reference pre-scaling and sample resolution (8, 10, and 12 bit).

#### 5.10.2.1 Analog Interface (ADC)

Table 21: Analog interface

| Signal Name                         | Pin No | I/O | Comments                                                                                                                                                                                                         |
|-------------------------------------|--------|-----|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| SIO_05/UART_RTS/AIN3 - Analog Input | 39     | 1   | This interface is an alternate function on each pin,                                                                                                                                                             |
| SIO_04/AIN2/SPI_MISO - Analog Input | 34     | 1   | configurable by <i>smart</i> BASIC. AIN configuration selected using GpioSetFunc() function.  Configurable 8, 10, 12-bit resolution.  Configurable voltage scaling 4, 2, 1/1, 1/3, 1/3, 1/4, 1/5, 1/6 (default). |
| SIO_03/AIN1 - Analog Input          | 49     | 1   |                                                                                                                                                                                                                  |
| SIO_02/AIN0 - Analog Input          | 50     | I   |                                                                                                                                                                                                                  |
| SIO_31/AIN7 – Analog Input          | 44     | 1   |                                                                                                                                                                                                                  |
| SIO_30/AIN6 - Analog Input          | 45     | I   | Configurable acquisition time 3uS, 5uS, 10uS (default),                                                                                                                                                          |
| SIO_29/AIN5 - Analog Input          | 48     | I   | - 15uS, 20uS, 40uS.<br>- Full scale input range (VDD)                                                                                                                                                            |
| SIO_28/AIN4 - Analog Input          | 46     | 1   |                                                                                                                                                                                                                  |

#### 5.10.3 PWM Signal Output on Up to 16 SIO Pins

The PWM output is an alternate function on ALL (GPIO) SIO pins, configurable by smartBASIC or the Nordic SDK.

The **PWM output** signal has a frequency and duty cycle property. Frequency is adjustable (up to 1 MHz) and the duty cycle can be set over a range from 0% to 100%.

29

![](_page_29_Picture_1.jpeg)

PWM output signal has a frequency and duty cycle property. PWM output is generated using dedicated hardware in the chipset. There is a trade-off between PWM output frequency and resolution.

For example:

- PWM output frequency of 500 kHz (2 uS) results in resolution of 1:2
- PWM output frequency of 100 kHz (10 uS) results in resolution of 1:10
- PWM output frequency of 10 kHz (100 uS) results in resolution of 1:100
- PWM output frequency of 1 kHz (1000 uS) results in resolution of 1:1000

#### 5.10.4 FREQ Signal Output on Up to 16 SIO Pins

The FREQ output is an alternate function on 16 (GPIO) SIO pins, configurable by smartBASIC or Nordic SDK.

Note: The frequency driving each of the 16 SIO pins is the same but the duty cycle can be independently set for each pin.

FREQ output signal frequency can be set over a range of 0Hz to 4 MHz (with 50% mark-space ratio).

#### 5.11 nRESET Pin

#### Table 22: nRESET pin

|             | r      |     |                                                                                                 |
|-------------|--------|-----|-------------------------------------------------------------------------------------------------|
| Signal Name | Pin No | 1/0 | Comments                                                                                        |
| nRESET      | 19     | I   | BL653 HW reset (active low). Pull the nRESET pin low for minimum 100 mS for the BL653 to reset. |

### 5.12 Two-Wire Interface SWD (JTAG)

The BL653 Firmware hex file consists of four elements:

- smartBASIC runtime engine
- Nordic Softdevice
- Master Bootloader

Ezurio BL653 *smart*BASIC firmware (FW) image part numbers are referenced as w.x.y.z (ex. V30.x.y.z). The BL653 *smart*BASIC runtime engine and Softdevice combined image can be upgraded by the customer over the UART interface.

You also have the option to use the two-wire SWD (JTAG) interface, during production, to clone the file system of a Golden preconfigured BL653 to others using the Flash Cloning process. This is described in the following application note *Flash Cloning for the BL653*. In this case the file system is also part of the .hex file.

| Signal Name | Pin No | I/O | Comments                    |
|-------------|--------|-----|-----------------------------|
| SWDIO       | 1      | 1/0 | Internal pull-up resistor   |
| SWDCLK      | 3      | 1   | Internal pull-down resistor |

The Ezurio development board incorporates an on-board SWD (JTAG) J-link programmer for this purpose. There is also the following SWD (JTAG) connector which allows on-board SWD (JTAG) J-link programmer signals to be routed off the development board. The only requirement is that you should use the following SWD (JTAG) connector on the host PCB.

The SWD (JTAG) connector MPN is as follows:

| Reference | Part     | Description and MPN (Manufacturers Part Number)       |
|-----------|----------|-------------------------------------------------------|
| JP1       | FTSH-105 | Header, 1.27mm, SMD, 10-way, FTSH-105-01-L-DV Samtech |

Note: Reference on the BL653 development board schematic (Figure 7) shows the DVK development schematic wiring only for the SWD (JTAG) connector and the BL653 module JTAG pins.

30

![](_page_30_Picture_1.jpeg)

![](_page_30_Picture_2.jpeg)

Figure 7: BL653 development board schematic

**Note:** The BL653 development board allows Ezurio on-board SWD (JTAG) J-link programmer signals to be routed off the development board by from connector JP1

SWD (JTAG) is required because Nordic SDK applications can only be loaded using the SWED (JTAG) (*smart*BASIC firmware can be loaded using SWD (JTAG) as well as over the UART). We recommend that you use SWD (JTAG) (2-wire SWD interface) to handle future BL653 module firmware upgrades. You **must** wire out the JTAG (2-wire SWD interface) on your host design (see Figure 7, where the following four lines should be wired out – SWDIO, SWDCLK, GND, and VCC). *smart*BASIC firmware upgrades can still be performed over the BL653 UART interface, but this is slower than using the BL653 JTAG (2-wire SWD interface) – (60 seconds using UART vs. 10 seconds when using JTAG).

### 5.13 BL653 Wakeup

#### 5.13.1 Waking Up BL653 From Host

Wake the BL653 from the host using wake-up pins (any SIO pin). You may configure the BL653's wakeup pins via *smart*BASIC to do any of the following:

- Wake up when signal is low
- Wake up when signal is high
- Wake up when signal changes

Refer to the smartBASIC user guide for details. You can access this guide from the Ezurio BL653 product page.

For BL653 wake-up using the Nordic SDK, refer to Nordic infocenter.nordicsemi.com.

#### 5.14 Low Power Modes

The BL653 has three power modes: Run, Standby Doze, and Deep Sleep.

The module is placed automatically in Standby Doze if there are no pending events (when WAITEVENT statement is encountered within a customer's *smart*BASIC script). The module wakes from Standby Doze via any interrupt (such as a received character on the UART Rx line). If the module receives a UART character from either the external UART or the radio, it wakes up.

Deep sleep is the lowest power mode. Once awakened, the system goes through a system reset.

For different Nordic power modes using the Nordic SDK, refer to Nordic infocenter.nordicsemi.com.

### 5.15 Temperature Sensor

The on-silicon temperature sensor has a temperature range greater than or equal to the operating temperature of the device. Resolution is  $0.25^{\circ}$ C degrees. The on-silicon temperature sensor accuracy is  $\pm 5^{\circ}$ C.

To read temperature from on-silicon temperature sensor (in tenth of centigrade, so 23.4°C is output as 234) using smartBASIC:

- In command mode, use ATI2024

  OR
- From running a smartBASIC application script, use SYSINFO(2024)

![](_page_31_Picture_1.jpeg)

### 5.16 Security/Privacy

#### 5.16.1 Random Number Generator

Exposed via an API in *smart*BASIC (see *smart*BASIC documentation available from the BL653 product page). The **rand()** function from a running *smart*BASIC application returns a value.

For Nordic related functionality, visit Nordic infocenter.nordicsemi.com

#### 5.16.2 AES Encryption/Decryption

Exposed via an API in *smart*BASIC (see *smart*BASIC documentation available from the BL653 product page). Function called **aesencrypt** and **aesdecrypt**.

For Nordic related functionality, visit Nordic infocenter.nordicsemi.com

#### 5.16.3 Readback Protection

The BL653 supports readback protection capability that disallows the reading of the memory on the nRF52833 using a JTAG interface. Available via *smart*BASIC or the Nordic SDK.

#### 5.16.4 Elliptic Curve Cryptography

The BL653 offers a range of functions for generating public/private keypair, calculating a shared secret, as well as generating an authenticated hash. Available via *smart*BASIC or the Nordic SDK.

## 6 Optional External 32.768 kHz Crystal

This is not required for normal BL653 module operation.

The BL653 uses the on-chip 32.76 kHz RC oscillator (LFCLK) by default (which has an accuracy of ±500 ppm) which requires regulator calibration (every eight seconds) to within ±500 ppm.

You can connect an optional external high accuracy (±20 ppm) 32.768 kHz crystal (and associated load capacitors) to the BL653SIO\_01/XL2 (pin 41) and SIO\_00/XL1 (pin 42) to provide improved protocol timing and to help with radio power consumption in the system standby doze/deep sleep modes by reducing the time that the RX window needs to be open.

Table 23 compares the current consumption difference between RC and crystal oscillator.

Table 23: Comparing current consumption difference between BL653 on-chip RC 32.76 kHz oscillator and optional external crystal (32.768kHz) based oscillator

|                                                                                                             | BL653 On-chip 32.768 kHz RC Oscillator<br>(±500 ppm) LFRC                                                                              | Optional External Higher Accuracy (±20 ppm)<br>32.768 kHz Crystal-based Oscillator LFXO |
|-------------------------------------------------------------------------------------------------------------|----------------------------------------------------------------------------------------------------------------------------------------|-----------------------------------------------------------------------------------------|
| Current Consumption of 32.768 kHz Block                                                                     | 0.7 uA                                                                                                                                 | 0.23 uA                                                                                 |
| Standby Doze Current<br>(SYSTEM ON IDLE +full<br>RAM retention +RTC run<br>current (0uA) + LFRC or<br>LFXO) | 2.5 uA                                                                                                                                 | 2.03 uA                                                                                 |
| Calibration                                                                                                 | Calibration required regularly (default eight seconds interval).                                                                       | Not applicable                                                                          |
|                                                                                                             | Calibration takes 32 ms; with DCDC used, the total charge of a calibration event is 15.1 uC (or 18.8uC with DCDC disabled).            |                                                                                         |
|                                                                                                             | The average current consumed by the calibration depends on the calibration interval and can be calculated using the following formula: |                                                                                         |

![](_page_32_Picture_1.jpeg)

#### BL653 On-chip 32.768 kHz RC Oscillator (±500 ppm) LFRC

Optional External Higher Accuracy (±20 ppm) 32.768 kHz Crystal-based Oscillator LFXO

CAL\_charge/CAL\_interval - The lowest calibration interval (0.25 seconds) provides an average current of (DCDC enabled):

#### 15.1uC/0.25s = 60.4uA

To get the 250-ppm accuracy, the BLE stack specification states that a calibration interval of 8 seconds is enough. This gives an average current

#### 15.1uC/8s = 1.89 uA

Added to the LFRC run current and Standby Doze (IDLE) base current shown above results in a total average current of:

LFRC + CAL = 2.5 + 1.89 = 4.39 uA

Total 4.39 uA 2.03 uA

Summary

- · Low current consumption
- Accuracy 250 ppm

- Lowest current consumption
- Needs external crystal
- High accuracy (depends on the crystal, usually 20 ppm)

Table 24: Optional external 32.768 kHz crystal specification

| Optional external 32.768kHz crystal                                                               | Min    | Тур          | Max      |
|---------------------------------------------------------------------------------------------------|--------|--------------|----------|
| Crystal Frequency                                                                                 | -      | 32.768 kHz   | -        |
| Frequency tolerance requirement of BLE stack                                                      | -      | -            | ±500 ppm |
| Load Capacitance                                                                                  | -      | -            | 12.5 pF  |
| Shunt Capacitance                                                                                 | -      | -            | 2 pF     |
| Equivalent series resistance                                                                      | -      | -            | 100 kOhm |
| Drive level                                                                                       | -      | -            | 1uW      |
| Input capacitance on XL1 and XL2 pads                                                             | -      | 4 pF         | -        |
| Run current for 32.768 kHz crystal based oscillator                                               | -      | 0.23 uA      | -        |
| Start-up time for 32.768 kHz crystal based oscillator                                             | -      | 0.25 seconds | -        |
| Peak to peak amplitude for external low swing clock input signal must not be outside supply rails | 200 mV | -            | 1000 mV  |

Be sure to tune the load capacitors on the board design to optimize frequency accuracy (at room temperature) so it matches that of the same crystal standalone, Drive Level (so crystal operated within safe limits) and oscillation margin (R<sub>neg</sub> is at least 3 to 5 times ESR) over the operating temperature range.

#### 453-00039 On-board PCB Antenna Characteristics

The 453-00039 on-board PCB trace monopole antenna radiated performance depends on the host PCB layout.

The BL653 development board was used for BL653 development and the 453-00039 PCB antenna performance evaluation. To obtain similar performance, follow guidelines in section PCB Layout on Host PCB for the 453-00039 to allow the on-board PCB antenna to radiate and reduce proximity effects due to nearby host PCB GND copper or metal covers.

|       | f(MHz) | Antenna<br>P (dBm) | TRP<br>(dBm) | Max EIRP<br>(dBm) | Efficienc<br>y (dB) | Gain<br>(dBi) |
|-------|--------|--------------------|--------------|-------------------|---------------------|---------------|
| BL653 | 2402   | 0.00               | -3.49        | 0.13              | -3.5                | 0.1           |
|       | 2440   | 0.00               | -3.89        | -0.10             | -3.9                | -0.1          |
|       | 2480   | 0.00               | -4.53        | -0.95             | -4.5                | -1.0          |

![](_page_33_Picture_1.jpeg)

**Notes:** Antenna P = Antenna input power (set).

TRP = Total Radiated Power (measured).

EIRP = Equivalent Isotropic (ideal) Radiated Power (measured).

Efficiency = TRP - Antenna P (calculated).
Gain = Max. EIRP - Antenna P (calculated).

![](_page_33_Picture_6.jpeg)

![](_page_33_Picture_7.jpeg)

Figure 8: Radiated Performance Measurement Setup

#### 6.1.1 2402MHz Radiated Performance

#### **EIRP Azimuth Cut**

![](_page_33_Figure_11.jpeg)

Power Summary at 2402 (MHz) min: -14.2 (dBm) max: 0.1 (dBm) avg: -3.6 (dBm)

#### 3D Plots:

![](_page_34_Picture_1.jpeg)

![](_page_34_Figure_2.jpeg)

Figure 9: 2402MHz Horizontal, Vertical, and Total Patterns

#### 6.1.2 2440MHz Radiated Performance

#### **EIRP Azimuth Cut**

![](_page_34_Figure_6.jpeg)

Power Summary at 2440 (MHz) min: -15.4 (dBm) max: -0.4 (dBm) avg: -4.0 (dBm)

#### 3D Plots:

![](_page_34_Figure_9.jpeg)

Figure 10: 2440MHz Horizontal, Vertical, and Total Patterns

![](_page_35_Picture_1.jpeg)

#### 6.1.3 2480MHz Radiated Performance

#### **EIRP Azimuth Cut**

![](_page_35_Figure_4.jpeg)

Power Summary at 2480 (MHz) min: -16.5 (dBm) max: -1.1 (dBm) avg: -4.7 (dBm)

#### 3D Plots:

![](_page_35_Figure_7.jpeg)

Figure 11: 2480MHz Horizontal, Vertical, and Total Patterns

![](_page_36_Picture_1.jpeg)

#### 6.1.4 451-00001 Return Loss Measurement

![](_page_36_Figure_3.jpeg)

Figure 12: 453-00039 on-board PCB antenna performance (Antenna Gain and S11 - whilst 453-00039 module sitting on Devboard 453-00039-K1)

# 7 Hardware Integration Suggestions

#### 7.1 Circuit

The BL653 is easy to integrate, requiring no external components on your board apart from those which you require for development and in your end application.

The following are suggestions for your design for the best performance and functionality.

Checklist (for Schematic):

#### • BL653 power supply options:

**Option 1** – Normal voltage power supply mode entered when the external supply voltage is connected to both the VDD and VDDH pins (so that VDD equals VDD\_HV). Connect external supply within range 1.7V to 3.6V range to BL653 VDD and VDD\_HV pins.

OR

**Option 2** - High voltage mode power supply mode (using BL653 VDD\_HV pin) entered when the external supply voltage in ONLY connected to the VDDH pin and the VDD pin is not connected to any external voltage supply. Connect external supply within range 2.5V to 5.5V range to BL653 VDD\_HV pin. BL653 VDD pin left unconnected.

In High Voltage mode (VDD\_HV), No external current draw (from VDD pin) is allowed (limitation of nRF52833 chipset).

For either option, if you use USB interface then the BL653 VBUS pin must be connected to external supply within the range 4.35V to 5.5V. When using the BL653 VBUS pin, you MUST externally fit a 4.7uF to ground.

**Note:** To avoid surpassing the maximum die temperature (110 °C), it is important to minimize current consumption when operating in the extended operating temperature conditions (+85 °C to +105 °C). It is therefore recommended to use the module device on Normal Voltage mode with DCDC (REG1) enabled.

![](_page_37_Picture_1.jpeg)

External power source should be within the operating range, rise time and noise/ripple specification of the BL653. Add decoupling capacitors for filtering the external source. Power-on reset circuitry within BL653 series module incorporates brown-out detector, thus simplifying your power supply design. Upon application of power, the internal power-on reset ensures that the module starts correctly.

#### • VDD and coin-cell operation

With a built-in DCDC (operating range 1.7V to 3.6V), that reduces the peak current required from a coin-cell, making it easier to use with a coin-cell.

#### • AIN (ADC) and SIO pin IO voltage levels

BL653 SIO voltage levels are at VDD. Ensure input voltage levels into SIO pins are at VDD also (if VDD source is a battery whose voltage will drop). Ensure ADC pin maximum input voltage for damage is not violated.

#### AIN (ADC) impedance and external voltage divider setup

If you need to measure with ADC a voltage higher than 3.6V, you can connect a high impedance voltage divider to lower the voltage to the ADC input pin.

#### JTAG (SWD)

This is REQUIRED as Nordic SDK applications can only be loaded using the SWD (JTAG) (*smart*BASIC firmware can be loaded using the JTAG as well as the UART).

We recommend that you use JTAG (2-wire interface) to handle future BL653 module firmware upgrades. You MUST wire out the JTAG (2-wire interface) on your host design (see Figure 7, where four lines should be wired out, namely SWDIO, SWDCLK, GND and VCC). Firmware upgrades can still be performed over the BL653 UART interface, but this is slower (60 seconds using UART vs. 10 seconds when using JTAG) than using the BL653 JTAG (2-wire interface).

JTAG may be used if you intend to use Flash Cloning during production to load *smart*BASIC scripts.

#### UART

Required for loading your *smart*BASIC application script during development (or for subsequent firmware upgrades (except JTAG for FW upgrades and/or Flash Cloning of the *smart*BASIC application script). Add connector to allow interfacing with UART via PC (UART-RS232 or UART-USB).

#### • UART\_RX and UART\_CTS

SIO\_08 (alternative function UART\_RX) is an input, set with internal weak pull-up (in firmware). The pull-up prevents the module from going into deep sleep when UART\_RX line is idling.

SIO\_07 (alternative function UART\_CTS) is an input, set with internal weak pull-down (in firmware). This pull-down ensures the default state of the UART\_CTS will be asserted which means can send data out of the UART\_TX line. Ezurio recommends that UART\_CTS be connected.

#### • nAutoRUN pin and operating mode selection

nAutoRUN pin needs to be externally held high or low to select between the two BL653 operating modes at power-up:

- Self-contained Run mode (nAutoRUN pin held at 0V).
- Interactive / development mode (nAutoRUN pin held at VDD).
   Make provision to allow operation in the required mode. Add jumper to allow nAutoRUN pin to be held high or low (BL653 has internal 13K pull-down by default) OR driven by host GPIO.

#### I2C

It is essential to remember that pull-up resistors on both I2C\_SCL and I2C\_SDA lines are not provided in the BL653 module and MUST be provided external to the module as per I2C standard.

#### SPI

Implement SPI chip select using any unused SIO pin within your smartBASIC application script or Nordic application then SPI\_CS is controlled from the software application allowing multi-dropping.

#### SIO pin direction

BL653 modules shipped from production with *smart*BASIC FW, all SIO pins (with default function of DIO) are mostly digital inputs (see Pin Definitions Table2). Remember to change the direction SIO pin (in your *smart*BASIC application script) if that particular pin is wired to a device that expects to be driven by the BL653 SIO pin configured as an output. Also, these SIO pins have the internal pull-up or pull-down resistorenabled by default in firmware (see Pin Definitions Table 2). This was done to avoid floating inputs, which can cause current consumption in low power modes (e.g. StandbyDoze) to drift with time. You can disable the PULL-UP or Pull-down through their *smart*BASIC application.

![](_page_38_Picture_1.jpeg)

Note: Internal pull-up, pull down takes current from VDD.

#### • SIO\_02 pin and OTA smartBASIC application download feature

SIO\_02 is an input, set with internal pull-down (in FW). Refer to latest firmware release documentation on how SIO\_02 is used for Over the Air smartBASIC application download feature. The SIO\_02 pin must be pulled high externally to enable the feature. Decide if this feature is required in production. When SIO\_02 is high, ensure nAutoRun is NOT high at same time; otherwise you cannot load the smartBASIC application script.

#### • NFC antenna connector

To make use of the Ezurio flexi-PCB NFC antenna. fit connector:

- Description FFC/FPC Connector, Right Angle, SMD/90d, Dual Contact, 1.2 mm Mated Height
- Manufacturer Molex
- Manufacturers Part number 512810594

Add tuning capacitors of 300 pF on NFC1 pin to GND and 300 pF on NFC2 pins to GND if the PCB track length is similar as development board.

#### • nRESET pin (active low)

Hardware reset. Wire out to push button or drive by host. By default module is out of reset when power applied to VCC pins.

#### • Optional External 32.768kHz crystal

If the optional external 32.768kHz crystal is needed, then use a crystal that meets specification and add load capacitors whose values should be tuned to meet all specification for frequency and oscillation margin.

#### • SIO\_38 special function pin

This is for future use by Ezurio. It is currently a Do Not Connect pin if using the *smart*BASIC FW.

#### BL653 module RF pad variant 453-00041

If using the BL653 module RF pad variant (453-00041), MUST copy the 50-Ohms GCPW RF track design, MUST add series 2nH RF inductor (Murata LQG15HN2N0B02# or Murata LQG15HS2N0B02) and RF connector IPEX MHF4 Receptacle (MPN: 20449-001E) Detailed in the following section: 50-Ohms RF Trace and RF Match Series 2nH RF Inductor on Host PCB for BL653 RF Pad Variant (453-00041).

### 7.2 PCB Layout on Host PCB - General

#### Checklist (for PCB):

- MUST locate BL653 module close to the edge of PCB (mandatory for the 453-00039 for on-board PCB trace antenna to radiate properly).
- Use solid GND plane on inner layer (for best EMC and RF performance).
- All module GND pins MUST be connected to host PCB GND.
- Place GND vias close to module GND pads as possible.
- Unused PCB area on surface layer can flooded with copper but place GND vias regularly to connect the copper flood to the inner GND plane. If GND flood copper is on the bottom of the module, then connect it with GND vias to the inner GND plane.
- Route traces to avoid noise being picked up on VDD, VDDH, VBUS supply and AIN (analogue) and SIO (digital) traces.
- Ensure no exposed copper is on the underside of the module (refer to land pattern of BL653 development board).

### 7.3 PCB Layout on Host PCB for the 453-00039

#### 1.1.1 Antenna Keep-Out on Host PCB

The 453-00039 has an integrated PCB trace antenna and its performance is sensitive to host PCB. It is critical to locate the 453-00039 on the edge of the host PCB (or corner) to allow the antenna to radiate properly. Refer to guidelines in section *PCB land pattern and antenna keep-out area for the 453-00039*. Some of those guidelines repeated below.

- Ensure there is no copper in the antenna keep-out area on any layers of the host PCB. Keep all mounting hardware and metal clear of the area to allow proper antenna radiation.
- For best antenna performance, place the 453-00039 module on the edge of the host PCB, preferably in the edge center.

![](_page_39_Picture_1.jpeg)

- The BL653 development board has the 453-00039 module on the edge of the board (not in the corner). The antenna keep-out area is defined by the BL653 development board which was used for module development and antenna performance evaluation is shown in Figure 13, where the antenna keep-out area is ~5 mm wide, ~39.95 mm long; with PCB dielectric (no copper) height ~1 mm sitting under the 453-00039 PCB trace antenna.
- The 453-00039 module on-board PCB trace antenna is tuned when the 453-00039 is sitting on development board (host PCB) with size of 125 mm x 85 mm x 1mm.
- A different host PCB thickness dielectric will have small effect on antenna.
- The antenna-keep-out defined in the Host PCB Land Pattern and Antenna Keep-out for the 453-00039 section.
- Host PCB land pattern and antenna keep-out for the BL653 applies when the 453-00039 is placed in the edge of the host PCB preferably in the edge center. Figure 13 shows an example.

![](_page_39_Picture_7.jpeg)

Figure 13: PCB trace Antenna keep-out area (shown in red), corner of the BL653 development board for the 453-00039 module.

#### Antenna Keep-out Notes:

| Note 1 | The BL653 module is | placed on the edge, preferably | y edge centre of the host PCB. |
|--------|---------------------|--------------------------------|--------------------------------|
|        |                     |                                |                                |

Note 2 Copper cut-away on all layers in the *Antenna Keep-out* area under the 453-00039 on host PCB.

![](_page_40_Picture_1.jpeg)

#### 7.3.1 Antenna Keep-Out and Proximity to Metal or Plastic

#### Checklist (for metal /plastic enclosure):

- Minimum safe distance for metals without seriously compromising the antenna (tuning) is 40 mm top/bottom and 30 mm left or right.
- Metal close to the 453-00039 PCB trace monopole antenna (bottom, top, left, right, any direction) will have degradation on the antenna performance. The amount of that degradation is entirely system dependent, meaning you will need to perform some testing with your host application.
- Any metal closer than 20 mm will begin to significantly degrade performance (S11, gain, radiation efficiency).
- It is best that you test the range with a mock-up (or actual prototype) of the product to assess effects of enclosure height (and materials, whether metal or plastic).

# 7.4 50-Ohms RF Trace and RF Match Series 2nH RF Inductor on Host PCB for BL653 RF Pad Variant (453-00041)

To use an external antenna, you need a BL653 module variant with RF pad (453-00041) and 50-Ohm RF trace (GCPW or Grounded Coplanar Waveguide) from RF\_pad (pin 72) of the module (BL653 453-00041) to RF antenna connector (IPEX MHF4) on host PCB. On this RF path, you *must* use a 2nH series RF inductor. The BL653 module GND pin 0 and GND pin 66 are used to support GCPW 50-Ohm RF trace.

#### Checklist for SCH

- MUST fit 2 nH RF inductor (R144) in series. RF inductor part number is Murata LQG15HN2N0B02# or Murata LQG15HS2N0B02# with 0402 body size. https://www.murata.com/en-eu/products/productdetail.aspx?partno=LQG15HN2N0B02%23 https://www.murata.com/en-eu/products/productdetail.aspx?partno=LQG15HS2N0B02%23
- MUST fit RF connector IPEX MHF4 Receptacle (MPN: 20449-001E), https://www.i-pex.com/product/mhf-4-smt#!

![](_page_40_Figure_13.jpeg)

Figure 14: BL6653 RF pad variant (453-00041) Host PCB 50-Ohm RF trace schematic with series 2nH inductor, RF connector

![](_page_41_Picture_1.jpeg)

![](_page_41_Figure_2.jpeg)

Layer2 (RF GND) and Layer2 copper cut-out under RF connector

![](_page_41_Figure_4.jpeg)

Figure 15: 50-Ohm RF trace design (Layer1 and Layer2) on BL653 development board (or host PCB) for use with BL653 (453-00041) module

![](_page_42_Picture_1.jpeg)

#### Checklist for PCB:

MUST use a 50-Ohm RF trace (GCPW, that is Grounded Coplanar Waveguide) from RF\_pad (pin72) of the module (BL653 453-00041) to RF antenna connector (IPEX MHF4) on host PCB.

To ensure regulatory compliance, MUST follow exactly the following considerations for 50-Ohms RF trace design and test verification:

![](_page_42_Picture_5.jpeg)

|                                      | Thickness | Dielectric  |                                 |
|--------------------------------------|-----------|-------------|---------------------------------|
|                                      | mil       | Constant Er |                                 |
| Solder Mask                          | 0.4       | 3.5         | 0                               |
| Layer1 Copper 0.5 oz+plating (Note1) | 1.3       |             | Stack up for 50<br>Ohms GCPW RF |
| Prepreg (2113)                       | 4         | 4.1         | track.                          |
| Layer2 Copper 1 oz                   | 1.4       | _           | Hack.                           |
| Core 0.6 mm                          | 24        | 4.4         |                                 |
| Layer3 Copper 1 oz                   | 1.4       |             |                                 |
| Prepreg (2113)                       | 4         | 4.1         |                                 |
| Layer1 Copper 0.5 oz+plating         | 1.3       |             |                                 |
| Solder Mask                          | 0.4       | 3.5         |                                 |

Note 1: The plating (ENIG) above base 0.5 oz copper is not listed, but plating expected to be ENIG.

#### Figure 16: BL653 development board PCB stack-up and L1 to L2 50-Ohms Grounded CPW RF trace design

- The 50-Ohms RF trace design MUST be Grounded Coplanar Waveguide (GCPW) with
  - Layer1 RF track width (W) of 6.5 mil and
  - Layer1 gap (G) to GND of 10mil and where the
  - Layer1 to Layer 2 dielectric thickness (H) MUST be 4 mil (dielectric constant Er 4.1).
  - Further the Layer1 base copper must be 0.5-ounce base copper (that is 0.7 mil) plus the plating and
  - Layer1 MUST be covered by solder mask of 0.4 mil thickness (dielectric constant Er 3.5).
- The 50-Ohms RF trace design MUST follow the PCB stack-up shown in Figure 16. (Layer1 to Layer2 thickness MUST be identical to the BL653 development board).
- The 50-Ohms RF track should be a controlled-impedance trace e.g. ±10%.
- The 50-Ohms RF trace length MUST be identical (as seen in Figure 16) (262.09mil) to that on the BL653 development board from BL653 module RF pad (pin72) to the RF connector IPEX MHF4 Receptable (MPN: 20449-001E).
- Place GND vias regularly spaced either side of 50-Ohms RF trace to form GCPW (Grounded coplanar waveguide) transmission line as shown in Figure 12 and use BL653 module GND pin0 and GND pin66.
- Cut away copper on Layer 2 GND layer under the RF connector IPEX MHF4 Receptable, as seen in Figure 16. This is to reduce RF detuning the 500hms of the RF connector (J49) when it sits on the PCB.
- Cut away copper on Layer 2 GND layer under the BL653 module RF pad (pin72), identical to the BL653 development board as seen in Figure 16.
- Use spectrum analyzer to confirm the radiated (and conducted) signal is within the certification limit.

![](_page_43_Picture_1.jpeg)

### 7.5 External Antenna Integration with 453-00041

Please refer to the regulatory sections for FCC, ISED, EU, RCM, KC, and Japan for details of use of BL653-with external antennas in each regulatory region.

The BL653 family has been designed to operate with the below external antennas (with a maximum gain of 2.0 dBi). The required antenna impedance is 50 ohms. See Table 25. External antennas improve radiation efficiency.

Table 25: External antennas for the BL653

|                                | Model                 | Ezurio<br>Part Number | Туре       | Connector | Peak Gain        |                  |
|--------------------------------|-----------------------|-----------------------|------------|-----------|------------------|------------------|
| Manufacturer                   |                       |                       |            |           | 2400-2500<br>MHz | 2400-2480<br>MHz |
| Ezurio (Laird<br>Connectivity) | NanoBlue              | EBL2400A1-<br>10MH4L  | PCB Dipole | IPEX MHF4 | 2 dBi            | -                |
| Ezurio (Laird<br>Connectivity) | FlexPIFA              | 001-0022              | PIFA       | IPEX MHF4 | -                | 2 dBi            |
| Mag.Layers                     | EDA-8709-2G4C1-B27-CY | 0600-00057            | Dipole     | IPEX MHF4 | 2 dBi            | -                |
| Ezurio (Laird<br>Connectivity) | mFlexPIFA             | EFA2400A3S-<br>10MH4L | PIFA       | IPEX MHF4 | -                | 2 dBi            |
| Ezurio (Laird<br>Connectivity) | Ezurio NFC            | 0600-00061            | NFC        | N/A       | -                | -                |

### 8 Mechanical Details

#### 8.1 BL653 Mechanical Details

![](_page_43_Picture_9.jpeg)

![](_page_43_Picture_10.jpeg)

![](_page_43_Figure_11.jpeg)

![](_page_44_Picture_1.jpeg)

### **Tolerances**

Board Outline:+/-0.13mm Board Height:+/-0.15mm

Figure 17: BL653 mechanical drawing

![](_page_44_Picture_5.jpeg)

![](_page_44_Picture_6.jpeg)

Figure 18: Mechanical Details - Integrated Antenna

![](_page_44_Picture_8.jpeg)

![](_page_44_Picture_9.jpeg)

Figure 19: Mechanical Details - Trace Pin

Development kit schematics can be found in the software downloads tab of the BL653 product page.

![](_page_45_Picture_1.jpeg)

### 8.2 Host PCB Land Pattern and Antenna Keep-out for the 453-00039

![](_page_45_Figure_3.jpeg)

Figure 20: Land pattern and Keep-out for the 453-00039

All dimensions are in millimeters.

#### Host PCB Land Pattern and Antenna Keep-out for the 453-00039 Notes:

| Note 1 | Ensure there is no copper in the antenna 'keep out area' on any layers of the host PCB. Also keep all mounting hardware or any metal clear of the area (Refer to 6.3.2) to reduce effects of proximity detuning the antenna and to help antenna radiate properly.                                                                               |
|--------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| Note 2 | For the best on-board antenna performance, the module 453-00039 MUST be placed on the edge of the host PCB and preferably in the edge centre and host PCB, the antenna "Keep Out Area" is extended (see Note 4).                                                                                                                                |
| Note 3 | BL653 development board has the 453-00039 placed on the edge of the PCB board (and not in corner) for that the Antenna keep out area is extended down to the corner of the development board, see section <i>PCB Layout on Host PCB for the 453-00039</i> , Figure 20. This was used for module development and antenna performance evaluation. |
| Note 4 | Ensure that there is no exposed copper under the module on the host PCB.                                                                                                                                                                                                                                                                        |
| Note 5 | You may modify the PCB land pattern dimensions based on their experience and/or process capability.                                                                                                                                                                                                                                             |

![](_page_46_Picture_1.jpeg)

# 9 Application Note for Surface Mount Modules

#### 9.1 Introduction

Ezurio's surface mount modules are designed to conform to all major manufacturing guidelines. This application note is intended to provide additional guidance beyond the information that is presented in the User Manual. This Application Note is considered a living document and will be updated as new information is presented.

The modules are designed to meet the needs of several commercial and industrial applications. They are easy to manufacture and conform to current automated manufacturing processes.

### 9.2 Shipping

#### 9.2.1 Tape and Reel Package Information

![](_page_46_Figure_8.jpeg)

Figure 21: Reel specifications

![](_page_47_Picture_1.jpeg)

![](_page_47_Figure_2.jpeg)

Figure 22: Tape specifications (Module Orientation – Antenna end of Module away from Tape Hole side)

There are 1,000 x BL653 modules taped in a reel (and packaged in a pizza box) and five boxes per carton (5000 modules per carton). Reel, boxes, and carton are labeled with the appropriate labels. See Carton Contents for more information.

#### 9.2.2 Carton Contents

The following are the contents of the carton shipped for the BL653 modules.

![](_page_48_Picture_1.jpeg)

![](_page_48_Figure_2.jpeg)

Figure 23: 453-00039R carton contents

![](_page_49_Picture_1.jpeg)

![](_page_49_Figure_2.jpeg)

Figure 24: 453-00041R carton contents

#### 9.2.3 Module Orientation in Cavity

![](_page_49_Picture_5.jpeg)

Figure 25: Module Orientation

#### 9.2.4 Labeling

The following labels are located on the antistatic bag:

![](_page_50_Picture_1.jpeg)

M/N: BL653 Rev 3 P/N:453-00039R D/C:SSYYWWD Q'TY:XXXX PCS BOX\_ID:BXXXXXYMDXXXXXX 

M/N: BL653 Rev 3 P/N:453-00041R D/C:SSYYWWD Q'TY:XXXX PCS BOX\_ID:BXXXXXYMDXXXXXX 

![](_page_50_Picture_4.jpeg)

Figure 266: Antistatic bag labels

The following package label is located on the box:

M/N: BL653 Rev 3 P/N:453-00039R D/C:SSYYWWD Q'TY:XXXX PCS BOX\_ID:BXXXXXYMDXXXXXX 

M/N: BL653 Rev 3. P/N:453-00041R D/C:SSYYWWD Q'TY:XXXX PCS BOX ID:BXXXXXYMDXXXXXX

Figure 27: Box package label

The following package label is located on the adjacent sides of the master carton:

![](_page_50_Figure_11.jpeg)

![](_page_50_Figure_13.jpeg)

Figure 27: Master carton label

### 9.3 Reflow Parameters

Prior to any reflow, it is important to ensure the modules were packaged to prevent moisture absorption. New packages contain desiccate (to absorb moisture) and a humidity indicator card to display the level maintained during storage and shipment. If directed to bake units on the card, see Table 26 and follow instructions specified by IPC/JEDEC J-STD-033. A copy of this standard is available from the JEDEC website: http://www.jedec.org/sites/default/files/docs/jstd033b01.pdf

Any modules not manufactured before exceeding their floor life should be re-packaged with fresh desiccate and a new humidity indicator card. Floor life for MSL (Moisture Sensitivity Level) 4 devices is 72 hours in ambient environment ≤30°C/60%RH.

![](_page_51_Picture_1.jpeg)

Table 26: Recommended baking times and temperatures

| MSL | 125°C        |                  | 90°C/≤5%RH   |                  | 40°C/≤5%RH   |                  |
|-----|--------------|------------------|--------------|------------------|--------------|------------------|
|     | Baking Temp. |                  | Baking Temp. |                  | Baking Temp. |                  |
|     | Saturated    | Floor Life Limit | Saturated    | Floor Life Limit | Saturated    | Floor Life Limit |
|     | @ 30°C/85%   | + 72 hours       | @ 30°C/85%   | + 72 hours       | @ 30°C/85%   | + 72 hours @     |
|     |              | @ 30°C/60%       |              | @ 30°C/60%       |              | 30°C/60%         |
| 4   | 11 hours     | 7 hours          | 37 hours     | 23 hours         | 15 days      | 9 days           |

Ezurio surface mount modules are designed to be easily manufactured, including reflow soldering to a PCB. Ultimately it is the responsibility of the customer to choose the appropriate solder paste and to ensure oven temperatures during reflow meet the requirements of the solder paste. Ezurio surface mount modules conform to J-STD-020D1 standards for reflow temperatures.

Important:

During reflow, modules should not be above 260° and not for more than 30 seconds. In addition, we recommend that the BL653 module **does not** go through the reflow process more than one time; otherwise the BL653 internal component soldering may be impacted.

![](_page_51_Figure_7.jpeg)

Figure 28: Recommended reflow temperature

 $Temperatures \ should \ not \ exceed \ the \ minimums \ or \ maximums \ presented \ in \ {\color{red} Table \ 27}.$ 

Table 27: Recommended maximum and minimum temperatures

| Specification                      | Value  | Unit     |
|------------------------------------|--------|----------|
| Temperature Inc./Dec. Rate (max)   | 1~3    | °C / Sec |
| Temperature Decrease rate (goal)   | 2-4    | °C / Sec |
| Soak Temp Increase rate (goal)     | .5 - 1 | °C / Sec |
| Flux Soak Period (Min)             | 70     | Sec      |
| Flux Soak Period (Max)             | 120    | Sec      |
| Flux Soak Temp (Min)               | 150    | °C       |
| Flux Soak Temp (max)               | 190    | °C       |
| Time Above Liquidous (max)         | 70     | Sec      |
| Time Above Liquidous (min)         | 50     | Sec      |
| Time In Target Reflow Range (goal) | 30     | Sec      |
| Time At Absolute Peak (max)        | 5      | Sec      |
| Liquidous Temperature (SAC305)     | 218    | °C       |

![](_page_52_Picture_1.jpeg)

| Specification                   | Value | Unit |
|---------------------------------|-------|------|
| Lower Target Reflow Temperature | 240   | °C   |
| Upper Target Reflow Temperature | 250   | °C   |
| Absolute Peak Temperature       | 260   | °C   |

![](_page_53_Picture_1.jpeg)

# 10 Regulatory

**Note:** For complete regulatory information, refer to the BL653 Regulatory Information document which is also available from the BL653 product page.

The BL653 holds current certifications in the following countries:

| Country/Region | Regulatory ID |
|----------------|---------------|
| USA (FCC)      | SQGBL653      |
| EU             | N/A           |
| Canada (ISED)  | 3147A-BL653   |
| Japan (MIC)    | 201-200063    |
| Korea (KC)     | R-C-L7C-BL653 |
| Australia      | N/A           |
| New Zealand    | N/A           |

# 11 Ordering Information

| Part Number  | Product Description                                                                 |
|--------------|-------------------------------------------------------------------------------------|
| 453-00039R   | BLE module (Nordic nRF52833) – Integrated antenna (Tape/Reel)                       |
| 453-00041R   | BLE module (Nordic nRF52833) - Trace pin (Tape/Reel)                                |
| 453-00039C   | BLE module (Nordic nRF52833) – Integrated antenna (Cut Tape)                        |
| 453-00041C   | BLE module (Nordic nRF52833) – Trace pin (Cut Tape)                                 |
| 453-00039-K1 | Development Kit for Bluetooth +802.15.4 + NFC module – Integrated antenna           |
| 453-00041-K1 | Development Kit for Bluetooth +802.15.4 + NFC module – Trace pin (External antenna) |

![](_page_54_Picture_1.jpeg)

# 12 Bluetooth Qualification process

#### 12.1 Overview

The Bluetooth Qualification Process promotes global product interoperability and reinforces the strength of the Bluetooth® brand and ecosystem to the benefit of all Bluetooth SIG members. The Bluetooth Qualification Process helps member companies ensure their products that incorporate Bluetooth technology comply with the Bluetooth Patent & Copyright License Agreement and the Bluetooth Trademark License Agreement (collectively, the Bluetooth License Agreement) and Bluetooth Specifications.

The Bluetooth Qualification Process is defined by the Qualification Program Reference Document (QPRD) v3.

To demonstrate that a product complies with the Bluetooth Specification(s), each member must for each of its products:

- Identify the product, the design included in the product, the Bluetooth Specifications that the design implements, and the features of each implemented specification
- Complete the Bluetooth Qualification Process by submitting the required documentation for the product under a user account belonging to your company

The Bluetooth Qualification Process consists of the phases shown below:

![](_page_54_Picture_10.jpeg)

To complete the Qualification Process the company developing a Bluetooth End Product shall be a member of the Bluetooth SIG. To start the application please use the following link: Apply for Adopter Membership

### 12.2 Scope

This guide is intended to provide guidance on the Bluetooth Qualification Process for End Products that reference a single existing design, that has not been modified, (refer to Section 3.2.1 of the Qualification Program Reference Document v3).

This option applies to a Member qualifying a Product that includes an existing Design that has a DN, QDID, or DID and that Design has not been modified (e.g., rebranding a Qualified Product from another Member). The Design identified by the DN, QDID, or DID may only implement Bluetooth Specifications that are active or deprecated at the time of Submission. No modifications may be made to the Design, including changes to the ICS Form.

Changes to the Product outside the Design are allowed, including:

- Enabling technologies
- Changes to the industrial design
- Changes to the communication technology other than Bluetooth
- · Changes to the features that are unrelated to Bluetooth, branding, packaging, colour, shape, Product name, or Model number

Members are responsible for assessing that modifications to the Product outside of the Design do not affect compliance with Bluetooth Specifications or result in a change to the ICS Form.

For the purposes of this document, it is assumed that the member is combining a single unmodified Core-Complete Configuration.

# 12.3 Qualification Steps When Referencing a single existing design, (unmodified) – Option 1 in the QPRDv3

For this qualification, follow these steps:

- 1. To start a listing, go to: https://qualification.bluetooth.com/
- 2. Select Start the Bluetooth Qualification Process.
- 3. Product Details to be entered:
  - Project Name (this can be the product name or the Bluetooth Design name).
  - · Product Description
  - Model Number

![](_page_55_Picture_1.jpeg)

- Product Publication Date (the product publication date may not be later than 90 days after submission)
- Product Website (optional)
- Internal Visibility (this will define if the product will be visible to other users prior to publication)
- If you have multiple End Products to list then you can select 'Import Multiple Products', firstly downloading and completing the template, then by 'Upload Product List'. This will populate Qualification Workspace with all your products.

#### 4. Specify the Design:

- Do you include any existing Design(s) in your Product? Answer Yes, I do.
- Enter the single DN or QDID used in your, (for Option 1 only one DN or QDID can be referenced)
- Once the DN or QDID is selected it will appear on the left-hand side, indicating the layers covered by the design.
- Select 'I'm finished entering DN's
- What do you want to do next? Answer, 'Use this Design without Modification'
- Save and go to Product Qualification Fee

#### 5. Product Qualification Fee:

- It's important to make sure a Prepaid Product Qualification fee is available as it is required at this stage to complete the Qualification Process.
- Prepaid Product Qualification Fee's will appear in the available list so select one for the listing.
- If one is not available select 'Pay Product Qualification Fee', payment can be done immediately via credit card, or you can pay via Invoice. Payment via credit will release the number immediately, if paying via invoice the number will not be released until the invoice is paid.
- Once you have selected the Prepaid Qualification Fee, select 'Save and go to Submission'

#### 6. Submission:

- Some automatic checks occur to ensure all submission requirements are complete.
- To complete the listing any errors must be corrected
- Once you have confirmed all design information is correct, tick all of the three check boxes and add your name to the signature page.
- Now select 'Complete the Submission'.
- You will be asked a final time to confirm you want to proceed with the submission, select 'Complete the Submission'.
- Qualification Workspace will confirm the submission has been submitted. The Bluetooth SIG will email confirmation once the submission has been accepted, (normally this takes 1 working day).
- 7. Download Product and Design Details:
  - a. You can now download a copy of the confirmed listing from the design listing page and save a copy in your Compliance Folder

For further information, please refer to the following webpage:

https://www.bluetooth.com/develop-with-bluetooth/qualification-listing/

### 12.4 Example Designs for reference

The following gives an example of a design possible under option 1:

Ezurio End Product design using Nordic Component based design

| Design Name  | Owner  | Declaration ID | QD ID  | Link to listing on the SIG website                        |
|--------------|--------|----------------|--------|-----------------------------------------------------------|
| BL653\BL653µ | Ezurio | D067611        | 232800 | https://qualification.bluetooth.com/ListingDetails/205357 |

### 12.5 Qualify More Products

If you develop further products based on the same design in the future, it is possible to add them free of charge. The new product must not modify the existing design i.e add ICS functionality, otherwise a new design listing will be required.

To add more products to your design, select 'Manage Submitted Products' in the Getting Started page, Actions, Qualify More Products. The tool will take you through the updating process.

![](_page_56_Picture_1.jpeg)

# 13 Reliability Tests

The BL653 module went through the below reliability tests and passed.

| Test<br>Sequence | Test Item      | Test Limits and<br>Pass | Test Conditions                                                              |
|------------------|----------------|-------------------------|------------------------------------------------------------------------------|
| 1                | Vibration Test | JESD22-B103B            | Sample: Unpowered.                                                           |
|                  |                | Vibration, Variable     | Sample number: 3.                                                            |
|                  |                | frequency               | Vibration waveform: Sine waveform.                                           |
|                  |                |                         | Vibration frequency /Displacement: 20 to 80Hz /1.52mm.                       |
|                  |                |                         | Vibration frequency /Acceleration: 80 to 2000Hz /20g.                        |
|                  |                |                         | Cycle time: 4 minutes.                                                       |
|                  |                |                         | Number of cycles: 4 cycles for each axis.                                    |
|                  |                |                         | Vibration axis: X, Y and Z (Rotating each axis on vertical vibration table). |
| 2                | Mechanical     | JESD22-B104C            | Sample: Unpowered.                                                           |
|                  | Shock          |                         | Sample number: 3.                                                            |
|                  |                |                         | Pulse shape: Half-sine waveform.                                             |
|                  |                |                         | Impact acceleration: 1500g.                                                  |
|                  |                |                         | Pulse duration: 0.5ms.                                                       |
|                  |                |                         | Number of shocks: 30 shocks (5 shocks for each face).                        |
|                  |                |                         | Orientation: Bottom, top, left, right, front and rear faces.                 |
| 3                | Thermal        | JESD22-A104E            | Sample: Unpowered.                                                           |
|                  | Shock          | Temperature             | Sample number: 3.                                                            |
|                  |                | cycling                 | Temperature transition time: Less than 30 seconds.                           |
|                  |                |                         | Temperature cycle: -40°C (10 minutes), +105 °C (10 minutes).                 |
|                  |                |                         | Number of cycles: 350.                                                       |

Before and after the testing, visual inspection showed no physical defect on samples.

After Vibration test and Mechanical Shock testing, the samples were functionally tested, and all samples functioned as normal. Then after Thermal shock test, the samples were functionally tested, and all samples functioned as normal.

![](_page_57_Picture_1.jpeg)

### 14 Additional Information

Please contact your local sales representative or our support team for further assistance:

| Headquarters      | Ezurio<br>50 S. Main St. Suite 1100<br>Akron, OH 44308 USA |  |  |  |
|-------------------|------------------------------------------------------------|--|--|--|
| Website           | http://www.ezurio.com                                      |  |  |  |
| Technical Support | http://www.ezurio.com/resources/support                    |  |  |  |
| Sales Contact     | http://www.ezurio.com/contact                              |  |  |  |

Note: Information contained in this document is subject to change.

Ezurio's products are subject to standard Terms & Conditions.

#### http://www.ezurio.com

© Copyright 2025 Ezurio All Rights Reserved. Any information furnished by Ezurio and its agents is believed to be accurate but cannot be guaranteed. All specifications are subject to change without notice. Responsibility for the use and application of Ezurio materials or products rests with the end user since Ezurio and its agents cannot be aware of all potential uses. Ezurio makes no warranties as to non-infringement nor as to the fitness, merchantability, or sustainability of any Ezurio materials or products for any specific or general uses. Ezurio or any of its affiliates or agents shall not be liable for incidental or consequential damages of any kind. All Ezurio products are sold pursuant to the Ezurio Terms and Conditions of Sale in effect from time to time, a copy of which will be furnished upon request. Nothing herein provides a license under any Ezurio or any third-party intellectual property right. Ezurio and its associated logos are trademarks owned by Ezurio and/or its affiliates.