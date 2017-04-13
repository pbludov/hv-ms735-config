/*
 *      Copyright 2017 Pavel Bludov <pbludov@gmail.com>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License along
 *      with this program; if not, write to the Free Software Foundation, Inc.,
 *      51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "ms735.h"
#include "qhiddevice.h"
#include "qhidmonitor.h"

#include <QRgb>

#define VENDOR 0x04D9
#define PRODUCT 0xA100
#define USAGE_PAGE 0xFF00

#define PAGE_SIZE 128

Q_LOGGING_CATEGORY(UsbIo, "usb")

#ifndef qCInfo
// QT 5.2 does not have qCInfo
#define qCInfo qCWarning
#endif

static char crc(const QByteArray data)
{
    char sum = -1;

    foreach (auto ch, data)
    {
        sum -= ch;
    }

    return sum;
}

MS735::MS735(QObject *parent)
    : QObject(parent)
    , device(new QHIDDevice(VENDOR, PRODUCT, USAGE_PAGE, this))
    , monitor(new QHIDMonitor(VENDOR, PRODUCT, this))
{
    connect(monitor, SIGNAL(deviceArrival(QString)), this, SLOT(deviceArrival(QString)));
    connect(monitor, SIGNAL(deviceRemove()), this, SLOT(deviceRemove()));
}

MS735::~MS735()
{
    foreach (auto page, cache)
    {
        delete[] page.second;
    }
}

void MS735::deviceArrival(const QString &path)
{
    qCInfo(UsbIo) << "Detected device arrival at" << path;
    connectChanged(device->open(VENDOR, PRODUCT, USAGE_PAGE));
}

void MS735::deviceRemove()
{
    qCInfo(UsbIo) << "Detected device removal";
    connectChanged(false);
}

QByteArray MS735::report(Command b1, char b2, char b3, char b4, char b5, char b6, char b7)
{
    QByteArray data;
    data.reserve(9);
    data.push_back('\x0');
    data.push_back((char)b1);
    data.push_back(b2);
    data.push_back(b3);
    data.push_back(b4);
    data.push_back(b5);
    data.push_back(b6);
    data.push_back(b7);
    data.push_back(crc(data));

    qCDebug(UsbIo) << "send" << data.toHex();
    int sent = device->sendFeatureReport(data.cbegin(), data.length());
    if (sent != data.length())
    {
        qCWarning(UsbIo) << "send failed: got" << sent << "expected" << data.length();
        return nullptr;
    }

    data.fill('\x0');
    int read = device->getFeatureReport(data.begin(), data.length());
    if (read != data.length())
    {
        qCWarning(UsbIo) << "recv failed: got" << read << "expected" << data.length();
        return nullptr;
    }

    qCDebug(UsbIo) << "recv" << data.toHex();
    return data;
}

char *MS735::readPage(Command page, int idx)
{
    auto cacheId = idx << 8 | page;
    auto iter = cache.find(cacheId);

    if (iter != cache.end())
        return iter->second;

    auto cmd = (Command)(CmdFlagGet | page);
    auto resp = report(cmd, idx);

    if (resp == nullptr || resp.length() < 4 || resp.at(1) != (char)cmd || resp.at(2) != idx
        || resp.at(3) != (char)PAGE_SIZE)
    {
        qCWarning(UsbIo) << "readPage: invalid response";
        return nullptr;
    }

    auto value = new char[PAGE_SIZE];
    Q_CHECK_PTR(value);

    auto read = device->read(value, PAGE_SIZE);
    if (read != PAGE_SIZE)
    {
        qCWarning(UsbIo) << "readPage: read failed: got" << read << "expected" << PAGE_SIZE;
        delete value;
        return nullptr;
    }

    qCDebug(UsbIo) << "readPage" << page << idx << QByteArray(value, PAGE_SIZE).toHex();

    dirtyPages[cacheId] = false;
    return cache[cacheId] = value;
}

bool MS735::writePage(const char *data, Command page, int idx)
{
    QByteArray cmd(9, '\x0');
    cmd[1] = page;
    cmd[2] = idx;
    cmd[3] = PAGE_SIZE;
    cmd[8] = crc(cmd);

    qCDebug(UsbIo) << "send" << cmd.toHex();
    int sent = device->sendFeatureReport(cmd.cbegin(), cmd.length());
    if (sent != cmd.length())
    {
        qCWarning(UsbIo) << "writePage: send failed: got" << sent << "expected" << cmd.length();
        return false;
    }

    qCDebug(UsbIo) << "writePage" << page << idx << QByteArray(data, PAGE_SIZE).toHex();
    auto written = device->write(0, data, PAGE_SIZE);
    if (written != PAGE_SIZE)
    {
        qCWarning(UsbIo) << "writePage: write failed: got" << written << "expected" << PAGE_SIZE;
        return false;
    }

    return true;
}

int MS735::button(ButtonIndex btn)
{
    auto bytes = readPage(CmdButtons);

    if (!bytes)
        return -1;

    auto btns = (const int *)bytes;
    return btns[btn];
}

void MS735::setButton(ButtonIndex btn, int value)
{
    auto bytes = readPage(CmdButtons);

    if (bytes)
    {
        auto btns = (int *)bytes;

        if (btns[btn] != value)
        {
            dirtyPages[CmdButtons] = true;
            btns[btn] = value;
        }
    }
}

QByteArray MS735::macro(int index)
{
    auto page = readPage(CmdMacro, index);
    return page ? QByteArray(page, PAGE_SIZE) : nullptr;
}

void MS735::setMacro(int index, const QByteArray &value)
{
    auto page = readPage(CmdMacro, index);
    auto length = qMin(PAGE_SIZE, value.length());

    if (page && memcmp(page, value.cbegin(), length))
    {
        memcpy(page, value.cbegin(), length);
        memset(page + length, 0, PAGE_SIZE - length);
        dirtyPages[index << 8 | CmdMacro] = true;
    }
}

int MS735::readByte(Command page, RegisterOffset offset)
{
    auto bytes = readPage(page);
    return bytes ? 0xFF & bytes[offset] : -1;
}

void MS735::writeByte(Command page, RegisterOffset offset, int value)
{
    auto bytes = readPage(page);
    if (bytes)
    {
        if ((0xFF & bytes[offset]) != (0xFF & value))
        {
            dirtyPages[page] = true;
            bytes[offset] = value;
        }
    }
}

int MS735::readColor(RegisterOffset offset, int index)
{
    auto bytes = readPage(CmdControl);
    if (!bytes)
        return -1;

    bytes += offset + index * 3;

    return qRgba(bytes[0], bytes[1], bytes[2], 0);
}

void MS735::writeColor(RegisterOffset offset, int index, int value)
{
    auto bytes = readPage(CmdControl);
    if (bytes)
    {
        bytes += offset + index * 3;

        if ((0xFF & bytes[0]) != qRed(value) || (0xFF & bytes[1]) != qGreen(value) || (0xFF & bytes[2]) != qBlue(value))
        {
            dirtyPages[CmdControl] = true;
            bytes[0] = qRed(value);
            bytes[1] = qGreen(value);
            bytes[2] = qBlue(value);
        }
    }
}

int MS735::profile()
{
    auto resp = report(CmdGetProfile);
    return resp.isNull() || resp.length() < 4 || resp.at(1) != (char)CmdGetProfile ? -1 : resp.at(3);
}

void MS735::setProfile(int value)
{
    Q_ASSERT(value > 0 && value <= MaxProfiles);

    report(CmdProfile, 0, value);
}

int MS735::numProfiles()
{
    return readByte(CmdControl, NumProfilesOffset);
}

void MS735::setNumProfiles(int value)
{
    Q_ASSERT(value > 0 && value <= MaxProfiles);

    writeByte(CmdControl, NumProfilesOffset, value);
}

int MS735::sensitivityX()
{
    return readByte(CmdControl, SensitivityXOffset);
}

void MS735::setSensitivityX(int value)
{
    Q_ASSERT(value > 0 && value <= 0xFF);

    writeByte(CmdControl, SensitivityXOffset, value);
}

int MS735::sensitivityY()
{
    return readByte(CmdControl, SensitivityYOffset);
}

void MS735::setSensitivityY(int value)
{
    Q_ASSERT(value > 0 && value <= 0xFF);

    writeByte(CmdControl, SensitivityYOffset, value);
}

int MS735::lightType()
{
    return readByte(CmdControl, LightTypeOffset);
}

void MS735::setLightType(int value)
{
    Q_ASSERT(value >= 0 && value <= 4);

    writeByte(CmdControl, LightTypeOffset, value);
}

int MS735::lightValue()
{
    return readByte(CmdControl, LightValueOffset);
}

void MS735::setLightValue(int value)
{
    Q_ASSERT(value >= 0 && value <= 20);

    writeByte(CmdControl, LightValueOffset, value);
}

int MS735::lightSource()
{
    return readByte(CmdControl, LightSourceOffset);
}

void MS735::setLightSource(int value)
{
    Q_ASSERT(value >= 0 && value <= 5);

    writeByte(CmdControl, LightSourceOffset, value);
}

int MS735::mainColor()
{
    return readColor(MainColorOffset, 0);
}

void MS735::setFixedColor(int value)
{
    Q_ASSERT(value >= 0 && value <= 0xFFFFFF);

    writeColor(MainColorOffset, 0, value);
}

int MS735::color(int index)
{
    Q_ASSERT(index >= 0 && index < MaxColors);

    return readColor(ColorsOffset, index);
}

void MS735::setColor(int index, int value)
{
    Q_ASSERT(index >= 0 && index < MaxColors);
    Q_ASSERT(value >= 0 && value <= 0xFFFFFF);

    writeColor(ColorsOffset, index, value);
}

bool MS735::profileEnabled(int profile)
{
    Q_ASSERT(profile >= 0 && profile < MaxProfiles);

    return readByte(CmdControl, ProfilesMaskOffset) & (1 << profile);
}

void MS735::setProfileEnabled(int profile, bool value)
{
    Q_ASSERT(profile >= 0 && profile < MaxProfiles);

    int byte = readByte(CmdControl, ProfilesMaskOffset);
    if (value)
        byte |= (1 << profile);
    else
        byte &= ~(1 << profile);

    writeByte(CmdControl, ProfilesMaskOffset, byte);
}

int MS735::profileDpiX(int profile)
{
    Q_ASSERT(profile >= 0 && profile < MaxProfiles);

    return readByte(CmdControl, (RegisterOffset)(DpiXOffset + profile));
}

void MS735::setProfileDpiX(int profile, int value)
{
    Q_ASSERT(profile >= 0 && profile < MaxProfiles);

    writeByte(CmdControl, (RegisterOffset)(DpiXOffset + profile), value);
}

int MS735::profileDpiY(int profile)
{
    return readByte(CmdControl, (RegisterOffset)(DpiYOffset + profile));
}

void MS735::setProfileDpiY(int profile, int value)
{
    Q_ASSERT(profile >= 0 && profile < MaxProfiles);

    writeByte(CmdControl, (RegisterOffset)(DpiYOffset + profile), value);
}

int MS735::profileColor(int profile)
{
    return readColor(ProfileColorsOffset, profile);
}

void MS735::setProfileColor(int profile, int value)
{
    Q_ASSERT(profile >= 0 && profile < MaxProfiles);
    Q_ASSERT(value >= 0 && value <= 0xFFFFFF);

    writeColor(ProfileColorsOffset, profile, value);
}

int MS735::profileLeds(int profile)
{
    Q_ASSERT(profile >= 0 && profile < MaxProfiles);

    return readByte(CmdControl, (RegisterOffset)(LedsOffset + profile));
}

void MS735::setProfileLeds(int profile, int value)
{
    Q_ASSERT(profile >= 0 && profile < MaxProfiles);
    Q_ASSERT(value > 0 && value <= 0xF);

    writeByte(CmdControl, (RegisterOffset)(LedsOffset + profile), value);
}

bool MS735::blink()
{
    auto resp = report(CmdBlink);
    return !resp.isNull() && resp.length() > 1 && resp.at(1) == (char)CmdBlink;
}

bool MS735::switchToFirmwareUpgradeMode()
{
    // Force reconnect to the device
    device->open(VENDOR, PRODUCT, USAGE_PAGE);

    auto resp = report(CmdFirmwareMode, '\xAA', '\x55', '\xCC', '\x33', '\xBB', '\x99');
    return !resp.isNull() && resp.length() > 1 && resp.at(1) == (char)CmdFirmwareMode;
}

bool MS735::unsavedChanges()
{
    return dirtyPages.cend() != std::find_if(dirtyPages.cbegin(), dirtyPages.cend(),
                                    [](const std::map<int, bool>::value_type &x) { return x.second; });
}

bool MS735::save()
{
    foreach (auto page, cache)
    {
        if (!dirtyPages[page.first])
            continue;

        if (writePage(page.second, (Command)(0xFF & page.first), 0xFF & (page.first >> 8)))
            dirtyPages[page.first] = false;
        else
            return false;
    }

    return true;
}

bool MS735::backupConfig(QIODevice *storage)
{
    int bytes = 0;

    auto page = readPage(CmdControl);
    if (page)
        bytes += storage->write(page, PAGE_SIZE);

    page = readPage(CmdButtons);
    if (page)
        bytes += storage->write(page, PAGE_SIZE);

    for (int i = MinMacroNum; i <= MaxMacroNum; ++i)
    {
        page = readPage(CmdMacro, i);
        if (page)
            bytes += storage->write(page, PAGE_SIZE);
    }

    return bytes == (MaxMacroNum + 2) * PAGE_SIZE;
}

bool MS735::restoreConfig(QIODevice *storage)
{
    if (storage->size() != (MaxMacroNum + 2) * PAGE_SIZE)
    {
        return false;
    }

    auto page = storage->read(PAGE_SIZE);
    if (page.isEmpty() || !writePage(page.cbegin(), CmdControl))
        return false;

    page = storage->read(PAGE_SIZE);
    if (page.isEmpty() || !writePage(page.cbegin(), CmdButtons))
        return false;

    for (int i = MinMacroNum; i <= MaxMacroNum; ++i)
    {
        page = storage->read(PAGE_SIZE);
        if (page.isEmpty() || !writePage(page.cbegin(), CmdMacro, i))
            return false;
    }

    return true;
}
