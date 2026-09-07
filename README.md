# Schulze Elektronik GmbH Archive and Reverse Engineering Project

An independent preservation, documentation and reverse engineering project dedicated to older battery chargers, balancers and related equipment from **Schulze Elektronik GmbH**.

The purpose of this repository is to preserve technical knowledge about these devices and to make the results of ongoing reverse engineering available to the public.

Many Schulze devices are still in use today, but detailed technical information, service documentation and development tools are increasingly difficult to find.

This project attempts to document the operation of these devices and provide information and tools that can help other enthusiasts with their repair, maintenance, study and further development.

---

## Projects

### Schulze ISL8-936G

Reverse engineering of the Schulze **ISL8-936G** charger family.

Current research areas include:

* Siemens 80C537 Processor Architecture
* ROM Firmware Analysis
* External SRAM and Timekeeper M48T08
* 93LC46 EEPROM
* Firmware Execution Tracing
* Address Bus Capture and Logging
* Firmware Reconstruction
* LCD Control and Display Routines
* Keyboard and I/O Decoding
* Serial Communication
* Command Tables and Internal Firmware Structures
* Measurement System
* Factory Calibration Routines
* Service and Diagnostic Functions
* Hardware Documentation
* Development of Custom Reverse Engineering Tools

The main part of this project is the development of a complete hardware/software system capable of monitoring firmware execution and reconstructing the internal operation of the charger.

---

### Schulze ISL6-636+

Reverse Engineering and Documentation of the Schulze ISL6-636+ Charger

The research includes:

* Siemens 80C535 processor
* ROM firmware
* 93LC46 EEPROM
* HD44780 compatible LCD
* keyboard interface
* serial communication
* firmware routines
* measurement and calibration functions
* hardware documentation

ISL6 provides an interesting comparison with the later and considerably more complex ISL8 architecture.

---

### XCAL for ISL8

Research and documentation on the Schulze calibration device and automatic calibration procedures.

This research was successfully confirmed and completed. XCAL has the same function as the original Schulze device.

It is a HW switch of specified voltages for charger calibration. This BETA test module was assembled, programmed and successfully tested.

The goal was to understand the original calibration system and reproduce the functionality needed to calibrate the existing Schulze device.

---

### XTOOLS for ISL8

Research and documentation on the Schulze calibration device and automatic calibration procedures.

This XTOOLS module was the first functional SW tool for reading and writing calibrations stored in the M48T08 timekeeper in the ISL8-936G. The first research lasted three years and revealed the way to use calibration constants.

It was manufactured and successfully tested, operating on the principle of a ROM adapter, with an IO switch ROM with XTOOLS and a ROM with FW850EN.

The goal was to understand the original calibration system and reproduce the functionality needed to calibrate the existing Schulze device.

---

### LipoProfiBal06

Hardware analysis of the Schulze LipoProfiBal08 and LipoProfiBal14 battery balancer.

This project includes:

* reverse engineering of the original hardware
* measurement system analysis
* power stage balancing
* MOSFET control
* isolated gate control circuits
* current measurement
* power supply architecture
* communication interface
* development of a modern replacement printed circuit board (PCB)

The new printed circuit board is to maintain the principles of operation of the original device while using components that are available today.

---

## Factory Calibration Research

Some Schulze charger firmware contains factory calibration and diagnostic functions that are typically inaccessible to the user.

Part of this project is to investigate these routines and document their operation. A number of ISL8 chargers have been successfully exposed and run as factory calibration.

Where firmware modifications are required, the preferred approach of this repository is to provide **patches and patching tools, rather than redistributing complete copyrighted firmware images**.

The user should obtain the original firmware from their own device and apply the documented modification locally.

The procedure for modifying the 8.50EN firmware for the ILS8-936G charger will be presented here, which will enable the factory hidden voltage calibration routine to be run.

---

## Repository Philosophy

This repository is primarily intended as a **technical archive and resource for collaborative research**.

The project is being developed in my spare time and it is not possible for me to work on all areas at the same time.

Anyone interested in a specific part of the project is welcome to study the available materials, create a fork and continue the research independently.

Useful discoveries, fixes, documentation and improvements are welcome.

It is not necessary to understand the entire system. Individual areas such as firmware analysis, 8051 assembly, analog electronics, PCB design, calibration, communication protocols or software tools can be explored independently.

---

## Contributions

Contributions are welcome.

You can:

* fork the repository
* explore any piece of hardware or firmware
* improve the documentation
* develop analytical tools
* verify existing knowledge
* submit fixes
* open an Issue with additional information
* submit a Pull Request with improvements

Since this is a project focused on preservation and leisure research, responses to Issues and Pull Requests may not always be immediate.

---

## Original material from Schulze

This repository distinguishes between:

1. original work created within the scope of this reverse engineering project and

2. copyrighted material originally created by Schulze Elektronik GmbH.

Reverse engineering notes, original software tools, measurements, photos, diagrams, and newly created documentation may be published here.

Original firmware images, manuals, and other copyrighted material from Schulze will only be included where it is known that redistribution rights allow it.

If redistribution rights are not clear, the repository will instead document the material and provide information needed for users who already own the original files or hardware.

---

## Security

Battery chargers and balancers can operate with significant electrical power.

Modified firmware, calibration procedures, and experimental hardware can disable or bypass normal protection mechanisms.

Improper calibration or modification can lead to inaccurate voltage or current measurements, battery damage, equipment damage, fire, or other hazardous conditions.

The information in this repository is provided for research, repair, and educational purposes. Anyone reproducing experiments or modifications is responsible for verifying the results and using appropriate safety precautions.

---

## Project Status

**Work in progress.**

A significant amount of reverse engineering has already been completed, particularly for the platform

It is not affiliated with, sponsored by, or endorsed by Schulze Elektronik.

All trademarks and original product names remain the property of their respective owners.

