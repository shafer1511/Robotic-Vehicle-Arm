# Remote-Controlled Robotic Vehicle with Integrated 4-DOF Robotic Arm

A dual-STM32 embedded system utilizing a Master-Slave architecture. This repository contains the firmware for both the handheld controller (Master) and the mobile robot actuator platform (Slave), featuring wireless control, non-blocking peripheral handling, and multi-servo motor synchronization.

![Remote-Controlled Robotic Vehicle with Integrated 4-DOF Robotic Arm](images/completed.jpg)
## System Architecture
The project is split into two main components:
1. **Master (Joystick Controller):** Samples analog data from two joysticks and digital states from buttons, packages the telemetry data, and transmits it via UART.
2. **Slave (Vehicle Actuator):** Decodes incoming UART commands in real-time, translating them into multi-channel PWM signals to control the DC drive motors and the 4-DOF robotic arm simultaneously.
---

## Hardware Specification & Pin Mapping

### 1. Master Controller (STM32F103C8T6)
Based on the hardware configuration designed in `Joystick.ioc`, the peripheral allocation is mapped as follows:

| Peripheral / Mode | Pin Name | Signal / Function Name | Hardware Connection | Description |
| :--- | :---: | :--- | :--- | :--- |
| **ADC1 (Analog)** | PA0 | `JOYSTICK_Y` | Joystick 1 - Y Axis | Vehicle movement control (Forward/Backward) |
| | PA1 | `JOYSTICK_X` | Joystick 1 - X Axis | Vehicle steering control (Left/Right) |
| | PA4 | `JOY2_Y` | Joystick 2 - Y Axis | Servo 3 (Elbow Joint) angular positioning |
| | PA5 | `JOY2_X` | Joystick 2 - X Axis | Servo 4 (Gripper Mechanism) claw control |
| **GPIO Input** | PB3 | `S1_LEFT_BTN` | Push Button (Pull-up) | Manual increment: Rotate Base Left |
| | PB4 | `S1_RIGHT_BTN` | Push Button (Pull-up) | Manual increment: Rotate Base Right |
| | PB5 | `S2_UP_BTN` | Push Button (Pull-up) | Manual increment: Move Shoulder Up |
| | PB6 | `S2_DOWN_BTN` | Push Button (Pull-up) | Manual increment: Move Shoulder Down |
| **EXTI (Interrupt)**| PB7 | `GRIPPER_HOLD_BTN` | Tactile Button (Rising IT)| Triggers External Interrupt for Gripper Lock Mode |
| **USART2 (Serial)** | PA2 | `USART2_TX` | TX Pin (9600 Baud) | Transmits control frame strings to Slave |
| | PA3 | `USART2_RX` | RX Pin (9600 Baud) | Receives telemetry feedback/logs |
| **GPIO Output** | PC13 | `LED` | Onboard Blue Pill LED | System status & UART transmit indicator |
| **System/Clock** | PD0 / PD1| `RCC_OSC_IN` / `OUT` | 8MHz Crystal | Main system clock configuration source |
| | PA13/PA14| `SWDIO` / `SWCLK` | ST-Link V2 Debugger | Serial Wire Debug (SWD) interface |

### 2. Slave Vehicle Platform (STM32F103C8T6)
Based on the hardware configuration designed in `Vehicle.ioc`, the peripheral allocation is mapped as follows:

| Peripheral / Mode | Pin Name | Signal / Function Name | Hardware Connection | Performance Parameters |
| :--- | :---: | :--- | :--- | :--- |
| **TIM1 (PWM Mode)** | PA8 | `M_A_IN1_PWM` | L298N Driver - IN1 | Motor A Speed Control |
| (Advanced Timer) | PA9 | `M_A_IN2_PWM` | L298N Driver - IN2 | **Frequency: 1kHz** (ARR=999, PSC=71) |
| | PA10 | `M_B_IN3_PWM` | L298N Driver - IN3 | Motor B Speed Control |
| | PA11 | `M_B_IN4_PWM` | L298N Driver - IN4 | System Clock: 72MHz |
| **TIM4 (PWM Mode)** | PB6 | `SERVO01_PWM` | RC Servo 1 - Base | **Frequency: 50Hz** (ARR=1999, PSC=359) |
| (General Timer) | PB7 | `SERVO02_PWM` | RC Servo 2 - Shoulder | Pulse Width: 0.7ms - 4.2ms |
| | PB8 | `SERVO03_PWM` | RC Servo 3 - Elbow | Standard RC Servo Angle Modulation |
| | PB9 | `SERVO04_PWM` | RC Servo 4 - Gripper | Precision $0^\circ - 180^\circ$ Control |
| **USART2 (Serial)** | PA2 | `USART2_TX` | Bluetooth HC-05 (TX) | Telemetry Debug Echo Out |
| | PA3 | `USART2_RX` | Bluetooth HC-05 (RX) | **9600 Baudrate**, 8-bit Data, 1 Stop Bit |
| **GPIO Output** | PC13 | `LED` | Onboard Blue Pill LED | Command Processing Execution Blinker |
| **System/Clock** | PD0 / PD1| `RCC_OSC_IN` / `OUT` | 8MHz Crystal | Main system clock configuration source |
| | PA13/PA14| `SWDIO` / `SWCLK` | ST-Link V2 Debugger | Serial Wire Debug (SWD) interface |

---

## Hardware & Tools
* **Microcontroller:** STM32F103C8T6 (Cortex-M3)
* **Wireless Communication:** HC-05 Bluetooth Module (UART interface 9600 baudrate)
* **Power Management:** Powered by dual 18650 Li-ion batteries (7.4V).
  * *Power Distribution:* The 7.4V rail directly powers the L298N H-Bridge for the DC motors. An **LM2596 Buck Converter** steps the voltage down to 5V to power the four RC servos, preventing voltage drops and MCU brownout resets when the motors/servos draw heavy current. A dedicated regulator provides clean 3.3V power to the STM32 and the HC-05 module.
* **Actuators & Drivers:** RC Servos, DC Motors, L298N Dual H-Bridge Driver
* **Inputs:** Dual Analog Joysticks, Tactile Push Buttons
* **IDE:** STM32CubeIDE (HAL Framework)

---

## Firmware Implementation Details

* **Software Dead-Zone Filtering:** Implemented a software dead-zone ($800 - 3200$) on the multi-channel ADC inputs to filter out joystick center-point noise and hardware potentiometer drift.
* **Non-Blocking Debouncing:** Used an External Interrupt (EXTI) for the Gripper Hold Button with `HAL_GetTick()` timestamp validation, debouncing the tactile switch without blocking the main execution loop.
* **Data Transmission Optimization:** Added state-change detection so control strings are only sent over UART when the user actually moves the joysticks or presses a button, reducing unnecessary serial bandwidth overhead.
* **Packet Parsing:** Implemented a non-blocking character accumulator over UART. Incoming data bytes are stored and parsed line-by-line using a newline delimiter (`\n`) to avoid buffer overflows.

---

## Repository Directory Tree
```text
Robotic-Vehicle-Arm/
├── Joystick/               # Master Controller Firmware Project
│   ├── Core/
│   │   ├── Inc/main.h      # Peripheral & pin macros definition
│   │   └── Src/main.c      # ADC Sampling, EXTI debounce, & UART encoding
│   └── Joystick.ioc        # CubeMX peripheral configuration matrix
├── Vehicle/                # Slave Actuator Platform Firmware Project
│   ├── Core/
│   │   └── Src/main.c      # Stream decoding, TIM1/TIM4 PWM generation
│   └── Vehicle.ioc         # Actuator peripheral allocation schematic
└── .gitignore              # Unified clean workspace filter for STM32CubeIDE