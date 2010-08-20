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

#ifndef SYNCMLCOMMON_H
#define SYNCMLCOMMON_H

const QString PROPS_FALSE                               = "false";
const QString PROPS_TRUE                                = "true";

// Properties found from storage plug-ins that can be used to configure storage
// adapter


// Preferred MIME type
const QString STORAGE_DEFAULT_MIME_PROP                 = "Type";

// Preferred MIME version
const QString STORAGE_DEFAULT_MIME_VERSION_PROP         = "Version";

// CTCaps for SyncML protocol
const QString STORAGE_SYNCML_CTCAPS_PROP_11             = "CTCaps_SyncML11";
const QString STORAGE_SYNCML_CTCAPS_PROP_12             = "CTCaps_SyncML12";

// Properties found from server/client plug-ins that can be used to configure storage
// adapter


// SyncML Source URI to associate with a storage
const QString STORAGE_SOURCE_URI                        = "Local URI";

// SyncML Target URI to associate with a storage
const QString STORAGE_REMOTE_URI                        = "Target URI";

// Profile properties

const QString PROF_SYNC_TRANSPORT     = "Sync Transport";
const QString PROF_SYNC_PROTOCOL      = "Sync Protocol";

const QString HTTP_TRANSPORT          = "HTTP";
const QString OBEX_TRANSPORT          = "OBEX";

const QString PROF_HTTP_PROXY_HOST    = "http_proxy_host";
const QString PROF_HTTP_PROXY_PORT    = "http_proxy_port";

const QString PROF_BT_ADDRESS         = "bt_address";
const QString PROF_REMOTE_ADDRESS     = "remote_id";
const QString PROF_BT_UUID            = "bt_uuid";

const QString PROF_REMOTE_URI         = "Remote database";
const QString PROF_USE_WBXML          = "use_wbxml";

const QString PROF_PASSWD             = "Password";
const QString PROF_USERID             = "Username";

const QString SYNCML11                = "SyncML11";
const QString SYNCML12                = "SyncML12";

const QString PROF_HTTP_XHEADERS      = "http_xheaders";

#endif  //  SYNCMLCOMMON_H
