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
#ifndef CALENDARSTORAGE_H
#define CALENDARSTORAGE_H

#include <libsyncpluginmgr/StoragePlugin.h>

//sync backend related includes
#include "definitions.h"
#include "StoragePlugin.h"
#include "StorageItem.h"
#include "CalendarBackend.h"

enum STORAGE_TYPE {VCALENDAR_FORMAT,ICALENDAR_FORMAT};

/// \brief StoragePlugin class for harmattan
class CalendarStorage : public Buteo::StoragePlugin
{


public:
    /// \brief constructor
    CalendarStorage(const QString& aPluginName);

    /// \brief destructor
    ~CalendarStorage();

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

private:

    KCalCore::Incidence::Ptr generateIncidence( Buteo::StorageItem& aItem );

    void retrieveItems( KCalCore::Incidence::List& aIncidences, QList<Buteo::StorageItem*>& aItems );

    Buteo::StorageItem* retrieveItem( KCalCore::Incidence::Ptr& aIncidence );

    void retrieveIds( KCalCore::Incidence::List& aIncidences, QList<QString>& aIds );

    QDateTime normalizeTime( const QDateTime& aTime ) const;

    QByteArray getCtCaps( const QString& aFilename ) const;

    CalendarStorage::OperationStatus mapErrorStatus(const CalendarBackend::ErrorStatus &aCalenderError) const;

    CalendarBackend iCalendar;
    STORAGE_TYPE    iStorageType;

    bool iCommitNow;

};

/*! \brief Creates calendar storage plugin
 *
 * @param aPluginName Human-readable name for plugin
 * @return Calendar storage plugin instance
 */
extern "C" Buteo::StoragePlugin* createPlugin( const QString& aPluginName );

/*! \brief Destroys calendar storage plugin
 *
 * @param aStorage Calendar storage plugin instance to destroy
 */
extern "C" void destroyPlugin( Buteo::StoragePlugin *aStorage );

#endif // CALENDARSTORAGE_H

