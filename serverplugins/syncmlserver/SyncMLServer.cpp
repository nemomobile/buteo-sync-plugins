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
#include <buteosyncml5/SyncAgentConfig.h>
#include <buteosyncfw5/PluginCbInterface.h>
#else
#include <buteosyncfw/SyncProfile.h>
#include <buteosyncfw/PluginCbInterface.h>
#include <buteosyncml/OBEXTransport.h>
#include <buteosyncml/SyncAgentConfig.h>
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
    ServerPlugin (pluginName, profile, cbInterface), mAgent (0), mConfig (0),
    mTransport (0), mCommittedItems (0), mConnectionType (Sync::CONNECTIVITY_USB),
    mIsSessionInProgress (false)
{
    FUNCTION_CALL_TRACE;
}

SyncMLServer::~SyncMLServer ()
{
    FUNCTION_CALL_TRACE;

    closeSyncAgentConfig ();
    closeSyncAgent ();
    closeUSBTransport ();
    closeBTTransport ();
    delete mTransport;
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

    closeSyncAgentConfig ();
    closeSyncAgent ();

    // uninit() is called after completion of every sync session
    // Do not invoke close of transports, since in server mode
    // sync would be initiated from external entities and so
    // transport has to be open

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

    bool listening = false;
    if (iCbInterface->isConnectivityAvailable (Sync::CONNECTIVITY_USB))
    {
        listening = createUSBTransport ();
    } else if (iCbInterface->isConnectivityAvailable (Sync::CONNECTIVITY_BT))
    {
        listening = createBTTransport ();
    } else
    {
        // No sync over IP as of now
    }

    return listening;
}

void
SyncMLServer::stopListen ()
{
    FUNCTION_CALL_TRACE;

    // Stop all connections
    closeUSBTransport ();
    closeBTTransport ();
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

    LOG_DEBUG ("Connectivity state changed event " << type << ". Connectivity changed to " << state);

    if (type == Sync::CONNECTIVITY_USB)
    {
        // Only connectivity changes would be USB enabled/disabled
        if (state)
        {
            LOG_DEBUG ("USB available. Starting sync...");
            createUSBTransport ();
        } else {
            LOG_DEBUG ("USB connection not available. Stopping sync...");
            closeUSBTransport ();

            // FIXME: Should we also abort any ongoing sync session?
        }
    } else if (type == Sync::CONNECTIVITY_BT)
    {
        if (state)
        {
            LOG_DEBUG ("BT connection is available. Creating BT connection...");
            createBTTransport ();
        } else
        {
            LOG_DEBUG ("BT connection unavailable. Closing BT connection...");
            closeBTTransport ();
        }
    }
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

bool
SyncMLServer::createBTTransport ()
{
    FUNCTION_CALL_TRACE;
    
    LOG_DEBUG ("Creating new BT connection");
    bool btInitRes = mBTConnection.init ();
    
    QObject::connect (&mBTConnection, SIGNAL (btConnected (int)),
                      this, SLOT (handleBTConnected (int)));
    
    return btInitRes;
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
SyncMLServer::closeBTTransport ()
{
    FUNCTION_CALL_TRACE;
    
    QObject::disconnect (&mBTConnection, SIGNAL (btConnected (int)),
                         this, SLOT (handleBTConnected (int)));
    mBTConnection.uninit ();
}

void
SyncMLServer::handleUSBConnected (int fd)
{
    FUNCTION_CALL_TRACE;
    Q_UNUSED (fd);

    if (mIsSessionInProgress)
    {
        LOG_DEBUG ("Sync session is in progress over transport " << mConnectionType);
        emit sessionInProgress (mConnectionType);
        return;
    }

    LOG_DEBUG ("New incoming data over USB");

    if (mTransport == NULL)
    {
        mTransport = new DataSync::OBEXTransport (mUSBConnection,
                                              DataSync::OBEXTransport::MODE_OBEX_SERVER,
                                              DataSync::OBEXTransport::TYPEHINT_USB);
    }
    
    if (!mTransport)
    {
        LOG_DEBUG ("Creation of USB transport failed");
        return;
    }

    if (!mAgent)
    {
        mConnectionType = Sync::CONNECTIVITY_USB;
        startNewSession ();
    }
}

void
SyncMLServer::handleBTConnected (int fd)
{
    FUNCTION_CALL_TRACE;
    Q_UNUSED (fd);

    if (mIsSessionInProgress)
    {
        LOG_DEBUG ("Sync session is in progress over transport " << mConnectionType);
        emit sessionInProgress (mConnectionType);
        return;
    }

    LOG_DEBUG ("New incoming connection over BT");
    
    if (mTransport == NULL)
    {
        mTransport = new DataSync::OBEXTransport (mBTConnection,
                                              DataSync::OBEXTransport::MODE_OBEX_SERVER,
                                              DataSync::OBEXTransport::TYPEHINT_BT);
    }
    
    if (!mTransport)
    {
        LOG_DEBUG ("Creation of BT transport failed");
        return;
    }
    
    if (!mAgent)
    {
        mConnectionType = Sync::CONNECTIVITY_BT;
        startNewSession ();
    }
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
    QObject::connect (mAgent, SIGNAL (itemProcessed (DataSync::ModificationType, DataSync::ModifiedDatabase, QString, QString, int)),
             this, SLOT (handleItemProcessed (DataSync::ModificationType, DataSync::ModifiedDatabase, QString, QString, int)));

    mIsSessionInProgress = true;

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

    LOG_DEBUG ("SyncML new state " << state);
}

void
SyncMLServer::handleSyncFinished (DataSync::SyncState state)
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG ("Sync finished with state " << state);
    bool errorStatus = true;

    switch (state)
    {
    case DataSync::SUSPENDED:
    case DataSync::ABORTED:
    case DataSync::SYNC_FINISHED:
    {
        generateResults (true);
        errorStatus = false;
        emit success (getProfileName (), QString::number (state));
        break;
    }

    case DataSync::INTERNAL_ERROR:
    case DataSync::DATABASE_FAILURE:
    case DataSync::CONNECTION_ERROR:
    case DataSync::INVALID_SYNCML_MESSAGE:
    {
        generateResults (false);
        emit error (getProfileName (), QString::number (state), 0);
        break;
    }

    default:
    {
        LOG_CRITICAL ("Unexpected state change");
        generateResults (false);

        emit error (getProfileName (), QString::number (state), 0);
        break;
    }
    }

    uninit ();

    // Signal the USBConnection that sync has finished
    if (mConnectionType == Sync::CONNECTIVITY_USB)
        mUSBConnection.handleSyncFinished (errorStatus);
    else if (mConnectionType == Sync::CONNECTIVITY_BT)
        mBTConnection.handleSyncFinished (errorStatus);

    mIsSessionInProgress = false;
}

void
SyncMLServer::handleStorageAccquired (QString type)
{
    FUNCTION_CALL_TRACE;

    // emit signal that storage has been acquired
    emit accquiredStorage (type);
}

void
SyncMLServer::handleItemProcessed (DataSync::ModificationType modificationType,
                                   DataSync::ModifiedDatabase modifiedDb,
                                   QString localDb,
                                   QString dbType,
                                   int committedItems)
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG ("Modification type:" << modificationType);
    LOG_DEBUG ("ModificationType database:" << modifiedDb);
    LOG_DEBUG ("Local database:" << localDb);
    LOG_DEBUG ("Database type:" << dbType);
    LOG_DEBUG ("Committed items:" << committedItems);

    mCommittedItems++;

    if (!receivedItems.contains (localDb))
    {
        ReceivedItemDetails details;
        details.added = details.modified = details.deleted = details.error = 0;
        details.mime = dbType;
        receivedItems[localDb] = details;
    }

    switch (modificationType)
    {
    case DataSync::MOD_ITEM_ADDED:
    {
        ++receivedItems[localDb].added;
        break;
    }
    case DataSync::MOD_ITEM_MODIFIED:
    {
        ++receivedItems[localDb].modified;
        break;
    }
    case DataSync::MOD_ITEM_DELETED:
    {
        ++receivedItems[localDb].deleted;
        break;
    }
    case DataSync::MOD_ITEM_ERROR:
    {
        ++receivedItems[localDb].error;
        break;
    }
    default:
    {
        Q_ASSERT (0);
        break;
    }
    }

    Sync::TransferDatabase db = Sync::LOCAL_DATABASE;
    if (modifiedDb == DataSync::MOD_LOCAL_DATABASE)
        db = Sync::LOCAL_DATABASE;
    else
        db = Sync::REMOTE_DATABASE;

    if (mCommittedItems == committedItems)
    {
        QMapIterator<QString, ReceivedItemDetails> itr (receivedItems);
        while (itr.hasNext ())
        {
            itr.next ();
            if (itr.value ().added)
                emit transferProgress (getProfileName (), db, Sync::ITEM_ADDED, itr.value ().mime, itr.value ().added);
            if (itr.value ().modified)
                emit transferProgress (getProfileName (), db, Sync::ITEM_MODIFIED, itr.value ().mime, itr.value ().modified);
            if (itr.value ().deleted)
                emit transferProgress (getProfileName (), db, Sync::ITEM_DELETED, itr.value ().mime, itr.value ().deleted);
            if (itr.value ().error)
                emit transferProgress (getProfileName (), db, Sync::ITEM_ERROR, itr.value ().mime, itr.value ().error);
        }

        mCommittedItems = 0;
        receivedItems.clear ();
    }
}

void
SyncMLServer::generateResults (bool success)
{
    FUNCTION_CALL_TRACE;

    mResults.setMajorCode (success ? Buteo::SyncResults::SYNC_RESULT_SUCCESS : Buteo::SyncResults::SYNC_RESULT_FAILED);

    mResults.setTargetId (mAgent->getResults().getRemoteDeviceId ());
    const QMap<QString, DataSync::DatabaseResults>* dbResults = mAgent->getResults ().getDatabaseResults ();

    if (dbResults->isEmpty ())
    {
        LOG_DEBUG("No items transferred");
    }
    else
    {
        QMapIterator<QString, DataSync::DatabaseResults> itr (*dbResults);
        while (itr.hasNext ())
        {
            itr.next ();
            const DataSync::DatabaseResults& r = itr.value ();
            Buteo::TargetResults targetResults(
                    itr.key(), // Target name
                    Buteo::ItemCounts (r.iLocalItemsAdded,
                                       r.iLocalItemsDeleted,
                                       r.iLocalItemsModified),
                    Buteo::ItemCounts (r.iRemoteItemsAdded,
                                       r.iRemoteItemsDeleted,
                                       r.iRemoteItemsModified));
            mResults.addTargetResults (targetResults);

            LOG_DEBUG("Items for" << targetResults.targetName () << ":");
            LOG_DEBUG("LA:" << targetResults.localItems ().added <<
                      "LD:" << targetResults.localItems ().deleted <<
                      "LM:" << targetResults.localItems ().modified <<
                      "RA:" << targetResults.remoteItems ().added <<
                      "RD:" << targetResults.remoteItems ().deleted <<
                      "RM:" << targetResults.remoteItems ().modified);
        }
    }
}
