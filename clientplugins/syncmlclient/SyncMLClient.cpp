/*
 * This file is part of buteo-sync-plugins package
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Sateesh Kavuri <sateesh.kavuri@nokia.com>
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
 *
 */

#include "SyncMLClient.h"

#include <QLibrary>
#include <QtNetwork>

#include <libsyncpluginmgr/PluginCbInterface.h>
#include <libmeegosyncml/SyncAgentConfig.h>
#include <libmeegosyncml/HTTPTransport.h>
#include <libmeegosyncml/OBEXTransport.h>
#include <libmeegosyncml/DeviceInfo.h>


#include "SyncMLConfig.h"
#include "SyncMLCommon.h"
#include "DeviceInfo.h"

#include <LogMacros.h>

// Number of times that the re-sending of initialization package over HTTP is
// attempted
const int MAXHTTPRETRIES = 3;

// Timeout in seconds for OBEX transport
const int OBEXTIMEOUT = 120;

extern "C" SyncMLClient* createPlugin( const QString& aPluginName,
                                       const Buteo::SyncProfile& aProfile,
                                       Buteo::PluginCbInterface *aCbInterface )
{
    return new SyncMLClient( aPluginName, aProfile, aCbInterface );
}

extern "C" void destroyPlugin( SyncMLClient *aClient )
{
    delete aClient;
}

SyncMLClient::SyncMLClient( const QString& aPluginName,
                            const Buteo::SyncProfile& aProfile,
                            Buteo::PluginCbInterface *aCbInterface )
 :  ClientPlugin( aPluginName, aProfile, aCbInterface ),
    iAgent( 0 ), iTransport( 0 ), iConfig( 0 )
{
    FUNCTION_CALL_TRACE;
}

SyncMLClient::~SyncMLClient()
{
    FUNCTION_CALL_TRACE;
}

bool SyncMLClient::init()
{
    FUNCTION_CALL_TRACE;

    iProperties = iProfile.allNonStorageKeys();

    if( initAgent() && initTransport() && initConfig() ) {
        return true;
    }
    else {
        // Uninitialize everything that was initialized before failure.
        uninit();

        return false;
    }

}

bool SyncMLClient::uninit()
{
    FUNCTION_CALL_TRACE;

    closeAgent();

    closeConfig();

    closeTransport();

    return true;
}

bool SyncMLClient::startSync()
{
    if ( iAgent == 0 || iConfig == 0 || iTransport == 0)
        return false;

    FUNCTION_CALL_TRACE;

    connect( iAgent, SIGNAL( stateChanged( DataSync::SyncState ) ),
             this, SLOT( syncStateChanged( DataSync::SyncState ) ) );

    connect( iAgent, SIGNAL( syncFinished( DataSync::SyncState ) ),
             this, SLOT( syncFinished( DataSync::SyncState ) ) );

    connect( iAgent, SIGNAL( itemProcessed( DataSync::ModificationType, DataSync::ModifiedDatabase,QString,QString) ),
             this, SLOT( receiveItemProcessed( DataSync::ModificationType, DataSync::ModifiedDatabase,QString,QString) ) );

    connect( iAgent, SIGNAL(storageAccquired(QString)),
		    this, SLOT( storageAccquired(QString)));

    iConfig->setTransport( iTransport );

    return iAgent->startSync( *iConfig );

}

void SyncMLClient::abortSync()
{
    FUNCTION_CALL_TRACE;

    if( iAgent && iAgent->abort() )
    {
        LOG_DEBUG( "SyncMLClient: Agent active, abort event posted" );
    }
    else
    {
        syncFinished( DataSync::ABORTED );
    }
}

bool SyncMLClient::cleanUp()
{
    FUNCTION_CALL_TRACE;

    iProperties = iProfile.allNonStorageKeys();
    initAgent();
    initConfig();

    bool retVal = iAgent->cleanUp(iConfig);

    closeAgent();
    closeConfig();
    return retVal;
}

void SyncMLClient::syncStateChanged( DataSync::SyncState aState )
{

    FUNCTION_CALL_TRACE;

#ifndef QT_NO_DEBUG
    LOG_DEBUG("***********  Sync Status has Changed to:" << toText( aState ) <<"****************");
#endif  //  QT_NO_DEBUG

}


void SyncMLClient::syncFinished( DataSync::SyncState aState )
{

    FUNCTION_CALL_TRACE;

#ifndef QT_NO_DEBUG
    LOG_DEBUG("***********  Sync has finished with:" << toText( aState ) <<"****************");
#endif  //  QT_NO_DEBUG

    switch(aState)
    {
        case DataSync::INTERNAL_ERROR:
        case DataSync::AUTHENTICATION_FAILURE:
        case DataSync::DATABASE_FAILURE:
        //FIXME! Add extra headers here
        case DataSync::CONNECTION_ERROR:
        case DataSync::INVALID_SYNCML_MESSAGE:
        {
            generateResults( false );
            emit error( getProfileName(), QString::number(aState), 0 );
            break;
        }
        case DataSync::SUSPENDED:
        case DataSync::ABORTED:
        case DataSync::SYNC_FINISHED:
        {
            generateResults( true );
            emit success( getProfileName(), QString::number(aState) );
            break;
        }
        case DataSync::NOT_PREPARED:
        case DataSync::PREPARED:
        case DataSync::LOCAL_INIT:
        case DataSync::REMOTE_INIT:
        case DataSync::SENDING_ITEMS:
        case DataSync::RECEIVING_ITEMS:
        case DataSync::FINALIZING:
        case DataSync::SUSPENDING:
        default:
        {
            // do nothing
            // @todo: do nothing??? We'll deadlock then! Fix this at some point!
            break;
        }
    }

}

void SyncMLClient::storageAccquired( QString aMimeType )
{
	FUNCTION_CALL_TRACE;
	LOG_DEBUG(" MimeType " << aMimeType   );
	emit accquiredStorage(aMimeType);
}


void SyncMLClient::receiveItemProcessed( DataSync::ModificationType aModificationType,
                                  DataSync::ModifiedDatabase aModifiedDatabase,
                                  QString aLocalDatabase,
                                  QString aMimeType)
{

    FUNCTION_CALL_TRACE;


    LOG_DEBUG("Modification Type " << aModificationType   );
    LOG_DEBUG("Modification Database " << aModifiedDatabase   );
    LOG_DEBUG(" Database " << aLocalDatabase   );
    LOG_DEBUG(" MimeType " << aMimeType   );

    Sync::TransferType type = Sync::ITEM_ADDED;
    Sync::TransferDatabase db = Sync::LOCAL_DATABASE;

    switch( aModificationType )
    {
        case DataSync::MOD_ITEM_ADDED:
        {
            type = Sync::ITEM_ADDED;
            break;
        }
        case DataSync::MOD_ITEM_MODIFIED:
        {
            type = Sync::ITEM_MODIFIED;
            break;
        }
        case DataSync::MOD_ITEM_DELETED:
        {
            type = Sync::ITEM_DELETED;
            break;
        }
        case DataSync::MOD_ITEM_ERROR:
        {
            type = Sync::ITEM_ERROR;
            break;
        }
        default:
        {
            Q_ASSERT(0);
            break;

        }

    }

    if( aModifiedDatabase == DataSync::MOD_LOCAL_DATABASE ) {
        db = Sync::LOCAL_DATABASE;
    }
    else {
        db = Sync::REMOTE_DATABASE;
    }

    emit transferProgress( getProfileName(), db, type ,aMimeType );

}

bool SyncMLClient::initAgent()
{

    FUNCTION_CALL_TRACE;

    LOG_DEBUG("Creating agent...");

    bool success = false;

    createSyncAgent_t createSyncAgent = (createSyncAgent_t) QLibrary::resolve( "libmeegosyncml.so", "createSyncAgent");
    if(createSyncAgent) {

        iAgent = createSyncAgent(0);
        if (!iAgent) {
            LOG_DEBUG("Agent creation failed");
        }
        else {
            success = true;
            LOG_DEBUG("Agent created");
        }
    } else {
        LOG_DEBUG("Could not find the library libmeegosyncml.so");
    }

    return success;

}

void SyncMLClient::closeAgent()
{

    FUNCTION_CALL_TRACE;

    LOG_DEBUG("Destroying agent...");

    if( iAgent ) {

        destroySyncAgent_t* destroySyncAgent = (destroySyncAgent_t*) QLibrary::resolve("libmeegosyncml.so", "destroySyncAgent");

        if( destroySyncAgent ) {
            destroySyncAgent( iAgent );
            iAgent = NULL;
            LOG_DEBUG("Agent destroyed");
        }
        else {
            LOG_DEBUG("Could not find the library libmeegosyncml.so");
        }

    }

}

bool SyncMLClient::initTransport()
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG("Initiating transport..." );

    bool success = false;
    QString transportType = iProperties[PROF_SYNC_TRANSPORT];

    if( transportType == HTTP_TRANSPORT ) {
        success = initHttpTransport();
    }
    else if( transportType == OBEX_TRANSPORT ) {
        success = initObexTransport();
    }
    else {
        LOG_DEBUG("Unknown transport type:" << transportType);
    }

    return success;
}

void SyncMLClient::closeTransport()
{

    FUNCTION_CALL_TRACE;

    LOG_DEBUG("Closing transport...");

    delete iTransport;
    iTransport = NULL;

    LOG_DEBUG("Transport closed");

}

bool SyncMLClient::initConfig()
{

    FUNCTION_CALL_TRACE;

    LOG_DEBUG("Initiating config...");

    if( !iStorageProvider.init( &iProfile, this, iCbInterface, false ) ) {
        LOG_CRITICAL( "Could not initialize storage provider" );
        return false;
    }

    QStringList storageNames = iProfile.subProfileNames( Buteo::Profile::TYPE_STORAGE );

    if( storageNames.isEmpty() ) {
        LOG_CRITICAL("No storages defined for profile, nothing to sync");
        return false;
    }

    iConfig = new DataSync::SyncAgentConfig;

    for( int i = 0; i < storageNames.count(); ++i ) {
        const Buteo::Profile *storageProfile =
                iProfile.subProfile( storageNames[i], Buteo::Profile::TYPE_STORAGE );

        QString sourceDb = storageProfile->key( STORAGE_SOURCE_URI );

	if( !storageProfile->isEnabled() ) {
            LOG_DEBUG( "Adding disabled sync target:" << sourceDb);
	    iConfig->addDisabledSyncTarget( sourceDb );
            continue;
        }


        QString targetDb = storageProfile->key( STORAGE_REMOTE_URI );
        LOG_DEBUG( "Adding sync target:" << sourceDb << "->" << targetDb );
        iConfig->addSyncTarget( sourceDb, targetDb );
    }

    iConfig->setStorageProvider( &iStorageProvider );
    iConfig->setDatabaseFilePath( QString( SyncMLConfig::getDatabasePath() + LIBMEEGOSYNCMLDB ) );

    QString remoteImei = iProperties[PROF_REMOTE_ADDRESS];
    LOG_DEBUG("RemoteDevice: " << remoteImei);

    if (!remoteImei.isEmpty())
	    iConfig->setRemoteDevice( iProperties[PROF_REMOTE_ADDRESS] ) ;


    QString DEV_INFO_FILE_PATH = SyncMLConfig::getDevInfoFile();
    QFile devInfoFile(DEV_INFO_FILE_PATH);

    if(!devInfoFile.exists()) {
        Buteo::DeviceInfo appDevInfo;
        QMap<QString , QString> deviceInfoMap = appDevInfo.getDeviceInformation();
        appDevInfo.saveDevInfoToFile(deviceInfoMap,DEV_INFO_FILE_PATH);
    }

    DataSync::DeviceInfo syncDeviceInfo;
    syncDeviceInfo.readFromFile(DEV_INFO_FILE_PATH);
    iConfig->setDeviceInfo(syncDeviceInfo);

    QString username = iProperties[PROF_USERID];
    QString password = iProperties[PROF_PASSWD];

    if( !username.isEmpty() ) {
        LOG_DEBUG("Setting username: ********");
        iConfig->setUsername( username );
    }

    if( !password.isEmpty() ) {
        LOG_DEBUG("Setting password: ********");
        iConfig->setPassword( password );
    }

    QString protocol = iProperties[PROF_SYNC_PROTOCOL];

    if( protocol == SYNCML11 ) {
        LOG_DEBUG( "Using SyncML DS 1.1 protocol" );
        iConfig->setProtocolVersion( DataSync::DS_1_1 );
    }
    else if( protocol == SYNCML12 ) {
        LOG_DEBUG( "Using SyncML DS 1.2 protocol" );
        iConfig->setProtocolVersion( DataSync::DS_1_2 );
    }
    else {
        LOG_WARNING( "Could not find, or unknown value, for property" << PROF_SYNC_PROTOCOL
                     << ", using default of" << SYNCML12 );
        iConfig->setProtocolVersion( DataSync::DS_1_2 );
    }

    QString transportType = iProperties[PROF_SYNC_TRANSPORT];

    if( transportType == HTTP_TRANSPORT ) {
        initHttpTransportConfig();

    }
    else if( transportType == OBEX_TRANSPORT ) {

        // @todo: This is disabled, we cannot just prefer usb over bt.
        //        Find a better way to do this
        /*
        if (isUsbCableConnected()) {
            initUsbTransportConfig();
        }
        else {
            initBtTransportConfig();
        }
        */
        initBtTransportConfig();

    }

    return true;

}

void SyncMLClient::closeConfig()
{

    FUNCTION_CALL_TRACE;

    LOG_DEBUG("Closing config...");

    delete iConfig;
    iConfig = NULL;

    if( !iStorageProvider.uninit() ) {
        LOG_CRITICAL( "Could not uninitialize storage provider" );
    }

    LOG_DEBUG("Config closed");

}

Buteo::SyncResults SyncMLClient::getSyncResults() const
{
    FUNCTION_CALL_TRACE;

    return iResults;
}

void SyncMLClient::connectivityStateChanged( Sync::ConnectivityType aType,
                                             bool aState )
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG( "Received connectivity change event:" << aType <<" changed to " << aState );
}

#ifndef QT_NO_DEBUG

// this function exists only debugging purposes.
// should not be used to send messages as feedback
// only the state of the app will be sent now
// and UI has to map to a localisation string based on
// the state of the stack
QString SyncMLClient::toText( const DataSync::SyncState& aState )
{

    {
        switch( aState )
        {
        case DataSync::NOT_PREPARED:
            return "NOT PREPARED";

        case DataSync::LOCAL_INIT:
        case DataSync::REMOTE_INIT:
            return "INITIALIZING";

        case DataSync::SENDING_ITEMS:
            return "SENDING ITEMS";

        case DataSync::RECEIVING_ITEMS:
            return "RECEIVING_ITEMS";

        case DataSync::SENDING_MAPPINGS:
            return "SENDING MAPPINGS";

        case DataSync::RECEIVING_MAPPINGS:
            return "RECEIVING MAPPINGS";

        case DataSync::FINALIZING:
            return "FINALIZING";

        case DataSync::SUSPENDING:
            return "SUSPENDING";

        case DataSync::PREPARED:
            return "PREPARED";

        case DataSync::SYNC_FINISHED:
            return "SYNC FINISHED";

        case DataSync::INTERNAL_ERROR:
            return "INTERNAL_ERROR";

        case DataSync::AUTHENTICATION_FAILURE:
            return "AUTHENTICATION FAILURE";

        case DataSync::DATABASE_FAILURE:
            return "DATABASE_FAILURE";

        //FIXME! Add extra headers here

        case DataSync::SUSPENDED:
            return "SUSPENDED";

        case DataSync::ABORTED:
            return "ABORTED";

        case DataSync::CONNECTION_ERROR:
            return "CONNECTION ERROR";

        case DataSync::INVALID_SYNCML_MESSAGE:
            return "INVALID SYNCML MESSAGE";

        default:
            return "UNKNOWN";
            break;
        }

    }

}
#endif //#ifndef QT_NO_DEBUG

/*!
    \fn SyncMLClient::initObexTransport()
 */
bool SyncMLClient::initObexTransport()
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG("Creating OBEX transport" );

    bool success = false;

    // @todo: This is disabled, we cannot just prefer usb over bt.
    //        Find a better way to do this
    /*
    if ( isUsbCableConnected()) {
        success = initUsbTransport();
    }
    else {
        success = initBtTransport();
    }
    */

    success = initBtTransport();

    return success;
}


/*!
    \fn SyncMLClient::initBtTransport()
 */
bool SyncMLClient::initBtTransport()
{
    FUNCTION_CALL_TRACE;

    QString btAddress = iProperties[PROF_BT_ADDRESS];
    bool success = false;

    if( !btAddress.isEmpty() ) {

        QString btService = iProperties[PROF_BT_UUID];

        LOG_DEBUG("Setting BT address:" <<btAddress );
        LOG_DEBUG("Setting BT service UUID:" <<btService );

        DataSync::OBEXTransport* transport = new DataSync::OBEXTransport( btAddress, btService, OBEXTIMEOUT );

        if( iProperties[PROF_USE_WBXML] == PROPS_TRUE ) {
            LOG_DEBUG("Using wbXML");
            transport->setWbXml( true );
        }
        else {
            LOG_DEBUG("Not using wbXML");
            transport->setWbXml( false );
        }

        iTransport = transport;
        success = true;
    }
    else {
        LOG_DEBUG("Could not find 'bt_address' property");
    }

    return success;
}

/*!
    \fn SyncMLClient::initUsbTransport()
 */
bool SyncMLClient::initUsbTransport()
{
    FUNCTION_CALL_TRACE;

    // @todo: this needs to be fixed to use file descriptor

    Q_ASSERT(0);
    /*
    bool success = false;
    QString usbPort = "/dev/ttyGS1";

    if (!usbPort.isEmpty()) {
        DataSync::OBEXTransport* transport = new DataSync::OBEXTransport( usbPort );

        if( iProperties[PROF_USE_WBXML] == PROPS_TRUE ) {
            LOG_DEBUG("Using wbXML");
            transport->setWbXml( true );
        }
        else {
            LOG_DEBUG("Not using wbXML");
            transport->setWbXml( false );
        }

        iTransport = transport;
        success = true;
    }
    else {
        LOG_DEBUG("Could not find usb address property");
    }

    return success;
    */

    return false;

}


/*!
    \fn SyncMLClient::initHttpTransport()
 */
bool SyncMLClient::initHttpTransport()
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG("Creating HTTP transport" );

    QString remoteURI = iProperties[PROF_REMOTE_URI];
    bool success = false;

    if( !remoteURI.isEmpty() ) {

        DataSync::HTTPTransport* transport = new DataSync::HTTPTransport();

        LOG_DEBUG("Setting remote URI to" <<remoteURI );
        transport->setRemoteLocURI( remoteURI );

        QString proxyHost = iProperties[PROF_HTTP_PROXY_HOST];
        if( !proxyHost.isEmpty() ) {

            QString proxyPort = iProperties[PROF_HTTP_PROXY_PORT];

            QNetworkProxy& proxy = transport->getProxyConfig();
            proxy.setType( QNetworkProxy::HttpProxy );
            proxy.setHostName( proxyHost );
            proxy.setPort( proxyPort.toInt() );

            LOG_DEBUG("Using proxy");
            LOG_DEBUG("Proxy host:" << proxyHost );
            LOG_DEBUG("Proxy port:" << proxyPort );
        }
        else {
            LOG_DEBUG("Not using proxy");
        }

        if( iProperties[PROF_USE_WBXML] == PROPS_TRUE ) {
            LOG_DEBUG("Using wbXML");
            transport->setWbXml( true );
        }
        else {
            LOG_DEBUG("Not using wbXML");
            transport->setWbXml( false );
        }


        transport->init();
        transport->setResendAttempts( MAXHTTPRETRIES );
        iTransport = transport;
        success = true;
    }
    else {
        LOG_DEBUG("Could not find 'Remote database' property" );
    }

    return success;
}



/*!
    \fn SyncMLClient::initHttpTransportConfig()
 */
void SyncMLClient::initHttpTransportConfig()
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG("Setting HTTP profile hardcoded values");

    DataSync::SyncInitiator initiator = DataSync::INIT_CLIENT;
    DataSync::SyncDirection direction = resolveSyncDirection( initiator );
    DataSync::ConflictResolutionPolicy policy = resolveConflictResolutionPolicy(initiator);
    DataSync::SyncMode mode( direction, initiator );
    iConfig->setSyncMode( mode );
    iConfig->setConflictResolutionPolicy( policy );
    iConfig->setAuthenticationType( DataSync::AUTH_BASIC );

    // Needed for ovi.com
    iConfig->setRemoteDevice( iProperties[PROF_REMOTE_URI] );
}

DataSync::SyncDirection SyncMLClient::resolveSyncDirection(
    const DataSync::SyncInitiator& aInitiator )
{
    FUNCTION_CALL_TRACE;

    Buteo::SyncProfile::SyncDirection directionFromProfile = iProfile.syncDirection();

    DataSync::SyncDirection direction = DataSync::DIRECTION_TWO_WAY;

    if( aInitiator == DataSync::INIT_CLIENT )
    {

        if( directionFromProfile == Buteo::SyncProfile::SYNC_DIRECTION_FROM_REMOTE )
        {
            direction = DataSync::DIRECTION_FROM_SERVER;
        }
        else if( directionFromProfile == Buteo::SyncProfile::SYNC_DIRECTION_TO_REMOTE )
        {
            direction = DataSync::DIRECTION_FROM_CLIENT;
        }
    }
    else if( aInitiator == DataSync::INIT_SERVER )
    {
        if( directionFromProfile == Buteo::SyncProfile::SYNC_DIRECTION_FROM_REMOTE )
        {
            direction = DataSync::DIRECTION_FROM_CLIENT;
        }
        else if( directionFromProfile == Buteo::SyncProfile::SYNC_DIRECTION_TO_REMOTE )
        {
            direction = DataSync::DIRECTION_FROM_SERVER;
        }
    }

    return direction;
}

DataSync::ConflictResolutionPolicy SyncMLClient::resolveConflictResolutionPolicy(
    const DataSync::SyncInitiator& aInitiator)
{
    FUNCTION_CALL_TRACE;

    Buteo::SyncProfile::ConflictResolutionPolicy crPolicyFromProfile =
        iProfile.conflictResolutionPolicy();

    DataSync::ConflictResolutionPolicy crPolicy = DataSync::PREFER_LOCAL_CHANGES;

    switch (crPolicyFromProfile)
    {
        case Buteo::SyncProfile::CR_POLICY_PREFER_SERVER:
        {
            if( aInitiator == DataSync::INIT_CLIENT )
            {
                crPolicy = DataSync::PREFER_REMOTE_CHANGES;
            }
            else if( aInitiator == DataSync::INIT_SERVER )
            {
                crPolicy = DataSync::PREFER_LOCAL_CHANGES;
            }
            break;
        }

        case Buteo::SyncProfile::CR_POLICY_PREFER_CLIENT:
        {
            if( aInitiator == DataSync::INIT_CLIENT )
            {
                crPolicy = DataSync::PREFER_LOCAL_CHANGES;
            }
            else if( aInitiator == DataSync::INIT_SERVER )
            {
                crPolicy = DataSync::PREFER_REMOTE_CHANGES;
            }
            break;
        }

        default:
        {
            break;
        }
    }

    return crPolicy;
}


/*!
    \fn SyncMLClient::initUsbTransportConfig()
 */
void SyncMLClient::initUsbTransportConfig()
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG("Setting USB profile hardcoded values");

    DataSync::SyncInitiator initiator = DataSync::INIT_SERVER;
    DataSync::SyncDirection direction = resolveSyncDirection( initiator );
    DataSync::ConflictResolutionPolicy policy = resolveConflictResolutionPolicy(initiator);
    DataSync::SyncMode mode( direction, initiator );
    iConfig->setSyncMode( mode );
    iConfig->setConflictResolutionPolicy( policy );
    iConfig->setAuthenticationType( DataSync::AUTH_NONE );
    //FIXME! Add extra headers here
}


/*!
    \fn SyncMLClient::initBtTransportConfig()
 */
void SyncMLClient::initBtTransportConfig()
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG("Setting BT profile hardcoded values\n");

    DataSync::SyncInitiator initiator;

    if ( isOviSuiteSync() ) {
        initiator = DataSync::INIT_CLIENT;
    } else {
        initiator = DataSync::INIT_SERVER;
    }

    DataSync::SyncDirection direction = resolveSyncDirection( initiator );
    DataSync::SyncMode mode( direction , initiator );
    DataSync::ConflictResolutionPolicy policy = resolveConflictResolutionPolicy(initiator);
    iConfig->setSyncMode( mode );
    iConfig->setConflictResolutionPolicy( policy );
    iConfig->setAuthenticationType( DataSync::AUTH_NONE );
    //FIXME! Add extra headers here
}


/*!
    \fn SyncMLClient::isOviSuiteSync()
 */
bool SyncMLClient::isOviSuiteSync()
{
    FUNCTION_CALL_TRACE;
    const QString OVISUITESERVICENAME = "ovisuite";
    return( iProfile.serviceProfile()->name() == OVISUITESERVICENAME );
}


void SyncMLClient::generateResults( bool aSuccessful )
{
    FUNCTION_CALL_TRACE;

    iResults.setResultCode( aSuccessful ? Buteo::SyncResults::SYNC_RESULT_SUCCESS : Buteo::SyncResults::SYNC_RESULT_FAILED );

    iResults.setTargetId(iAgent->getResults().getRemoteDeviceId());
    const QMap<QString, DataSync::DatabaseResults>* dbResults = iAgent->getResults().getDatabaseResults();

    if (dbResults->isEmpty())
    {
        LOG_DEBUG("No items transferred");
    }
    else
    {
        QMapIterator<QString, DataSync::DatabaseResults> i( *dbResults );
        while ( i.hasNext() )
        {
            i.next();
            const DataSync::DatabaseResults& r = i.value();
            Buteo::TargetResults targetResults(
                    i.key(), // Target name
                    Buteo::ItemCounts( r.iLocalItemsAdded,
                                       r.iLocalItemsDeleted,
                                       r.iLocalItemsModified ),
                    Buteo::ItemCounts( r.iRemoteItemsAdded,
                                       r.iRemoteItemsDeleted,
                                       r.iRemoteItemsModified ));
            iResults.addTargetResults( targetResults );

            LOG_DEBUG("Items for" << targetResults.targetName() << ":");
            LOG_DEBUG("LA:" << targetResults.localItems().added <<
                      "LD:" << targetResults.localItems().deleted <<
                      "LM:" << targetResults.localItems().modified <<
                      "RA:" << targetResults.remoteItems().added <<
                      "RD:" << targetResults.remoteItems().deleted <<
                      "RM:" << targetResults.remoteItems().modified);
        }
    }
}
