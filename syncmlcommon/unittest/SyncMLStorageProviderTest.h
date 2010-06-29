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

#ifndef SYNCMLSTORAGEPROVIDERTEST_H
#define SYNCMLSTORAGEPROVIDERTEST_H

#include "SyncMLStorageProvider.h"
#include <libsyncprofile/Profile.h>
#include <libsyncpluginmgr/SyncPluginBase.h>
#include <libsyncpluginmgr/PluginCbInterface.h>
#include <libsyncpluginmgr/PluginManager.h>

namespace Buteo {

class TempSyncPluginBase;
class TempPluginCbInterface;

class SyncMLStorageProviderTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testStorages();

private:
    SyncMLStorageProvider *iSyncMLStorageProvider;
    Buteo::Profile *iProfile;
    TempSyncPluginBase *iTempSyncPluginBase;
    TempPluginCbInterface *iTempPluginCbInterface;
};


class TempSyncPluginBase :public SyncPluginBase
{
private:
    bool init(){return true;}
    bool uninit(){return true;}
    void connectivityStateChanged( Sync::ConnectivityType aType, bool aState ){Q_UNUSED(aType);Q_UNUSED(aState);}

public:
    TempSyncPluginBase(PluginCbInterface *myPluginCbInterface) :SyncPluginBase("dummyPlugin", "dummyProfile", myPluginCbInterface){}
};


class TempPluginCbInterface : public PluginCbInterface
{
public:
    bool requestStorage(const QString&, const SyncPluginBase *aCaller) {Q_UNUSED(aCaller);return true;}
    void releaseStorage(const QString&, const SyncPluginBase *aCaller) {Q_UNUSED(aCaller);}
    QString getDeviceIMEI() {return QString("0");}
    bool isConnectivityAvailable(Sync::ConnectivityType) {return true;}

    // defined in source file - required for creating & releasing storage
    StoragePlugin* createStorage(const QString&);
    void destroyStorage(StoragePlugin *aStorage);

private:
    PluginManager *iPluginManager;
};

} // end - Buteo namespace


#endif // SYNCMLSTORAGEPROVIDERTEST_H
