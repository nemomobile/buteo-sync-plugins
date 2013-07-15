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
#ifndef USBCONNECTION_H
#define USBCONNECTION_H

#include <QSocketNotifier>

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <buteosyncml5/OBEXConnection.h>
#else
#include <buteosyncml/OBEXConnection.h>
#endif

/*! \brief Class for creating connection to a PC that acts as a USB
 *         host for synchronization of data using buteosyncml
 *
 */
class USBConnection : public DataSync::OBEXConnection, public QObject
{
    Q_OBJECT

public:

    USBConnection ();

    virtual ~USBConnection ();

    /*! \sa DataSync::OBEXConnection::connect ()
     *
     */
    virtual int connect ();

    /*! \sa DataSync::OBEXConnection::isConnected ()
     *
     */
    virtual bool isConnected () const;

    /*! \sa DataSync::OBEXConnection::disconnect ()
     *
     */
    virtual void disconnect ();

signals:

    void usbConnected (int fd);

protected slots:

    void handleUSBActivated (int fd);

private:

    void setupFdListener (const int fd);

    int                      mFd;

    QSocketNotifier*         mReadNotifier;
};

#endif // USBCONNECTION_H
