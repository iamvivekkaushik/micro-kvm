# 🌐 Network-Controlled PC Power Switch (ESP8266)

This project allows you to **remotely power ON / OFF a PC over the 
network** using a **NodeMCU ESP8266**.  
It works by **simulating a physical power button press** on the 
motherboard using an **optocoupler**, ensuring complete electrical 
isolation and safety.

The existing **PC case power button continues to work normally**, as it is 
wired in parallel with the ESP8266 control circuit.

---

## ✨ Features

- 🌐 HTTP-based control using simple GET requests
- 🔒 Optically isolated (PC817 / EL817 optocoupler)
- 🖱️ Physical power button remains functional
- ⏱️ Accurate momentary press simulation (300–500 ms)
- 📄 Lightweight JSON responses (no ArduinoJson dependency)
- 📡 Wi-Fi diagnostics endpoint
- 🏠 Easy integration with Home Assistant, curl, scripts, or browsers

---

## 🧠 How It Works

1. ESP8266 connects to your Wi-Fi network
2. Runs a lightweight HTTP server
3. On API request:
   - Activates a GPIO pin briefly
   - Drives an optocoupler LED
   - Optocoupler transistor shorts the motherboard **PWR_SW** pins
4. Motherboard detects a normal power button press

The optocoupler ensures:
- No shared ground
- No voltage injection
- Complete electrical isolation

---

## 🔌 Hardware Requirements

- NodeMCU ESP8266
- PC817 / EL817 optocoupler
- 220Ω resistor
- Jumper wires
- Access to motherboard **PWR_SW** header

---

## 🔗 Wiring Overview

- ESP8266 GPIO → Optocoupler LED (via 220Ω resistor)
- Optocoupler transistor → Motherboard PWR_SW pins
- Case power button remains connected in parallel

> ⚠️ **Never connect ESP8266 GPIO pins directly to the motherboard**

---

## 🌐 Available API Endpoints

### 1️⃣ Power Button Trigger
**Simulates a single press of the PC power button**

## Wiring Diagram

ESP8266 NODEMCU V3                          CIRCUIT BOARD / WIRING                               PC MOTHERBOARD & CASE
    +--------------------+                                                                      +---------------------+
    |                    |                                                                      |                     |
    |                    |          [POWER SWITCH CIRCUIT]                                      |  Front Panel Header |
    |                    |                                                                      |                     |
    |  D1 (GPIO 5)    ---|----[ 220 Ohm ]----(Pin 1) PC817 Optocoupler (Pin 4)-+--<Wire A>------|-- POWER SW (+)      |
    |  (Output: Control) |                           [   .   ]        _________|                |                     |
    |                    |                           [       ]        |                         |                     |
    |  GND --------------|-------------------(Pin 2) [_______] (Pin 3)--------+---<Wire B>------|-- POWER SW (-)      |
    |                    |                                 ___________|       |                 |                     |
    |                    |                                 |                  |                 |                     |
    |                    |                                 +------[ O ]-------+                 +---------------------+
    |                    |                                 Physical Case Button                  (Wires A & B connect
    |                    |                                  (Pass-Through)                        in parallel)
    |                    |
    |                    |
    |                    |                                                                                            
    |                    |          [POWER LED STATUS CIRCUIT]                                                         
    |                    |                                                                      +---------------------+
    |                    |                                                                      |                     |
    |  D5 (GPIO 14)   ---|----------(Pin 4) PC817 Optocoupler (Pin 1)-<Wire C>-[ 220 Ohm ]--+---|-- PLED (+)          |
    |  (Input: Sense)    |                    [   .   ]                                     |   |  (5V Output)        |
    |                    |                    [       ]                                _____|   |                     |
    |                    |                    [       ]                               |         |                     |
    |                    |                    [       ]                               |         |  PLED (-)           |
    |  GND --------------|---------+--(Pin 3) [_______] (Pin 2)-----<Wire D>-----------------+--|-- (Ground)          |
    |                    |                                                            |      |  |                     |
    |                    |                                                            |      |  +---------------------+
    +--------------------+                                                            |      |
                                                                                [ ( + ) LED ( - ) ]
                                                                                 Physical Case LED
                                                                                   (Pass-Through)
