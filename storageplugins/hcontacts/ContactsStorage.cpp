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

#include <QFile>
#include <LogMacros.h>
#include "ContactsStorage.h"
#include "SimpleItem.h"
#include "SyncMLCommon.h"
#include "SyncMLConfig.h"
#include <qcontact.h>
#include <qcontactmanager.h>
#include <qversitdocument.h>


QTM_USE_NAMESPACE

const char* CTCAPSFILENAME11 = "CTCaps_contacts_11.xml";
const char* CTCAPSFILENAME12 = "CTCaps_contacts_12.xml";


extern "C" Buteo::StoragePlugin*  createPlugin(const QString& aPluginName)
		{
	return new ContactStorage(aPluginName);
		}

extern "C" void destroyPlugin(Buteo::StoragePlugin* storage)
{
	delete storage;
}

ContactStorage::ContactStorage(const QString& aPluginName) :
        		Buteo::StoragePlugin(aPluginName)
        		{
	FUNCTION_CALL_TRACE;
	iPluginName = aPluginName;
	iBackend = NULL;
        		}

ContactStorage::~ContactStorage()
{
	FUNCTION_CALL_TRACE;
	if(iBackend) {
		qWarning()  << "uninit method has not been called" ;
		delete iBackend;
	}
}

QList<Buteo::StorageItem*> ContactStorage::getStoreList(QList<QContactLocalId> &aStrIDList)
{
	FUNCTION_CALL_TRACE;
    
	QList<Buteo::StorageItem*> itemList;


    if (iBackend != NULL) {
        QMap<QContactLocalId,QString> idDataMap;
        iBackend->getContacts(aStrIDList, idDataMap);
        QMapIterator<QContactLocalId, QString > iter(idDataMap);

        while (iter.hasNext()) {
            iter.next();
            SimpleItem* item = convertVcardToStorageItem(iter.key(), iter.value());

            if (item  != NULL) {
                itemList.append(item);
            }
            
        }
    }

	return itemList;
}

////////////////////////////////////////////////////////////////////////////////////
////////////       Below functions are derived from storage plugin   ///////////////
////////////////////////////////////////////////////////////////////////////////////

bool ContactStorage::init( const QMap<QString, QString>& aProperties )
{
	FUNCTION_CALL_TRACE;
   
    QVersitDocument::VersitType vCardVersion;	
    iDeletedItems.uninit();

    if (!initDeletedItemsIdStorage()) {
        return false;
    }

	iProperties = aProperties;

	QString propVersion = iProperties[STORAGE_DEFAULT_MIME_VERSION_PROP];

	if(propVersion == "3.0") {
		qDebug()  << "Storage is using VCard version 3.0\n";
		vCardVersion =  QVersitDocument::VCard30Type;
		iProperties[STORAGE_DEFAULT_MIME_PROP]          = "text/vcard";
	}
	else {
		qDebug()  << "Storage is using VCard version 2.1\n";
		vCardVersion = QVersitDocument::VCard21Type;
	}

	iProperties[STORAGE_SYNCML_CTCAPS_PROP_11]      = getCtCaps( CTCAPSFILENAME11 );
	iProperties[STORAGE_SYNCML_CTCAPS_PROP_12]      = getCtCaps( CTCAPSFILENAME12 );

	iBackend = new ContactsBackend(vCardVersion);
    
	if(!iBackend)  {
		qDebug()  <<"Memory Allocation Failed ";
		return false;
	}
    else if(!iBackend->init()) {
		qDebug() << "Failed To Init the Contacts Backend";
		return false;
	}

    if (!updateDeletedItemsList()) {
        return false;
    }
    
	return true;
}

bool ContactStorage::uninit()
{
	FUNCTION_CALL_TRACE;

    updateSnapshot(iSnapshotCreationTime);

    // If the backend object is NULL, there is nothing to do anyway, 
    // so the default value can be 'true' here.
    bool backendUninitOk = true;
    
    if (iBackend != NULL) {
        backendUninitOk = iBackend->uninit();
        delete iBackend;
        iBackend = NULL;
    }

    bool deleteItemsIdStorageUninitOk = iDeletedItems.uninit();

    return (backendUninitOk && deleteItemsIdStorageUninitOk);
}

QByteArray ContactStorage::getCtCaps( const QString& aFilename ) const
		{
	FUNCTION_CALL_TRACE;

	QFile ctCapsFile( SyncMLConfig::getXmlDataPath() + aFilename  );
	QByteArray ctCaps;

	if( ctCapsFile.open(QIODevice::ReadOnly)) {
		ctCaps = ctCapsFile.readAll();
		ctCapsFile.close();
	} else {
		qWarning()  << "Failed to open CTCaps file for contacts storage:" << aFilename ;
	}

	return ctCaps;
		}

bool ContactStorage::getAllItems(QList<Buteo::StorageItem*> &aItems)
{
	FUNCTION_CALL_TRACE;
	bool operationStatus = false;
	QList<QContactLocalId>  list;

	if(iBackend) {
		list = iBackend->getAllContactIds();
		if(list.size() != 0) {
			qDebug()  << " Number of items retrieved from Contacts " << list.size();
			aItems = getStoreList(list);
		}
		operationStatus = true;
	}
	return operationStatus;
}

bool ContactStorage::getAllItemIds( QList<QString>& aItems )
{
	FUNCTION_CALL_TRACE;
	bool operationStatus = false;
	QList<QContactLocalId>  list;
	if(iBackend) {
		list = iBackend->getAllContactIds();
		qDebug() << " Number of items retrieved from Contacts " << list.size();
		foreach(QContactLocalId id , list) {
			qDebug()  << "appending id " << id;
			aItems.append(QString::number(id));
		}
		operationStatus = true;
	}
	return operationStatus;
}

bool ContactStorage::getNewItems(QList<Buteo::StorageItem*> &aItems ,const QDateTime& aTime)
{
	FUNCTION_CALL_TRACE;
	bool operationStatus = false;
	QList<QContactLocalId>  list;
	if(iBackend) {
		qDebug()  << "****** getNewItems : Added After: ********" << aTime;
		list = iBackend->getAllNewContactIds(aTime);
		if(list.size() != 0) {
			qDebug()  << "New Item List Size is " << list.size();
			aItems = getStoreList(list);
		}
		operationStatus = true;
	}
	return operationStatus;
}

bool ContactStorage::getNewItemIds( QList<QString>& aNewItemIds, const QDateTime& aTime )
{
	FUNCTION_CALL_TRACE;
	bool operationStatus = false;
	QList<QContactLocalId>  list;

	if(iBackend) {
		qDebug()  << "****** getNewItem Ids : Added After: ********" << aTime;
		list = iBackend->getAllNewContactIds(aTime);

		foreach(QContactLocalId id , list) {
			aNewItemIds.append(QString::number(id));
		}

		operationStatus = true;
	}
	return operationStatus;
}

bool ContactStorage::getModifiedItems( QList<Buteo::StorageItem*>& aModifiedItems, const QDateTime& aTime )
{
	FUNCTION_CALL_TRACE;
	QList<QContactLocalId>  list;
	bool operationStatus = false;

	if(iBackend) {
		qDebug() << "******* getModifiedItems: From ********" << aTime;

		list = iBackend->getAllModifiedContactIds(aTime);

		aModifiedItems = getStoreList(list);

		operationStatus = true;
	}

	return operationStatus;
}

bool ContactStorage::getModifiedItemIds( QList<QString>& aModifiedItemIds, const QDateTime& aTime )
{
	FUNCTION_CALL_TRACE;
	QList<QContactLocalId>  list;
	bool operationStatus = false;

	if(iBackend) {
		qDebug() << "******* getModifiedItemIds : From ********" << aTime;

		list = iBackend->getAllModifiedContactIds(aTime);

		foreach(QContactLocalId id , list) {
			aModifiedItemIds.append(QString::number(id));
		}
		operationStatus = true;
	}
	return operationStatus;
}

bool ContactStorage::getDeletedItemIds( QList<QString>& aDeletedItemIds, const QDateTime& aTime )
{
	FUNCTION_CALL_TRACE;
    LOG_DEBUG( "Getting deleted contacts since" << aTime );
    
    return iDeletedItems.getDeletedItems( aDeletedItemIds, aTime );
}

Buteo::StorageItem* ContactStorage::newItem()
{
	FUNCTION_CALL_TRACE;
	return new SimpleItem;
}

Buteo::StorageItem* ContactStorage::getItem( const QString& aItemId )
{
	FUNCTION_CALL_TRACE;
    
	SimpleItem *newItem = NULL;
    
	if(iBackend) {
		QString contactData;
        iBackend->getContact(aItemId.toUInt(), contactData);
        
		if(!contactData.isEmpty()) {
			newItem = new SimpleItem;
			newItem->setId(aItemId);
			newItem->setType(iProperties[STORAGE_DEFAULT_MIME_PROP]);
			newItem->write(0,contactData.toUtf8());
		}
        else {
			qDebug()  << "Contact does not exist. the id given is : " << aItemId;
		}
	}
    
	return newItem;
}

ContactStorage::OperationStatus ContactStorage::addItem(Buteo::StorageItem& aItem)
{
	FUNCTION_CALL_TRACE;
	QString strID("");
        QList<QDateTime> creationTimes;
        QList<QString> newSnapShot;
	ContactStorage::OperationStatus status = STATUS_ERROR;
	if(iBackend) {
		QByteArray data;

		aItem.read(0, aItem.getSize(), data );
		QString Contact = QString::fromUtf8( data.data() );

		qDebug() << "Adding an Item with data : " << Contact;
		QContactManager::Error error = iBackend->addContact(Contact ,strID, creationTimes);

		status = mapErrorStatus(error);
		qDebug() << "After Adding an Item backend returned its id as : " << strID;

		aItem.setId(strID);

                iAddedItemsList.append(strID.toUInt());
                iCreationTimes += creationTimes;

	}else {
		qDebug()  << "Backend is not properly initialised";
	}
	return status;
}

QList<ContactStorage::OperationStatus> ContactStorage:: addItems( const QList<Buteo::StorageItem*>& aItems )
{
	FUNCTION_CALL_TRACE;
	QList<ContactStorage::OperationStatus> storageErrorList;

	if(iBackend) {

		QList<QString> contactsList;
		foreach(Buteo::StorageItem *item , aItems) {
			QByteArray data;
			item->read(0,item->getSize(),data);
			contactsList.append(QString::fromUtf8(data.data()));
		}

		QList<QString> contactsIdList;
		QList<QDateTime> creationTimes;
                QList<QString> newSnapShot;
		QMap<int, QContactManager::Error> contactsErrorMap; 
		bool retVal = iBackend->addContacts(contactsList, contactsErrorMap, creationTimes);

		if((contactsErrorMap.size()  != contactsList.size()) || !retVal)
		{

			LOG_WARNING("Something Wrong with Batch Addition in Contacts Backend : " << retVal);
			LOG_DEBUG("contactsErrorMap.size() " << contactsErrorMap.size() );
			LOG_DEBUG("contactsList.size()" << contactsList.size());

			for ( int i = 0; i < aItems.size(); i++) {
					storageErrorList.append(STATUS_ERROR);
			}

		} else  {

			QMapIterator<int ,QContactManager::Error> i(contactsErrorMap);
			int j = 0;
			while (i.hasNext()) {
				i.next();
				Buteo::StorageItem *item = aItems[j];
				item->setId(QString::number(i.key()));
                                iAddedItemsList.append(i.key());
				storageErrorList.append(mapErrorStatus(i.value()));
				j++;
			}
                        iCreationTimes += creationTimes;

		}
	} else {
		for ( int i = 0; i < aItems.size(); i++) {
			storageErrorList.append(STATUS_ERROR);
		}
	}

	return storageErrorList;
}


ContactStorage::OperationStatus ContactStorage::modifyItem(Buteo::StorageItem& aItem)
{
	FUNCTION_CALL_TRACE;

	ContactStorage::OperationStatus status = STATUS_ERROR;

	if(iBackend ) {
		QString strID = aItem.getId();
		QByteArray data;
		aItem.read( 0, aItem.getSize(), data );
		QString Contact = QString::fromUtf8( data );
		qDebug()  << "Modifying an Item with data : " << Contact;
		qDebug() << "Modifying an Item with ID : "  << strID;
		QContactManager::Error error = iBackend->modifyContact(strID ,Contact);
		status = mapErrorStatus(error);
		qDebug()  << "After Modification String ID  is " << strID;
	}

	return status;
}

QList<ContactStorage::OperationStatus> ContactStorage::modifyItems(const QList<Buteo::StorageItem *> &aItems)
{
	FUNCTION_CALL_TRACE;
	QList<ContactStorage::OperationStatus> storageErrorList;

	qDebug()  << "Items to Modify :"  << aItems.size();

	if(iBackend) {

        QStringList contactsList;
        QStringList contactsIdList;
		foreach(Buteo::StorageItem *item , aItems) {
			QByteArray data;
			item->read(0,item->getSize(),data);
			contactsList.append(QString::fromUtf8(data.data()));
			contactsIdList.append(item->getId());
		}

        QMap<int, QContactManager::Error> contactsErrorMap =
                iBackend->modifyContacts(contactsList, contactsIdList);

		if(contactsErrorMap.size()  != contactsList.size())
		{

			LOG_WARNING("Something Wrong with Batch Mofication in Contacts Backend");
			LOG_DEBUG("contactsErrroMap.size() " << contactsErrorMap.size() );
			LOG_DEBUG("contactsList.size()" << contactsList.size() );
		        for ( int i = 0; i < aItems.size(); i++) {
			    storageErrorList.append(STATUS_ERROR);
		        }

		} else  {

            QMapIterator<int, QContactManager::Error> i(contactsErrorMap);
			int j = 0;
			while (i.hasNext()) {
				i.next();
				Buteo::StorageItem *item = aItems[j];
				item->setId(QString::number(i.key()));
				LOG_DEBUG("Id set in Storage " << item->getId());
				storageErrorList.append(mapErrorStatus(i.value()));
				j++;
			}

		} // end if  contactsErrroMap.size()  != contactsList.size()

	} else {

		for ( int i = 0; i < aItems.size(); i++) {
			storageErrorList.append(STATUS_ERROR);
		}

	}
	return storageErrorList;
}

ContactStorage::OperationStatus ContactStorage::deleteItem( const QString& aItemId )
{
	FUNCTION_CALL_TRACE;
    
	ContactStorage::OperationStatus status = STATUS_ERROR;

    QString itemIdString = aItemId;
    QContactLocalId itemId = (QContactLocalId)aItemId.toUInt();
    QDateTime creationTime;
    
	if(iBackend != NULL) {
                creationTime = iBackend->creationTime(itemId);
		QContactManager::Error error = iBackend->deleteContact( itemIdString );
		status = mapErrorStatus(error);
                QDateTime deletionTime = QDateTime::currentDateTime();
                iDeletedItems.addDeletedItem( aItemId, creationTime, deletionTime);
	}
    
	return status;
}

QList<ContactStorage::OperationStatus> ContactStorage::deleteItems(const QList<QString>& aItemIds )
{
	FUNCTION_CALL_TRACE;
	QList<ContactStorage::OperationStatus> statusList;

	if(iBackend) {

		if(iBackend->batchUpdatesEnabled()) {

			QMap<int , QContactManager::Error> errorMap  =  iBackend->deleteContacts(aItemIds);
			if(errorMap.size() !=aItemIds.size() ) {
				LOG_WARNING("Something wrong with batch deletion of Contacts");
			LOG_DEBUG("contactsErrroMap.size() " << errorMap.size() );
			LOG_DEBUG("contactsList.size()" << aItemIds.size() );
		        for ( int i = 0; i < aItemIds.size(); i++) {
			    statusList.append(STATUS_ERROR);
		        }
			}
                        else
                        {

			QMapIterator<int ,QContactManager::Error> i(errorMap);
			while (i.hasNext()) {
				i.next();
				statusList.append(mapErrorStatus(i.value()));
			}
                        }

		} else {

			foreach(QString id, aItemIds) {
				statusList.append(this->deleteItem(id));
			}

		} // end if(iBackend->batchUpdatesEnabled())

	} else {

		for(int i =0 ; i < aItemIds.size() ; i++) {
			statusList.append(STATUS_ERROR);
		}

	}

	return statusList;
}

ContactStorage::OperationStatus ContactStorage::mapErrorStatus\
(const QContactManager::Error &aContactError) const
{
	ContactStorage::OperationStatus iStorageStatus = STATUS_ERROR;
	switch(aContactError) {
	case QContactManager::NoError:
		iStorageStatus = STATUS_OK;
		break;

	case QContactManager::DoesNotExistError:
		iStorageStatus = STATUS_NOT_FOUND;
		break;

	case QContactManager::AlreadyExistsError:
		iStorageStatus = STATUS_DUPLICATE;
		break;

	case QContactManager::InvalidDetailError:
	case QContactManager::VersionMismatchError:
	case QContactManager::InvalidContactTypeError:
		iStorageStatus = STATUS_INVALID_FORMAT;
		break;

	case QContactManager::OutOfMemoryError:
		iStorageStatus = STATUS_STORAGE_FULL;
		break;

	case QContactManager::LimitReachedError:
		iStorageStatus = STATUS_OBJECT_TOO_BIG;
		break;

	case QContactManager::InvalidRelationshipError:
	case QContactManager::LockedError:
	case QContactManager::DetailAccessError:
	case QContactManager::PermissionsError:
	case QContactManager::NotSupportedError:
	case QContactManager::BadArgumentError:
	case QContactManager::UnspecifiedError:
	default:
		iStorageStatus = STATUS_ERROR;
		break;
	}
	return iStorageStatus;
}


/*!
    \fn ContactStorage::initDeletedItemsIdStorage()
 */
bool ContactStorage::initDeletedItemsIdStorage()
{
    FUNCTION_CALL_TRACE;

    const QString dbFile = "hcontacts.db";
    QString fullDbPath = SyncMLConfig::getDatabasePath() + dbFile;

    return iDeletedItems.init(fullDbPath);
}


/*!
    \fn ContactStorage::updateSnaphot()
 */
void ContactStorage::updateSnapshot(QDateTime aSnapshotCreationtime)
{
    FUNCTION_CALL_TRACE;

    Q_UNUSED(aSnapshotCreationtime);
    
    QList<QString> newSnapshot;
    QList<QDateTime> creationTimes;
    QList<QContact> contactList;

    iBackend->getContacts(iItemsInBackend,contactList);
    LOG_DEBUG(iItemsInBackend.size() << " items in backend");
    LOG_DEBUG(iAddedItemsList.size() << " items added");

    foreach (QContact contact, contactList) {
	creationTimes.append(iBackend->creationTime(contact));
	newSnapshot.append(QString::number(contact.id().localId()));
    }
    foreach( QContactLocalId id, iAddedItemsList )
    {
        newSnapshot.append( QString::number( id ) );
    }
    creationTimes += iCreationTimes;
    iDeletedItems.setSnapshot( newSnapshot, creationTimes );

    iAddedItemsList.clear();
}


/*!
    \fn ContactStorage::readBackendItems()
 */
void ContactStorage::readBackendItems()
{
    FUNCTION_CALL_TRACE;
    
    iItemsInBackend = iBackend->getAllContactIds();
    iSnapshotCreationTime = QDateTime::currentDateTime();
}


/*!
    \fn ContactStorage::updateDeletedItemsList()
 */
bool ContactStorage::updateDeletedItemsList()
{
    FUNCTION_CALL_TRACE;
    
    QList<QString> items;
    QList<QDateTime> creationTimes;
    
    if( !iDeletedItems.getSnapshot(items, creationTimes) ) {
        return false;
    }

    readBackendItems();
   
    LOG_DEBUG( "Found" << items.count() << "items from snapshot" );
    LOG_DEBUG( "Found" << iItemsInBackend.count() << "items from backend" );

    QDateTime deletionTime = QDateTime::currentDateTime();
    
    // Item has been deleted if it can be found from snapshot but not from given items
    foreach (QString snapshotItemId, items) {
        if (!iItemsInBackend.contains(snapshotItemId.toUInt())) {
            LOG_DEBUG("Adding item " << snapshotItemId << " to deleted items list");
            int index = items.indexOf(snapshotItemId);
            iDeletedItems.addDeletedItem( items.at(index), creationTimes.at(index), deletionTime );
        }
    }

    return true;
}


/*!
    \fn ContactStorage::convertVcardToStorageItem(const QContactLocalId, const QString&)
 */
SimpleItem* ContactStorage::convertVcardToStorageItem(const QContactLocalId aItemKey,
                                                      const QString& aItemData)
{
    FUNCTION_CALL_TRACE;

    SimpleItem* storageItem = new SimpleItem;
    
    if(storageItem != NULL) {
        qDebug() << "ID is " << aItemKey;
        qDebug() << "Data is " << aItemData;
        storageItem->write( 0, aItemData.toUtf8() );
        storageItem->setId(QString::number(aItemKey));
        storageItem->setType(iProperties[STORAGE_DEFAULT_MIME_PROP]);
    }
    else {
        qWarning() << "Memory Allocation Failed";
    }
    
    return storageItem;
}
