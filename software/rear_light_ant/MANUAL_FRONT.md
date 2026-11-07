# Bike Light Front (ANT+) - User Manual

## Overview

The front light uses the same board, button, sensors, battery and charging as the rear light, with a white LED. It has three light modes (Smart, Standard, Bright), ambient light sensing, parking with automatic switch-on when you ride off, and can be controlled from an ANT+ bike computer.

This manual covers what differs from the rear light and what you need day to day. Charging, battery and technical details are the same as in the [rear light manual](./MANUAL.md).

## Operating Modes

A short press steps through the modes:

```
Deep Sleep → Smart → Standard → Bright → Smart → ...
```

The three light modes are the **Active Modes**. After Bright a short press starts again at Smart: a short press never switches the light off. Switch it off with a **long press**.

### Smart
- **Dark** (below about 30 lux): Standard brightness (30 %).
- **Daylight** (from about 40 lux): main LED off.
- **No brake boost**: braking does not change the front light.
- **Parking**: after 2.5 minutes without motion the light parks. The main LED is off and the status LED gives a very short flash (30 ms) every 5 seconds. Moving the bike (a jolt above about 0.3 g, such as riding off) switches Smart back on within about a second.
- About every 30 s in the dark the LED switches off for about 0.1 s to measure the ambient light without its own light (the same as on the rear light).

### Standard
- Steady white light at **30 %** (300 µs of the 1 ms PWM period).

### Bright
- Steady white light at **80 %** (800 µs of 1 ms).
- Uses much more battery than Standard, and the LED and housing get warmer. Use it when you need to see, not to be seen.

### Off (from the bike computer only)
The light is dark but awake and reachable, so the bike computer can switch it on again. The status LED pulses briefly every 2 s. A short press starts Smart.

## Button

- **Short press** (released within 1 second): next mode, Smart → Standard → Bright → Smart, at any time. It never switches the light off.
- **Long press** (1 second): the only way to switch the light off with the button, from any mode: Deep Sleep, or Idle Charging when USB power is present. In Idle Charging it does nothing.
- **Wake**: in Deep Sleep a press wakes the light into Smart. The wake press does nothing else.

## Status LED

| Priority | Condition | Status LED |
| --- | --- | --- |
| 1 | Low battery | 500 ms blink |
| 2 | Charging (USB power present) | 300 ms blink |
| 3 | Smart parked | very short flash (30 ms) every 5 s |
| 4 | Light in an active mode | solid on |
| 5 | Off from the bike computer | short pulse (0.1 s) every 2 s |
| 6 | Deep Sleep | off |

## Charging and battery

As on the rear light: USB-C charging also in Deep Sleep, a low battery is shown only by the status LED, and below about 3.0 V the light switches off. See the [rear light manual](./MANUAL.md#charging--battery-behavior).

## Controlling the rear light

The front light also switches the rear light over ANT+, the same way a bike computer does. Nothing has to be set up.

- **Finding the rear:** each time the front light wakes, it looks for the **nearest rear light** and uses that one until it is switched off again. Keep both lights on the bike when you switch on. When riding in a group, a stranger's rear light that is closer at that moment could be picked.
- **What the rear follows:** the changes you make at the front light:

| Front light | Rear light |
| --- | --- |
| Smart | Smart |
| Standard | Steady |
| Bright | Flash |
| Off (long press) | Off: dark, but awake, so the front light switches it on again next time |
| Wake / switched on | Smart |

- **Not passed on:** changes from the bike computer, the front's low-battery cut-off (the rear stays on if the front battery runs flat), plugging in USB, and parking.
- **With a bike computer:** the bike computer still sees and switches both lights. The last change wins: if the bike computer changes the rear, it stays like that until you change the front light by hand.
- A rear that was switched Off by the front light stays dark until the front light, the bike computer or the rear's own button switches it on.
- The rear ignores a change that comes within 3 seconds of a press on its own button.

## Control from an ANT+ bike computer

The front light is an **ANT+ bike light** and reports itself as a **headlight**. ANT+ commands are not authenticated: any ANT+ controller in range can change the mode.

1. Wake the light with the button (it starts in Smart).
2. On the bike computer, search for sensors and add the ANT+ light. It shows up as a headlight.
3. The bike computer lists the modes:

| Bike computer | Light |
| --- | --- |
| Steady 21-40 % | Standard |
| Steady 61-80 % | Bright |
| Auto | Smart |
| Off | Off (dark, awake and reachable) |
| Battery | New, good, ok, low, critical from the cell voltage; "charging" while USB power is present |

- Requests from the bike computer are ignored for 3 seconds after any change at the light itself (button, boot).
- If the bike computer goes quiet for 2 minutes, the light returns to "unconnected" but keeps its mode.
- A Wahoo controls the front and the rear light at the same time.

## Building and flashing

```sh
make LIGHT=front          # build/front/light_front.hex
make LIGHT=front flash    # first time: SoftDevice + application
```

See the [README](./README.md) for keys, options and the bench tools (add `LIGHT=front` to every command).

## License

Copyright © 2025 Matthias Dippold. Released under **GPLv3 + NonCommercial**, see the LICENSE file.
