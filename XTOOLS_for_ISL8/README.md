# XTOOLS for Schulze ISL8

**XTOOLS** is an independent service and maintenance tool developed for the **Schulze ISL8 battery charger**.

It was the first project created during the ISL8 reverse-engineering work and originally started as a tool for accessing and managing the charger voltage calibration data.

Unlike a PC-based diagnostic utility, **XTOOLS runs directly on the original ISL8 hardware**.

It contains its own initialization routines and does not require the original Schulze firmware to operate.

---

## Overview

After startup, XTOOLS performs the initialization required to take control of the ISL8 hardware.

This includes:

* initialization of the main CPU
* initialization and configuration of the LCD
* initialization of required peripherals
* configuration of the charger hardware
* placing the charger power circuitry into a safe idle state
* initialization of the memory interfaces

After the hardware initialization is complete, the XTOOLS service environment is started.

The program then provides direct access to selected service and maintenance functions of the charger.

---

## Calibration Data

XTOOLS was originally developed to investigate and service the ISL8 voltage calibration system.

The tool can:

* read the existing calibration values
* display calibration information
* access calibration data stored in the Timekeeper memory
* write calibration values back to the Timekeeper

This made it possible to investigate the calibration system independently of the original charger firmware.

---

## Timekeeper Service

The ISL8 uses a Timekeeper memory for persistent configuration and operating data.

XTOOLS provides service functions for this memory, including:

* reading stored data
* writing selected data
* clearing the Timekeeper contents
* preparing a replacement Timekeeper for installation

This is particularly useful when the original Timekeeper has failed or must be replaced.

### New Timekeeper Initialization

XTOOLS provides a procedure for correctly preparing a new Timekeeper for use in an ISL8 charger.

During this process, the tool restores information required by the charger and reconstructs the charger serial number using data stored separately in an EEPROM on the ISL8 main board.

This makes XTOOLS an important service tool when replacing a failed Timekeeper device.

---

## User Data Service

XTOOLS also provides maintenance functions for selected user configuration data.

Available functions include:

* user password reset
* user name reset
* clearing selected persistent user data

These functions can be useful when servicing a charger for which the original user configuration is no longer known.

---

## Standalone Operation

One of the important characteristics of XTOOLS is that it is a **standalone firmware environment**.

The original Schulze ISL8 firmware is not required for XTOOLS operation.

XTOOLS performs its own:

1. CPU initialization
2. LCD initialization
3. hardware initialization
4. safe-state configuration
5. memory initialization
6. service application startup

The ISL8 therefore effectively operates as dedicated XTOOLS service hardware while the tool is active.

---

## XTOOLS Hardware Adapter

The project includes a dedicated hardware adapter that allows the ISL8 to switch between the original charger ROM and the XTOOLS ROM.

The adapter makes it possible to retain normal operation of the charger while providing access to the independent XTOOLS service environment when required.

The switching system is controlled by a **PIC microcontroller**.

---

## Repository Contents

This directory contains the complete XTOOLS project, including:

### XTOOLS Firmware

The complete source project for the XTOOLS firmware.

The firmware was developed using the **Keil development environment**.

### XTOOLS Adapter

Complete hardware project for the XTOOLS adapter used with the ISL8 charger.

### PCB Manufacturing Data

Manufacturing files for the XTOOLS adapter PCB, including Gerber data required for PCB production.

### PIC Switch Controller

Source/project files for the PIC microcontroller responsible for switching between:

* the original ISL8 ROM
* the XTOOLS ROM

This allows the charger to operate normally with its original firmware or boot into XTOOLS for servicing.

---

## Project Purpose

XTOOLS was created primarily as a **repair, calibration, preservation and reverse-engineering tool** for existing Schulze ISL8 chargers.

The project demonstrates that the ISL8 hardware can be initialized and operated independently of the original firmware and provides direct access to service functions that are useful when maintaining aging hardware.

In particular, XTOOLS can assist with:

* calibration data investigation
* calibration data backup and restoration
* Timekeeper replacement
* Timekeeper initialization
* recovery of charger identification data
* password recovery/reset
* user configuration reset
* hardware research and reverse engineering

---

## Important: Back Up Original Data

Before modifying or clearing the Timekeeper, always create a backup of the original contents whenever possible.

Some data stored in the charger may be unique to the individual device.

Do not erase or overwrite the original memory until a verified backup has been created.

---

## Safety Warning

XTOOLS operates directly on the hardware of a battery charger.

While XTOOLS initializes the charger hardware into an idle state before entering the service application, modifications to calibration data or persistent memory can affect subsequent charger operation.

Incorrect calibration data can cause incorrect voltage measurement and may result in unsafe battery charging.

After modifying calibration or replacing persistent memory, the charger should be fully verified and calibrated before being returned to normal operation.

Use this project only if you understand the hardware and the consequences of modifying its persistent configuration.

---

## Development Status

XTOOLS is a functional project developed from reverse engineering of the Schulze ISL8 hardware.

Additional documentation will be added as the historical development files, hardware documentation and source code are organized for public release.

---

## Disclaimer

XTOOLS is an independent project.

It is not an official Schulze Elektronik product and is not affiliated with, sponsored by, or endorsed by Schulze Elektronik.

Schulze product names are used only to identify the hardware for which this service tool was developed.
