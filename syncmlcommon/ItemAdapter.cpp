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

#include "ItemAdapter.h"

#include <libsyncpluginmgr/StorageItem.h>

ItemAdapter::ItemAdapter( Buteo::StorageItem* aItem ) : iItem( aItem )
{
}

ItemAdapter::~ItemAdapter()
{
    delete iItem;
    iItem = NULL;
}

bool ItemAdapter::isValid()
{
    if( iItem ) {
        return true;
    }
    else {
        return false;
    }
}

Buteo::StorageItem& ItemAdapter::getItem() const
{
    return *iItem;
}

qint64 ItemAdapter::getSize() const
{
    return iItem->getSize();
}

bool ItemAdapter::read( qint64 aOffset, qint64 aLength, QByteArray& aData ) const
{
    return iItem->read( aOffset, aLength, aData );
}

bool ItemAdapter::write( qint64 aOffset, const QByteArray& aData )
{
    return iItem->write( aOffset, aData );
}

bool ItemAdapter::resize( qint64 aLen )
{
    return iItem->resize( aLen );
}
