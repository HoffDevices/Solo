# Hoff Solo

## Description

The Solo is a programmable MIDI master clock and controller, built by Hoff Devices.

## Getting Started

### Dependencies

* Raspberry Pi Pico C/C++ SDK

### Installing on Windows

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

### Build the project

* This is how I do this on Windows 10, with the one-click installer. I'm sure there are better ways of doing this but I found this to work for me.
    * Right-click in the Solo folder and select "open with Code". This sets the Visual Studio code environment to this folder.
    * Exit, then run the "Pico- Visual Studio Code" app that was installed. This should open the same folder, but with the build environment set up correctly.
    * You may need to set the active kit.
    * Build the project by clicking on the "Build" button on the bottom bar.
    * Hopefully it should build and create the UF2 file.

### Update the Pico
* Connect the Pico to your PC using the USB cable
    * It should show up as a USB drive (D:)
* Go to the build folder (...\pico-projects\Solo\build) and copy the UF2 file to the USB drive (D:)
* As soon as it's done copying, the unit should restart and will be updated.

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

