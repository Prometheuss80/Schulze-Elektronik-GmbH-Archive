# Schulze Elektronik Archive & Reverse Engineering Project

An independent preservation, documentation and reverse-engineering project dedicated to legacy **Schulze Elektronik** battery chargers, balancers and related equipment.

The purpose of this repository is to preserve technical knowledge about these devices and to make the results of ongoing reverse-engineering work publicly available.

Many Schulze devices are still in use today, but detailed technical information, service documentation and development tools are becoming increasingly difficult to find.

This project attempts to document how these devices work and to provide information and tools that may help other enthusiasts repair, maintain, study and further develop them.

---

## Projects

### Schulze ISL8

Reverse engineering of the Schulze ISL8 charger family, primarily the **ISL8-936G**.

Current areas of research include:

* Siemens 80C537 processor architecture
* EPROM firmware analysis
* external SRAM and Timekeeper memory
* firmware execution tracing
* address-bus capture and logging
* firmware reconstruction
* LCD controller and display routines
* keyboard and I/O decoding
* command tables and internal firmware structures
* measurement system
* factory calibration routines
* service and diagnostic functions
* hardware documentation
* development of custom reverse-engineering tools

A major part of this project is the development of a complete hardware/software system capable of monitoring firmware execution and reconstructing the internal operation of the charger.

---

### Schulze ISL6

Reverse engineering and documentation of the earlier Schulze ISL6 charger.

Research includes:

* Siemens 80C535 processor
* EPROM firmware
* 93LC46 EEPROM
* HD44780-compatible LCD
* keyboard interface
* serial communication
* firmware routines
* measurement and calibration functions
* hardware documentation

The ISL6 provides an interesting comparison with the later and considerably more complex ISL8 architecture.

---

### XCAL

Research and documentation related to Schulze calibration equipment and calibration procedures.

The goal is to understand the original calibration system and, where possible, reproduce the functionality required to service and calibrate surviving Schulze equipment.

---

### LipoProfiBal06

Hardware analysis and redevelopment of the Schulze LipoProfiBal battery balancer.

This project includes:

* reverse engineering of the original hardware
* analysis of the measurement system
* balancing power stages
* MOSFET control
* isolated gate-drive circuitry
* current measurement
* power supply architecture
* communication interface
* development of a modern replacement PCB

The new PCB is intended to preserve the operating principles of the original device while using components that are available today.

---

## Factory Calibration Research

Some Schulze firmware contains factory calibration and diagnostic functionality that is normally inaccessible to the user.

Part of this project investigates these routines and documents how they operate.

Where firmware modifications are required, the preferred approach of this repository is to provide **patches and patching tools rather than redistributing complete copyrighted firmware images**.

A user should obtain the original firmware from their own device and apply the documented modification locally.

---

## Repository Philosophy

This repository is intended primarily as a **technical archive and collaborative research resource**.

The project is developed in my spare time, and it is not possible for me to work on all areas simultaneously.

Anyone interested in a particular part of the project is welcome to study the available material, create a fork and continue the research independently.

Useful discoveries, corrections, documentation and improvements are welcome.

There is no requirement to understand the entire system. Individual areas such as firmware analysis, 8051 assembly, analog electronics, PCB design, calibration, communication protocols or software tools can be investigated independently.

---

## Contributing

Contributions are welcome.

You can:

* fork the repository
* investigate any part of the hardware or firmware
* improve documentation
* develop analysis tools
* verify existing findings
* submit corrections
* open an Issue with additional information
* submit a Pull Request with improvements

Because this is a spare-time preservation and research project, responses to Issues and Pull Requests may not always be immediate.

---

## Original Schulze Material

This repository distinguishes between:

1. original work produced as part of this reverse-engineering project, and
2. copyrighted material originally produced by Schulze Elektronik.

Reverse-engineering notes, original software tools, measurements, photographs, diagrams and newly created documentation may be published here.

Original Schulze firmware images, manuals and other copyrighted material will only be included where redistribution rights are known to permit it.

Where redistribution rights are unclear, the repository will instead document the material and provide information necessary for users who already possess the original files or hardware.

---

## Safety

Battery chargers and balancers can operate with significant electrical power.

Modified firmware, calibration procedures and experimental hardware may disable or bypass normal protection mechanisms.

Incorrect calibration or modification can result in inaccurate voltage or current measurement, battery damage, equipment damage, fire or other hazardous conditions.

Information in this repository is provided for research, repair and educational purposes. Anyone reproducing the experiments or modifications is responsible for verifying the results and using appropriate safety precautions.

---

## Project Status

**Work in progress.**

A substantial amount of reverse-engineering work has already been completed, particularly for the ISL8 platform, but much of the existing research still needs to be organized and transferred into this public repository.

Documentation and tools will therefore be added progressively.

If you have experience with Schulze equipment, 8051 firmware, electronics repair or reverse engineering, contributions are welcome.

---

## Disclaimer

This is an **independent community preservation and reverse-engineering project**.

It is not affiliated with, sponsored by, or endorsed by Schulze Elektronik.

All trademarks and original product names remain the property of their respective owners.

