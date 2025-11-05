# **Emu68EDID** version 1.0

## **Description**

The `Emu68EDID` program is an AmigaOS command line tool, for [PiStorm](https://github.com/captain-amygdala/pistorm)/[Emu68](https://github.com/michalsc/Emu68/releases), to gather information from the connected display (monitor or TV), through the Raspberry Pi > Mailbox interface > [Get EDID block](https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface#get-edid-block).

This program retrieves the [EDID](https://en.wikipedia.org/wiki/Extended_Display_Identification_Data) data from your monitor, parse it, and outputs the decoded data.

This allows identifies, for example, the best native preferred resolution supported by a display device (old display may not support this feature).

Written by `Philippe CARPENTIER`, 2025.

Compiled with SAS/C 6.59 for AmigaOS/M68K.

Freely distributed for non-commercial purposes.

## **Arguments**

```
DUMP/S:    Outputs the EDID binary data in standard hexadecimal representation.
TO/K:      Saves the EDID binary data to file (for example: TO=EDID.bin).
PARSE/S:   Outputs the EDID decoded data, in human readable format.
FULL/S:    Outputs the EDID decoded data, in a more exhaustive way.
FROM/K:    Outputs the EDID decoded data, from a previously saved EDID binary data file.
```

## **Syntax**

```
Emu68EDID DUMP [TO=<file>]
Emu68EDID PARSE [FROM=<file>] [FULL]
```

## **Example**

```
Emu68EDID ?
Emu68EDID DUMP
Emu68EDID DUMP TO=RAM:EDID.bin
Emu68EDID PARSE
Emu68EDID PARSE FULL
Emu68EDID PARSE FROM=RAM:EDID.bin
Emu68EDID PARSE FROM=RAM:EDID.bin FULL
```

## **Remarks**

The `EDID` data is composed of blocks of 128 bytes. The most important data are in the first block 0.
Then, optionally, there can be more, those are blocks of `EDID` extensions data.
Most of the times we get 2 blocks of 128 bytes, so a dumped file is usually 256 bytes.

The internal `EDID` decoder used in this program is "borrowed" from the `SDL2` project [here](https://github.com/libsdl-org/SDL/blob/main/src/video/x11/edid-parse.c).

The byte data obtain with the `DUMP` option can be copy/pasted alternatively into any valid Online EDID parsers, such as:

> https://www.edidreader.com/

> https://hverkuil.home.xs4all.nl/edid-decode/edid-decode.html

Type `EDID reader` or `EDID decoder` in any web search engine.

## **Output example**

```
RAM:> Emu68Edid PARSE FULL
Checksum: 0 (correct)
Manufacturer Code: BNQ
Product Code: 0x8013
Serial Number: 21573
Production Week: 33
Production Year: 2017
Model Year: unspecified
EDID revision: 1.3
Display is digital
Bits Per Primary: 8
Interface: undefined
RGB 4:4:4: yes
YCrCb 4:4:4: no
YCrCb 4:2:2: no
Width: 340 mm
Height: 270 mm
Aspect Ratio: undefined
Gamma: 2.200000
Standby: no
Suspend: no
Active Off: yes
SRGB is Standard: yes
Preferred Timing Includes Native: yes
Continuous Frequency: no
Red   X: 0.644531
Red   Y: 0.339844
Green X: 0.288086
Green Y: 0.606445
Blue  X: 0.146484
Blue  Y: 0.066406
White X: 0.313477
White Y: 0.329102
Established Timings:
  800 x 600 @ 60 Hz
  640 x 480 @ 75 Hz
  640 x 480 @ 60 Hz
  720 x 400 @ 70 Hz
  1280 x 1024 @ 75 Hz
  1024 x 768 @ 75 Hz
  1024 x 768 @ 60 Hz
  832 x 624 @ 75 Hz
  800 x 600 @ 75 Hz
  1152 x 870 @ 75 Hz
Standard Timings:
  1280 x 720 @ 60 Hz
  1280 x 800 @ 60 Hz
  1280 x 960 @ 60 Hz
  1280 x 1024 @ 60 Hz
Timing (Preferred): 
  Pixel Clock: 108000000
  H Addressable: 1280
  H Blank: 408
  H Front Porch: 48
  H Sync: 112
  V Addressable: 1024
  V Blank: 42
  V Front Porch: 1
  V Sync: 3
  Width: 338 mm
  Height: 270 mm
  Right Border: 0
  Top Border: 0
  Stereo: No Stereo
  Digital Sync:
    composite: no
    serrations: no
    negative vsync: no
    negative hsync: yes
Detailed Product information:
  Product Name: BenQ BL702
  Serial Number: T8H00835SL0
  Unspecified String:
```

## **Screenshot**

<img width="687" height="1080" alt="image" src="https://github.com/user-attachments/assets/5b24f568-e9d8-43a7-84af-43371601083d" />

.
