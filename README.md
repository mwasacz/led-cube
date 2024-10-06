# LED Cube

A 4x4x4 LED cube based on ATtiny85 and MAX7219, with SD card support.

![LED Cube](Photos/Led%20Cube%205.jpg)

## Description

This LED cube is a volumetric display capable of displaying simple animations. The on/off state of each LED can be
controlled individually, and the global brightness is adjustable. The controller supports up to 99 animations which are
stored on an SD card.

## Hardware

The controller is based on the ATtiny85 MCU and MAX7219 LED driver. The circuit is simple enough that it can be built on
a protoboard. Refer to [LedCube.sch.png](PCB/LedCube.sch.png) for the schematic. The most difficult part is soldering
the LEDs. Some photos are provided in the [Photos](Photos) directory, which can be used as guidance. The spacing between
LEDs is 2.5cm.

Although the cube size is 4x4x4, logically the LEDs are connected as a multiplexed 8x8 grid. As shown in the
[photo](Photos/Led%20Cube%204.jpg), there are 8 horizontal wires which form the 8 anodes of the matrix. There are 12
vertical wires, but each wire on the left-hand side is internally connected to the corresponding wire on the right-hand
side. This gives us the 8 cathodes of the matrix. These 8 cathodes and 8 anodes should be connected to the CATHODES and
ANODES ports on the schematic.

## Compilation

The code should be compiled with AVR-GCC (version 5.4.0 is recommended). To build the project, follow the instructions:

- Windows: open [LedCube.atsln](Source/LedCube.atsln) in Microchip Studio 7.0 (formerly Atmel Studio 7.0) and compile
the project. Make sure the Release configuration is selected.
- Linux: you can try [as2make](https://github.com/noctarius/as2make) to generate a makefile from
[LedCube.cproj](Source/LedCube/LedCube.cproj), but I haven't tested it.

Use an ISP programmer (such as USBasp) to flash the MCU. Remember to program the Flash and Fuse Bits (see
[main.c](Source/LedCube/main.c) for Fuse Bit settings).

## Usage

The device requires an SD card with animation files. The card should be formatted as FAT12, FAT16, or FAT32. The
animation files should be stored at the root directory and named `01.txt`, `02.txt`, `03.txt`, etc. Up to 99 files are
supported. The animation files must be saved in a custom binary format (despite the `.txt` extension, for historical
reasons).

The easiest way to create animations is to use [LED Cube Animator](https://github.com/mwasacz/led-cube-animator). Make
sure to set cube size to 4 and select Mono or MonoBrightness color mode. Note that brightness control of individual
voxels is not supported, only the whole cube brightness is affected in MonoBrighness mode. Once the animation is
finished, you can export it using File > Export > MW LED Cube file (.txt), and save it on the SD card. In the future I
may update the code to support the standard `.3db` file format used by LED Cube Animator.

After preparing the SD card, insert it into the device and connect power supply (4.5V to 5.5V DC). By default, the
display will work in automatic mode, meaning that it will show all animation files in sequence. By pressing the button
you can switch to manual mode where one animation is shown in a loop. Press the button again to switch to the next
animation. If there is no next animation, the display will go back to automatic mode.

In case of an error, the display will show the error code. After eliminating the problem, press the button to reset the
device. Refer to the following table for possible error codes:

| Error code | Name             | Description                              |
|------------|------------------|------------------------------------------|
| E1         | FR_DISK_ERR      | Internal card error                      |
| E2         | FR_NOT_READY     | No card / Card not responding            |
| E3         | FR_NO_FILE       | Animation file not found                 |
| E4         | FR_NOT_OPENED    | Internal program error                   |
| E5         | FR_NOT_ENABLED   | Internal program error                   |
| E6         | FR_NO_FILESYSTEM | File system not found                    |
| E7         | FILE_ERR         | Animation file structure error           |
| E8         | TIMING_ERR       | Animation file contains too short frames |

## License

This project is released under the [MIT license](LICENSE). It uses a modified version of Petit FatFs library (stored at
[PetitFatFs](Source/LedCube/PetitFatFs) directory). The original version of
[Petit FatFs](http://elm-chan.org/fsw/ff/00index_p.html) by ChaN is released under a
[custom license](http://elm-chan.org/fsw/ff/pf/appnote.html) (similar to BSD 1-Clause license).
