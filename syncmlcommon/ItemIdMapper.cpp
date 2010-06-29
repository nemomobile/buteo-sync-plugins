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
    iRecursionGuard(false)
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
        if(!iDb.open()) //CID 29154
        	LOG_CRITICAL( "Cannot open database" );
    }


    if( !iDb.isOpen() ) {
        LOG_CRITICAL( "Could open ID database file:" << aDbFile );
        return false;
    }

    QString queryString;
    queryString.append( "CREATE TABLE if not exists " );
    queryString.append( aStorageId );

    // The value of primary key starts from 1 (in SQLite).
    queryString.append(" (value integer primary key autoincrement, key varchar(512))" );

    QSqlQuery query( iDb );

    query.prepare( queryString );
    query.exec();

    if( query.lastError().isValid() ) {
        LOG_WARNING("Query failed: " << query.lastError());
        return false;
    }
    else {
        LOG_DEBUG( "Ensured database table:" << aStorageId );
    }

    iStorageId = aStorageId;

    LOG_DEBUG( "ID mapper initiated" );

    return true;

}


void ItemIdMapper::uninit()
{
    FUNCTION_CALL_TRACE;

    LOG_DEBUG( "Uninitiating ID mapper..." );

    iDb.close();
    iDb = QSqlDatabase();
    QSqlDatabase::removeDatabase( iConnectionName );

    LOG_DEBUG( "ID mapper uninitiated" );

}


QString ItemIdMapper::key( const QString& aValue )
{
    FUNCTION_CALL_TRACE;

    // NB#153991:In case SyncML stack asks for empty key, we shouldn't treat
    // it as an error situation, rather just not do mapping in that case.

    if( aValue.isEmpty() ) {
        LOG_DEBUG("Value is empty, mapping not done");
        return aValue;
    }

    QString key = aValue;

    QString queryString;
    queryString.append( "SELECT key FROM " );
    queryString.append( iStorageId );
    queryString.append( " WHERE value = :value" );

    QSqlQuery query( iDb );

    query.prepare( queryString );
    query.bindValue( ":value", aValue );
    query.exec();

    if( query.lastError().isValid() ) {
        LOG_WARNING("Query failed: " << query.lastError());
    }
    else if( !query.next() ) {
        LOG_DEBUG( "Could not find key for value" << aValue );
    }
    else {
        key = query.value(0).toString();
    }

    LOG_DEBUG( "Value" << aValue << "mapped to key" << key );

    return key;
}


QString ItemIdMapper::value( const QString& aKey )
{
    FUNCTION_CALL_TRACE;

    if (aKey.isEmpty()) {
        LOG_WARNING("Key is empty. Not trying to do mapping");
        return aKey;
    }

    QString value = aKey;

    // If the key is already an integer, no mapping is needed.
    bool keyIsInt;
    int id = aKey.toInt(&keyIsInt);
    Q_UNUSED(id);

    if (!keyIsInt)
    {
        QString queryString;
        queryString.append( "SELECT value FROM " );
        queryString.append( iStorageId );
        queryString.append( " WHERE key = :key" );

        QSqlQuery query( iDb );

        query.prepare( queryString );
        query.bindValue( ":key", aKey );
        query.exec();

        if( query.lastError().isValid() ) {
            LOG_WARNING("Query failed: " << query.lastError());
        }
        else if( !query.next() ) {

            if (iRecursionGuard) {
                LOG_WARNING("Infinite recursion hazard detected!. Safely exiting. No mapping done.");
            }
            else{
                LOG_DEBUG( "Mapping for key" << aKey << "not found, generating..." );
                value = add(aKey);
            }

        }
        else {
            value = query.value(0).toString();
        }
    }

    LOG_DEBUG( "Key" << aKey << "mapped to value" << value );

    return value;
}



/*!
    \fn ItemIdMapper::add(const QString& aKey)
 */
QString ItemIdMapper::add(const QString& aKey)
{
    FUNCTION_CALL_TRACE;

    QString queryString;

    queryString.append( "INSERT INTO ");
    queryString.append( iStorageId );
    queryString.append( " (key) values(:key)" );

    QSqlQuery query( iDb );
    query.prepare( queryString );
    query.bindValue( ":key", aKey );
    query.exec();

    QString mappedKey = aKey;
    
    if( query.lastError().isValid() ) {
        LOG_WARNING("Query failed: " << query.lastError());
    }
    else {
        iRecursionGuard = true;
        mappedKey = value(aKey);
    }

    iRecursionGuard = false;
    
    return mappedKey;
}
