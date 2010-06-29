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

#ifndef SYNCMLSTORAGEPROVIDER_H
#define SYNCMLSTORAGEPROVIDER_H

#include <libmeegosyncml/StorageProvider.h>

namespace Buteo {
    class Profile;
    class SyncPluginBase;
    class PluginCbInterface;
    class SyncMLStorageProviderTest;
}

/*! \brief Module that provides storages to libmeegosyncml in syncml
 *         client/server plugins
 *
 * This storage provider presumes that all DataSync::StoragePlugin instances
 * passed as parameters to function of this storage provider are originally
 * from this storage provider
 */
class SyncMLStorageProvider : public DataSync::StorageProvider
{
public:

    /*! \brief Constructor
     *
     */
    SyncMLStorageProvider();

    /*! \brief Destructor
     *
     */
    virtual ~SyncMLStorageProvider();

    /*! \brief Initializes the storage provider
     *
     * @param aProfile Profile with storage sub-profiles
     * @param aPlugin Plugin utilizing this storage provider
     * @param aCbInterface Callback interface to use to acquire and release storages
     * @param aRequestStorages If true, storage provider will request storages while creating them
     * @return True on success, otherwise false
     */
    bool init( Buteo::Profile* aProfile,
               Buteo::SyncPluginBase* aPlugin,
               Buteo::PluginCbInterface* aCbInterface,
               bool aRequestStorages );

    /*! \brief Uninitializes the storage provider
     *
     * @return True on success, otherwise false
     */
    bool uninit();

    /*! \see DataSync::StorageProvider::acquireStorageByURI()
     *
     */
    virtual DataSync::StoragePlugin* acquireStorageByURI( const QString& aURI );

    /*! \see DataSync::StorageProvider::acquireStorageByMIME()
     *
     */
    virtual DataSync::StoragePlugin* acquireStorageByMIME( const QString& aMIME );

    /*! \see DataSync::StorageProvider::releaseStorage()
     *
     */
    virtual void releaseStorage( DataSync::StoragePlugin* aStorage );

private:

    DataSync::StoragePlugin* acquireStorage( const Buteo::Profile* aProfile );

    Buteo::Profile*            iProfile;
    Buteo::SyncPluginBase*     iPlugin;
    Buteo::PluginCbInterface*  iCbInterface;
    bool                       iRequestStorages;

    friend class Buteo::SyncMLStorageProviderTest;

};

#endif  //  SYNCMLSTORAGEPROVIDER_H
