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

#ifndef ITEMIDMAPPER_H
#define ITEMIDMAPPER_H

#include <QString>
#include <QtSql>

/*! \brief Storage for persistently mapping ID's supplied by storage plugins to
 *         formats suitable for remote SyncML parties
 *
 */
class ItemIdMapper {

public:
    /*! \brief Constructor
     *
     */
    ItemIdMapper();

    /*! \brief Destructor
     *
     */
    virtual ~ItemIdMapper();

    /*! \brief Initializes ID mapper for storage
     *
     * @param aDbFile Path to database to use as persistent storage
     * @param aStorageId Identifier for storage
     * @return True if successfully initialized, otherwise false
     */
    bool init( const QString& aDbFile, const QString& aStorageId );

    /*! \brief Uninitializes ID mapper for storage
     *
     */
    void uninit();

    /*! \brief Maps the specified value to key
     *
     * @param aValue Value
     * @return Key
     */
    QString key( const QString& aValue );

    /*! \brief Maps the specified key to value
     *
     * @param aKey Key
     * @return Value
     */
    QString value( const QString& aKey );

protected:

    /**
     * \brief Adds a new mapping for aKey
     * @param aKey An item ID that need mapped value
     * @return Mapping for aKey
     */
    QString add(const QString& aKey);

private:

    QSqlDatabase    iDb;
    QString         iConnectionName;
    QString         iStorageId;

    // Because value() may use add(), and add() recursively uses values(),
    // a recursion guard is used to prevent infinite loop. Although is should
    // never happen, this is an easy way to eliminate even the possibility of it.
    bool            iRecursionGuard;
    
    friend class ItemIdMapperTest;

};

#endif  //  ITEMIDMAPPER_H
