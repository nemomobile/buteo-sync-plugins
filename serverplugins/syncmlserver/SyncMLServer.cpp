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
#include "SyncMLServer.h"
#include <LogMacros.h>

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <buteosyncfw5/SyncProfile.h>
#else
#include <buteosyncfw/SyncProfile.h>
#endif

extern "C" SyncMLServer* createPlugin(const QString& pluginName,
        const Buteo::SyncProfile& profile,
        Buteo::PluginCbInterface *cbInterface) {
    return new SyncMLServer(pluginName, profile, cbInterface);
}

extern "C" void destroyPlugin(SyncMLServer *server) {
    delete server;
}

SyncMLServer::SyncMLServer (const QString& pluginName,
                            const Buteo::Profile profile,
                            Buteo::PluginCbInterface *cbInterface) :
    ServerPlugin (pluginName, profile, cbInterface)
{
    FUNCTION_CALL_TRACE;
}

SyncMLServer::~SyncMLServer ()
{
    FUNCTION_CALL_TRACE;
}

bool
SyncMLServer::init ()
{
}

bool
SyncMLServer::uninit ()
{
}

void
SyncMLServer::abortSync (Sync::SyncStatus status)
{
}

bool
SyncMLServer::cleanUp ()
{
}

Buteo::SyncResults
SyncMLServer::getSyncResults () const
{
}

bool
SyncMLServer::startListen ()
{
    FUNCTION_CALL_TRACE;
}

void
SyncMLServer::stopListen ()
{
    FUNCTION_CALL_TRACE;
}

void
SyncMLServer::suspend ()
{
    FUNCTION_CALL_TRACE;
}

void
SyncMLServer::resume ()
{
    FUNCTION_CALL_TRACE;
}

