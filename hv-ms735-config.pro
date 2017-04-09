###############################################################################
#
#      Copyright 2017 Pavel Bludov <pbludov@gmail.com>
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

isEmpty(PREFIX): PREFIX   = /usr
DEFINES += PREFIX=$$PREFIX

QT       += core gui widgets

include (libqxt/libqxt.pri)
include (libqhid/libqhid.pri)

# GCC tuning
*-g++*:QMAKE_CXXFLAGS += -std=c++0x

TARGET = hv-ms735-config
VERSION = 1.0
TEMPLATE = app

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

RC_ICONS = res/hv-ms735-config.ico
QMAKE_TARGET_COPYRIGHT = Pavel Bludov <pbludov@gmail.com>
QMAKE_TARGET_DESCRIPTION = HAVIT Magic Eagle mouse configuration utility.

target.path=$$PREFIX/bin
man.files=doc/hv-ms735-config.1
man.path=$$PREFIX/share/man/man1
shortcut.files = hv-ms735-config.desktop
shortcut.path = $$PREFIX/share/applications
icon.files = res/hv-ms735-config.png
icon.path = $$PREFIX/share/icons/hicolor/48x48/apps

INSTALLS += target man icon shortcut
