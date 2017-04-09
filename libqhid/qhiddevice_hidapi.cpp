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
#include "qhiddevice_hidapi.h"

#include <QDebug>

#ifdef WITH_LIBUSB_1_0
#include <libusb.h>

static void getInOutSize(int vendorId, int deviceId, int interfaceNumber, int *inBufferLength, int *outBufferLength)
{
    libusb_device **devs;
    auto count = libusb_get_device_list(nullptr, &devs);

    for (ssize_t i = 0; i < count; ++i)
    {
        auto dev = devs[i];
        libusb_device_descriptor desc;

        if (libusb_get_device_descriptor(dev, &desc) < 0)
            continue;

        if (desc.idVendor != vendorId || desc.idProduct != deviceId)
            continue;

        libusb_config_descriptor *confDesc = nullptr;

        if (libusb_get_active_config_descriptor(dev, &confDesc) < 0)
            continue;

        if (interfaceNumber > confDesc->bNumInterfaces)
        {
            qWarning() << "Need interface" << interfaceNumber << "have only" << confDesc->bNumInterfaces;
        }
        else
        {
            auto interface = confDesc->interface[interfaceNumber];

            for (int ifIdx = 0; ifIdx < interface.num_altsetting; ++ifIdx)
            {
                auto intfDesc = interface.altsetting[ifIdx];

                for (int epIdx = 0; epIdx < intfDesc.bNumEndpoints; ++epIdx)
                {
                    auto ep = intfDesc.endpoint[epIdx];

                    if ((ep.bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_IN)
                        *inBufferLength = ep.wMaxPacketSize;
                    else if ((ep.bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_OUT)
                        *outBufferLength = ep.wMaxPacketSize;
                }
            }
        }

        libusb_free_config_descriptor(confDesc);
        break;
    }

    libusb_free_device_list(devs, 1);
}
#endif

QHIDDevicePrivate::QHIDDevicePrivate(QHIDDevice *q_ptr, int vendorId, int deviceId, int interfaceNumber)
    : device(nullptr)
    , q_ptr(q_ptr)
{
    if (hid_init() != 0)
    {
        qWarning() << "hid_init failed, error" << errno;
        return;
    }

#ifdef WITH_LIBUSB_1_0
    getInOutSize(vendorId, deviceId, interfaceNumber, &q_ptr->inputBufferLength, &q_ptr->outputBufferLength);
#endif
    auto devices = hid_enumerate(vendorId, deviceId);

    for (auto dev = devices; dev != nullptr; dev = dev->next)
    {
        if (dev->interface_number == interfaceNumber)
        {
            device = hid_open_path(dev->path);

            if (device == nullptr)
            {
                qWarning() << "Failed to open" << dev->path << "error" << errno;
            }
        }
    }

    hid_free_enumeration(devices);

    if (device == nullptr)
    {
        qWarning() << "No such device" << vendorId << deviceId << interfaceNumber;
    }
}

QHIDDevicePrivate::~QHIDDevicePrivate()
{
    if (device)
    {
        hid_close(device);
        device = nullptr;
    }

    hid_exit();
}

bool QHIDDevicePrivate::isValid() const
{
    return !!device;
}

int QHIDDevicePrivate::sendFeatureReport(const char *buffer, int length)
{
    return device == nullptr ? -1 : hid_send_feature_report(device, (const unsigned char *)buffer, length);
}

int QHIDDevicePrivate::getFeatureReport(char *buffer, int length)
{
    return device == nullptr ? -1 : hid_get_feature_report(device, (unsigned char *)buffer, length);
}

int QHIDDevicePrivate::write(const char *buffer, int length)
{
    return hid_write(device, (const unsigned char *)buffer, length);
}

int QHIDDevicePrivate::read(char *buffer, int length)
{
    return hid_read_timeout(device, (unsigned char *)buffer, length, 30000);
}
