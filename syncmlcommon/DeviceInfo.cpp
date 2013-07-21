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
#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

using namespace Buteo;

const QString XML_KEY_MANUFACTURER("Manufacturer");
const QString XML_KEY_MODEL("Model");
const QString XML_KEY_OEM("OEM");
const QString XML_KEY_HW_VER("HwVersion");
const QString XML_KEY_SW_VER("SwVersion");
const QString XML_KEY_FW_VER("FwVersion");
const QString XML_KEY_ID("Id");
const QString XML_KEY_DEV_TYPE("DeviceType");


const QString IMEI("IMEI:");
const QString DUMMY_IMEI("000000000000000");
const QString DEVINFO_DEVTYPE("phone");

DeviceInfo::DeviceInfo()
{
        FUNCTION_CALL_TRACE;

    iProperties << XML_KEY_MANUFACTURER << XML_KEY_MODEL << XML_KEY_HW_VER << XML_KEY_SW_VER << XML_KEY_FW_VER  << XML_KEY_ID << XML_KEY_DEV_TYPE;

    iSource = ReadFromSystem;
}

DeviceInfo::~DeviceInfo()
{
        FUNCTION_CALL_TRACE;
}

QString DeviceInfo::getDeviceIMEI()
{
        FUNCTION_CALL_TRACE;

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
        /// @todo returning first IMEI for now; needs fixing on multisim devices
        // FIXME: As of 21July2013, this method aborts
        //if (deviceInfo.imeiCount () > 0)
        //    return IMEI + deviceInfo.imei(0);
        //else
            return IMEI + DUMMY_IMEI;
#else
        return IMEI + deviceInfo.imei();
#endif
}

QString DeviceInfo::getManufacturer()
{
    FUNCTION_CALL_TRACE;

    return deviceInfo.manufacturer();
}

QString DeviceInfo::getModel()
{
    FUNCTION_CALL_TRACE;

    return deviceInfo.model();
}


QString DeviceInfo::getSwVersion()
{
    FUNCTION_CALL_TRACE;

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    return deviceInfo.version(QDeviceInfo::Firmware);
#else
    return systemInfo.version(QtMobility::QSystemInfo::Firmware);
#endif
}


QString DeviceInfo::getHwVersion()
{
    FUNCTION_CALL_TRACE;

    //Empty now
    return iHwVersion;
}

QString DeviceInfo::getFwVersion()
{
    FUNCTION_CALL_TRACE;

    return getSwVersion();
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
        foreach(const QString& property,iProperties) {
            if(property == XML_KEY_MANUFACTURER) {
                deviceMap.insert(property,getManufacturer());
            }else if (property == XML_KEY_MODEL) {
                deviceMap.insert(property,getModel());
            }else if (property == XML_KEY_SW_VER){
                deviceMap.insert(property,getSwVersion());
            }else if (property == XML_KEY_HW_VER) {
                deviceMap.insert(property,getHwVersion());
            } else if (property == XML_KEY_FW_VER) {
                deviceMap.insert(property,getFwVersion());
            }else if(property == XML_KEY_ID) {
                deviceMap.insert(property,getDeviceIMEI());
            } else if (property == XML_KEY_DEV_TYPE){
                deviceMap.insert(property,getDeviceType());
            } else {
                LOG_DEBUG("Unknown Property:" << property);
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
                     if(reader.tokenType() == QXmlStreamReader::StartElement) {
                         if(reader.name() ==  "DevInfo" ) {
                                  reader.readNext();
                         } else {
                            QString key =  reader.name().toString();
                            reader.readNext();
                            deviceMap.insert(key,reader.text().toString());
                            reader.readNext();
                         }
                     }
                        reader.readNext();
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
        LOG_DEBUG(i.key() << ": " << i.value());
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
