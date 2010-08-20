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
#include <QMap>

#include "DeviceInfo.h"
#include "DeviceInfoTest.h"

using namespace Buteo;

void DeviceInfoTest::initTestCase()
{
	iDevInfo = new Buteo::DeviceInfo();
}

void DeviceInfoTest::cleanupTestCase()
{
	delete iDevInfo;
}

void DeviceInfoTest::testSourceToRead()
{
	Buteo::DeviceInfo::Source source = Buteo::DeviceInfo::ReadFromXml;
	iDevInfo->setSourceToRead(source);
	QCOMPARE(iDevInfo->getSourceToRead(),Buteo::DeviceInfo::ReadFromXml);
}

void DeviceInfoTest::testSetDeviceXml()
{
	QString file("test.xml");
	QVERIFY(iDevInfo->setDeviceXmlFile(file));
}

void DeviceInfoTest::testGetDeviceInformation()
{
	QMap<QString,QString> deviceMap;
	deviceMap = iDevInfo->getDeviceInformation();
	QVERIFY(deviceMap.size() != 0);
	QCOMPARE(deviceMap.size(),4);
}

void DeviceInfoTest::testSaveDeviceInformation()
{
	QString file("test.xml");
	QMap<QString,QString> deviceMap;
	deviceMap.insert("Id","1234567789");
	deviceMap.insert("Model","N900");
	deviceMap.insert("SwVersion","RX-blahblahblah");
	deviceMap.insert("HwVErsion","0322");
	iDevInfo->saveDevInfoToFile(deviceMap,file);
}

void DeviceInfoTest::testGetSysInfo()
{
	QString dummy = iDevInfo->getSysInfo("/component/product-name");
	QVERIFY(dummy.size() == 0);
}

void DeviceInfoTest::testGetDeviceIMEI()
{
	QString imei = iDevInfo->getDeviceIMEI();
	QVERIFY(!imei.isEmpty());
}

void DeviceInfoTest::testGetSwVersion()
{
	QString swver = iDevInfo->getSwVersion();
	QVERIFY(swver.isEmpty());
}

void DeviceInfoTest::testGetHwVersion()
{
	QString hwver = iDevInfo->getHwVersion();
	QVERIFY(hwver.isEmpty());
}

void DeviceInfoTest::testGetFwVersion()
{
	QString fwver = iDevInfo->getFwVersion();
	QVERIFY(fwver.isEmpty());
}

void DeviceInfoTest::testGetModel()
{
	QString model = iDevInfo->getModel();
	QVERIFY(model.isEmpty());
}

void DeviceInfoTest::testGetDeviceType()
{
	QString devType = iDevInfo->getDeviceType();
	QVERIFY(!devType.isEmpty());
	QVERIFY(devType == "phone" );
}
