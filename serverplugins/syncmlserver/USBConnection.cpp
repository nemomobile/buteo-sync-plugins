/*
* This file is part of buteo-sync-plugins package
*
* Copyright (C) 2013 Jolla Ltd. and/or its subsidiary(-ies).
*
* Author: Sateesh Kavuri <sateesh.kavuri@gmail.com>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* This library is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
* 02110-1301 USA
*/
#include "USBConnection.h"

#include <LogMacros.h>

#include <fcntl.h>
#include <sys/socket.h>

USBConnection::USBConnection () :
    mFd (-1)
{
    FUNCTION_CALL_TRACE;
}

USBConnection::~USBConnection ()
{
    FUNCTION_CALL_TRACE;
}

int
USBConnection::connect ()
{
    FUNCTION_CALL_TRACE;

    if (isConnected ()) {
        LOG_WARNING ("USB connection already open with fd " << mFd);
        return mFd;
    }

    const QString USB_DEVICE ("/dev/ttyGS1");

    mFd = open (USB_DEVICE.toLocal8Bit ().constData (),
                   O_RDWR | O_NOCTTY);

    if (mFd < 0) {
        LOG_CRITICAL ("Count not open USB device");
        return -1;
    }

    LOG_DEBUG ("Opened USB device " << USB_DEVICE);

    return mFd;
}

void
USBConnection::disconnect ()
{
    FUNCTION_CALL_TRACE;

    if (isConnected ()) {
        shutdown (mFd, SHUT_RDWR);
        close (mFd);
        mFd = -1;

        LOG_DEBUG ("Closed USB connection with fd " << mFd);
    }
}

bool
USBConnection::isConnected () const
{
    if (mFd == -1)
        return false;
    else
        return true;
}
