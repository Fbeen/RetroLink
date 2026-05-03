# RetroLink Firmware (CH559)

Firmware for the RetroLink adapter: a USB-to-retro controller interface (Atari ST / Amiga / C64).

This firmware runs on the **CH559 (8051 + USB host)** and translates USB HID input (mouse / joystick / gamepad) into classic digital and quadrature signals.

---

## About the Firmware Language

This firmware is written in a specialized dialect of **C targeting the 8051 architecture**, specifically for the **WCH CH559 microcontroller**.

Although it looks like standard C, there are several important differences compared to desktop or ARM-based embedded development:

- Uses **8051-specific memory qualifiers** such as:
  - `__code` (flash memory)
  - `__xdata`, `__idata`, etc. (different RAM regions)
- Strong awareness of **very limited RAM**
- Heavy reliance on **interrupt-driven design**
- Direct manipulation of **special function registers (SFRs)**
- Tight coupling to hardware timing (especially for quadrature signals)

Because of this, the code is closer to **"embedded C for 8-bit MCUs"** than modern C used on platforms like ARM Cortex (e.g. STM32 or RP2040).

### Why this matters

If you plan to modify or extend this firmware:

- Be mindful of **memory placement** (RAM vs flash)
- Avoid large stack usage or dynamic allocation
- Keep interrupt routines **fast and deterministic**
- Understand that some constructs are compiler- or architecture-specific

In short: this is **low-level, hardware-near C**, optimized for a small 8-bit microcontroller rather than a general-purpose system.

---

# Purpose of this README

This document is intended to:

- Help new developers quickly understand the project
- Make maintenance and extensions easier
- Provide insight into architecture, timing, and hardware interaction

---

# Project Structure

The firmware is modular and split by functionality:

    main.c              entry point + main loop + timers
    USBHost.c           USB host stack + HID handling
    hid_mouse.c         USB mouse report parsing
    hid_joystick.c      USB joystick/gamepad parsing

    retro_mouse.c       quadrature signal generation (Atari/Amiga)
    retro_joystick.c    digital joystick output

    hardware.c          GPIO setup and hardware initialization
    led.c               LED timing and animations
    console.c           UART debug / configuration interface

    config.c            persistent configuration + mappings
    util.c              helper functions

---

# Firmware Architecture

## 1. Data Flow (High-Level)

### Mouse pipeline:

    USB HID report
     hid_mouse_parse()
     movement buffer (mx/my)
     timer interrupt
     rm_nextStep()
     quadrature output (PORT2)

### Joystick pipeline:

    USB HID report
     hid_joystick_parse()
     mapping via config
     direct GPIO output (ACTIVE LOW)

---

## 2. Main Loop (main.c)

`main.c` is responsible for:

- Initialization:
  - Clock setup
  - USB host
  - GPIO
  - Timers
- Polling:
  - USB devices (USBHost_*)
- Mode switching:
  - Mouse vs joystick

---

# GPIO & Hardware

## GPIO Definition

GPIO is centrally managed in:

    hardware.c

This file defines:

- Direction (input/output)
- Open-drain / push-pull modes
- Default signal levels

### Important:

- **All joystick signals are ACTIVE LOW**
- Idle = HIGH
- Active = LOW

---

## Output Mapping

Typically using `PORT2`:

| Function | Signal |
|----------|--------|
| UP       | GPIO |
| DOWN     | GPIO |
| LEFT     | GPIO |
| RIGHT    | GPIO |
| FIRE     | GPIO |

Mouse signals:

- Quadrature signals (X1/X2, Y1/Y2)
- Layout differs between Atari ST and Amiga

---

## Open Drain Behavior

The firmware uses:

- Open-drain / open-collector style outputs
- Safe for retro hardware
- Mimics original joystick electronics

---

# Timers

## Timer0 (Main Timing Engine)

Used for:

- Mouse quadrature timing
- Autofire
- LED timing
- Button handling

Configured in `main.c`:

    #define mTimer0ClkFsys()

---

## Timing Functions

### 1. Mouse Stepping

Executed in ISR:

    rm_nextStep()

- Processes mx/my buffers
- Generates correct quadrature phase sequence

---

### 2. Autofire

Key variables:

    autofire_counter
    autofire_state

- ~2400 Hz interrupt base
- ~10 Hz output signal

---

### 3. Button Timing

    BTN_STEP   = 10   ~1 second
    BTN_T_BOOT = 100  ~10 seconds

Used for:

- Short press detection
- Long press detection
- Menu selection

---

# USB HID Processing

## USB Stack

File:

    USBHost.c

Responsibilities:

- Device enumeration
- HID interface selection
- Polling

---

## Device Detection

- Protocol 2 Mouse
- Otherwise Joystick/Gamepad

---

## Parsing

### Mouse:

    hid_mouse.c

- X/Y movement
- Buttons

### Joystick:

    hid_joystick.c

- Axis direction
- Buttons fire/autofire

---

# Retro Output

## Mouse (retro_mouse.c)

- Maintains mx/my buffers
- Generates quadrature signals
- Supports:
  - Atari ST
  - Amiga

---

## Joystick (retro_joystick.c)

- Direct GPIO mapping
- Active LOW output
- Configurable mapping

---

# Configuration

File:

    config.c

Contains:

- Controller mappings
- Mouse speed
- Autofire settings
- Button swap
- Mode (ST/Amiga)

Storage:

- Flash memory
- Version/magic validation

---

# LED & UI

File:

    led.c

Handles:

- Status feedback
- Menu navigation

---

## Button + LED UI

| Action       | Result             |
|--------------|-------------------|
| Short press  | Show menu (UART)  |
| Long press   | Select option     |
| LED flashes  | Menu index        |

---

# Console (UART)

File:

    console.c

Features:

- Debug output
- Configuration interface
- Menu system

---

# Utilities

File:

    util.c

Contains:

- Helper functions
- Parsing utilities
- General shared logic

---

# Important Notes

## 1. Limited RAM

- CH559 has very limited RAM
- Store constant data in `__code` (flash)

---

## 2. Timing is Critical

- Mouse quadrature must be precise
- ISR must remain lightweight

---

## 3. USB Polling

- Must run frequently
- Avoid blocking code in main loop

---

## 4. Open Drain Outputs

- Never force push-pull on retro lines
- Prevents hardware damage

---

# How to Extend

## Adding New Features

Typical workflow:

1. Extend parsing in:
   - hid_*
2. Add mapping in:
   - config.c
3. Adjust output in:
   - retro_*
4. Extend UI:
   - console.c
   - led.c

---

## Adding New Hardware

- Update GPIO config in `hardware.c`
- Keep mappings consistent

---

# Summary

This firmware is built around:

- **USB HID input**
- **Timer-driven output**
- **Strict timing**
- **Modular design**

Core principles:

- Keep ISR minimal
- Use buffers for decoupling
- Drive hardware safely

---

# Final Notes

If you want to understand or extend this project:

1. Start with `main.c` (overall flow)
2. Then study `USBHost.c` (input layer)
3. Finally inspect `retro_*` (output layer)

This gives you a complete understanding of the system.