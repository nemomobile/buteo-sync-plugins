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

#ifndef FOLDERITEMPARSER_H
#define FOLDERITEMPARSER_H

#include <QString>
#include <QDateTime>

/*! \brief Reads and writes data in application/vnd.omads-folder+xml format
 */
class FolderItemParser
{
public:

    //! Folder item MIME type.
    static const QString FOLDER_MIME_TYPE;

    //! Structure for storing folder data.
    struct FolderData {
    	//! Name of the folder
        QString iName;
        //! Created Time
        QDateTime iCreated;
        //! Modified Time
        QDateTime iModified;
    };

    /*! \brief Parses folder data from the provided byte array.
     *
     * @param aRawData Raw data in vnd.omads-folder-xml format.
     * @param aFolderData Parsed folder data.
     * @return Success indicator.
     */
    static bool readFolderData(const QByteArray &aRawData, FolderData &aFolderData);

    /*! \brief Writes folder data to a byte array.
     *
     * @param aFolderData Folder data to write.
     * @return Folder data in vnd.omads-folder+xml format.
     */
    static QByteArray writeFolderData(const FolderData &aFolderData);

private:

    /*! \brief Parses date and time from the given string.
     *
     * @param aDateTime String containing the date and time.
     * @return Parsed date/time, specified using Qt::LocalTime.
     */
    static QDateTime parseDateTime(const QString &aDateTime);

    /*! \brief Encodes date and time to a string in UTC format.
     *
     * @param aDateTime Date/time to encode.
     * @return Encoded date/time string.
     */
    static QString encodeDateTime(const QDateTime &aDateTime);
friend class FolderItemParserTest;
};

#endif
