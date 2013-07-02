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

#ifndef ITEMADAPTER_H
#define ITEMADAPTER_H

#include <LogMacros.h>

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <buteosyncml5/SyncItem.h>
#else
#include <buteosyncml/SyncItem.h>
#endif

namespace Buteo {
    class StorageItem;
}

/*! \brief Adapter to adapt framework storage item to SyncML stack sync item
 *
 */
class ItemAdapter : public DataSync::SyncItem
{

public:
    /*! \brief Constructor
     *
     * @param aItem Item that this instance should adapt. Ownership transferred
     */
    ItemAdapter( Buteo::StorageItem* aItem );

    /*! \brief Destructor
     *
     */
    virtual ~ItemAdapter();

    /*! \brief Returns if this adapter instance is valid
     *
     * @return True if this adapter instance is valid, otherwise false
     */
    bool isValid();

    /*! \brief Return the FW item instance
     *
     * @return Item
     *
     */
    Buteo::StorageItem& getItem() const;

    /*! \see DataSync::SyncItem::getSize()
     *
     */
    virtual qint64 getSize() const;

    /*! \see DataSync::SyncItem::read()
     *
     */
    virtual bool read( qint64 aOffset, qint64 aLength, QByteArray& aData ) const;

    /*! \see DataSync::SyncItem::write()
     *
     */
    virtual bool write( qint64 aOffset, const QByteArray& aData );

    /*! \see DataSync::SyncItem::resize()
     *
     */
    virtual bool resize( qint64 aLen );

protected:

private:

    Buteo::StorageItem*    iItem;

};

#endif  //  ITEMADAPTER_H
