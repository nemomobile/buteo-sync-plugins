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
#include <termios.h>

USBConnection::USBConnection () :
    mFd (-1), mReadNotifier (0)
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

    mFd = openUSBDevice ();

    addFdListener ();

    return mFd;
}

void
USBConnection::disconnect ()
{
    FUNCTION_CALL_TRACE;

    removeFdListener ();

    closeUSBDevice ();
}

bool
USBConnection::isConnected () const
{
    FUNCTION_CALL_TRACE;
    if (mFd == -1)
        return false;
    else
        return true;
}

int
USBConnection::openUSBDevice ()
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

    long flags = fcntl (mFd, F_GETFL);
    fcntl (mFd, F_SETFL, flags & ~O_NONBLOCK);

    struct termios opts;
    tcgetattr (mFd, &opts);
    cfmakeraw (&opts);
    opts.c_oflag &= ~ONLCR;
    tcsetattr (mFd, TCSANOW, &opts);

    int arg = fcntl (mFd, F_GETFL);
    if (arg < 0)
    {
        LOG_WARNING ("Unable to get file attributes");
        close (mFd);
        return -1;
    }

    arg |= O_NONBLOCK;
    if (fcntl (mFd, F_SETFL, arg) < 0)
    {
        LOG_WARNING ("Could not set file attributes");
        close (mFd);
        return -1;
    }

    return mFd;
}

void
USBConnection::closeUSBDevice ()
{
    FUNCTION_CALL_TRACE;

    if (isConnected ())
    {
        LOG_DEBUG ("Closing USB device with fd " << mFd);

        shutdown (mFd, SHUT_RDWR);
        close (mFd);
        mFd = -1;
    }
}

void
USBConnection::addFdListener ()
{
    FUNCTION_CALL_TRACE;
    mReadNotifier = new QSocketNotifier (mFd, QSocketNotifier::Read, this);
    mReadNotifier->setEnabled (true);

    QObject::connect (mReadNotifier, SIGNAL (activated (int)),
                      this, SLOT (handleUSBActivated (int)));
}

void
USBConnection::removeFdListener ()
{
    QObject::disconnect (mReadNotifier, SIGNAL (activated (int)),
                         this, SLOT (handleUSBActivated (int)));
}

void
USBConnection::signalNewSession ()
{
    emit usbConnected (mFd);
}

void
USBConnection::handleUSBActivated (int fd)
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG ("USB is activated. Emitting signal to handle incoming data");

    emit usbConnected (fd);
}
