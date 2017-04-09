# hv-ms735-config

[![Build Status](https://api.travis-ci.org/pbludov/hv-ms735-config.svg?branch=master)](https://travis-ci.org/pbludov/hv-ms735-config)
[![Build status](https://ci.appveyor.com/api/projects/status/c9xnl0k4vdi79mor?svg=true)](https://ci.appveyor.com/project/pbludov/hv-ms735-config)

## Introduction
HAVIT Magic Eagle mouse configuration utility.

This program is **not** an official utility from the product vendor.

It is strongly recommended to use the software from the official website:
[http://www.havit.hk/downloads/]

Use this utility only if your system is not supported by the manufacturer,
or if you need some extra features that the official software does not provide.

*Absolutely no warranty*. Perhaps (and in some cases definitely),
the device will be broken. To restore the device from the "brick" state
follow the [instruction](doc/unbrick.md).

## Installation from sources

### Requirements
For compiling hv-ms735-config yourself, you will need the QT (>= 5.2).
It is free and available at [http://www.qt.io]. You may also need its
dependency libraries and header files, such as libusb-1.0, hidapi-libusb.

Furthermore you need, of course, a C++ compiler and the Make tool.
The GNU versions of these tools are recommended, but not required.

### Making hv-ms735-config with gcc

    qmake
    make

### Making hv-ms735-config with mingw

    qmake
    mingw32-make

### Making hv-ms735-config with Visual Studio

    qmake
    nmake

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

