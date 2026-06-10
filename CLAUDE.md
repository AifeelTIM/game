# CLAUDE.md - 电子负载能量回馈装置 (Electronic Load Energy Feedback Device)

## Project Overview

This is an **STM32G474VET6** embedded firmware project for a **2025 National Undergraduate Electronics Design Contest (全国大学生电子设计竞赛) Problem D: Electronic Load Energy Feedback Device (电子负载能量回馈装置)**.

The device functions as a programmable electronic load with energy feedback capability — it absorbs DC power from a source under test (simulating CC/CV/CR/CP load modes) and feeds the absorbed energy back to the input side (energy recycling), rather than dissipating it as heat.

## Hardware Platform

- **MCU:** STM32G474VET6 (Cortex-M4F, 170MHz, 512KB Flash, 128KB RAM, LQFP100)
- **Clock:** 8MHz HSE → PLL (×85, ÷2) → SYSCLK 170MHz, all APB/AHB buses @ 170MHz
- **Toolchain:** GCC ARM (`arm-none-eabi-gcc`), CMake + Ninja build system
- **Framework:** STM32CubeMX 6.17.0, STM32Cube FW_G4 V1.6.3, HAL library

## Pin Mapping

| Pin  | Function          | Purpose                              |
|------|-------------------|--------------------------------------|
| PA0  | ADC1_IN1          | U1 input voltage sampling            |
| PA1  | ADC1_IN2          | I1 input current sampling            |
| PA2  | ADC1_IN3          | U0 output voltage sampling           |
| PA3  | ADC1_IN4          | I0 output current sampling           |
| PA13 | SYS_JTMS-SWDIO    | Debug SWDIO                          |
| PA14 | SYS_JTCK-SWCLK    | Debug SWCLK                          |
| PA15 | I2C1_SCL          | OLED display (SSD1306)               |
| PB0  | TIM1_CH2N         | TIM1 CH2 complementary (reserved)    |
| PB7  | I2C1_SDA          | OLED display (SSD1306)               |
| PB12 | HRTIM1_CHC1       | Buck stage PWM (Timer C, Output 1)   |
| PB13 | HRTIM1_CHC2       | Buck stage PWM (Timer C, Output 2, complementary) |
| PB14 | HRTIM1_CHD1       | Boost stage PWM (Timer D, Output 1)  |
| PB15 | HRTIM1_CHD2       | Boost stage PWM (Timer D, Output 2, complementary) |
| PC0  | TIM1_CH1          | TIM1 CH1 (reserved)                  |
| PC1  | TIM1_CH2          | TIM1 CH2 (reserved)                  |
| PC4  | USART1_TX         | UART TX (VOFA+ data output)          |
| PC5  | USART1_RX         | UART RX                              |
| PC13 | TIM1_CH1N         | TIM1 CH1 complementary (reserved)    |
| PF0  | RCC_OSC_IN        | 8MHz HSE                             |
| PF1  | RCC_OSC_OUT       | 8MHz HSE                             |

## Peripheral Configuration Summary

### HRTIM1 — High-Resolution PWM (互补输出 + 死区)

- **Timer C** (Buck stage): continuous up-counting, period = 34000
  - Prescaler: MUL4 (×4), timer clock = 170MHz × 4 = 680MHz
  - Switching frequency: 680MHz / 34000 = **20kHz**
  - **Complementary PWM via Deadtime Insertion (DTEN)** — NOT push-pull mode
  - Output 1 (PB12, TC1): Set = TIMPER, Reset = TIMCMP1 (main switch)
  - Output 2 (PB13, TC2): auto-complement of Output 1 via DTEN hardware
  - Dead-time: DIV1 prescaler (~1.47ns/tick), 68 ticks ≈ **100ns**
- **Timer D** (Boost stage): same configuration as Timer C
  - Output 1 (PB14, TD1): main switch
  - Output 2 (PB15, TD2): auto-complement via DTEN
- **ADC Trigger 1:** Event 13 — Timer C period match → triggers ADC1 sampling
- Compare value (CMP1) updated directly in ADC ISR for real-time control
- **PushPull mode DISABLED** — PushPull pairs outputs across different timers (TC1+TB2, TD1+TC2), which is NOT what we want for same-timer complementary PWM. Deadtime Insertion (DTEN) is the correct approach for complementary outputs on the same timer.

### TIM1 — Auxiliary Timer (reserved)
- 170MHz clock, period = 8499, PWM freq ≈ 20kHz
- CH1/CH1N on PC0/PC13, CH2/CH2N on PC1/PB0
- Started as base timer only; PWM outputs not currently used (commented out)

### ADC1 — 4-Channel Synchronous Sampling
- 12-bit resolution, scan mode, triggered by HRTIM1 trigger 1 (Timer C period)
- **DMA1 Channel 1** in circular mode: continuously fills `adc_buffer[4]`
- 4 channels, 47.5 cycle sampling time each:
  - CH1 (PA0): U1 — input voltage (×10 divider, cal: ×1.024)
  - CH2 (PA1): I1 — input current (×4.667 gain, cal: ×1.145)
  - CH3 (PA2): U0 — output voltage (×10 divider, cal: /0.988)
  - CH4 (PA3): I0 — output current (×5 gain, cal: /0.7818)

### I2C1 — OLED Display
- Fast mode (400kHz), timing register = 0x40621236
- Drives SSD1306 128×64 OLED via I2C address 0x78

### USART1 — Debug/Data Output
- Baud rate 2Mbps, async mode
- Used for VOFA+ JustFloat protocol data streaming

## Source Code Structure

```
Core/
├── Inc/
│   ├── main.h              — Main header, HAL includes
│   ├── adc.h / dma.h / gpio.h / hrtim.h / i2c.h / usart.h
│   ├── oled.h              — OLED SSD1306 driver (Baud-dance library)
│   ├── font.h              — Font/image structures (Baud-dance library)
│   └── justfloat.h         — VOFA+ JustFloat protocol
├── Src/
│   ├── main.c              — Application entry, PI controller
│   ├── adc.c               — ADC1 init with DMA
│   ├── hrtim.c             — HRTIM1 PWM configuration
│   ├── oled.c              — OLED driver (SSD1306, I2C)
│   ├── font.c              — Font data (ASCII + Chinese)
│   ├── justfloat.c         — JustFloat UART transmit
│   ├── system_stm32g4xx.c  — CMSIS system init
│   ├── sysmem.c / syscalls.c
│   └── stm32g4xx_hal_msp.c / stm32g4xx_it.c
```

## Current Application Logic (main.c)

The firmware implements a **dual-loop PI controller** for a synchronous buck-boost converter:

### Control Loop (ADC ISR, runs at 20kHz PWM frequency)

1. **ADC ISR** (`HAL_ADC_ConvCpltCallback`): triggered each PWM period by HRTIM Timer C
   - Reads 4 ADC channels from `adc_buffer[4]` via DMA
   - Converts raw values to actual voltages/currents using hardware calibration factors
   - **Voltage loop (Timer C):** `CCR += kp * (tarvoltage - u1_measured)`, clamped to [100, 34000]
   - **Current loop (Timer D):** `CCR2 -= kp2 * (tarcurrent - i1_measured)`, clamped to [17000, 34000]
   - Writes CMP1 registers directly: `HRTIM1->sTimerxRegs[TIMER_C].CMP1xR = CCR`
   - Kp = 0.0001 (voltage), Kp2 = 0.0005 (current), P-only control

2. **Display loop** (main while loop, ~40Hz):
   - IIR low-pass filter (alpha = 0.0005) on measured values for display smoothing
   - OLED shows: U1, I1, U0, I0, P1 (calculated input power)
   - Updates every 500 ADC samples

3. **Key input** (blocking debounce in main loop):
   - KEY1/KEY2: adjust target voltage ±1V (tarvoltage, default 15V)
   - KEY3/KEY4: adjust target current ±10mA (tarcurrent, default 1.98A)

### Power Stage Topology

- **Timer C → PB12/PB13:** Synchronous buck stage (input side)
  - TC1: main buck switch, TC2: synchronous rectifier (complementary + 100ns dead-time)
  - Duty cycle controlled by voltage PI loop
- **Timer D → PB14/PB15:** Synchronous boost stage (output/feedback side)
  - TD1: main boost switch, TD2: synchronous rectifier (complementary + 100ns dead-time)
  - Duty cycle controlled by current PI loop
  - Minimum duty 50% (CCR2 ≥ 17000) to ensure boost operation

### Key Variables (main.c)

| Variable | Type | Purpose |
|----------|------|---------|
| `adc_buffer[4]` | uint32_t | DMA circular buffer for 4 ADC channels |
| `value` / `value2` | float | Raw U1 (mV) / I1 (mA) measurements |
| `value3` / `value4` | float | Raw U0 (mV) / I0 (mA) measurements |
| `err` / `err2` | float | Voltage / current error |
| `CCR` / `CCR2` | float | PI output → CMP1 for Timer C / Timer D |
| `tarvoltage` | float | Voltage setpoint (default 15000 mV = 15V) |
| `tarcurrent` | float | Current setpoint (default 1980 mA = 1.98A) |
| `kp` / `kp2` | float | Proportional gain (0.0001 / 0.0005) |
| `disp_u1/i1/u0/i0` | float | IIR-filtered display values |
| `float_data[5]` | float | JustFloat protocol buffer (currently unused) |

## Build & Flash

```bash
# Configure (from project root)
cmake --preset debug-stm32

# Build
cmake --build cmake-build-debug-stm32

# Flash (via ST-LINK or similar)
openocd -f interface/stlink.cfg -f target/stm32g4x.cfg -c "program cmake-build-debug-stm32/game.elf verify reset exit"
```

## Key Notes

- **ADC:** 4 channels (PA0-PA3) scan mode with DMA. Calibration factors are hardware-specific (voltage divider ratios, current sense amplifier gains) — do not blindly copy to a different hardware revision.
- **Complementary PWM** is achieved via **Deadtime Insertion (DTEN)** in HRTIM WaveformTimerConfig (`DeadTimeInsertion = ENABLED`, `PushPull = DISABLED`). When DTEN is set, Output 2 is automatically the complement of Output 1 with dead-time inserted — Output 2's Set/Reset sources in CubeMX are grayed out because they are IGNORED by hardware in DTEN mode.
- **DO NOT manually clear TIMxCR bits** — the prescaler (CK_PSC) occupies TIMxCR bits [2:0]. Any `CLEAR_BIT(TIMxCR, ...)` will corrupt the prescaler frequency. UDM (up-down mode) is in TIMxCR2 bit [4], not in TIMxCR at all, and HAL already sets it correctly to UP mode.
- The PI controller is currently P-only; integral (I) term not yet implemented.
- The `justfloat.c` protocol sends data in VOFA+ JustFloat format (4-byte float + 4-byte tail marker `00 00 80 7F`). Currently commented out in main loop.
- The OLED driver supports Chinese font rendering via UTF-8 encoded strings (requires compiler charset = UTF-8).
- Font data and images are generated using the Baud-dance LED tool (https://led.baud-dance.com).
- HRTIM output polarity is active HIGH, idle level inactive. Dead-time ~100ns at 680MHz timer clock (DIV1 prescaler, 68 ticks).
- ADC and PWM are synchronized: ADC samples at Timer C period match, ensuring clean sampling away from switching noise.
- Button debounce is blocking (HAL_Delay). Control loop runs in ADC ISR so regulation is not affected, but OLED refresh pauses during button presses.
- TIM1 PWM channels (PC0/PC13, PC1/PB0) are configured but not actively used — may be repurposed for auxiliary functions.
- Switching frequency: `f_sw = f_HRTIM × CK_PSC / Period = 170MHz × 4 / 34000 = 20kHz`
