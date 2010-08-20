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

#ifndef DEFAULTDEVICEINFO_H
#define DEFAULTDEVICEINFO_H

#include <QString>
#include <QStringList>
#include <QMutex>
#include <QMap>
#include "DeviceInfo.h"

namespace Buteo {

    /*! \brief Default Implementaiton of DeviceInfo class
 *
 */
    class DeviceInfo 
    {

    public:

        //! getDeviceInformation call checks this enum
        enum Source {
	    //! read from system 	
            ReadFromSystem,

	    //! read from xml 
            ReadFromXml
        };

    /*! \brief Constructor
     *
     */
        DeviceInfo();

    /*! \brief Destructor
     *
     */
        virtual ~DeviceInfo();

        /*! \brief set properties to read from the devicse
         *
         * This API sets the source to read from
         * @return none
         */
        void setSourceToRead(Source &);


        /*! \brief get method for source
         *
         * @return Source
         */
        Source getSourceToRead();


        /*! \brief set the file path to read device information from
         *
         * This API sets the xml file on the system
         * from where the device info has to be read from
         * @return value of the QFile exists method
         */
        bool setDeviceXmlFile(QString &fileName);


        /*! \brief get the file name from where the device information is read from
         *
         * get method
         * @return value of the fileName set
         */
        QString DeviceXmlFile();


        /*! \brief Retrieves Device Information as Map
         *
         * This API reads the device information as per the respective implementation
         * @return key , value Map
         */
        QMap<QString,QString> getDeviceInformation();

        /*! \brief Saves Device Information to the filename
         *
         * This API saves the  device information to the  file
         */
        void saveDevInfoToFile(QMap<QString,QString> &aDevInfo , QString &aFileName);

    protected:

    private:


        Source iSource;
        QStringList iProperties;
        QString iDeviceInfoFile;
        QString iDeviceIMEI;
        QString iSwVersion;
        QString iHwVersion;
        QString iFwVersion;
        QString iModel;
        QString iDeviceType;

        /* @brief common method that queries the sysinfo client for a specific key
         * the key is of the specfic format.
         * on the device these keys can be obtained using the following command
         * 'sysinfoclient --list'
         * @return - value corresponding to the key */
        QString getSysInfo(const QString &key);


        QString getDeviceIMEI();
        QString getSwVersion();
        QString getHwVersion();
        QString getFwVersion();
        QString getModel();
        QString getDeviceType();

#ifdef SYNC_APP_UNITTESTS
       friend class DeviceInfoTest;
#endif


    };

}


#endif // DEFAULTDEVICEINFO_H
