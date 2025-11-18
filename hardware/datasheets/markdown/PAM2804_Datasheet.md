![](_page_0_Picture_1.jpeg)

## 1A STEP-DOWN CONSTANT CURRENT, HIGH EFFICIENCY LED DRIVER

## **Description**

The PAM2804 is a step-down constant current LED driver. When the input voltage is down to lower than LED forward voltage, then PAM2804 goes into LDO mode.

The PAM2804 supports a range of input voltages from 2.5V to 6.0V, allowing the use of a single Li+/Li-polymer cell, 3AA or 4AA cell, USB, and other standard power sources.

The FB voltage is only 0.1V to achieve high efficiency.

PAM2804 employs internal power switch and synchronous rectifier to minimize external part count and realize high efficiency.

During shutdown, the input is disconnected from the output and the shutdown current is less than  $1\mu A$ . Other key features include undervoltage lockout to prevent deep battery discharge of the Li+battery.

## **Features**

- Efficiency up to 93%
- 180µA(typ) Quiescent Current
- Output Current: Up to 1A
- Internal Synchronous Rectifier
- 1.5MHz Switching Frequency
- Soft-Start
- Undervoltage Lockout
- Short LED Protection
- Open LED Protection
- Thermal Shutdown
- 5-Pin Small TSOT25 Packages
- Pb-Free Package

## **Pin Assignments**

![](_page_0_Figure_23.jpeg)

## **Applications**

- 3AA or 4AA Batteries Powered Flashlight
- 1 Cell Li-Ion Battery Powered Flashlight

# **Typical Applications Circuit**

![](_page_0_Picture_28.jpeg)

 $I_{LED} = 0.1/R_{S}$ 

![](_page_1_Picture_1.jpeg)

# **Pin Description**

| Pin<br>Number | Pin<br>Name | Function                                                                                                         |  |
|---------------|-------------|------------------------------------------------------------------------------------------------------------------|--|
| 1             | EN          | Enable control input. Force this pin voltage above 1.5V, enables the chip, and below 0.3V shuts down the device. |  |
| 2             | GND         | Ground                                                                                                           |  |
| 3             | SW          | The drains of the internal main and synchronous power MOSFET.                                                    |  |
| 4             | VIN         | Chip main power supply pin.                                                                                      |  |
| 5             | FB          | Feedback voltage to internal error amplifier, the threshold voltage is 0.1V.                                     |  |

## **Block Diagram**

![](_page_1_Figure_5.jpeg)

## Absolute Maximum Ratings (@T<sub>A</sub> = +25°C, unless otherwise specified.)

These are stress ratings only and functional operation is not implied. Exposure to absolute maximum ratings for prolonged time periods may affect device reliability. All voltages are with respect to ground.

2 of 10

| Parameter                  | Rating                         | Unit |
|----------------------------|--------------------------------|------|
| Input Pin Voltage          | -0.3 to +6.5                   |      |
| EN, FB Pin Voltage         | -0.3 to V <sub>IN</sub>        | V    |
| SW Pin Voltage             | -0.3 to (V <sub>IN</sub> +0.3) |      |
| Junction Temperature Range | 150                            |      |
| Storage Temperature Range  | -65 to +150                    | °C   |
| Soldering Temperature      | +300, 5sec                     |      |

![](_page_2_Picture_1.jpeg)

# Recommended Operating Conditions (@TA = +25°C, unless otherwise specified.)

| Parameter                   | Rating      | Unit |
|-----------------------------|-------------|------|
| Supply Voltage              | 2.5 to 6.0  |      |
| Operation Temperature Range | -40 to +85  | °C   |
| Junction Temperature Range  | -40 to +125 |      |

## Thermal Information

| Parameter                                | Symbol        | Package         | Max | Unit |
|------------------------------------------|---------------|-----------------|-----|------|
| Thermal Resistance (Junction to Case)    | $\theta_{JC}$ | TSOT25 (Note 1) | 130 | °C/W |
| Thermal Resistance (Junction to Ambient) | θЈА           | TSOT25          | 250 | C/VV |
| Internal Power Dissipation               | $P_{D}$       | TSOT25          | 400 | mW   |

Note: 1. The maximum output current for TSOT25 package is limited by internal power dissipation capacity as described in Application Information herein after.

## **Electrical Characteristics**

 $(@T_A = +25 ^{\circ}C,\ V_{IN} = 4.2 V,\ Real\ WLED\ load,\ L = 4.7 \mu H,\ C_{IN} = 10 \mu F,\ C_O = 10 \mu F,\ unless\ otherwise\ specified.)$ 

| Parameter                          | Symbol              | Test Conditions         |          | Min   | Тур   | Max   | Units |
|------------------------------------|---------------------|-------------------------|----------|-------|-------|-------|-------|
| Input Voltage Range                | V <sub>IN</sub>     | -                       |          | 2.5   | -     | 6.0   | V     |
| Regulated Feedback Voltage         | $V_{FB}$            | -                       |          | 0.095 | 0.100 | 0.105 | V     |
| Peak Inductor Current              | I <sub>PK</sub>     | V <sub>IN</sub> = 5V    |          | -     | 1.5   | -     | Α     |
| Quiescent Current                  | ΙQ                  | No Load                 |          | -     | 180   | _     | μΑ    |
| Shutdown Current                   | I <sub>SD</sub>     | V <sub>EN</sub> = 0V    |          | -     | -     | 1     | μΑ    |
| Oscillator Frequency               | fosc                | V <sub>O</sub> = 100%   |          | 1.2   | 1.5   | 1.8   | MHz   |
| Drain-Source On-State Resistance   | R <sub>DS(ON)</sub> | I <sub>DS</sub> = 100mA | P MOSFET | -     | 0.30  | 0.45  | Ω     |
| Diani-Source On-State Resistance   |                     |                         | N MOSFET | -     | 0.35  | 0.50  | Ω     |
| SW Leakage Current                 | I <sub>LSW</sub>    | -                       |          | -     | ±0.01 | 1     | μA    |
| High Efficiency                    | η                   | -                       |          | -     | 93    | -     | %     |
| EN Threshold High                  | $V_{EH}$            | -                       |          | 1.5   | -     | -     | V     |
| EN Threshold Low V <sub>EL</sub> - |                     | -                       | -        | 0.3   | V     |       |       |
| EN Leakage Current                 | I <sub>EN</sub>     | -                       |          | -     | ±0.01 | -     | μΑ    |
| Over-Temperature Protection        | OTP                 | -                       |          | -     | +150  | -     | °C    |
| OTP Hysteresis                     | OTH                 | -                       |          | -     | +30   | -     | °C    |

![](_page_3_Picture_1.jpeg)

# Typical Performance Characteristics (@ $T_A = +25$ °C, L = 4.7 $\mu$ F, $C_{IN} = 10\mu$ F, $C_O = 10\mu$ F, unless otherwise specified.)

![](_page_3_Figure_3.jpeg)

Input Voltage

Temperature(°C)

![](_page_4_Picture_1.jpeg)

# Typical Performance Characteristics (cont.) $@T_A = +25^{\circ}C$ , $L = 4.7 \mu F$ , $C_{IN} = 10 \mu F$ , $C_O = 10 \mu F$ , unless otherwise specified.)

### Quiescent Current Vs Input Voltage

![](_page_4_Figure_4.jpeg)

## Start up with Enable

![](_page_4_Figure_6.jpeg)

## PWM Dimming: ILED vs. Duty Cycle

![](_page_4_Figure_8.jpeg)

![](_page_5_Picture_1.jpeg)

## **Application Information**

The PAM2804 can be used to dim LED current dimming by driving the EN pin via PWM waveform. The SW pin current is then effectively switched on and off causing the LED current to turn on and off. The recommended PWM frequency is 500Hz (see Figure on page 5 for details).

![](_page_5_Picture_5.jpeg)

### **Inductor Selection**

The basic PAM2804 application circuit is shown in Page 1. External component selection is determined by the load requirement, selecting L first and then C<sub>IN</sub> and C<sub>OUT</sub>.

For most applications, the value of the inductor will fall within the range of 1µH to 4.7µH. Its value is chosen based on the desired ripple current. Large value inductors lower ripple current and small value inductors result in higher ripple currents. Higher V<sub>IN</sub> or V<sub>OUT</sub> also increases the ripple current as shown in equation 1. A reasonable starting point for setting ripple current is ΔI<sub>L</sub> = 400mA (40% of 1A).

$$\Delta I_{L} = \frac{1}{(f)(L)} V_{OUT} \left( 1 - \frac{V_{OUT}}{V_{IN}} \right)$$
 Equation (1)

The DC current rating of the inductor should be at least equal to the maximum load current plus half the ripple current to prevent core saturation. Thus, a 1.4A rated inductor should be enough for most applications (1A + 400mA). For better efficiency, choose a low DC-resistance inductor.

### **Using Ceramic Input Output Capacitors**

Higher values, lower cost ceramic capacitors are now becoming available in smaller case sizes. Their high ripple current, high voltage rating and low ESR make them ideal for switching regulator applications. Using ceramic capacitors can achieve very low output ripple and small circuit size.

When choosing the input and output ceramic capacitors, choose the X5R or X7R dielectric formulations. These dielectrics have the best temperature and voltage characteristics of all the ceramics for a given value and size.

### **Thermal Consideration**

Thermal protection limits power dissipation in the PAM2804. When the junction temperature exceeds +150°C, the OTP (Overtemperature Protection) starts the thermal shutdown and turns the pass transistor off. The pass transistor resumes operation after the junction temperature drops below +120°C.

For continuous operation, the junction temperature should be maintained below +125°C. The power dissipation is defined as:

$$P_{D} = I_{O}^{2} \frac{V_{O} R_{DS(ON)H} + (V_{IN} - V_{O}) R_{DS(ON)L}}{V_{IN}} + (t_{SW} F_{S} I_{O} + I_{Q}) V_{IN}$$

IQ is the step-down converter quiescent current. The term tsw is used to estimate the full load step-down converter switching losses.

For the condition where the step-down converter is in dropout at 100% duty cycle, the total device dissipation reduces to:

$$P_D = I_O^2 R_{DS(ON)H} + I_O V_{IN}$$

6 of 10 PAM2804 September 2015 www.diodes.com Document number: DS36403 Rev. 3 - 2 © Diodes Incorporated

![](_page_6_Picture_1.jpeg)

## **Application Information (cont.)**

Since R<sub>DS(ON)</sub>, quiescent current, and switching losses all vary with input voltage, the total losses should be investigated over the complete input voltage range. The maximum power dissipation depends on the thermal resistance of IC package, PCB layout, the rate of surrounding airflow and temperature difference between junction and ambient. The maximum power dissipation can be calculated by the following formula:

$$P_D = \frac{T_{J(MAX)} - T_A}{\theta_{JA}}$$

Where  $T_{J(MAX)}$  is the maximum allowable junction temperature +125°C.  $T_A$  is the ambient temperature and  $\theta_{JA}$  is the thermal resistance from the junction to the ambient. Based on the standard JEDEC for a two layers thermal test board, the thermal resistance  $\theta_{JA}$  of TSOT25 package is 250°C/W. The maximum power dissipation at  $T_A$  = +25°C can be calculated by following formula:

$$P_D = (125^{\circ}C - 25^{\circ}C)/250^{\circ}C/W = 0.4W$$

## **Setting the Output Current**

The internal feedback (FB) voltage is 0.1V (Typical). The output current is calculated as below:

$$I_{LED} = 0.1/R_{S}$$

The output Current is given by the following table.

| R <sub>S</sub> (Ω) | I <sub>LED</sub> (mA) |  |  |
|--------------------|-----------------------|--|--|
| 0.286              | 350                   |  |  |
| 0.143              | 700                   |  |  |
| 0.1                | 1,000                 |  |  |

As the input voltage approaches the LED forward voltage, the PAM2804 turns the P-Channel transistor continuously on. In this mode, the voltage drop on LED is equal to the input voltage minus the voltage drop across the P-Channel transistor, Inductor and current resistor:

$$V_{LEDDROP} = V_{IN} - ILED(R_{DS(ON)} + R_L + R_S)$$

Where  $R_{DS(ON)} = P$ -Channel switch ON resistance, ILED = LED current,  $R_L = Inductor DC$  Resistance,  $R_S = Inductor DC$  Resistance.

### Thermal Shutdown

When the die temperature exceeds +150°C, a reset occurs and the reset remains until the temperature decrease to +120°C, at which time the circuit can be restarted.

### **PCB Layout Check List**

When laying out the printed circuit board, the following checklist should be used to ensure proper operation of the PAM2804. These items are also illustrated graphically in Figure 1. Check the following in your layout:

- 1. The power traces, consisting of the GND trace, the SW trace and the VIN trace should be kept short, direct and wide.
- 2. Does the V<sub>FB</sub> pin connect directly to the current sense resistor? The current sense resistor to GND trace should be kept short, direct and wide.
- 3. Does the (+) plate of C<sub>IN</sub> connect to V<sub>IN</sub> as closely as possible? This capacitor provides the AC current to the internal power MOSFETs.
- 4. Keep the switching node, SW, away from the sensitive VFB node.
- 5. Keep the (-) plates of C<sub>IN</sub> and C<sub>OUT</sub> as close as possible.

![](_page_7_Picture_1.jpeg)

# **Ordering Information**

![](_page_7_Figure_3.jpeg)

| Part Number   | Marking                               | Package Type | Standard Package        |
|---------------|---------------------------------------|--------------|-------------------------|
| PAM2804AAB010 | Refer to Marking<br>Information Below | TSOT25       | 3,000 Units/Tape & Reel |

## **Marking Information**

![](_page_7_Figure_6.jpeg)

![](_page_7_Picture_7.jpeg)

EE: Product Code of PAM2804

X: Internal Code

Y: Year W: Week

![](_page_8_Picture_1.jpeg)

# **Package Outline Dimensions**

## TSOT25

![](_page_8_Figure_4.jpeg)

| TSOT25               |          |      |      |  |
|----------------------|----------|------|------|--|
| Dim                  | Min      | Max  | Тур  |  |
| Α                    | -        | 1.00 | -    |  |
| A1                   | 0.01     | 0.10 | -    |  |
| A2                   | 0.84     | 0.90 | -    |  |
| b                    | 0.30     | 0.45 | -    |  |
| С                    | 0.12     | 0.20 | -    |  |
| D                    | -        | -    | 2.90 |  |
| Е                    | -        | -    | 2.80 |  |
| E1                   | -        | -    | 1.60 |  |
| е                    | 0.95 BSC |      |      |  |
| e1                   | 1.90 BSC |      |      |  |
| L                    | 0.30     | 0.50 |      |  |
| L2                   | 0.25 BSC |      |      |  |
| θ                    | 0°       | 8°   | 4°   |  |
| θ1                   | 4°       | 12°  | -    |  |
| All Dimensions in mm |          |      |      |  |

# **Suggested Pad Layout**

### TSOT25

![](_page_8_Picture_8.jpeg)

| Dimensions | Value (in mm) |
|------------|---------------|
| С          | 0.950         |
| Х          | 0.700         |
| Y          | 1.000         |
| Y1         | 3.199         |

![](_page_9_Picture_1.jpeg)

### **IMPORTANT NOTICE**

DIODES INCORPORATED MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, WITH REGARDS TO THIS DOCUMENT, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE (AND THEIR EQUIVALENTS UNDER THE LAWS OF ANY JURISDICTION).

Diodes Incorporated and its subsidiaries reserve the right to make modifications, enhancements, improvements, corrections or other changes without further notice to this document and any product described herein. Diodes Incorporated does not assume any liability arising out of the application or use of this document or any product described herein; neither does Diodes Incorporated convey any license under its patent or trademark rights, nor the rights of others. Any Customer or user of this document or products described herein in such applications shall assume all risks of such use and will agree to hold Diodes Incorporated and all the companies whose products are represented on Diodes Incorporated website, harmless against all damages.

Diodes Incorporated does not warrant or accept any liability whatsoever in respect of any products purchased through unauthorized sales channel. Should Customers purchase or use Diodes Incorporated products for any unintended or unauthorized application, Customers shall indemnify and hold Diodes Incorporated and its representatives harmless against all claims, damages, expenses, and attorney fees arising out of, directly or indirectly, any claim of personal injury or death associated with such unintended or unauthorized application.

Products described herein may be covered by one or more United States, international or foreign patents pending. Product names and markings noted herein may also be covered by one or more United States, international or foreign trademarks.

This document is written in English but may be translated into multiple languages for reference. Only the English version of this document is the final and determinative format released by Diodes Incorporated.

### LIFE SUPPORT

Diodes Incorporated products are specifically not authorized for use as critical components in life support devices or systems without the express written approval of the Chief Executive Officer of Diodes Incorporated. As used herein:

- A. Life support devices or systems are devices or systems which:
  - 1. are intended to implant into the body, or
  - 2. support or sustain life and whose failure to perform when properly used in accordance with instructions for use provided in the labeling can be reasonably expected to result in significant injury to the user.
- B. A critical component is any component in a life support device or system whose failure to perform can be reasonably expected to cause the failure of the life support device or to affect its safety or effectiveness.

Customers represent that they have all necessary expertise in the safety and regulatory ramifications of their life support devices or systems, and acknowledge and agree that they are solely responsible for all legal, regulatory and safety-related requirements concerning their products and any use of Diodes Incorporated products in such safety-critical, life support devices or systems, notwithstanding any devices- or systems-related information or support that may be provided by Diodes Incorporated. Further, Customers must fully indemnify Diodes Incorporated and its representatives against any damages arising out of the use of Diodes Incorporated products in such safety-critical, life support devices or systems.

Copyright © 2015, Diodes Incorporated

www.diodes.com

PAM2804 10 of 10 September 2015
Document number: DS36403 Rev. 3 - 2 www.diodes.com © Diodes Incorporated