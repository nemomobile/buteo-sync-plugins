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

#ifndef SIMPLEITEM_H
#define SIMPLEITEM_H

#include <buteosyncfw/StorageItem.h>

/*! \brief Simple implementation for storage item
 *
 * This implementation can be used when data of the item is so small in size
 * that it can be cached in memory
 */
class SimpleItem : public Buteo::StorageItem
{
public:

    /*! \brief Constructor
     *
     */
    SimpleItem();

    /*! \brief Destructor
     *
     */
    virtual ~SimpleItem();

    /*! \see StorageItem::write()
     *
     */
    virtual bool write( qint64 aOffset, const QByteArray& aData );

    /*! \see StorageItem::read()
     *
     */
    virtual bool read( qint64 aOffset, qint64 aLength, QByteArray& aData ) const;

    /*! \see StorageItem::resize()
     *
     */
    virtual bool resize( qint64 aLen );

    /*! \see StorageItem::getSize()
     *
     */
    virtual qint64 getSize() const;

protected:

private:

    QByteArray iData;
};

#endif  //  SIMPLEITEM_H
