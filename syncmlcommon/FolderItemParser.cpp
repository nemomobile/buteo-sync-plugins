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

#include "FolderItemParser.h"
#include <LogMacros.h>
#include <QDomDocument>

static const QString TAG_FOLDER("Folder");
static const QString TAG_FOLDER_NAME("name");
static const QString TAG_FOLDER_CREATED("created");
static const QString TAG_FOLDER_MODIFIED("modified");
static const QString DATE_TIME_FORMAT("yyyyMMddThhmmssZ");

const QString FolderItemParser::FOLDER_MIME_TYPE("application/vnd.omads-folder+xml");

bool FolderItemParser::readFolderData(const QByteArray &aRawData,
    FolderData &aFolderData)
{
    FUNCTION_CALL_TRACE;

    QDomDocument doc;
    if (!doc.setContent(aRawData))
        return false;

    QDomElement root = doc.documentElement();
    if (root.tagName() != TAG_FOLDER)
        return false;

    QDomElement name = root.firstChildElement(TAG_FOLDER_NAME);
    if (name.isNull())
        return false;

    aFolderData.iName = name.text();

    QDomElement created = root.firstChildElement(TAG_FOLDER_CREATED);
    if (!created.isNull())
    {
        aFolderData.iCreated = parseDateTime(created.text());
    }
    QDomElement modified = root.firstChildElement(TAG_FOLDER_MODIFIED);
    if (!modified.isNull())
    {
        aFolderData.iModified = parseDateTime(modified.text());
    }

    return true;
}

QByteArray FolderItemParser::writeFolderData(const FolderData &aFolderData)
{
    FUNCTION_CALL_TRACE;

    QDomDocument doc;
    QDomElement root = doc.createElement(TAG_FOLDER);
    doc.appendChild(root);

    QDomElement name = doc.createElement(TAG_FOLDER_NAME);
    root.appendChild(name);
    name.appendChild(doc.createTextNode(aFolderData.iName));

    if (!aFolderData.iCreated.isNull())
    {
        QDomElement created = doc.createElement(TAG_FOLDER_CREATED);
        root.appendChild(created);
        created.appendChild(doc.createTextNode(encodeDateTime(aFolderData.iCreated)));
    }
    if (!aFolderData.iModified.isNull())
    {
        QDomElement modified = doc.createElement(TAG_FOLDER_MODIFIED);
        root.appendChild(modified);
        modified.appendChild(doc.createTextNode(encodeDateTime(aFolderData.iModified)));
    }

    return doc.toByteArray(2); // Indent = 2
}

QDateTime FolderItemParser::parseDateTime(const QString &aDateTime)
{
    QDateTime dateTime;
    if (aDateTime.endsWith('Z', Qt::CaseInsensitive))
    {
        // UTC format.
        dateTime = QDateTime::fromString(aDateTime, DATE_TIME_FORMAT);
        dateTime.setTimeSpec(Qt::UTC);
        dateTime = dateTime.toLocalTime();
    }
    else
    {
        // Local time format.
        dateTime = QDateTime::fromString(aDateTime,
            DATE_TIME_FORMAT.left(DATE_TIME_FORMAT.size() - 1));
        dateTime.setTimeSpec(Qt::LocalTime);
    }

    return dateTime;
}

QString FolderItemParser::encodeDateTime(const QDateTime &aDateTime)
{
    return aDateTime.toUTC().toString(DATE_TIME_FORMAT);
}

