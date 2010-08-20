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
#ifndef CONTACT_STORAGE_HEADER_1717
#define CONTACT_STORAGE_HEADER_1717

#include <QDateTime>
#include <QMap>

#include "StoragePlugin.h"
#include "ContactsBackend.h"
#include "libsyncpluginmgr/DeletedItemsIdStorage.h"

class SimpleItem;

//! \brief Harmattan Contact storage plugin
//
//  Interface to Storage Plugin towards Sync FW
class ContactStorage : public Buteo::StoragePlugin
{

public:
    /*! \brief Constructor
     *
     */
    ContactStorage(const QString& aPluginName);

    /*! \brief Destructor
     *
     */
    virtual ~ContactStorage();

    ////////////////////////////////////////////////////////////////////////////////
    //             Functions below are derived from storage plugin   ///////////////
    ////////////////////////////////////////////////////////////////////////////////

    /*! \brief Initializes the plugin
     *
     *
     * @param aProperties Properties that should be set for this plugin
     * @return True on success, otherwise false
     */
    virtual bool init( const QMap<QString, QString>& aProperties );

    /*! \brief Uninitializes the plugin
     *
     */
    virtual bool uninit();

    /*! \brief Returns all known items
     *
     * @param aItems Array where to place items
     * @return True on success, otherwise false
     */
    virtual bool getAllItems(QList<Buteo::StorageItem*> &aItems);


    /*! \brief Returns id's of all known items
         *
     * @param aItems Array where to place item id's
     * @return True on success, otherwise false
     */
    virtual bool getAllItemIds( QList<QString>& aItems );


    /*! \brief Returns all new items since aTime
     *
     * @param aNewItems Array where to place items
     * @param aTime Timestamp
     * @return True on success, otherwise false
     */
    virtual bool getNewItems( QList<Buteo::StorageItem*>& aNewItems, const QDateTime& aTime );


    /*! \brief Returns id's of all new items since aTime
     *
     * @param aNewItemIds Array where to place item id's
     * @param aTime Timestamp
     * @return True on success, otherwise false
     */
    virtual bool getNewItemIds( QList<QString>& aNewItemIds, const QDateTime& aTime );

    /*! \brief Returns all modified items since aTime
     *
     * @param aModifiedItems Array where to place items
     * @param aTime Timestamp
     * @return True on success, otherwise false
     */
    virtual bool getModifiedItems( QList<Buteo::StorageItem*>& aModifiedItems, const QDateTime& aTime );

    /*! \brief Returns id's of all modified items since aTime
     *
     * @param aModifiedItemIds Array where to place item id's
     * @param aTime Timestamp
     * @return True on success, otherwise false
     */
    virtual bool getModifiedItemIds( QList<QString>& aModifiedItemIds, const QDateTime& aTime );


    /*! \brief Returns id's of all deleted items since aTime
     *
     * @param aDeletedItemIds Array where to place item id's
     * @param aTime Timestamp
     * @return True on success, otherwise false
     */
    virtual bool getDeletedItemIds( QList<QString>& aDeletedItemIds, const QDateTime& aTime );

    /*! \brief Generates a new item
     *
     * Returned item is temporary. Therefore returned item ALWAYS has its id
     * set as empty ID (""). ID will be assigned only after addItem() has been
     * called for the item.
     *
     * @return On success pointer to the item generated, otherwise NULL
     */
    virtual Buteo::StorageItem* newItem();


    /*! \brief Returns an item based on id
     *
     * @param aItemId Id of the item to return
     * @return On success pointer to the item, otherwise NULL
     */
    virtual Buteo::StorageItem* getItem( const QString& aItemId );

    /*! \brief Returns items based on ids
     *
     * @param aItemIdList Ids of the items
     * @return List of items
     */
        virtual QList<Buteo::StorageItem*> getItems( const QStringList& aItemIdList );

    /*! \brief Adds an item to the storage
     *
     * Upon successful addition, item is updated with its
     * assigned ID.
     *
     * @param aItem Item to add
     * @return Operation status code
     */
    virtual OperationStatus addItem( Buteo::StorageItem& aItem );


    /*! \brief Adds items to the storage
     *
     * Upon successful addition, items are updated with its
     * assigned ID.
     *
     * @param aItems Items to add
     * @return Operation status codes
     */
    virtual QList<OperationStatus> addItems( const QList<Buteo::StorageItem*>& aItems );


    /*! \brief Modifies an item in the storage
     *
     * @param aItem Item to modify
     * @return Operation status code
     */
    virtual OperationStatus modifyItem( Buteo::StorageItem& aItem );

    /*! \brief Modifies item in the storage
     *
     * @param aItems Items to add
     * @return Operation status codes
     */
    virtual QList<OperationStatus> modifyItems( const QList<Buteo::StorageItem*>& aItems );

    /*! \brief Deletes an item from the storage
     *
     * @param aItemId Id of the item to be deleted
     * @return Operation status code
     */
    virtual OperationStatus deleteItem( const QString& aItemId );

    /*! \brief Deletes an item from the storage
     *
     * @param aItemIds Id's of the item to be deleted
     * @return Operation status codes
     */
    virtual QList<OperationStatus> deleteItems( const QList<QString>& aItemIds );


private:

    bool doInitItemAnalysis();

    bool doUninitItemAnalysis();

    /*! \brief convert list of contacts into vector of storage items
     *
     *
     * @param aList List of contacts
     * @return list of storage items
     */
    QList<Buteo::StorageItem*> getStoreList(QList<QContactLocalId>&aList);

    QByteArray getCtCaps( const QString& aFilename ) const;

    ContactStorage::OperationStatus mapErrorStatus(const QContactManager::Error &aContactError) const;

    /**
     * \brief Converts a vcard data to a storage item object
     * @param aItemKey ID of the item
     * @param aItemData Data of the item
     * @return Storage item object (a pointer to SimpleItem)
     */
    SimpleItem* convertVcardToStorageItem(const QContactLocalId aItemKey,
                                          const QString& aItemData);
    
    ContactsBackend*                    iBackend;

    Buteo::DeletedItemsIdStorage        iDeletedItems; ///< Backend for tracking deleted items

    QMap<QContactLocalId, QDateTime>    iSnapshot;
    QList<QContactLocalId>              iFreshItems;


};

/// \brief returns a pointer to new contact storage object
extern "C" Buteo::StoragePlugin*  createPlugin(const QString& aPluginName);

/// \brief destroy contact storage object
extern "C" void destroyPlugin(Buteo::StoragePlugin* storage);

#endif //CONTACT_STORAGE_HEADER_1717

