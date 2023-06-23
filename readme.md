# Hoff Solo

## Description

The Solo is a programmable MIDI master clock and controller, built by Hoff Devices.

## Getting Started

### Dependencies

* Raspberry Pi Pico C/C++ SDK

### Installing

* For the latest SDK:
    * https://www.raspberrypi.com/documentation/microcontrollers/raspberry-pi-pico.html
        * Click on the "one-click installer" to download and install pico-setup-windows-x64-standalone.exe
    * Or download and run [the latest release]
        * https://github.com/raspberrypi/pico-setup-windows/releases/latest/download/pico-setup-windows-x64-standalone.exe

* Go to the Pico folder in %USERPROFILE%
```
cd C:\Users\YourUserName\Documents\Pico-v1.5.1
```

* Create a folder for your projects
```
mkdir pico-projects
```

* Clone the latest repository
```
git clone https://github.com/HoffDevices/Solo
```

* Make sure your PICO_SDK_PATH environment variable is set to the correct path, e.g.
    * C:\Program Files\Raspberry Pi\Pico SDK v1.5.1

### Build the UF2 file

* Delete the build\CMmakeCache.txt file if you've updated to a newer SDK
* Create a new makefile
    * cmake -G "MinGW Makefiles" ..

* Build the project
```
cd Solo\build
make
```

### Update the Pico
* Connect the Pico to your PC using the USB cable
    * It should show up as a USB drive (D:)

* copy the UF2 file to the Pico
```
cd Solo\build
cp ./main.uf2 d:/

```

## Help

* TODO: add link to manual

## Authors

Contributors names and contact info

ex. [@hoff devices](https://www.hoffdevices.com/)

## Version History

* 0.0
    * TODO

## License

This project is licensed under the [NAME HERE] License - see the LICENSE.md file for details

