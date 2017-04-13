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

#include <errno.h>

#ifdef WITH_LIBUSB_1_0
#include <libusb.h>

static bool findUsagePage(int usagePage, const uint8_t *desc, size_t size)
{
    unsigned int i = 0;
    int dataLen, keySize;

    while (i < size)
    {
        int key = desc[i];

        if ((key & 0xf0) == 0xf0)
        {
            /* This is a Long Item. The next byte contains the
               length of the data section (value) for this key.
               See the HID specification, version 1.11, section
               6.2.2.3, titled "Long Items." */
            dataLen = i + 1 < size ? desc[i + 1] : 0;
            keySize = 3;
        }
        else
        {
            /* This is a Short Item. The bottom two bits of the
               key contain the size code for the data section
               (value) for this key.  Refer to the HID
               specification, version 1.11, section 6.2.2.2,
               titled "Short Items." */
            dataLen = key & 0x3;
            if (dataLen == 3)
                ++dataLen; // 0,1,2,4
            keySize = 1;
        }

        if ((key & 0xfc) == 0x04)
        {
            if (i + dataLen >= size)
            {
                // Truncated report?
                return false;
            }

            int page = 0;
            for (int offset = dataLen; offset > 0; --offset)
            {
                page <<= 8;
                page |= desc[i + offset];
            }

            if (page == usagePage)
                return true;
        }

        // Skip over this key and it's associated data.
        i += dataLen + keySize;
    }

    return false;
}

static void hidapiMissingFeatures(
    int vendorId, int deviceId, int usagePage, int *interfaceNumber, int *inBufferLength, int *outBufferLength)
{
    libusb_context *ctx = nullptr;
    int rc = libusb_init(&ctx);
    if (LIBUSB_SUCCESS != rc)
    {
        qWarning() << "libusb_init failed" << rc << libusb_error_name(rc);
        return;
    }

    libusb_device **devs;
    auto count = libusb_get_device_list(ctx, &devs);

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

        libusb_device_handle *handle;
        if (libusb_open(dev, &handle) < 0)
            continue;

        unsigned char buffer[256];
        for (int iface = 0; iface < confDesc->bNumInterfaces; ++iface)
        {
            bool detached = false;
            bool match = false;

            rc = libusb_kernel_driver_active(handle, iface);
            if (rc == 1)
            {
                rc = libusb_detach_kernel_driver(handle, iface);
                if (rc < 0)
                {
                    qWarning() << "Failed to detach kernel driver" << rc << libusb_error_name(rc);
                    continue;
                }
                detached = true;
            }

            if (libusb_claim_interface(handle, iface) >= 0)
            {
                // Get the HID Report Descriptor.
                rc = libusb_control_transfer(handle, LIBUSB_ENDPOINT_IN | LIBUSB_RECIPIENT_INTERFACE,
                    LIBUSB_REQUEST_GET_DESCRIPTOR, LIBUSB_DT_REPORT << 8, iface, buffer, sizeof(buffer), 1000);

                if (rc < 0)
                {
                    qWarning() << "Failed to read HID report descriptor" << rc << libusb_error_name(rc);
                }
                else
                {
                    match = findUsagePage(usagePage, buffer, rc);
                }

                libusb_release_interface(handle, iface);
            }

            if (detached)
            {
                // Re-attach kernel driver if necessary.
                rc = libusb_attach_kernel_driver(handle, iface);
                if (rc < 0)
                {
                    qWarning() << "Failed to re-attach kernel driver" << rc << libusb_error_name(rc);
                }
            }

            if (!match)
                continue;

            *interfaceNumber = iface;
            auto interface = confDesc->interface[iface];

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

            break;
        }
        libusb_close(handle);

        libusb_free_config_descriptor(confDesc);
        break;
    }

    libusb_free_device_list(devs, 1);
    libusb_exit(ctx);
}
#endif

QHIDDevicePrivate::QHIDDevicePrivate(QHIDDevice *q_ptr, int vendorId, int deviceId, int usagePage)
    : device(nullptr)
    , q_ptr(q_ptr)
{
    if (hid_init() != 0)
    {
        qWarning() << "hid_init failed, error" << errno;
        return;
    }

    int interfaceNumber = -1;
#ifdef WITH_LIBUSB_1_0
    hidapiMissingFeatures(
        vendorId, deviceId, usagePage, &interfaceNumber, &q_ptr->inputBufferLength, &q_ptr->outputBufferLength);
#endif
    auto devices = hid_enumerate(vendorId, deviceId);

    for (auto dev = devices; dev != nullptr; dev = dev->next)
    {
        if ((dev->usage_page > 0 && dev->usage_page == usagePage)
            || (dev->interface_number >= 0 && dev->interface_number == interfaceNumber))
        {
            device = hid_open_path(dev->path);

            if (device != nullptr)
            {
                break;
            }

            qWarning() << "Failed to open" << dev->path << "error" << errno;
        }
    }

    hid_free_enumeration(devices);

    if (device == nullptr)
    {
        qWarning() << "No such device" << vendorId << deviceId << usagePage;
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
