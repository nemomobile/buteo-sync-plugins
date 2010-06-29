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

#ifndef STORAGEADAPTER_H
#define STORAGEADAPTER_H

#include <QMap>
#include <QVector>

#include <libsyncpluginmgr/StoragePlugin.h>
#include <libmeegosyncml/StoragePlugin.h>
#include <libmeegosyncml/SyncItemKey.h>

#include "ItemIdMapper.h"

class StoragePlugin;
class StorageItem;

/*! \brief Adapter to adapt framework storage plugin to SyncML stack storage
 *         plugin
 *
 * This adapter presumes that all DataSync::SyncItem instances passed as parameters
 * to functions of this adapter are originally created by this adapter.
 */
class StorageAdapter : public DataSync::StoragePlugin
{

public:

    /*! \brief Constructor
     *
     * @param aPlugin Plugin that this instance should adapt. Not owned
     */
    StorageAdapter( Buteo::StoragePlugin* aPlugin );

    /*! \brief Destructor
     *
     */
    virtual ~StorageAdapter();

    /*! \brief Returns if this adapter instance is valid
     *
     * @return True if this adapter instance is valid, otherwise false
     */
    bool isValid();

    /*! \brief Returns the FW plugin instance
     *
     * @return Plugin
     */
    Buteo::StoragePlugin* getPlugin() const;

    /*! \brief Initializes adapter
     *
     * Sets up SyncML storage plugin based on FW plugin properties
     *
     * @return True if initialization was successful, otherwise false
     */
    bool init();

    /*! \brief Uninitializes adapter
     *
     * @param True if uninitialization was successful, otherwise false
     */
    bool uninit();

    /*! \see DataSync::StoragePlugin::getSourceURI()
     *
     */
    virtual const QString& getSourceURI() const;

    /*! \see DataSync::StoragePlugin::getMaxObjSize()
     *
     */
    virtual qint64 getMaxObjSize() const;

    /*! \see DataSync::StoragePlugin::getSupportedFormats()
     *
     */
    virtual const QList<ContentFormat>& getSupportedFormats() const;

    /*! \see DataSync::StoragePlugin::getPreferredFormat()
     *
     */
    virtual const ContentFormat& getPreferredFormat() const;

    /*! \see DataSync::StoragePlugin::getPluginCTCaps()
     *
     */
    virtual QByteArray getPluginCTCaps( DataSync::ProtocolVersion aVersion ) const;

    /*! \see DataSync::StoragePlugin::getAll()
     *
     */
    virtual bool getAll( QList<DataSync::SyncItemKey>& aKeys );

    /*! \see DataSync::StoragePlugin::getModifications()
     *
     */
    virtual bool getModifications( QList<DataSync::SyncItemKey>& aNewKeys,
                                   QList<DataSync::SyncItemKey>& aReplacedKeys,
                                   QList<DataSync::SyncItemKey>& aDeletedKeys,
                                   const QDateTime& aTimeStamp );

    /*! \see DataSync::StoragePlugin::newItem()
     *
     */
    virtual DataSync::SyncItem* newItem();

    /*! \see DataSync::StoragePlugin::getSyncItem()
     *
     */
    virtual DataSync::SyncItem* getSyncItem( const DataSync::SyncItemKey& aKey );

    /*! \see DataSync::StoragePlugin::addItems()
     *
     */
    virtual QList<StoragePluginStatus> addItems( const QList<DataSync::SyncItem*>& aItems );

    /*! \see DataSync::StoragePlugin::replaceItems()
     *
     */
    virtual QList<StoragePluginStatus> replaceItems( const QList<DataSync::SyncItem*>& aItems );

    /*! \see DataSync::StoragePlugin::deleteItems()
     *
     */
    virtual QList<StoragePluginStatus> deleteItems( const QList<DataSync::SyncItemKey>& aKeys );

protected:

private:

    DataSync::StoragePlugin::StoragePluginStatus convertStatus( Buteo::StoragePlugin::OperationStatus aStatus ) const;

    Buteo::StorageItem* toStorageItem( const DataSync::SyncItem* aSyncItem ) const;

    Buteo::StoragePlugin*       iPlugin;

    QString                     iType;

    QList<ContentFormat>        iFormats;
    QString                     iSourceDB;
    QString                     iTargetDB;

    ItemIdMapper                iIdMapper;

};

#endif  //  STORAGEADAPTER_H
