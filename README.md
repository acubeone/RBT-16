# Overview

&nbsp;&nbsp;&nbsp;&nbsp;A micro-computer inspired by the Atari ST and Amiga 500.

&nbsp;&nbsp;&nbsp;&nbsp;The RBT-16 is a micro-computer designed to be an Atari STe and Amiga 500
successor, not limited by their technology and history, but by what we have
and know today. The RBT-16 is a middle ground between 3 worlds: Atari,
Commodore and IBM.<br>
&nbsp;&nbsp;&nbsp;&nbsp;Our design goal is to have something fun and interesting to play and use.

---

## Hardware Specifications

- CPU: Motorola 68000 at ~8/12MHz
- VDP: Tang Primer 20K. Codename - Killy
- APU: YM3812 (OPL2) + DAC
- ROM: 256KB Flash; Stores BIOS and Kernel
- RAM: Shipped with 256KB of Static RAM; Has four RAM expansion modules,
  with up to 4MB max RAM
- RTC: DS3231 module

> CPU can be boosted by software up to 12MHz

> ROM can be rewritten and/or replaced. <br>

> ROM0 -> /UDS chip (D8-D15, even addresses) <br>
> ROM1 -> /LDS chip (D0-D7, odd addresses)

- [RAM Subsystem](docs/ram.md)

### IO Connectors:

- 1 x HD-15 VGA Output
- 1 x Mini-DIN 4-pin S-Video Output
- 1 x RCA Composite Output
- 1 x RCA Audio Jack
- 1 x 3.5mm Audio Jack
- 1 x microSD Card Slot
- 2 x SNES Controllers
- 2 x PS/2 Mouse and Keyboard Ports
- 4 x 2x50 Card Edge Connectors/Expansion Card (Parallel Bus)

### Chips references

- [Video Display Processor](docs/vdp.md)

<!--
NOTES:
	(uotlaf) - CPU: Possibility to change CPU clock via software using
custom wiring.
	(YAN) - CPU: Test clock frequencies stability.
	(aCube) - ROM: Should the ROM be updated from the microSD card? It will
require a read-only bootloader section.
	(YAN) - APU: Use the YM2413 OPLL FM chip as an alternative audio chip
	(aCube) - SD Card: SPI controller should reside inside the VDP. This
simplify implementation.
	(aCube) - Add 525-line interlaced NTSC/PAL-M mode for CRT TV compatibility.
reference: https://github.com/fvdhoef/vera-module

REFS:
	CPU: MC68000P12 | MC68010P12
	VIDEO:
		- Tang Primer 20K -> VDP (Killy)
		- AD724 -> RGB to Composite/S-Video encoder
		Connectors:
			- HD-15 -> VGA Connector
			- Mini-DIN 4-pin -> S-Video Connector
			- RCA -> Composite Jack
		NOTES:
			- VGA and S-Video are share LCD connector signals
			- S-Video uses AD724 encoder fed from LCD connector
			- VGA and S-Video can be used at the same time
			- HDMI is auto-detected; Turns VGA and S-Video off if connected
			- Buy debug dock for the Tang Primer 20K
	APU:
		- YM3812
		- YM3014B -> DAC
		- LM358 | TL072 | NE5532 -> OpAmp
	IO:
		- ATmega32A | ATmega324P -> IO Controller
		- 16MHz crystal oscillator -> Controller Clock
	SRAM:
		- AS6C4008-55PCN -> 512Kx8 = 512KB
		- AS6C1008-55PCN -> 128Kx8 = 128K
	ROM:
		- SST39SF010A -> Flash 128Kx8 = 128KB
		- SST39SF040  -> Flash 512Kx8 = 512KB
	ICs:
		- 74HC138 -> Decoder
		- 74HC139 -> Decoder
		- 74HCT244 -> one-way 8-bit buffer, 5V |-> 3.3V
		- 74HCT245 -> bidirectional 8-bit buffer, 5V <-> 3.3V
	PINs:
		- 2x50 Card Edge Connectors
		- 2x30 Card Edge Connectors
	RTC:
		- DS3231 | DS1307 -> I2C, battery-backed, SQW output
		- warn: Remove 200ohms charging resistor if using CR2032 battery
		- 32.768 kHz crystal oscillator -> RTC Clock
		NOTES:
			- The DS1307 has 56-byte SRAM battery-backed
			- The DS3231 has internal crystal oscillator
			- Decide which one to use
-->

---

> Documentation is licensed under CC BY-SA 4.0. <br>
> See https://creativecommons.org/licenses/by-sa/4.0/
