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

#ifndef SYNCMLCONFIG_H
#define SYNCMLCONFIG_H

#include <QString>

/*! \brief Common configuration class for SyncML related things
 *
 */
class SyncMLConfig
{
public:

    /*! \brief Constructor
     *
     */
    SyncMLConfig();

    /*! \brief Destructor
     *
     */
    ~SyncMLConfig();

    /*! \brief Returns path that should be used for storing
     *         databases
     *
     */
    static QString getDatabasePath();

    /*! \brief Returns path that stores misc xml data
     */
    static QString getXmlDataPath();

    /*! \brief Returns file used to store the device information
     *
     * @return QString - contains the full path to the file.
     */
    static QString getDevInfoFile();

    /*! \brief Returns the config XML files for configuring SyncML stack
     *
     * @param aDefaultConfigFile - the default config file, out parameter
     * @param aExtConfigFile - the extended properties config file, out parameter
     * @return void
     */
    static void syncmlConfigFilePaths (QString& aDefaultConfigFile, QString& aExtConfigFile);

protected:

private:

    static QString getHomePath();
};


#endif  //  SYNCMLCONFIG_H
