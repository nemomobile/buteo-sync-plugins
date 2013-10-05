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
const int BT_CLIENT_CHANNEL = 25;

typedef struct {
    uint8_t b[6];
} __attribute__((packed)) btbdaddr_t;

struct sockaddr_rc {
    sa_family_t     rc_family;
    btbdaddr_t      rc_bdaddr;
    uint8_t         rc_channel;
};

static QString
btAddrInHex (const btbdaddr_t* ba, char* str)
{
    sprintf(str, "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X",
            ba->b[5], ba->b[4], ba->b[3], ba->b[2], ba->b[1], ba->b[0]);
    QString btAddr (str);
    return btAddr.toUpper ();
}

BTConnection::BTConnection() :
    mServerFd (-1), mClientFd (-1), mPeerSocket (-1), mMutex (QMutex::Recursive),
    mDisconnected (true), mClientServiceRecordId (-1), mServerServiceRecordId (-1),
    mServerReadNotifier (0), mServerWriteNotifier (0), mServerExceptionNotifier (0),
    mClientReadNotifier (0), mClientWriteNotifier (0), mClientExceptionNotifier (0),
    mServerFdWatching (false), mClientFdWatching (false)
{
    FUNCTION_CALL_TRACE;
}

BTConnection::~BTConnection ()
{
    FUNCTION_CALL_TRACE;
    
    if (mServerReadNotifier)
    {
        delete mServerReadNotifier;
        mServerReadNotifier = 0;
    }

    if (mServerWriteNotifier)
    {
        delete mServerWriteNotifier;
        mServerWriteNotifier = 0;
    }
    
    if (mServerExceptionNotifier)
    {
        delete mServerExceptionNotifier;
        mServerExceptionNotifier = 0;
    }

    if (mClientReadNotifier)
    {
        delete mClientReadNotifier;
        mClientReadNotifier = 0;
    }

    if (mClientWriteNotifier)
    {
        delete mClientWriteNotifier;
        mClientWriteNotifier = 0;
    }
    
    if (mClientExceptionNotifier)
    {
        delete mClientExceptionNotifier;
        mClientExceptionNotifier = 0;
    }
}

int
BTConnection::connect ()
{
    FUNCTION_CALL_TRACE;
    
    return mPeerSocket;
}

bool
BTConnection::isConnected () const
{
    FUNCTION_CALL_TRACE;

    if (mPeerSocket == -1)
        return false;
    else
        return true;
}

void
BTConnection::disconnect ()
{
    FUNCTION_CALL_TRACE;

    if (mPeerSocket != -1)
    {
        close (mPeerSocket);
        mPeerSocket = -1;
    }
}

void
BTConnection::handleSyncFinished (bool isSyncInError)
{
    FUNCTION_CALL_TRACE;
    
    if (isSyncInError == true)
    {
        // If sync error, then close the BT connection and reopen it
        removeFdListener (BT_SERVER_CHANNEL);
        removeFdListener (BT_CLIENT_CHANNEL);
        closeBTSocket (mServerFd);
        closeBTSocket (mClientFd);
        openBTSocket (BT_SERVER_CHANNEL);
        openBTSocket (BT_CLIENT_CHANNEL);

        addFdListener (BT_SERVER_CHANNEL, mServerFd);
        addFdListener (BT_CLIENT_CHANNEL, mClientFd);
    } else
    {
        // No errors during sync. Add the fd listener
        LOG_DEBUG ("Sync finished. Adding fd listener");
        addFdListener (BT_SERVER_CHANNEL, mServerFd);
        addFdListener (BT_CLIENT_CHANNEL, mClientFd);
    }
}

int
BTConnection::openBTSocket (const int channelNumber)
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
    localAddr.rc_channel = channelNumber;

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
    
    LOG_DEBUG ("Opened BT socket with fd " << sock << " for channel " << channelNumber);
    return sock;
}

void
BTConnection::closeBTSocket (int& fd)
{
    FUNCTION_CALL_TRACE;

    if (fd != -1)
    {
        close (fd);
        fd = -1;
    }
}

void
BTConnection::addFdListener (const int channelNumber, int fd)
{
    FUNCTION_CALL_TRACE;
    
    if ((channelNumber == BT_SERVER_CHANNEL) && (mServerFdWatching == false) && (fd != -1))
    {
        mServerReadNotifier = new QSocketNotifier (fd, QSocketNotifier::Read);
    	mServerWriteNotifier = new QSocketNotifier (fd, QSocketNotifier::Write);
    	mServerExceptionNotifier = new QSocketNotifier (fd, QSocketNotifier::Exception);
    
    	mServerReadNotifier->setEnabled (true);
    	mServerWriteNotifier->setEnabled (true);
    	mServerExceptionNotifier->setEnabled (true);
    
    	QObject::connect (mServerReadNotifier, SIGNAL (activated (int)),
                      	this, SLOT (handleIncomingBTConnection (int)), Qt::BlockingQueuedConnection);
    	QObject::connect (mServerWriteNotifier, SIGNAL (activated (int)),
                      	this, SLOT (handleIncomingBTConnection (int)), Qt::BlockingQueuedConnection);
    	QObject::connect (mServerExceptionNotifier, SIGNAL (activated (int)),
                      	this, SLOT (handleBTError (int)), Qt::BlockingQueuedConnection);

        LOG_DEBUG ("Added listener for server socket " << fd);
        mServerFdWatching = true;
    }

    if ((channelNumber == BT_CLIENT_CHANNEL) && (mClientFdWatching == false) && (fd != -1))
    {
    	mClientReadNotifier = new QSocketNotifier (fd, QSocketNotifier::Read);
    	mClientWriteNotifier = new QSocketNotifier (fd, QSocketNotifier::Write);
    	mClientExceptionNotifier = new QSocketNotifier (fd, QSocketNotifier::Exception);
    
    	mClientReadNotifier->setEnabled (true);
    	mClientWriteNotifier->setEnabled (true);
    	mClientExceptionNotifier->setEnabled (true);
    
    	QObject::connect (mClientReadNotifier, SIGNAL (activated (int)),
                      	this, SLOT (handleIncomingBTConnection (int)), Qt::BlockingQueuedConnection);
    	QObject::connect (mClientWriteNotifier, SIGNAL (activated (int)),
                      	this, SLOT (handleIncomingBTConnection (int)), Qt::BlockingQueuedConnection);
    	QObject::connect (mClientExceptionNotifier, SIGNAL (activated (int)),
                      	this, SLOT (handleBTError (int)), Qt::BlockingQueuedConnection);

        LOG_DEBUG ("Added listener for client socket " << fd);
        mClientFdWatching = true;
    }
        
    mDisconnected = false;
}

void
BTConnection::removeFdListener (const int channelNumber)
{
    FUNCTION_CALL_TRACE;
    if (channelNumber == BT_SERVER_CHANNEL)
    {
        mServerReadNotifier->setEnabled (false);
    	mServerWriteNotifier->setEnabled (false);
    	mServerExceptionNotifier->setEnabled (false);
    
    	QObject::disconnect (mServerReadNotifier, SIGNAL (activated (int)),
                      	this, SLOT (handleIncomingBTConnection (int)));
    	QObject::disconnect (mServerWriteNotifier, SIGNAL (activated (int)),
                      	this, SLOT (handleIncomingBTConnection (int)));
    	QObject::disconnect (mServerExceptionNotifier, SIGNAL (activated (int)),
                      	this, SLOT (handleBTError (int)));
        
        mServerFdWatching = false;
    } else if (channelNumber == BT_CLIENT_CHANNEL)
    {
        mClientReadNotifier->setEnabled (false);
    	mClientWriteNotifier->setEnabled (false);
    	mClientExceptionNotifier->setEnabled (false);

    	QObject::disconnect (mClientReadNotifier, SIGNAL (activated (int)),
                      	this, SLOT (handleIncomingBTConnection (int)));
    	QObject::disconnect (mClientWriteNotifier, SIGNAL (activated (int)),
                      	this, SLOT (handleIncomingBTConnection (int)));
    	QObject::disconnect (mClientExceptionNotifier, SIGNAL (activated (int)),
                      	this, SLOT (handleBTError (int)));
        
        mClientFdWatching = false;
    }
}

void
BTConnection::handleIncomingBTConnection (int fd)
{
    FUNCTION_CALL_TRACE;
    
    LOG_DEBUG ("Incoming BT connection. Emitting signal to handle the incoming data");

    struct sockaddr_rc remote;
    socklen_t len = sizeof (remote);
    
    mPeerSocket = accept (fd, (struct sockaddr*)&remote, &len);
    if (mPeerSocket < 0)
    {
        LOG_DEBUG ("Error in accept:" << strerror (errno));
    } else
    {
        char buf[128] = { 0 };
        QString btAddr = btAddrInHex (&remote.rc_bdaddr, buf);
        emit btConnected (mPeerSocket, btAddr);
    }
    
    // Disable event notifier
    if (fd == mServerFd)
        removeFdListener (BT_SERVER_CHANNEL);
    else if (fd == mClientFd)
        removeFdListener (BT_CLIENT_CHANNEL);
}

void
BTConnection::handleBTError (int fd)
{
    FUNCTION_CALL_TRACE;
    
    LOG_DEBUG ("Error in BT connection");
    
    // Should this be similar to USB that we close and re-init BT?
    
    // FIXME: Ugly API for fd listeners. Add a more decent way
    if (fd == mServerFd)
        removeFdListener (BT_SERVER_CHANNEL);
    else if (fd == mClientFd)
        removeFdListener (BT_CLIENT_CHANNEL);

    closeBTSocket (fd);

    if (fd == mServerFd)
        openBTSocket (BT_SERVER_CHANNEL);
    else if (fd == mClientFd)
        openBTSocket (BT_CLIENT_CHANNEL);

    if (fd == mServerFd)
        addFdListener (BT_SERVER_CHANNEL, fd);
    else if (fd == mClientFd)
        addFdListener (BT_CLIENT_CHANNEL, fd);
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

    // Open the server and client socket
    mServerFd = openBTSocket (BT_SERVER_CHANNEL);
    mClientFd = openBTSocket (BT_CLIENT_CHANNEL);
    
    if (mServerFd == -1 || mClientFd == -1)
    {
        LOG_WARNING ("Error in opening BT client and server sockets");
        return false;
    }

    addFdListener (BT_SERVER_CHANNEL, mServerFd);
    addFdListener (BT_CLIENT_CHANNEL, mClientFd);
    
    return true;
}

void BTConnection::uninit()
{
    // Remove listeners
    removeFdListener (BT_SERVER_CHANNEL);
    removeFdListener (BT_CLIENT_CHANNEL);

    // Close the fd's
    closeBTSocket (mServerFd);
    closeBTSocket (mClientFd);

    // Close the peer socket
    closeBTSocket (mPeerSocket);

    // Remove service records
    removeServiceRecords ();
}

bool
BTConnection::addServiceRecord (const QByteArray& sdp, quint32& recordId)
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
