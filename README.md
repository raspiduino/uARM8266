# uARM8266
Real Linux (Ubuntu 9) running on ARM emulator on esp8266.

## What is this?
This is a port of [Dmitry Grinberg](http://dmitry.gr/)'s project to run on esp8266. His original project is to [run Linux on AVR](http://dmitry.gr/?r=05.Projects&proj=07.%20Linux%20on%208bit).

## How?
Because there is no native Linux support for esp8266 (although it's based on xtensa architecture (particularly LX106), which is 32-bits and have MMU, and there is [Linux support for <b>*some*</b> of the xtensa](http://www.linux-xtensa.org/)), so we have to find an architecture that Linux can run on. And I have found the super portable emulator on his project, and port it to esp8266.

## Why?
For fun :)
<br>It will be wonderful if we can run something like Linux on our tiny esp8266.

## How to build?
The project have been written in Arduino-compatible format since esp8266's nonsdk (bare mental) is too complicated (especially the SDIO). So simply you can open the `uARM8266.ino` in the `uARM8266` folder and build it (and upload it) with Arduino IDE.
<br> You can also change the esp8266's freq in Arduino IDE from 80Mhz to 160Mhz for faster emulation.

## Wiring
![wiring](https://user-images.githubusercontent.com/68118236/124540105-f0915500-de48-11eb-84ea-e605d387a7e6.png)
To flash the module, connect GPIO0 to GND. To run connect to VCC.
<br>SD card's pin CS by default connect to GPIO4. but you can change it if you want in `config.h`
<br><b>Optional:</b>You can enable print speed mode in `config.h`. After that you can connect a button to the pin you choosed and when you press it will print out current emulator's speed.

## How to run?
Download the image file from [here](https://github.com/raspiduino/uARM-old/blob/main/jaunty.rel.v2.bz2?raw=true), extract it and when you got a file named `jaunty.rel.v2`, copy it to SD card's root directory (<b>DO NOT</b> use `dd`, just copy it like a normal file!). Also, create a blank file named `uARMram.bin` also in the root directory of the SD card.
<br>Connect GPIO0 to VCC. Plug the RXD0 and TXD0 to the USB-to-serial. You might want to use a capacitor since Esp8266 require a lot of energy and an USB-to-serial cannot provide enough.
<br>Open the terminal and connect to the right COM port with baudrate 115200. Enjoy your Linux!

## Credit
- 99% of the work was done by Dmitry Grinberg

## License
Dmitry's license in LICENSE.txt:
```
This code can be used for non-commercial purposes as long as you publish all code changes and this license file unmodified with them. If integrated in any hardware, the paperwork that comes with the hardware must include this notice.
For commercial use terms, contact me.

--
Dmitry Grinberg
dmtirygr@gmail.com
http://dmitry.co/
```
