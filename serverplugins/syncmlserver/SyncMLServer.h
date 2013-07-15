/*
* This file is part of buteo-sync-plugins package
*
* Copyright (C) 2013 Jolla Ltd. and/or its subsidiary(-ies).
*
* Author: Sateesh Kavuri <sateesh.kavuri@gmail.com>
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
*/
#ifndef SYNCMLSERVER_H
#define SYNCMLSERVER_H

#include "syncmlserver_global.h"

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <buteosyncfw5/ServerPlugin.h>
#include <buteosyncfw5/SyncCommonDefs.h>
#include <buteosyncfw5/SyncResults.h>
#else
#include <buteosyncfw/ServerPlugin.h>
#include <buteosyncfw/SyncCommonDefs.h>
#include <buteosyncfw/SyncResults.h>
#endif

namespace Buteo {
    class ServerPlugin;
    class Profile;
}

class SYNCMLSERVERSHARED_EXPORT SyncMLServer : public Buteo::ServerPlugin
{
    Q_OBJECT

public:
    SyncMLServer (const QString& pluginName,
                  const Buteo::Profile profile,
                  Buteo::PluginCbInterface *cbInterface);

    virtual ~SyncMLServer ();

    virtual bool init ();

    virtual bool uninit ();

    virtual void abortSync (Sync::SyncStatus status = Sync::SYNC_ABORTED);

    virtual bool cleanUp ();

    virtual Buteo::SyncResults getSyncResults () const;

    virtual bool startListen ();

    virtual void stopListen ();

    virtual void suspend ();

    virtual void resume ();

public slots:

    virtual void connectivityStateChanged (Sync::ConnectivityType type, bool state);

};

#endif // SYNCMLSERVER_H
