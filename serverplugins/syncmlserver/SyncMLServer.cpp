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
#include "SyncMLServer.h"
#include <LogMacros.h>

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <buteosyncfw5/SyncProfile.h>
#include <buteosyncml5/OBEXTransport.h>
#else
#include <buteosyncfw/SyncProfile.h>
#include <buteosyncml/OBEXTransport.h>
#endif

#include "SyncMLConfig.h"
#include "DeviceInfo.h"

extern "C" SyncMLServer* createPlugin(const QString& pluginName,
        const Buteo::SyncProfile& profile,
        Buteo::PluginCbInterface *cbInterface) {
    return new SyncMLServer(pluginName, profile, cbInterface);
}

extern "C" void destroyPlugin(SyncMLServer *server) {
    delete server;
}

SyncMLServer::SyncMLServer (const QString& pluginName,
                            const Buteo::Profile profile,
                            Buteo::PluginCbInterface *cbInterface) :
    ServerPlugin (pluginName, profile, cbInterface)
{
    FUNCTION_CALL_TRACE;
}

SyncMLServer::~SyncMLServer ()
{
    FUNCTION_CALL_TRACE;

    closeSyncAgentConfig ();
    closeSyncAgent ();
    closeUSBTransport ();
}

bool
SyncMLServer::init ()
{
    FUNCTION_CALL_TRACE;

    return true;
}

bool
SyncMLServer::uninit ()
{
    FUNCTION_CALL_TRACE;

    return true;
}

void
SyncMLServer::abortSync (Sync::SyncStatus status)
{
    FUNCTION_CALL_TRACE;

    DataSync::SyncState state = DataSync::ABORTED;

    if (status == Sync::SYNC_ERROR)
        state = DataSync::CONNECTION_ERROR;

    if (mAgent && mAgent->abort (state))
    {
        LOG_DEBUG ("Signaling SyncML agent abort");
    } else
    {
        handleSyncFinished (DataSync::ABORTED);
    }
}

bool
SyncMLServer::cleanUp ()
{
    FUNCTION_CALL_TRACE;

    // FIXME: Perform necessary cleanup
    return true;
}

Buteo::SyncResults
SyncMLServer::getSyncResults () const
{
    return mResults;
}

bool
SyncMLServer::startListen ()
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG ("Starting listener");

    if (createUSBTransport ()) {
        LOG_DEBUG ("USB transport created");
        return true;
    }
    else {
        LOG_DEBUG ("USB transport not created");
        return false;
    }
}

void
SyncMLServer::stopListen ()
{
    FUNCTION_CALL_TRACE;

    // Stop all connections
    closeUSBTransport ();
}

void
SyncMLServer::suspend ()
{
    FUNCTION_CALL_TRACE;

    // Not implementing suspend
}

void
SyncMLServer::resume ()
{
    FUNCTION_CALL_TRACE;

    // Not implementing suspend
}

void
SyncMLServer::connectivityStateChanged (Sync::ConnectivityType type, bool state)
{
    FUNCTION_CALL_TRACE;

    // TODO: Handle state changes for the corresponding connection (USB/BT/...)
    LOG_DEBUG ("Connectivity state changed event " << type << ". Connectivity changed to " << state);
}

bool
SyncMLServer::initSyncAgent ()
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG ("Creating SyncML agent...");

    mAgent = new DataSync::SyncAgent ();
    return true;
}

void
SyncMLServer::closeSyncAgent ()
{
    delete mAgent;
    mAgent = 0;
}

DataSync::SyncAgentConfig*
SyncMLServer::initSyncAgentConfig ()
{
    FUNCTION_CALL_TRACE;

    mTransport = new DataSync::OBEXTransport (mUSBConnection,
                                              DataSync::OBEXTransport::MODE_OBEX_SERVER,
                                              DataSync::OBEXTransport::TYPEHINT_USB);

    if (!mTransport || !mStorageProvider.init (&iProfile, this, iCbInterface, true))
        return 0;

    mConfig = new DataSync::SyncAgentConfig ();

    QString defaultSyncMLConfigFile, extSyncMLConfigFile;
    SyncMLConfig::syncmlConfigFilePaths (defaultSyncMLConfigFile, extSyncMLConfigFile);
    if (!mConfig->fromFile (defaultSyncMLConfigFile))
    {
        LOG_CRITICAL ("Unable to read default SyncML config");
        delete mConfig;
        mConfig = 0;
        return mConfig;
    }

    if (!mConfig->fromFile (extSyncMLConfigFile))
    {
        LOG_DEBUG ("Could not find external configuration file");
    }

    mConfig->setStorageProvider (&mStorageProvider);
    mConfig->setTransport (mTransport);

    // Do we need to read the device info from file?
    QString DEV_INFO_FILE = SyncMLConfig::getDevInfoFile ();
    QFile devInfoFile (DEV_INFO_FILE);

    if (!devInfoFile.exists ())
    {
        Buteo::DeviceInfo devInfo;
        QMap<QString,QString> deviceInfoMap = devInfo.getDeviceInformation ();
        devInfo.saveDevInfoToFile (deviceInfoMap, DEV_INFO_FILE);
    }

    DataSync::DeviceInfo syncDeviceInfo;
    syncDeviceInfo.readFromFile (DEV_INFO_FILE);
    mConfig->setDeviceInfo (syncDeviceInfo);

    return mConfig;
}

void
SyncMLServer::closeSyncAgentConfig ()
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG ("Closing config...");

    delete mConfig;
    mConfig = 0;

    if (!mStorageProvider.uninit ())
        LOG_CRITICAL ("Unable to close storage provider");
}

bool
SyncMLServer::createUSBTransport ()
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG ("Opening new USB connection");

    mUSBConnection.connect ();

    QObject::connect (&mUSBConnection, SIGNAL (usbConnected (int)),
                      this, SLOT (handleUSBConnected (int)));

    return mUSBConnection.isConnected ();
}

void
SyncMLServer::closeUSBTransport ()
{
    FUNCTION_CALL_TRACE;

    QObject::disconnect (&mUSBConnection, SIGNAL (usbConnected (int)),
                this, SLOT (handleUSBConnected (int)));
    mUSBConnection.disconnect ();
}

void
SyncMLServer::handleUSBConnected (int fd)
{
    FUNCTION_CALL_TRACE;
    Q_UNUSED (fd);

    LOG_DEBUG ("New incoming data over USB");

    startNewSession ();
}

bool
SyncMLServer::startNewSession ()
{
    FUNCTION_CALL_TRACE;

    if (!initSyncAgent () || !initSyncAgentConfig ())
        return false;

    QObject::connect (mAgent, SIGNAL (stateChanged (DataSync::SyncState)),
             this, SLOT (handleStateChanged (DataSync::SyncState)));
    QObject::connect (mAgent, SIGNAL (syncFinished (DataSync::SyncState)),
             this, SLOT (handleSyncFinished (DataSync::SyncState)));
    QObject::connect (mAgent, SIGNAL (storageAccquired (QString)),
             this, SLOT (handleStorageAccquired (QString)));
    QObject::connect (mAgent, SIGNAL (itemProcessed (DataSync::ModificationType, DataSync::ModifiedDatabase, QString, QString)),
             this, SLOT (handleItemProcessed (DataSync::ModificationType, DataSync::ModifiedDatabase, QString, QString)));

    if (mAgent->listen (*mConfig))
    {
        LOG_DEBUG ("New session started");
        return true;
    } else
    {
        return false;
    }
}

void
SyncMLServer::handleStateChanged (DataSync::SyncState state)
{
    FUNCTION_CALL_TRACE;
}

void
SyncMLServer::handleSyncFinished (DataSync::SyncState state)
{
    FUNCTION_CALL_TRACE;

    // FIXME: Handle the state correctly
    emit syncFinished (Sync::SYNC_DONE);
}

void
SyncMLServer::handleStorageAccquired (QString type)
{
    FUNCTION_CALL_TRACE;
}

void
SyncMLServer::handleItemProcessed (DataSync::ModificationType modificationType,
                                   DataSync::ModifiedDatabase modifiedDb,
                                   QString localDb,
                                   QString dbType)
{
    FUNCTION_CALL_TRACE;
}

void
SyncMLServer::generateResults (bool success)
{
    FUNCTION_CALL_TRACE;
}
