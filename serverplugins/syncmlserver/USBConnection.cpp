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

#ifdef GLIB_FD_WATCH
#include <glib.h>
#endif

USBConnection::USBConnection () :
#ifdef GLIB_FD_WATCH
    mFd (-1), mIOChannel (0), mIdleEventSource (0), mFdWatchEventSource (0),
    mFdWatching (false), mDisconnected (true)
#else
    mFd (-1), mReadNotifier (0), mWriteNotifier (0), mExceptionNotifier (0)
#endif
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

#ifdef GLIB_FD_WATCH
    removeEventSource ();
#endif
    removeFdListener ();

    closeUSBDevice ();

    mDisconnected = true;
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
USBConnection::handleSyncFinished (bool isSyncInError)
{
    FUNCTION_CALL_TRACE;

    if (isSyncInError == true)
    {
        // What needs to be done here? Should we close the USB?
    } else
    {
        // No errors during sync. Just add channel watcher,
        // which was removed at the start of the sync
        addFdListener ();
    }
}

void
USBConnection::addFdListener ()
{
    FUNCTION_CALL_TRACE;

#ifdef GLIB_FD_WATCH
    if (!mFdWatching && isConnected ())
    {
        mIOChannel = g_io_channel_unix_new (mFd);

        g_io_channel_set_close_on_unref (mIOChannel, FALSE);

        mFdWatchEventSource = g_io_add_watch_full (mIOChannel,
                                                   G_PRIORITY_DEFAULT,
                                                   (GIOCondition) (G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL),
                                                   handleIncomingUSBEvent,
                                                   this,
                                                   NULL);
        g_io_channel_unref (mIOChannel);

        mFdWatching = true;
        mDisconnected = false;
    }
#else
    mReadNotifier = new QSocketNotifier (mFd, QSocketNotifier::Read, this);
    mWriteNotifier = new QSocketNotifier (mFd, QSocketNotifier::Write, this);
    mExceptionNotifier = new QSocketNotifier (mFd, QSocketNotifier::Exception, this);

    mReadNotifier->setEnabled (true);

    QObject::connect (mReadNotifier, SIGNAL (activated (int)),
                      this, SLOT (handleUSBActivated (int)));
    QObject::connect (mExceptionNotifier, SIGNAL (activated (int)),
                      this, SLOT (handleUSBError (int)));
#endif
}

void
USBConnection::removeFdListener ()
{
    FUNCTION_CALL_TRACE;

#ifdef GLIB_FD_WATCH
    if ((mFdWatchEventSource > 0) && g_source_remove (mFdWatchEventSource))
    {
        LOG_DEBUG ("Removed fd wacher with event source " << mFdWatchEventSource);
        mFdWatchEventSource = 0;
    }
#else
    QObject::disconnect (mReadNotifier, SIGNAL (activated (int)),
                         this, SLOT (handleUSBActivated (int)));
#endif
    mFdWatching = false;
}

void
USBConnection::signalNewSession ()
{
    emit usbConnected (mFd);
}

#ifdef GLIB_FD_WATCH
void
USBConnection::setFdWatchEventSource (guint fdEventSource)
{
    mFdWatchEventSource = fdEventSource;
}

void
USBConnection::setIdleEventSource (guint idleEventSource)
{
    mIdleEventSource = idleEventSource;
}

guint
USBConnection::fdWatchEventSource ()
{
    return mFdWatchEventSource;
}

guint
USBConnection::idleEventSource ()
{
    return mIdleEventSource;
}

void
USBConnection::removeEventSource ()
{
    FUNCTION_CALL_TRACE;

    if ((mIdleEventSource > 0) && (g_source_remove (mIdleEventSource)))
    {
        LOG_DEBUG ("Removing idle event source " << mIdleEventSource);
        mIdleEventSource = 0;
    }
}

gboolean
USBConnection::handleIncomingUSBEvent (GIOChannel *ioChannel, GIOCondition condition, gpointer user_data)
{
    FUNCTION_CALL_TRACE;

    USBConnection* connection = (USBConnection*) user_data;

    int fd = g_io_channel_unix_get_fd (ioChannel);

    LOG_DEBUG ("Channel fd: " << fd);

    if (condition & (G_IO_HUP | G_IO_ERR))
    {
        // Receved a hangup or an error. Remove the watchers and
        // also disconnect the listeners

        if (condition & G_IO_HUP)
            LOG_DEBUG ("HUP signal received");
        else
            LOG_DEBUG ("ERR signal received");

        if (connection->isConnected ())
        {
            guint eventSource = connection->idleEventSource ();
            if ((eventSource > 0) && g_source_remove (eventSource))
            {
                LOG_DEBUG ("Removed event source " << eventSource);
                connection->setIdleEventSource (0);
            }

            // Add an idle loop to reopen USB incase of HUP or ERR
            eventSource = g_idle_add (reopenUSB, connection);

            connection->setIdleEventSource (eventSource);
            LOG_DEBUG ("Added watch on the idle event source " << eventSource);
        } else
            LOG_DEBUG ("Unable to remove event source");

        // If in error, remove the fd listner and also close the USB device
        connection->removeFdListener ();

        connection->closeUSBDevice ();
    } else if (condition & G_IO_IN)
    {
        // Some incoming data. Better remove the listener
        connection->removeFdListener ();

        // Signal a new session
        connection->signalNewSession ();
    }

    connection->setFdWatchEventSource (0);
    return false;
}

gboolean
USBConnection::reopenUSB (gpointer data)
{
    FUNCTION_CALL_TRACE;

    USBConnection* connection = (USBConnection*) data;

    if (!connection->mDisconnected && !connection->isConnected ())
    {
        LOG_DEBUG ("USB Not disconnected and not listening");
        connection->openUSBDevice ();
        connection->addFdListener ();
    } else
    {
        LOG_DEBUG ("Already listening. No need to reopen");
    }

    connection->setIdleEventSource (0);
    return false;
}

#else
void
USBConnection::handleUSBActivated (int fd)
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG ("USB is activated. Emitting signal to handle incoming data");

    emit usbConnected (fd);

    // Disable the event notifier
    mReadNotifier->setEnabled (false);
    mWriteNotifier->setEnabled (false);
}

void
USBConnection::handleUSBError (int fd)
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG ("Error in USB connection");

    mReadNotifier->setEnabled (false);
    mWriteNotifier->setEnabled (false);

    QObject::disconnect (mReadNotifier, SIGNAL (activated (int)),
                         this, SLOT (handleUSBActivated (int)));
    QObject::disconnect (mWriteNotifier, SIGNAL (activated (int)),
                         this, SLOT (handleUSBActivated (int)));
}
#endif
