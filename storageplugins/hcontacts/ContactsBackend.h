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
#ifndef CONTACTSBACKEND_H_
#define CONTACTSBACKEND_H_

#include <qcontactmanager.h>
#include <qcontact.h>
#include <qcontactchangelogfilter.h>
#include <qcontactid.h>
#include <qversitdocument.h>
#include <QStringList>

QTM_USE_NAMESPACE;

enum VCARD_VERSION { VCARD_VERSION21, VCARD_VERSION30 };

struct ContactsStatus
{
    int id;
    QContactManager::Error errorCode;
};
     
//! \brief Harmattan Contact storage plugin backend interface class
///
/// This class interfaces with the backend implementation of contact manager on harmattan
/// device
class ContactsBackend
{

public:
	
    /*!
     * \brief Constructor
     * @param aVerType 
     */
    ContactsBackend(QVersitDocument::VersitType aVerType);

	
    /*!
     * \brief Destructor
     */
    ~ContactsBackend();

    /*!
     * \brief Searches for available storage plugins and sets the manager to that plugin
     * @return 
     */
    bool init();

    /*!
     * \brief releases the resources held.
     * @return 
     */
    bool uninit();
    
    /*!
     * \brief Return ids of all contacts retrieved from the backend
     * @return List of contact IDs
     */
    QList<QContactLocalId> getAllContactIds();

    /*!
     * \brief Return all new contacts ids in a QList of QStrings
     * @param aTimeStamp Timestamp of the oldest contact ID to be returned
     * @return List of contact IDs
     */
    QList<QContactLocalId> getAllNewContactIds(const QDateTime& aTimeStamp);

    /*!
     * \brief Return all modified contact ids in a QList of QStrings
     * @param aTimeStamp Timestamp of the oldest contact ID to be returned
     * @return List of contact IDs
     */
    QList<QContactLocalId> getAllModifiedContactIds(const QDateTime& aTimeStamp);


    /*!
     * \brief Return all deleted contacts ids in a QList of QStrings
     * @param aTimeStamp Timestamp of the oldest contact ID to be returned
     * @return List of contact IDs
     */
    QList<QContactLocalId> getAllDeletedContactIds(const QDateTime& aTimeStamp);
    
    /*!
     * \brief Get contact data for a given gontact ID as a QContact object
     * @param aContactId The ID of the contact
     * @param aContact The returned data of the contact
     */
    void getContact(const QContactLocalId& aContactId,
                    QContact& aContact);
    
	
    /*!
     * \brief Get multiple contacts at once as vcards
     * @param aContactIDs List of contact IDs to be returned
     * @param aContactData Returned contact data
     */
    void getContacts(const QList<QContactLocalId> &aContactIDs,
                     QMap<QContactLocalId,QString>& aContactData );


    /*!
     * \brief Get multiple contacts at once as QContact objects
     * @param aContactIds List of contact IDs
     * @param aContacts List of returned contact data
     */
    void getContacts(const QList<QContactLocalId>& aContactIds,
                     QList<QContact>& aContacts);

    /*!
     * \brief Batch addition of contacts
     * @param aContactDataList Contact data
     * @param aStatusMap Returned status data
     * @return Errors
     */
    bool addContacts( const QStringList &aContactDataList,
                      QMap<int, ContactsStatus> &aStatusMap );

    // Functions for modifying contacts

    /*!
     * \brief Modify a contact that whose data and ID are  given as Input
     * @param id Contact ID
     * @param contactdata Contact data
     * @return Error
     */
    QContactManager::Error modifyContact(const QString &id, const QString &contactdata);

    /*!
     * \brief Batch modification
     * @param aContactDataList Contact data
     * @param aContactsIdList Contact IDs
     * @return Errors
     */
    QMap<int, ContactsStatus> modifyContacts(const QStringList &aContactDataList,
                                             const QStringList &aContactsIdList);

    /*!
     * \brief Batch deletion of contacts
     * @param aContactIDList Contact IDs
     * @return Errors
     */
    QMap<int, ContactsStatus> deleteContacts(const QStringList &aContactIDList);


    /*!
     * \brief Tells if batch updates are enabled
     * @return True if enabled, false if not
     */
    inline bool batchUpdatesEnabled() {  return true;     }
    
    /*!
     * \brief Returns the last time the contact was modified
     * @param  aContactId Id of the contact
     * @return Timestamp of contact's last modification time
     */
    QDateTime lastModificationTime(const QContactLocalId &aContactId);

    /*! \brief Return creation time of single contact
     *
     * @param aContact Contact
     * @return Creation time
     */
    QDateTime getCreationTime( const QContact& aContact );

    /*! \brief Returns creation times of the contacts
     *
     * @param aContactIds Ids of the contacts
     * @return Creation times
     */
    QList<QDateTime> getCreationTimes( const QList<QContactLocalId>& aContactIds );


    /*! \brief Converts a QContact to a VCard
     *
     * @param aContact Contact
     * @return VCard
     */
    QString convertQContactToVCard(const QContact &aContact);
private: // functions

    QMap<QContactLocalId, QString> convertQContactListToVCardList \
    					(const QList<QContact> &aContactList);

    QList<QContact> convertVCardListToQContactList \
    				(const QStringList &aVCardList);

    /*!
     * \brief Returns contact IDs specified by event type and timestamp
     * @param aEventType Added/changed/removed contacts
     * @param aTimeStamp Contacts older than aTimeStamp are filtered out
     * @param aIdList Returned contact IDs
     */
    void getSpecifiedContactIds(const QContactChangeLogFilter::EventType aEventType,
                                const QDateTime &aTimeStamp,
                                QList<QContactLocalId> &aIdList);

    /*!
     * \brief Constructs and returns the filter for accessing only contacts allowed to be synchronized
     * Contacts not allowed to be synchronized are Instant messaging contacts and contacts with origin from other sync backends;
     * those contacts have QContactSyncTarget::SyncTarget value different from address book or buteo sync clients.
     * It is designed that buteo sync clients don't restrict access to contacts among themselves
     * - value for QContactSyncTarget::SyncTarget written by this backend is "buteo".
     */
    QContactFilter getSyncTargetFilter() const;

private: // data
    
    // if there is more than one Manager we need to have a list of Managers
    QContactManager                *iMgr;      ///< A pointer to contact manager

    QVersitDocument::VersitType    iVCardVer;  ///< VCard Version type to operate on

};





#endif /* CONTACTSBACKEND_H_ */



