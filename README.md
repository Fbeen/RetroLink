# RetroLink

RetroLink is a small **USB input adapter for retro computers**.

It allows modern **USB mice and USB joysticks** to be used on classic systems such as:

- Atari ST
- Commodore Amiga
- Commodore 64

The device translates USB input into the signals expected by the original hardware.  
The goal of this project was to create a **simple, flexible and hackable adapter** that hobbyists can build themselves.

This project started mostly as a fun experiment — and like many hobby projects it slowly grew into something more useful.

---

# Features

- USB mouse support
- USB joystick support
- Works with **Atari ST**, **Amiga**, and **C64**
- Configurable **mouse speed**
- Switch between **ST mouse** and **Amiga mouse**
- **Swap mouse buttons**
- Configurable **autofire frequency**
- **Joystick learning wizard** for mapping any USB controller
- Configuration stored in flash memory
- Simple **terminal based configuration interface**

---

# Hardware

Information about building the hardware can be found here:

```
/hardware/README.md
```

You can either:

- build the PCB yourself  
- or order a **fully assembled board from JLCPCB**

---

# Connecting RetroLink

1. Connect the **RetroLink adapter** to your computer's joystick/mouse port.
2. Connect a **USB mouse or joystick** to the USB-A port.
3. Connect the **USB debug port** to your PC if you want to configure the device.

Configuration is done through a **serial terminal**.

---

# Opening the configuration console

RetroLink provides a **serial terminal interface** for configuration.

Connect the device to your PC and open a serial terminal.

Common tools:

- PuTTY
- TeraTerm
- minicom
- Arduino Serial Monitor

Typical settings:

```
115200 baud
8 data bits
no parity
1 stop bit
```

After connecting you should see something like:

```
RetroLink v1.02
USB Input Adapter
ST / Amiga / C64
github.com/Fbeen/RetroLink
```

---

# Main Menu

The console shows the following menu:

```
1. Set Mouse Speed
2. Emulate ST or Amiga mouse
3. Swap Mouse Buttons
4. Learn Controller
5. Autofire Frequency
```

Simply press the number of the option you want to change.

---

# Setting the Mouse Speed

The mouse speed can be adjusted to match your personal preference.

Available settings:

```
1 - Very Slow
2 - Slow
3 - Normal
4 - Fast
5 - Turbo
```

The selected value is stored in the device configuration.

---

# Switching between Atari ST and Amiga mouse

RetroLink can emulate both mouse protocols.

Menu option:

```
2. Emulate ST or Amiga mouse
```

Each time you select it, the mode toggles between:

- Atari ST mouse
- Amiga mouse

---

# Swapping Mouse Buttons

Some users prefer the mouse buttons reversed.

Menu option:

```
3. Swap Mouse Buttons
```

This toggles between:

- normal button layout
- swapped buttons

---

# Configuring a USB Joystick

USB controllers often use different report formats.  
To support almost any controller, RetroLink includes a **learning wizard**.

Select:

```
4. Learn Controller
```

The wizard will guide you through the setup.

Example flow:

```
Controller Learning Wizard
--------------------------------

Release joystick and scanning...

Push UP
Push DOWN
Push LEFT
Push RIGHT
Press FIRE
Press AUTOFIRE
```

Follow the instructions and move the joystick as requested.

After the last step the configuration will be saved automatically.

---

# Autofire Frequency

RetroLink can generate autofire signals for the joystick.

Menu option:

```
5. Autofire Frequency
```

Available options:

```
1 - Autofire 8 Hz
2 - Autofire 9 Hz
3 - Autofire 10 Hz
4 - Autofire 11 Hz
5 - Autofire 12 Hz
```

Choose the speed that feels best for the game you are playing.

---

# Firmware

Firmware can be compiled and flashed using the scripts in:

```
/scripts
```

More information can be found here:

```
/hardware/README.md
```

---

# Repository

Project repository:

https://github.com/Fbeen/RetroLink

---

# Final notes

This is a **hobby project** built for fun and experimentation with retro hardware.

Feel free to:

- build your own
- modify the hardware
- improve the firmware
- or use the code in your own projects

Contributions and ideas are always welcome.

---

## Disclaimer

This project is provided **as-is**, without any warranty.

Use this hardware and firmware **entirely at your own risk**.  
The author cannot be held responsible for **any damage to hardware, software, data, or other equipment** resulting from the use or misuse of this project.

Building and using the RetroLink adapter is a **DIY hobby activity**, and you are responsible for ensuring it is used safely with your own equipment.