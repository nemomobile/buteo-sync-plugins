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
#ifndef NOTESSTORAGE_H
#define NOTESSTORAGE_H


#include <libsyncpluginmgr/StoragePlugin.h>

#include "NotesBackend.h"

/*! \brief Harmattan notes storage plugin
 *
 *
 */
class NotesStorage : public Buteo::StoragePlugin
{

public:

    /*! \brief Constructor
     *
     * @param aPluginName aPluginName Name of the plugin
     */
    NotesStorage( const QString& aPluginName );

    /*! \brief Destructor
     *
     */
    virtual ~NotesStorage();

    /*! \see StoragePlugin::init()
     *
     */
    virtual bool init( const QMap<QString, QString>& aProperties );

    /*! \see StoragePlugin::uninit()
     *
     */
    virtual bool uninit();

    /*! \see StoragePlugin::getAllItems()
     *
     */
    virtual bool getAllItems( QList<Buteo::StorageItem*>& aItems );

    /*! \see StoragePlugin::getAllItemIds()
     *
     */
    virtual bool getAllItemIds( QList<QString>& aItemIds );

    /*! \see StoragePlugin::getNewItems()
     *
     */
    virtual bool getNewItems( QList<Buteo::StorageItem*>& aNewItems, const QDateTime& aTime );

    /*! \see StoragePlugin::getNewItemIds()
     *
     */
    virtual bool getNewItemIds( QList<QString>& aNewItemIds, const QDateTime& aTime );

    /*! \see StoragePlugin::getModifiedItems()
     *
     */
    virtual bool getModifiedItems( QList<Buteo::StorageItem*>& aModifiedItems, const QDateTime& aTime );

    /*! \see StoragePlugin::getModifiedItemIds()
     *
     */
    virtual bool getModifiedItemIds( QList<QString>& aModifiedItemIds, const QDateTime& aTime );

    /*! \see StoragePlugin::getDeletedItemIds()
     *
     */
    virtual bool getDeletedItemIds( QList<QString>& aDeletedItemIds, const QDateTime& aTime );

    /*! \see StoragePlugin::newItem()
     *
     */
    virtual Buteo::StorageItem* newItem();

    /*! \see StoragePlugin::getItem()
     *
     */
    virtual Buteo::StorageItem* getItem( const QString& aItemId );

    /*! \see StoragePlugin::getItems()
     *
     */
    virtual QList<Buteo::StorageItem*> getItems( const QStringList& aItemIdList );

    /*! \see StoragePlugin::addItem()
     *
     */
    virtual OperationStatus addItem( Buteo::StorageItem& aItem );

    /*! \see StoragePlugin::addItems()
     *
     */
    virtual QList<OperationStatus> addItems( const QList<Buteo::StorageItem*>& aItems );

    /*! \see StoragePlugin::modifyItem()
     *
     */
    virtual OperationStatus modifyItem( Buteo::StorageItem& aItem );

    /*! \see StoragePlugin::modifyItems()
     *
     */
    virtual QList<OperationStatus> modifyItems( const QList<Buteo::StorageItem*>& aItems );

    /*! \see StoragePlugin::deleteItem()
     *
     */
    virtual OperationStatus deleteItem( const QString& aItemId );

    /*! \see StoragePlugin::deleteItems()
     *
     */
    virtual QList<OperationStatus> deleteItems( const QList<QString>& aItemIds );

protected:

private:

    QDateTime normalizeTime( const QDateTime& aTime ) const;

    QByteArray getCTCaps( const QString& aFilename ) const;

    NotesBackend    iBackend;

    bool            iCommitNow;
};

/*! \brief Creates notes storage plugin
 *
 * @param aPluginName Human-readable name for plugin
 * @return Notes storage plugin instance
 */
extern "C" Buteo::StoragePlugin* createPlugin( const QString& aPluginName );

/*! \brief Destroys notes storage plugin
 *
 * @param aStorage Notes storage plugin instance to destroy
 */
extern "C" void destroyPlugin( Buteo::StoragePlugin* aStorage );

#endif  //  NOTESSTORAGE_H
