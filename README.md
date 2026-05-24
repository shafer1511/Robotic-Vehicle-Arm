# Remote-Controlled Robotic Vehicle with Integrated 4-DOF Robotic Arm

An advanced embedded system featuring a Master-Slave architecture implemented on dual STM32 microcontrollers. This repository contains the complete firmware for both the handheld control unit (Master) and the mobile robot actuator platform (Slave), showcasing real-time wireless telemetry, non-blocking peripheral handling, and precise mechanical actuation.

## System Architecture
The system operates on a decentralized Master-Slave topography:
1. **Master (Joystick Controller):** Samples analog data from dual joysticks and digital state changes from control buttons, encrypts the telemetry into lightweight data frames, and transmits them via UART.
2. **Slave (Vehicle Actuator):** Decodes the incoming UART command streams in real-time, translating data into multi-channel PWM signals to drive the DC propulsion motors and a 4-DOF robotic arm concurrently.

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

## Key Firmware Engineering Highlights

* **Software Dead-Zone Filtering:** Configured a dynamic software dead-zone threshold ($800 - 3200$) on multi-channel ADC inputs to effectively filter out hardware potentiometer drift, sensor aging, and joystick center-point noise.
* **Non-Blocking Debouncing via EXTI ISR:** Developed an isolated asynchronous External Interrupt routine for the Gripper Hold Button. Employs non-blocking timestamp validations (`HAL_GetTick`) to bypass hardware button bouncing without stalling execution loops.
* **Bandwidth & Packet Optimization:** Implemented state-change detection logic on telemetry parameters. Control strings are transmitted over UART *only* upon deliberate user input variation, drastically cutting down protocol bandwidth overhead.
* **Deterministic Packet Parsing:** Engineered a non-blocking character stream accumulator over UART. Packets are parsed line-by-line via deterministic delimiter tracking (`\n`), preventing buffer overflows and processing latencies.

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