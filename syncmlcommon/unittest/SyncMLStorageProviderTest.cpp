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

#include "SyncMLStorageProviderTest.h"
#include <buteosyncml/StoragePlugin.h>
#include <QDomDocument>

#include <QtTest/QtTest>


using namespace Buteo;

void SyncMLStorageProviderTest :: initTestCase()
{
    iSyncMLStorageProvider = new SyncMLStorageProvider();

    iProfile = new Buteo::Profile("dummyprofile", Buteo::Profile::TYPE_STORAGE);
    iTempPluginCbInterface = new TempPluginCbInterface();
    iTempSyncPluginBase = new TempSyncPluginBase(iTempPluginCbInterface);

    // call init()
    QVERIFY(iSyncMLStorageProvider->init(iProfile, iTempSyncPluginBase, iTempPluginCbInterface, true));
}

void SyncMLStorageProviderTest :: cleanupTestCase()
{
    // uninit() is always returning true
    QVERIFY(iSyncMLStorageProvider->uninit());

    // deallocate the memory
    delete iSyncMLStorageProvider;
    delete iTempSyncPluginBase;
    delete iTempPluginCbInterface;
    delete iProfile;

}

void SyncMLStorageProviderTest :: testStorages()
{
    // test acqurieStorage() & releaseStorage() without any sub profiles

    // acquireStorageByURI() & acquireStorageByMIME() internally calls acquireStorage()
    DataSync::StoragePlugin *uriStorage = iSyncMLStorageProvider->acquireStorageByURI("./contacts");
    DataSync::StoragePlugin *mimeStorage = iSyncMLStorageProvider->acquireStorageByMIME("text/x-vcard");

    QVERIFY(uriStorage == 0);
    QVERIFY(mimeStorage == 0);

    iSyncMLStorageProvider->releaseStorage(uriStorage);
    iSyncMLStorageProvider->releaseStorage(mimeStorage);


    // test acquireStorage() & releaseStorage() functions with sub profiles

    // create a profile from XML
    const QString profileXML =
            " <profile name=\"syncml\" type=\"server\" > "
                " <key name=\"bt_transport\" value=\"true\"/> "
                " <profile name=\"hcontacts\" type=\"storage\" > "
                        " <key name=\"enabled\" value=\"true\" /> "
                        " <key name=\"Local URI\" value=\"./contacts\" /> "                        
                        " <key name=\"Type\" value=\"text/x-vcard\" /> "
                        " <key name=\"Version\" value=\"2.1\" /> "
                "</profile>"
             "</profile>";

    QDomDocument doc;
    QVERIFY(doc.setContent(profileXML, false));
    Profile anotherProfile(doc.documentElement());
    anotherProfile.setName("anotherProfile");

    SyncMLStorageProvider *tempSyncMLStorageProvider = new SyncMLStorageProvider();
    tempSyncMLStorageProvider->init(&anotherProfile, iTempSyncPluginBase, iTempPluginCbInterface, true);

    uriStorage = tempSyncMLStorageProvider->acquireStorageByURI("./contacts");
    QVERIFY(uriStorage);
    tempSyncMLStorageProvider->releaseStorage(uriStorage);

    mimeStorage = tempSyncMLStorageProvider->acquireStorageByMIME("text/x-vcard");
    QVERIFY(mimeStorage);
    tempSyncMLStorageProvider->releaseStorage(mimeStorage);

    // deallocate memory
    tempSyncMLStorageProvider->uninit();
    delete tempSyncMLStorageProvider;
}

/* #####################################
   TempPluginCbInterface class functions
   #####################################
 */

StoragePlugin* TempPluginCbInterface ::createStorage(const QString& aStorage)
{
    StoragePlugin* plugin = NULL;
    QDir dir = QDir::current();
    QString path = dir.absolutePath();

    // need to replace the library path with dummy library path
    if (dir.cd("/usr/lib"))
    {
        path = dir.absolutePath();
    }

    iPluginManager = new PluginManager(path) ;

    if (!aStorage.isEmpty())
        plugin = iPluginManager->createStorage(aStorage);
    return plugin;
}


void TempPluginCbInterface ::destroyStorage(StoragePlugin *aStorage)
{
    iPluginManager->destroyStorage(aStorage);
    delete iPluginManager;
}
