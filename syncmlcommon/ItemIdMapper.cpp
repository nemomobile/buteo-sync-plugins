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

#include "ItemIdMapper.h"

#include <LogMacros.h>

const QString CONNECTIONNAME( "idmapper" );

ItemIdMapper::ItemIdMapper() :
    iNextValue(1)
{
    FUNCTION_CALL_TRACE;
}

ItemIdMapper::~ItemIdMapper()
{
    FUNCTION_CALL_TRACE;
}

bool ItemIdMapper::init( const QString& aDbFile, const QString& aStorageId )
{
    FUNCTION_CALL_TRACE;

    static unsigned connectionNumber = 0;

    LOG_DEBUG( "Initiating ID mapper..." );

    if( !iDb.isOpen() ) {
        iConnectionName = CONNECTIONNAME + QString::number( connectionNumber++ );
        iDb = QSqlDatabase::addDatabase( "QSQLITE", iConnectionName );
        iDb.setDatabaseName( aDbFile );
        if(!iDb.open()) { //CID 29154
            LOG_CRITICAL( "Could open ID database file:" << aDbFile );
            return false;
        }
    }

    iStorageId = aStorageId;

    QString queryString;
    QSqlQuery query;

    queryString.append( "CREATE TABLE if not exists " );
    queryString.append( iStorageId );
    queryString.append(" (value integer primary key, key varchar(512))" );

    query = QSqlQuery( queryString, iDb );
    if( !query.exec() ) {
        LOG_CRITICAL("Create Query failed: " << query.lastError());
        return false;
    }


    // Load the key,value pairs in memory
    queryString.clear();
    queryString.append( "SELECT key, value FROM " );
    queryString.append( iStorageId );
    query = QSqlQuery( queryString, iDb );
    if( query.exec() )
    {
        while( query.next() )
        {
            iKeyToValueMap[query.value(0).toString()] = query.value(1).toUInt();
            iValueToKeyMap[query.value(1).toUInt()] = query.value(0).toString();
            LOG_DEBUG("Mapped key " << query.value(0).toString() << " and value " << query.value(1).toString());
        }
        iNextValue = iKeyToValueMap.count() + 1;
    }

    LOG_DEBUG( "ID mapper initiated" );
    return true;

}


void ItemIdMapper::uninit()
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG( "Uninitiating ID mapper..." );

    {
        QString queryString;
        QSqlQuery query;

        bool supportsTransaction = iDb.transaction();
        if( !supportsTransaction )
        {
            LOG_DEBUG("Db doesn't support transactions");
        }

        queryString.append( "DELETE FROM " );
        queryString.append( iStorageId );
        query = QSqlQuery( queryString, iDb );
        if( !query.exec() )
        {
            LOG_WARNING("Delete Query failed: " << query.lastError());
        }

        queryString.clear();
        queryString.append( "INSERT INTO " );
        queryString.append( iStorageId );
        queryString.append( " (value, key) values(:values, :key)" );
        query = QSqlQuery( queryString, iDb );
        QVariantList keys, values;
        for( int i = 0; i < iValueToKeyMap.count(); ++i )
        {
            values << (i + 1);
            keys << iValueToKeyMap[i+1];
        }
        query.addBindValue( values );
        query.addBindValue( keys );
        if( !query.execBatch() )
        {
            LOG_CRITICAL("Save Query failed: " << query.lastError());
        }

        if( supportsTransaction )
        {
            if( !iDb.commit() )
            {
                LOG_CRITICAL("Commit failed");
            }
        }
    }
    iDb.close();
    iDb = QSqlDatabase();
    QSqlDatabase::removeDatabase( iConnectionName );

    iNextValue = 1;

    LOG_DEBUG( "ID mapper uninitiated" );

}


QString ItemIdMapper::key( const QString& aValue )
{
    FUNCTION_CALL_TRACE;

    // NB#153991:In case SyncML stack asks for empty key, we shouldn't treat
    // it as an error situation, rather just not do mapping in that case.

    QString key = aValue;

    if( !iValueToKeyMap.contains( aValue.toUInt() ) ) {
        LOG_DEBUG("Value is empty, mapping not done");
    }
    else {
        key = iValueToKeyMap.value( aValue.toUInt() );
    }

    return key;
}


QString ItemIdMapper::value( const QString& aKey )
{
    FUNCTION_CALL_TRACE;

    QString value = aKey;

    // If the key is already an integer, no mapping is needed.
    bool keyIsInt;
    int id = aKey.toInt(&keyIsInt);
    Q_UNUSED(id);

    if (aKey.isEmpty()) {
        LOG_WARNING("Key is empty. Not trying to do mapping");
    }
    else if( !keyIsInt ) {
        if( !iKeyToValueMap.contains( aKey ) )
        {
           value = add( aKey );
        }
        else
        {
            value = QString::number( iKeyToValueMap.value( aKey ) );
        }
    }

    return value;
}

QString ItemIdMapper::add( const QString &aKey )
{
   iKeyToValueMap[aKey] = iNextValue;
   iValueToKeyMap[iNextValue] = aKey;
   return QString::number( iNextValue++ );
}
