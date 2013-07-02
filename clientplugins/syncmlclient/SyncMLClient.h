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

#ifndef SYNCMLCLIENT_H
#define SYNCMLCLIENT_H

#include "BTConnection.h"
#include "SyncMLStorageProvider.h"
#include <ClientPlugin.h>
#include <SyncResults.h>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <buteosyncml5/SyncAgent.h>
#else
#include <buteosyncml/SyncAgent.h>
#endif

namespace DataSync {

class Transport;
class SyncAgentConfig;

}

/*! \brief Implementation for SyncML client plugin
 *
 */
class SyncMLClient : public Buteo::ClientPlugin
{
    Q_OBJECT;
public:

    /*! \brief Constructor
     *
     * @param aPluginName Name of this client plugin
     * @param aProfile Sync profile
     * @param aCbInterface Pointer to the callback interface
     */
    SyncMLClient( const QString& aPluginName,
                  const Buteo::SyncProfile& aProfile,
                  Buteo::PluginCbInterface *aCbInterface );

    /*! \brief Destructor
     *
     * Call uninit before destroying the object.
     */
    virtual ~SyncMLClient();

    //! @see SyncPluginBase::init
    virtual bool init();

    //! @see SyncPluginBase::uninit
    virtual bool uninit();

    //! @see ClientPlugin::startSync
    virtual bool startSync();

    //! @see SyncPluginBase::abortSync
    virtual void abortSync(Sync::SyncStatus aStatus = Sync::SYNC_ABORTED);

    //! @see SyncPluginBase::getSyncResults
    virtual Buteo::SyncResults getSyncResults() const;

    //! @see SyncPluginBase::cleanUp
    virtual bool cleanUp();

public slots:

	//! @see SyncPluginBase::connectivityStateChanged
    virtual void connectivityStateChanged( Sync::ConnectivityType aType,
                                           bool aState );

protected slots:

	/*!
	 * \brief state change slot for DataSync::SyncAgent::stateChanged signal
	 * \param aState - new state
	 */
    void syncStateChanged( DataSync::SyncState aState );

    /*!
     * \brief sync Finished slot for DataSync::SyncAgent::syncFinished signal
     * \param aState - final state
     */
    void syncFinished( DataSync::SyncState aState );

    /*!
     * \brief slot for DataSync::SyncAgent::storageAcquired signal
     * \param aMimeType - mimetype of the storage acquired.
     */
    void storageAccquired(QString aMimeType);

    /*!
     * \brief slot for DataSync::SyncAgent::itemProcessed signal
     * \param aModificationType - modification type
     * \param aModifiedDatabase - modified database .
     * \param aLocalDatabase - local database
     * \param aMimeType - mime type of the database
     * \param aCommittedItems - No. of items committed for this operation
     */
    void receiveItemProcessed( DataSync::ModificationType aModificationType,
								DataSync::ModifiedDatabase aModifiedDatabase,
								QString aLocalDatabase,
								QString aMimeType,
                                                                int aCommittedItems );

private:

    bool initAgent();

    void closeAgent();

    bool initTransport();

    void closeTransport();

    bool initConfig();

    void closeConfig();

    /**
     * \brief Subroutine for obex transport initiation
     * @return True is success, false if not
     */
    bool initObexTransport();

    /**
     * \brief Subroutine for http transport initiation
     * @return True is success, false if not
     */
    bool initHttpTransport();

    /*! \brief Resolves sync direction from current profile
     *
     * @param aInitiator Initiator of the sync
     * @return Resolved sync direction
     */
    DataSync::SyncDirection resolveSyncDirection(
        const DataSync::SyncInitiator& aInitiator );

    /*! \brief Resolves conflict resolution policy from current profile
     *
     * @param aInitiator Initiator of the sync
     * @return Resolved conflict resolution policy
     */
    DataSync::ConflictResolutionPolicy resolveConflictResolutionPolicy(
        const DataSync::SyncInitiator& aInitiator );

    void generateResults( bool aSuccessful );

#ifndef QT_NO_DEBUG
    // helper function for debugging
    // does nothing in release mode
    QString toText( const DataSync::SyncState& aState );
#endif //#ifndef QT_NO_DEBUG

    QMap<QString, QString>      iProperties;

    DataSync::SyncAgent*        iAgent;

    BTConnection                iBTConnection;
    DataSync::Transport*        iTransport;

    DataSync::SyncAgentConfig*  iConfig;

    Buteo::SyncResults          iResults;

    SyncMLStorageProvider       iStorageProvider;

    quint32                     iCommittedItems;

};

/*! \brief Creates SyncML client plugin
 *
 * @param aPluginName Name of this client plugin
 * @param aProfile Profile to use
 * @param aCbInterface Pointer to the callback interface
 * @return Client plugin on success, otherwise NULL
 */
extern "C" SyncMLClient* createPlugin( const QString& aPluginName,
                                       const Buteo::SyncProfile& aProfile,
                                       Buteo::PluginCbInterface *aCbInterface );

/*! \brief Destroys SyncML client plugin
 *
 * @param aServer SyncML client plugin instance to destroy
 */
extern "C" void destroyPlugin( SyncMLClient *aClient );

#endif  //  SYNCMLCLIENT_H
