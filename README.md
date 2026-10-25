# ThreeMan_SW

## Description
Firmware for the ThreeMan dice game (see [ThreeMan_HW](https://github.com/znibb/ThreeMan_HW)), running on an ATtiny85.

On every boot the firmware:
1. Seeds the random number generator from ADC noise on the floating PB2 pin, mixed with a boot counter kept in EEPROM so that every boot gets a different seed.
2. Rolls two dice (1-6, without modulo bias).
3. Shifts the pip pattern for both dice out to the 74HC595 (DATA on PB3, CLOCK on PB4) and then sleeps in power-down mode.

There is no main loop: the push-button resets the MCU, which triggers a new roll. Since SHCLK and STCLK share one clock net, a 9th clock pulse is sent after the 8 data bits to latch the output.

The EEPROM boot counter is preserved across flashes (EESAVE fuse) and can be read with `make read-counter`.

## Fuse settings
Target: ATtiny85. Values are set in the `Makefile` (`LFUSE`, `HFUSE`, `EFUSE`).
Use `make read-fuses` to read them from the chip and `make fuses` to write them.
A fuse bit of 0 means *programmed*, 1 means *unprogrammed*.

| Fuse | Value |
| --- | --- |
| Low | `0xe2` |
| High | `0xd7` |
| Extended | `0xff` |

### Low fuse `0xe2` = `1110 0010`
| Bit | Name | Value | Effect |
| --- | --- | --- | --- |
| 7 | CKDIV8 | 1 | Clock not divided, so the MCU runs at 8 MHz |
| 6 | CKOUT | 1 | Clock not output on a pin |
| 5:4 | SUT1:0 | `10` | Start-up time 14 CK + 64 ms (slowly rising power) |
| 3:0 | CKSEL3:0 | `0010` | Internal 8 MHz RC oscillator |

### High fuse `0xd7` = `1101 0111`
| Bit | Name | Value | Effect |
| --- | --- | --- | --- |
| 7 | RSTDISBL | 1 | PB5 remains the reset pin (needed for the push-button) |
| 6 | DWEN | 1 | debugWIRE disabled |
| 5 | SPIEN | 0 | ISP programming enabled (needed to flash) |
| 4 | WDTON | 1 | Watchdog not forced on |
| 3 | EESAVE | 0 | EEPROM preserved across chip erase (boot counter survives flashing) |
| 2:0 | BODLEVEL | `111` | Brown-out detection disabled |

### Extended fuse `0xff`
| Bit | Name | Value | Effect |
| --- | --- | --- | --- |
| 0 | SELFPRGEN | 1 | Self-programming disabled |

### Notes
- `F_CPU` in the `Makefile` (8 MHz) matches the real clock because CKDIV8 is unprogrammed.
- Never program RSTDISBL: PB5 would stop working as reset and ISP access is lost.
- `make read-counter` prints the boot counter stored in EEPROM (16-bit, little-endian at address 0) in decimal. It counts resets, not strictly rolls.

## Changelog
The format is based on [Keep a Changelog](https://keepachangelog.com/) using a version naming convention based on [Semantic Versioning](https://semver.org/).  

All notable changes to this PCB project will be documented in [CHANGELOG.md](CHANGELOG.md).