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

#include "DeviceInfo.h"
#include "LogMacros.h"
//#include <sysinfo.h>
#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

using namespace Buteo;

const QString SYSINFO_KEY_IMEI("/certs/npc/esn/gsm");

const QString SYSINFO_KEY_MODEL("/component/product-name");

const QString SYSINFO_KEY_HW_VER("/device/hw-version");

const QString SYSINFO_KEY_SW_VER("/device/sw-release-ver");

const QString XML_KEY_ID("Id");

const QString XML_KEY_MODEL("Model");

const QString XML_KEY_HW_VER("HwVersion");

const QString XML_KEY_SW_VER("SwVersion");

const QString XML_KEY_FW_VER("FwVersion");

const QString XML_KEY_DEV_TYPE("DeviceType");

const QString IMEI("IMEI:");

const QString DUMMY_IMEI("000000000000000");

const QString DEVINFO_DEVTYPE("phone");

DeviceInfo::DeviceInfo()
{
	FUNCTION_CALL_TRACE;

    iProperties << XML_KEY_ID << XML_KEY_MODEL << XML_KEY_HW_VER << XML_KEY_SW_VER << XML_KEY_FW_VER << XML_KEY_DEV_TYPE;

    iSource = ReadFromSystem;
}

DeviceInfo::~DeviceInfo()
{
	FUNCTION_CALL_TRACE;
}

QString DeviceInfo::getDeviceIMEI()
{
	FUNCTION_CALL_TRACE;


	if( iDeviceIMEI.isEmpty() || iDeviceIMEI == DUMMY_IMEI) {
        iDeviceIMEI = IMEI + getSysInfo(SYSINFO_KEY_IMEI);
        if(iDeviceIMEI.isEmpty()) {
            LOG_CRITICAL("Failed retrieving the IMEI. HARDCODING THE IMEI ");
            iDeviceIMEI = IMEI + getSysInfo(SYSINFO_KEY_IMEI);
		}
	}

	return iDeviceIMEI;
}

QString DeviceInfo::getSysInfo(const QString &key)
{
    QString value;
//    struct system_config *sc = 0;
//    if( sysinfo_init(&sc) == 0 )
//    {
//        uint8_t *data = 0;
//        unsigned long size = 0;
//        QByteArray ba = key.toLatin1();
//        const char * key = ba.data();
//        if( sysinfo_get_value(sc, key, &data, &size) == 0 )
//        {
//            unsigned long k;
//            for( k = 0; k < size; ++k )
//            {
//                int c = data[k];
//                value.append(c);
//            }
//        }
//        sysinfo_finish(sc);
//    }

    LOG_DEBUG("Key is  " << key << "it's Value is " << value);
    return value;
}

QString DeviceInfo::getModel()
{
    FUNCTION_CALL_TRACE;

    if( iModel.isEmpty()) {
        iModel = getSysInfo(SYSINFO_KEY_MODEL);
        if(iModel.isEmpty()) {
            LOG_DEBUG("Failed retrieving the Model");
        }
    }

    return iModel;
}


QString DeviceInfo::getSwVersion()
{
    FUNCTION_CALL_TRACE;

    if( iSwVersion.isEmpty()) {
        iSwVersion = getSysInfo(SYSINFO_KEY_SW_VER);
        if(iSwVersion.isEmpty()) {
            LOG_DEBUG("Failed retrieving the software version");
        }
    }

    return iSwVersion;
}


QString DeviceInfo::getHwVersion()
{
    FUNCTION_CALL_TRACE;

    if( iHwVersion.isEmpty()) {
        iHwVersion = getSysInfo(SYSINFO_KEY_HW_VER);
        if(iHwVersion.isEmpty()) {
            LOG_DEBUG("Failed retrieving the hardware version");
        }
    }

    return iHwVersion;
}

QString DeviceInfo::getFwVersion()
{
    FUNCTION_CALL_TRACE;

    return getHwVersion();
}


QString DeviceInfo::getDeviceType()
{
    FUNCTION_CALL_TRACE;

    if( iDeviceType.isEmpty()) {
        iDeviceType = DEVINFO_DEVTYPE;
    }

    return iDeviceType;
}


void DeviceInfo::setSourceToRead(Source &aSource)
{
    FUNCTION_CALL_TRACE;

    iSource = aSource;

}


DeviceInfo::Source DeviceInfo::getSourceToRead()
{
    FUNCTION_CALL_TRACE;

    return iSource;
}


bool DeviceInfo::setDeviceXmlFile(QString &aFileName)
{
    FUNCTION_CALL_TRACE;
    QFile file(aFileName);
    bool status = false;

    if(file.exists()) {
        iDeviceInfoFile = aFileName;
        status = true;
    }

    return status;
}


QString DeviceInfo::DeviceXmlFile()
{
    FUNCTION_CALL_TRACE;
    return iDeviceInfoFile;
}

QMap<QString,QString> DeviceInfo::getDeviceInformation()
{
    FUNCTION_CALL_TRACE;
    QMap<QString,QString> deviceMap;

    switch(iSource) {

    case ReadFromSystem:
        foreach(QString property,iProperties) {
            if(property == XML_KEY_ID) {
                deviceMap.insert(property,getDeviceIMEI());
            }else if (property == XML_KEY_MODEL) {
                deviceMap.insert(property,getModel());
            }else if (property == XML_KEY_SW_VER){
                deviceMap.insert(property,getSwVersion());
            }else if (property == XML_KEY_HW_VER) {
                deviceMap.insert(property,getHwVersion());
            } else if (property == XML_KEY_FW_VER) {
                deviceMap.insert(property,getFwVersion());
            } else if (property == XML_KEY_DEV_TYPE){
                deviceMap.insert(property,getDeviceType());
            } else {
                LOG_DEBUG("Unknown Property");
            }
        }
        break;
   case ReadFromXml:
        {
            QFile file(iDeviceInfoFile);
            if(file.open(QIODevice::ReadOnly))
            {
                QByteArray data = file.readAll();
                QXmlStreamReader reader(data);
                while(!reader.atEnd()) {
                    reader.readNext();
                    QString key =  reader.name().toString();
                    reader.readNext();
                    deviceMap.insert(key,reader.text().toString());
                }
                file.close();
            } else {
                LOG_DEBUG("Failed to open the file " << iDeviceInfoFile );
            }
        }
        break;
    default:
        LOG_DEBUG("Source to read the system information is not set ");
        break;
    }

    return deviceMap;
}


void DeviceInfo::saveDevInfoToFile(QMap<QString,QString> &aDevInfo , QString &aFileName)
{
    FUNCTION_CALL_TRACE;
    QByteArray data;
    QXmlStreamWriter writer(&data);
    writer.setAutoFormatting(true);

    writer.writeStartDocument();
    writer.writeStartElement("DevInfo");
    // @TODO - add a DTD

    QMapIterator<QString, QString> i(aDevInfo);
    while (i.hasNext()) {
        i.next();
        LOG_DEBUG(i.key() << ": " << i.value() << endl);
        writer.writeTextElement(i.key(),i.value());
    }

    writer.writeEndElement();
    writer.writeEndDocument();

    QFile file(aFileName);

    if(file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        file.write(data);
        file.close();
    }

}
