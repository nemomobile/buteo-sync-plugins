/*
* This file is part of buteo-sync-plugins package
*
* Copyright (C) 2010 Nokia Corporation. All rights reserved.
*
* Contact: Sateesh Kavuri <sateesh.kavuri@nokia.com>
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
* Neither the name of Nokia Corporation nor the names of its contributors may
* be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
* THE POSSIBILITY OF SUCH DAMAGE.
*
*/
#ifndef BTCONNECTION_H
#define BTCONNECTION_H

#include <QString>

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <buteosyncml5/OBEXConnection.h>
#else
#include <buteosyncml/OBEXConnection.h>
#endif

/*! \brief Class for creating a connection to another device over
 *         Bluetooth for libmeegosyncml
 *
 */
class BTConnection : public DataSync::OBEXConnection
{
public:

    /*! \brief Constructor
     *
     */
    BTConnection();


    /*! \brief Destructor
     *
     */
    virtual ~BTConnection();

    /*! \brief Sets connection info
     *
     * @param aBTAddress Bluetooth addess of remote device
     * @param aServiceUUID Service UUID of remote service
     */
    void setConnectionInfo( const QString& aBTAddress,
                            const QString& aServiceUUID );

    /*! \sa DataSync::OBEXConnection::connect()
     *
     */
    virtual int connect();

    /*! \sa DataSync::OBEXConnection::isConnected()
     *
     */
    virtual bool isConnected() const;

    /*! \sa DataSync::OBEXConnection::disconnect()
     *
     */
    virtual void disconnect();

private:

    QString connectDevice( const QString& aBTAddress, const QString& aServiceUUID );

    void disconnectDevice( const QString& aBTAddress, const QString& aDevice );

    bool fdRawMode( int aFD );

private:
    QString         iBTAddress;
    QString         iServiceUUID;
    int             iFd;
    QString         iDevice;


};

#endif // BTCONNECTION_H
