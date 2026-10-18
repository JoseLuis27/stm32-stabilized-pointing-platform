# STM32 Stabilized Pointing Platform

This device is a 2-axis pan tilt where the top stays stabilized using an IMU to measure the movement and the servos to correct the discrepancies.

## Status

- [x] Phase 1 — Bare-metal GPIO (LED + button)
- [x] Phase 2 — Interrupt-driven UART driver with ring buffer
- [ ] Phase 3 — I2C IMU driver
- [ ] Phase 4 — Timers & PWM servo control
- [ ] Phase 5 — Sensor fusion
- [ ] Phase 6 — PID control
- [ ] Phase 7 — FreeRTOS
- [ ] Phase 8 — Fault tolerance
- [ ] Phase 9 — Testing & static analysis

## Hardware

| Part | Why this one |
|---|---|
| NUCLEO-F446RE (STM32F446RE, Cortex-M4F) | It has a built in ST-LINK giving us the ability to flash and debug without needing a programmer, letting us send and receive data with a USB B mini cable. It also comes with a Cortex-M4 chip with an FPU, helping us with Phase 5 since it contains heavy floating point math. |
  | Adafruit LSM6DSO32 6-axis IMU (#4692) | This IMU is still actively produced and also from the same manufacturer as the micro-controller. Unlike the popular MPU-6050 which is end-of-life. |
| Adafruit mini pan-tilt kit with servos (#1967) | 2 servos that gives us 2 axes and it comes with a pre-attached mount. The servos are driven by PWM in Phase 4|
| Adafruit STEMMA QT to male header cable (#4209) | The IMU contains a STEMMA QT plug, using this cable it converts it into breadboard pins to avoid the need of soldering. |
| Adafruit 4×AA battery holder (#3784) | To prevent brownouts and resets from the servos since the board's USB power cannot safely output the right amount of power. A separate power supply avoids this.|
| Adafruit barrel jack to screw terminal (#368) | The battery holder has a barrel plug which the board doesn't have so it converts it to bare wires for the breadboard.|
| Half-size breadboard + jumper wires | Allows us to avoid soldering and gives us the ability to rewire easily.|
| 8-channel 24 MHz logic analyzer | Gives us actual signals on the wires to verify timings. This is when printf can't tell us what's wrong.|

## Tools

- STM32CubeIDE 2.2.0 (build, flash, debug)
- `screen` at 115200 baud (serial terminal)
- Git + GitHub Desktop
- Reference docs: RM0390, DS10693, UM1724, PM0214

## Approach

This project uses bare-metal register programming instead of ST's HAL library, to build a deeper understanding of how embedded systems work. Vendor libraries are specific to one manufacturer's chips, but the concepts underneath them such as clocks, registers, pin configuration, interrupts are transferable to any MCU. And since every vendor library is just code writing those same registers, this understanding makes it possible to debug a library when it misbehaves instead of treating it as a black box.


## Build & flash

1. Clone this repository.
2. In STM32CubeIDE, choose **File → Open Projects from File System…** and select the `firmware` folder.
3. Connect the NUCLEO-F446RE over USB (Mini-B), then click **Run** to build and flash.
4. Open a serial terminal at 115200 baud (macOS example):
   a. ls /dev/tty.usbmodem* 
   b. screen //dev/tty.usbmodem<NUMBER> 115200
   c. Quit 'screen' by pressing Ctrl+A, K, then y.

