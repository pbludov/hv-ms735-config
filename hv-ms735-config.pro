###############################################################################
#
#      Copyright 2017-2018 Pavel Bludov <pbludov@gmail.com>
#
#      This program is free software; you can redistribute it and/or modify
#      it under the terms of the GNU General Public License as published by
#      the Free Software Foundation; either version 2 of the License, or
#      (at your option) any later version.
#
#      This program is distributed in the hope that it will be useful,
#      but WITHOUT ANY WARRANTY; without even the implied warranty of
#      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#      GNU General Public License for more details.
#
#      You should have received a copy of the GNU General Public License along
#      with this program; if not, write to the Free Software Foundation, Inc.,
#      51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
###############################################################################
lessThan(QT_MAJOR_VERSION, 5) \
    | equals(QT_MAJOR_VERSION, 5) : lessThan(QT_MINOR_VERSION, 2) \
        : error (QT 5.2 or newer is required)

isEmpty(PREFIX): PREFIX   = /usr
DEFINES += PREFIX=$$PREFIX
CONFIG  += c++11
QT      += core gui widgets

include (libqxt/libqxt.pri)
include (libqhid/libqhid.pri)

TEMPLATE = app
TARGET   = hv-ms735-config
VERSION  = 1.1.1

DEFINES += PRODUCT_NAME=\\\"$$TARGET\\\" \
    PRODUCT_VERSION=\\\"$$VERSION\\\"

SOURCES += src/buttonedit.cpp \
    src/colorbutton.cpp \
    src/enumedit.cpp \
    src/macroedit.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/mousebuttonbox.cpp \
    src/ms735.cpp \
    src/pagelight.cpp \
    src/pagemacro.cpp \
    src/pagesensitivity.cpp \
    src/profileedit.cpp \
    src/usbcommandedit.cpp \
    src/usbscancodeedit.cpp

HEADERS  += src/buttonedit.h \
    src/colorbutton.h \
    src/enumedit.h \
    src/macroedit.h \
    src/mainwindow.h \
    src/micewidget.h \
    src/mousebuttonbox.h \
    src/ms735.h \
    src/pagelight.h \
    src/pagemacro.h \
    src/pagesensitivity.h \
    src/profileedit.h \
    src/usbcommandedit.h \
    src/usbscancodeedit.h

FORMS    += ui/mainwindow.ui \
    ui/pagelight.ui \
    ui/pagemacro.ui \
    ui/pagesensitivity.ui

RESOURCES += \
    res/hv-ms735-config.qrc

# MacOS specific
ICON = res/hv-ms735-config.icns
QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.9
QMAKE_TARGET_BUNDLE_PREFIX = github.pbludov

# Windows specific
RC_ICONS = res/hv-ms735-config.ico
QMAKE_TARGET_COPYRIGHT = Pavel Bludov <pbludov@gmail.com>
QMAKE_TARGET_DESCRIPTION = HAVIT Magic Eagle mouse configuration utility.

# Linux specific
target.path=$$PREFIX/bin
man.files=doc/hv-ms735-config.1
man.path=$$PREFIX/share/man/man1
shortcut.files = hv-ms735-config.desktop
shortcut.path = $$PREFIX/share/applications
icon.files = res/hv-ms735-config.png
icon.path = $$PREFIX/share/icons/hicolor/48x48/apps
udev.files = 51-hv-ms735-mouse.rules
udev.path = /etc/udev/rules.d

INSTALLS += target man icon shortcut udev
