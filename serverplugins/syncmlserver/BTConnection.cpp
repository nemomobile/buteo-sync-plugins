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

#include <LogMacros.h>
#include <stdint.h>
#include <sys/socket.h>
#include <errno.h>
#include <fcntl.h>

#include "BTConnection.h"

const QString CLIENT_BT_SR_FILE ("syncml_client_sdp_record.xml");
const QString SERVER_BT_SR_FILE ("syncml_server_sdp_record.xml");

const int BT_RFCOMM_PROTO = 3;
const int RFCOMM_LM = 0x03;
const int SOL_RFCOMM = 18;
const int RFCOMM_LM_SECURE = 0x0200;
const int BT_SERVER_CHANNEL = 26;

typedef struct {
    uint8_t b[6];
} __attribute__((packed)) btbdaddr_t;

struct sockaddr_rc {
    sa_family_t     rc_family;
    btbdaddr_t      rc_bdaddr;
    uint8_t         rc_channel;
};

BTConnection::BTConnection() :
    mFd (-1), mMutex (QMutex::Recursive), mDisconnected (true),
    mClientServiceRecordId (-1), mServerServiceRecordId (-1)
{
    FUNCTION_CALL_TRACE;
}

int
BTConnection::connect ()
{
    FUNCTION_CALL_TRACE;
    
    return mFd;
}

bool
BTConnection::isConnected () const
{
    FUNCTION_CALL_TRACE;

    if (mFd == -1)
        return false;
    else
        return true;
}

void
BTConnection::disconnect ()
{
    FUNCTION_CALL_TRACE;
    
}

int
BTConnection::openBTSocket ()
{
    FUNCTION_CALL_TRACE;

    int sock = socket (AF_BLUETOOTH, SOCK_STREAM, BT_RFCOMM_PROTO);
    if (sock < 0)
    {
        LOG_WARNING ("Unable to open bluetooth socket");
        return -1;
    }
    
    int lm = RFCOMM_LM_SECURE;
    if (setsockopt (sock, SOL_RFCOMM, RFCOMM_LM, &lm, sizeof (lm)) < 0)
    {
        LOG_WARNING ("Unable to set socket options." << errno);
        return -1;
    }
    
    struct sockaddr_rc localAddr;
    memset (&localAddr, 0, sizeof (localAddr));
    localAddr.rc_family = AF_BLUETOOTH;
    btbdaddr_t anyAddr = {{0, 0, 0, 0, 0, 0}}; // bind to any local bluetooth address
    localAddr.rc_channel = BT_SERVER_CHANNEL; // we open a bt server channel for SyncML

    memcpy (&localAddr.rc_bdaddr, &anyAddr, sizeof (btbdaddr_t));
    
    // Bind the socket
    if (bind (sock, (struct sockaddr*)&localAddr, sizeof (localAddr)) < 0)
    {
        LOG_WARNING ("Unable to bind to local address");
        return -1;
    }
    
    // Listen for incoming connections
    if (listen (sock, 1) < 0) // We allow a max of 1 connection per SyncML session
    {
        LOG_WARNING ("Error while starting listening");
        return -1;
    }
    
    // Set the socket into non-blocking mode
    long flags = fcntl (sock, F_GETFL);
    if (flags < 0)
    {
        LOG_WARNING ("Error while getting flags for socket");
    } else
    {
        flags |= O_NONBLOCK;
        if (fcntl (sock, F_SETFL, flags) < 0)
        {
            LOG_WARNING ("Error while setting socket into non-blocking mode");
        }
    }
    mFd = sock;
    return mFd;
}

void
BTConnection::closeBTSocket ()
{
    FUNCTION_CALL_TRACE;
}

bool
BTConnection::init ()
{
    FUNCTION_CALL_TRACE;
    
    // Add client and server SyncML bluetooth records
    // Read the client record from file and if fails, read from code
    QByteArray clientSDP;
    if (!readSRFromFile (CLIENT_BT_SR_FILE, clientSDP))
    {
        clientSDP = clientServiceRecordDef ().toLatin1 ();
    }

    addServiceRecord (clientSDP, mClientServiceRecordId);

    // Add client and server bluetooth sdp records
    QByteArray serverSDP;
    if (!readSRFromFile (SERVER_BT_SR_FILE, serverSDP))
    {
        serverSDP = serverServiceRecordDef ().toLatin1 ();
    }
    
    addServiceRecord (serverSDP, mServerServiceRecordId);
    
    return true;
}

bool
BTConnection::addServiceRecord (const QByteArray& sdp, quint32& recordId)
{
    // Get the Bluez manager dbus interface
    QDBusInterface mgrIface ("org.bluez", "/", "org.bluez.Manager", QDBusConnection::systemBus ());
    if (!mgrIface.isValid ())
    {
        LOG_WARNING ("Unable to get bluez manager iface");
        return false;
    }

    // Fetch the default bluetooth adapter
    QDBusReply<QDBusObjectPath> reply = mgrIface.call (QLatin1String ("DefaultAdapter"));
    
    QString adapterPath = reply.value ().path ();
    LOG_DEBUG ("Bluetooth adapter path:" << adapterPath);
    
    QDBusInterface serviceIface ("org.bluez", adapterPath, "org.bluez.Service", QDBusConnection::systemBus ());
    if (!serviceIface.isValid ())
    {
        LOG_WARNING ("Unable to get bluez service iface");
        return false;
    }

    QDBusReply<quint32> response = serviceIface.call (QLatin1String ("AddRecord"),
                                                      QLatin1String (sdp));
    
    if (!response.isValid ())
    {
        LOG_WARNING ("Unable to add client bluetooth service record");
        return false;
    }
    
    recordId = response.value ();
    
    return true;
}

bool
BTConnection::removeServiceRecords ()
{
    FUNCTION_CALL_TRACE;
    
    // Get the Bluez manager dbus interface
    QDBusInterface mgrIface ("org.bluez", "/", "org.bluez.Manager", QDBusConnection::systemBus ());
    if (!mgrIface.isValid ())
    {
        LOG_WARNING ("Unable to get bluez manager iface");
        return false;
    }

    // Fetch the default bluetooth adapter
    QDBusReply<QDBusObjectPath> reply = mgrIface.call (QLatin1String ("DefaultAdapter"));
    
    QString adapterPath = reply.value ().path ();
    LOG_DEBUG ("Bluetooth adapter path:" << adapterPath);
    
    QDBusInterface serviceIface ("org.bluez", adapterPath, "org.bluez.Service", QDBusConnection::systemBus ());
    if (!serviceIface.isValid ())
    {
        LOG_WARNING ("Unable to get bluez service iface");
        return false;
    }

    QDBusReply<void> response = serviceIface.call (QLatin1String ("RemoveRecord"),
                                                   mClientServiceRecordId);
    
    if (!response.isValid ())
    {
        LOG_DEBUG ("Unable to remove client bluetooth service record");
        return false;
    }

    response = serviceIface.call (QLatin1String ("RemoveRecord"),
                                  mServerServiceRecordId);
    
    if (!response.isValid ())
    {
        LOG_DEBUG ("Unable to remove server bluetooth service record");
        return false;
    }
    return true;
}

bool
BTConnection::readSRFromFile (const QString filename, QByteArray &record)
{
    FUNCTION_CALL_TRACE;
    
    QFile srFile (filename);
    if (!srFile.open (QIODevice::ReadOnly))
    {
        LOG_WARNING ("Unable to open service record files");
        return false;
    }
 
    record = srFile.readAll ();

    srFile.close ();
    return true;
}

const QString
BTConnection::clientServiceRecordDef () const
{
    return
"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>                        \
<!-- As per the SyncML OBEX Binding for BT specification at         \
     http://technical.openmobilealliance.org/Technical/release_program/docs/Common/V1_2_1-20070813-A/OMA-TS-SyncML_OBEXBinding-V1_2-20070221-A.pdf  \
-->                                                                 \
<record>                                                            \
  <attribute id=\"0x0001\">                                         \
    <sequence>                                                      \
      <uuid value=\"00000002-0000-1000-8000-0002ee000002\" />       \
    </sequence>                                                     \
  </attribute>                                                      \
  <attribute id=\"0x0004\">                                         \
    <sequence>                                                      \
      <sequence>                                                    \
        <uuid value=\"0x0100\" />                                   \
      </sequence>                                                   \
      <sequence>                                                    \
        <uuid value=\"0x0003\" />                                   \
        <uint8 value=\"25\" />                                      \
      </sequence>                                                   \
      <sequence>                                                    \
        <uuid value=\"0x0008\" />                                   \
      </sequence>                                                   \
    </sequence>                                                     \
  </attribute>                                                      \
  <attribute id=\"0x0005\">                                         \
    <sequence>                                                      \
      <uuid value=\"0x1002\" />                                     \
    </sequence>                                                     \
  </attribute>                                                      \
  <attribute id=\"0x0009\">                                         \
    <sequence>                                                      \
      <sequence>                                                    \
        <uuid value=\"00000002-0000-1000-8000-0002ee000002\" />     \
        <uint16 value=\"0x0100\" />                                 \
      </sequence>                                                   \
    </sequence>                                                     \
  </attribute>                                                      \
  <attribute id=\"0x0100\">                                         \
    <text value=\"SyncML Client\" />                                \
  </attribute>                                                      \
</record>";
}

const QString
BTConnection::serverServiceRecordDef () const
{
    return
"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>                        \
<!-- As per the SyncML OBEX Binding for BT specification at         \
     http://technical.openmobilealliance.org/Technical/release_prog	ram/docs/Common/V1_2_1-20070813-A/OMA-TS-SyncML_OBEXBinding-V1_2-20070221-A.pdf	\
-->                                                                 \
<record>                                                            \
  <attribute id=\"0x0001\">                                         \
    <sequence>                                                      \
      <uuid value=\"00000001-0000-1000-8000-0002ee000001\" />       \
    </sequence>                                                     \
  </attribute>                                                      \
  <attribute id=\"0x0004\">                                         \
    <sequence>                                                      \
      <sequence>                                                    \
        <uuid value=\"0x0100\" />                                   \
      </sequence>                                                   \
      <sequence>                                                    \
        <uuid value=\"0x0003\" />                                   \
        <uint8 value=\"26\" /> <!-- A fixed channel number -->      \
      </sequence>                                                   \
      <sequence>                                                    \
        <uuid value=\"0x0008\" />                                   \
      </sequence>                                                   \
    </sequence>                                                     \
  </attribute>                                                      \
  <attribute id=\"0x0005\">                                         \
    <sequence>                                                      \
      <uuid value=\"0x1002\" />                                     \
    </sequence>                                                     \
  </attribute>                                                      \
  <attribute id=\"0x0009\">                                         \
    <sequence>                                                      \
      <sequence>                                                    \
        <uuid value=\"00000001-0000-1000-8000-0002ee000001\" />     \
        <uint16 value=\"0x0100\" />                                 \
      </sequence>                                                   \
    </sequence>                                                     \
  </attribute>                                                      \
  <attribute id=\"0x0100\">                                         \
    <text value=\"SyncML Server\" />                                \
  </attribute>                                                      \
</record>";
}
