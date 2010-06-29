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

#include "SyncMLConfig.h"

#include <QDir>

#include <LogMacros.h>

const QString XMLDIR( "/etc/sync/xml/");
const QString DBDIR( "/.sync/sync-app/" );
const QString DEVINFO_FILE_NAME("devInfo.xml");

SyncMLConfig::SyncMLConfig()
{
    FUNCTION_CALL_TRACE;
}

SyncMLConfig::~SyncMLConfig()
{
    FUNCTION_CALL_TRACE;
}


QString SyncMLConfig::getDatabasePath()
{
	FUNCTION_CALL_TRACE;

	QString home = QDir::homePath();
	QString path = home + DBDIR;

	QDir dir( path );
	if( !dir.exists() ) {
		dir.mkpath( path );
	}

	return path;

}

QString SyncMLConfig::getXmlDataPath()
{
    FUNCTION_CALL_TRACE;

    return XMLDIR;
}

QString SyncMLConfig::getDevInfoFile()
{
    return SyncMLConfig::getDatabasePath() + DEVINFO_FILE_NAME;
}


