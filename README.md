# hv-ms735-config

[![Build Status](https://api.travis-ci.org/pbludov/hv-ms735-config.svg?branch=master)](https://travis-ci.org/pbludov/hv-ms735-config)
[![Build status](https://ci.appveyor.com/api/projects/status/c9xnl0k4vdi79mor?svg=true)](https://ci.appveyor.com/project/pbludov/hv-ms735-config)

## Introduction
HAVIT Magic Eagle mouse configuration utility.

This program **is not** an official utility from the product vendor.

It is strongly recommended to use the software from the official website:
[http://www.havit.hk/downloads/]

Use this utility only if your system is not supported by the manufacturer,
or if you need some extra features that the official software does not provide.

*Absolutely no warranty*. Perhaps (and in some cases definitely),
the device will be broken. To restore the device from the "brick" state
follow the [instructions](doc/unbrick.md).

* Magic Eagle is a registered trademark of [HAVIT Group](http://www.havit.hk/).

## Installation from sources

### Requirements
For compiling hv-ms735-config yourself, you will need the QT (>= 5.2).
It is free and available at [http://www.qt.io]. You may also need its
dependency libraries and header files, such as libusb-1.0, hidapi-libusb.

Furthermore you need, of course, a C++ compiler and the Make tool.
The GNU versions of these tools are recommended, but not required.

### Making hv-ms735-config with gcc/clang

    qmake
    make

### Making hv-ms735-config with mingw

    qmake
    mingw32-make

### Making hv-ms735-config with Visual Studio

    qmake
    nmake

### Building the DEB package (Debian/Ubuntu/Mint)
    
    dpkg-buildpackage -us -uc -I.git -rfakeroot

### Building the RPM package (Fedora/SUSE/CentOS)
    
    tar czf /tmp/hv-ms735-config.tar.gz * --exclude=.git && rpmbuild -ta /tmp/hv-ms735-config.tar.gz

### Building the MSI package (Windows)
    
    set CONFIGURATION=release
    windeployqt.exe --no-svg --no-angle --no-opengl-sw --no-system-d3d-compiler --no-translations --libdir qtredist --plugindir qtredist %CONFIGURATION%\hv-ms735-config.exe
    heat dir qtredist -cg CG_QtRedist -var var.QtRedistDir -ag -srd -sfrag -dr INSTALLDIR -out qtredist.wxs
    candle -dConfiguration=%CONFIGURATION% -dQtRedistDir=qtredist hv-ms735-config.wxs qtredist.wxs
    light hv-ms735-config.wixobj qtredist.wixobj -out hv-ms735-config.msi

### Building the DMG package (MacOS)

    macdeployqt hv-ms735-config.app -dmg

## Galery
![side buttons](doc/sidebuttons.png)
![macros](doc/macros.png)
![profiles](doc/profiles.png)

### License
hv-ms735-config is distributed under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2.1 of the License,
or (at your option) any later version.  A copy of this license
can be found in the file COPYING included with the source code of this program.

&copy; 2017 Pavel Bludov <pbludov@gmail.com>

