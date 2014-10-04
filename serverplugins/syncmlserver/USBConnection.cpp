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
#include <QDateTime>

#include <fcntl.h>
#include <sys/socket.h>
#include <termios.h>
#include <unistd.h>

#ifdef GLIB_FD_WATCH
#include <glib.h>
#endif

USBConnection::USBConnection () :
    mFd (-1), mMutex (QMutex::Recursive), mDisconnected (true), mFdWatching (false),
#ifdef GLIB_FD_WATCH
    mIOChannel (0), mIdleEventSource (0), mFdWatchEventSource (0)
#else
    mReadNotifier (0), mWriteNotifier (0), mExceptionNotifier (0)
#endif
{
    FUNCTION_CALL_TRACE;
}

USBConnection::~USBConnection ()
{
    FUNCTION_CALL_TRACE;

#ifdef GLIB_FD_WATCH
    // Make sure glibc does not try to deliver more events to us.
    // This also frees mIOChannel by removing the last ref, if any.
    removeFdListener();
#else
    if (mReadNotifier)
    {
        delete mReadNotifier;
        mReadNotifier = 0;
    }
    if (mWriteNotifier)
    {
        delete mWriteNotifier;
        mWriteNotifier = 0;
    }
    if (mExceptionNotifier)
    {
        delete mExceptionNotifier;
        mExceptionNotifier = 0;
    }
#endif
}

int
USBConnection::connect ()
{
    FUNCTION_CALL_TRACE;

    QMutexLocker lock (&mMutex);

    if (isConnected ())
    {
        LOG_DEBUG ("Already connected. Returning fd");
    } else
    {
        mFd = openUSBDevice ();

        addFdListener ();
    }

    return mFd;
}

void
USBConnection::disconnect ()
{
    FUNCTION_CALL_TRACE;

    QMutexLocker lock (&mMutex);

#ifdef GLIB_FD_WATCH
    removeEventSource ();
#endif
    removeFdListener ();

    // In server mode, we do not disconnect the connection, since
    // the host (PC/device/...) might initiate sync again
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

    QMutexLocker lock (&mMutex);

    if (isConnected ()) {
        LOG_WARNING ("USB connection already open with fd " << mFd);
        return mFd;
    }

    const QString USB_DEVICE ("/dev/ttyGS1");

    mFd = open (USB_DEVICE.toLocal8Bit ().constData (),
                   O_RDWR | O_NOCTTY);

    if (mFd < 0) {
        LOG_WARNING ("Count not open USB device");
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

    LOG_DEBUG ("Opened USB device with fd " << mFd);
    return mFd;
}

void
USBConnection::closeUSBDevice ()
{
    FUNCTION_CALL_TRACE;

    QMutexLocker lock (&mMutex);

    if (isConnected ())
    {
        LOG_DEBUG ("Closing USB device with fd " << mFd);

        shutdown (mFd, SHUT_RDWR);
        close (mFd);
        mFd = -1;

        mDisconnected = true;
    }
}

void
USBConnection::handleSyncFinished (bool isSyncInError)
{
    FUNCTION_CALL_TRACE;

    QMutexLocker lock (&mMutex);

    if (isSyncInError == true)
    {
        // What needs to be done here? Should we close the USB?
        // For now completely closing USB device and reopening again
        // syncml-ds-tool sends LINKERR for closing USB connection. This causes
        // buteosyncml stack to think that a link error has occured and marks the
        // sync session as error
        // The following would ensure that the server plugin would return back
        // to sane state incase of any unexpected sync errors
        removeFdListener ();
        closeUSBDevice ();
        openUSBDevice ();
        addFdListener ();
    } else
    {
        // No errors during sync. Just add channel watcher,
        // which was removed at the start of the sync
        LOG_DEBUG ("Handling sync finished. Adding fd listener");
        addFdListener ();
    }
}

void
USBConnection::addFdListener ()
{
    FUNCTION_CALL_TRACE;

    QMutexLocker lock (&mMutex);

    if ((mFdWatching == false) && isConnected ())
    {
#ifdef GLIB_FD_WATCH
        mIOChannel = g_io_channel_unix_new (mFd);

        g_io_channel_set_close_on_unref (mIOChannel, FALSE);

        mFdWatchEventSource = g_io_add_watch_full (mIOChannel,
                                                   G_PRIORITY_DEFAULT,
                                                   (GIOCondition) (G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL),
                                                   handleIncomingUSBEvent,
                                                   this,
                                                   NULL);
        g_io_channel_unref (mIOChannel);

        LOG_DEBUG ("Added fd listner for fd " << mFd << " with event source " << mFdWatchEventSource);
#else

        mReadNotifier = new QSocketNotifier (mFd, QSocketNotifier::Read);
    	mWriteNotifier = new QSocketNotifier (mFd, QSocketNotifier::Write);
    	mExceptionNotifier = new QSocketNotifier (mFd, QSocketNotifier::Exception);

    	mReadNotifier->setEnabled (true);
    	mWriteNotifier->setEnabled (true);
    	mExceptionNotifier->setEnabled (true);

    	QObject::connect (mReadNotifier, SIGNAL (activated (int)),
                      	this, SLOT (handleUSBActivated (int)), Qt::BlockingQueuedConnection);
    	QObject::connect (mWriteNotifier, SIGNAL (activated (int)),
                      	this, SLOT (handleUSBActivated (int)), Qt::BlockingQueuedConnection);
    	QObject::connect (mExceptionNotifier, SIGNAL (activated (int)),
                      	this, SLOT (handleUSBError (int)), Qt::BlockingQueuedConnection);
#endif
        mFdWatching = true;
        mDisconnected = false;
    }
}

void
USBConnection::removeFdListener ()
{
    FUNCTION_CALL_TRACE;

    QMutexLocker lock (&mMutex);

#ifdef GLIB_FD_WATCH
    if (mFdWatchEventSource > 0)
    {
        gboolean removed = g_source_remove (mFdWatchEventSource);
        if (removed)
        {
            LOG_DEBUG ("Removed fd listener with event source " << mFdWatchEventSource);
            mFdWatchEventSource = 0;
        }
    }
#else
    mWriteNotifier->setEnabled (false);
    mReadNotifier->setEnabled (false);
    mExceptionNotifier->setEnabled (false);

    QObject::disconnect (mReadNotifier, SIGNAL (activated (int)),
                      this, SLOT (handleUSBActivated (int)));
    QObject::disconnect (mWriteNotifier, SIGNAL (activated (int)),
                      this, SLOT (handleUSBActivated (int)));
    QObject::disconnect (mExceptionNotifier, SIGNAL (activated (int)),
                      this, SLOT (handleUSBError (int)));
#endif
    mFdWatching = false;
}

void
USBConnection::signalNewSession ()
{
    FUNCTION_CALL_TRACE;

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
        }

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

    if ((connection->mDisconnected == true) && !connection->isConnected ())
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
    removeFdListener ();
}

void
USBConnection::handleUSBError (int fd)
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG ("Error in USB connection");

    removeFdListener ();
    closeUSBDevice ();
    openUSBDevice ();
    addFdListener ();
}
#endif
