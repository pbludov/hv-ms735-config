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

#include "qhiddevice.h"
#include "qhiddevice_win32.h"

#include <QDebug>

#include <SetupAPI.h>
extern "C" {
#include <Hidsdi.h>
}

QHIDDevicePrivate::QHIDDevicePrivate(QHIDDevice *q_ptr, int vendorId, int deviceId, int interfaceNumber, int)
    : hDevice(INVALID_HANDLE_VALUE)
    , q_ptr(q_ptr)
{
    const GUID InterfaceClassGuid = {0x4d1e55b2, 0xf16f, 0x11cf, {0x88, 0xcb, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30}};
    auto dis = SetupDiGetClassDevs(&InterfaceClassGuid, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    if (dis == INVALID_HANDLE_VALUE)
    {
        qWarning() << "SetupDiGetClassDevs failed, error" << GetLastError();
        return;
    }

    SP_DEVICE_INTERFACE_DATA did;
    SP_DEVICE_INTERFACE_DETAIL_DATA *pdidd;
    ZeroMemory(&did, sizeof(did));
    did.cbSize = sizeof(did);

    for (int idx = 0; SetupDiEnumDeviceInterfaces(dis, nullptr, &InterfaceClassGuid, idx, &did); ++idx)
    {
        DWORD size = 0;
        SetupDiGetDeviceInterfaceDetail(dis, &did, nullptr, 0, &size, nullptr);
        pdidd = (SP_DEVICE_INTERFACE_DETAIL_DATA *)malloc(size);

        if (!pdidd)
        {
            qWarning() << "failed to allocate(" << size << ") bytes, error" << GetLastError();
            continue;
        }

        pdidd->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

        if (!SetupDiGetDeviceInterfaceDetail(dis, &did, pdidd, size, &size, nullptr))
        {
            qWarning() << "SetupDiGetDeviceInterfaceDetail(data) failed, error" << GetLastError();
        }
        else
        {
            auto name = QString::fromUtf16((const ushort *)&pdidd->DevicePath[0]);
            int vid = -1, pid = -1, iid = -1;

            for (auto ids = name.split(QRegExp("[#&_]")); !ids.isEmpty(); ids.pop_front())
            {
                if (ids.front().compare("vid", Qt::CaseInsensitive) == 0)
                {
                    ids.pop_front();
                    vid = strtol(ids.front().toUtf8(), nullptr, 16);
                    continue;
                }

                if (ids.front().compare("pid", Qt::CaseInsensitive) == 0)
                {
                    ids.pop_front();
                    pid = strtol(ids.front().toUtf8(), nullptr, 16);
                    continue;
                }

                if (ids.front().compare("mi", Qt::CaseInsensitive) == 0)
                {
                    ids.pop_front();
                    iid = strtol(ids.front().toUtf8(), nullptr, 16);
                    continue;
                }
            }

            if (vid == vendorId && pid == deviceId && iid == interfaceNumber)
            {
                hDevice = CreateFile(pdidd->DevicePath, GENERIC_WRITE | GENERIC_READ,
                    FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, 0);

                if (isValid())
                {
                    HIDP_CAPS caps;
                    PHIDP_PREPARSED_DATA ppData = nullptr;

                    if (HidD_GetPreparsedData(hDevice, &ppData))
                    {
                        auto hr = HidP_GetCaps(ppData, &caps);

                        if (hr != HIDP_STATUS_SUCCESS)
                        {
                            qWarning() << "HidP_GetCaps failed, error" << hr;
                        }
                        else
                        {
                            // For Windows it's wMaxPacketSize + 1 (report byte), so we decrement.
                            q_ptr->inputBufferLength = caps.InputReportByteLength - 1;
                            q_ptr->outputBufferLength = caps.OutputReportByteLength - 1;
                        }

                        HidD_FreePreparsedData(ppData);
                    }

                    break;
                }

                qWarning() << "CreateFile(pdidd->DevicePath) failed, error" << GetLastError();
            }
        }

        free(pdidd);
    }

    SetupDiDestroyDeviceInfoList(dis);
}

QHIDDevicePrivate::~QHIDDevicePrivate()
{
    if (isValid())
    {
        CloseHandle(hDevice);
        hDevice = INVALID_HANDLE_VALUE;
    }
}

bool QHIDDevicePrivate::isValid() const
{
    return hDevice != INVALID_HANDLE_VALUE;
}

int QHIDDevicePrivate::sendFeatureReport(const char *buffer, int length)
{
    // Is it safe to cast const void* to void* here?
    return isValid() && HidD_SetFeature(hDevice, (PVOID)buffer, length) ? length : -1;
}

int QHIDDevicePrivate::getFeatureReport(char *buffer, int length)
{
    return isValid() && HidD_GetFeature(hDevice, buffer, length) ? length : -1;
}

int QHIDDevicePrivate::write(const char *buffer, int length)
{
    if (!isValid())
    {
        return -1;
    }

    Q_Q(QHIDDevice);
    DWORD written = 0;

    if (length < q->outputBufferLength + 1)
    {
        QByteArray tmp(buffer, length);
        tmp.resize(q->outputBufferLength + 1);
        return WriteFile(hDevice, tmp.cbegin(), tmp.length(), &written, nullptr) ? written : -1;
    }

    return WriteFile(hDevice, buffer, length, &written, nullptr) ? written : -1;
}

int QHIDDevicePrivate::read(char *buffer, int length)
{
    Q_Q(QHIDDevice);
    DWORD read = 0;
    QByteArray tmp(q->inputBufferLength + 1, Qt::Uninitialized);
    if (ReadFile(hDevice, tmp.begin(), tmp.length(), &read, nullptr) && (int)read == tmp.length())
    {
        // Remove the first byte (report id).
        memcpy(buffer, tmp.cbegin() + 1, length);
        return length;
    }

    return -1;
}
