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
#include "FolderItemParserTest.h"
//#include "FolderItemParser.cpp"

void FolderItemParserTest::initTestCase()
{
}
void FolderItemParserTest::cleanupTestCase()
{
}
void FolderItemParserTest::testWriteFolderData()
{
	FolderItemParser::FolderData folderData;
	FolderItemParser::FolderData fldrData;
	QByteArray byte;
	folderData.iName = "Parser";
	
	QVERIFY(folderData.iCreated.isNull());
	QVERIFY(folderData.iModified.isNull());
	byte = iFolder.writeFolderData(folderData);
	//qDebug()<<"Byte is"<<byte;
	QVERIFY(byte.contains("Parser"));
	QVERIFY(byte.contains("Folder"));
	QVERIFY(byte.contains("name"));
	QCOMPARE(iFolder.readFolderData(byte,fldrData), true);
	QCOMPARE(fldrData.iName,(QString)"Parser");
	
	QDateTime created;
	created = QDateTime::currentDateTime();
	folderData.iCreated = created;
	QCOMPARE(folderData.iCreated.isNull(), false);
	QVERIFY(folderData.iModified.isNull());
	QString str;
	str = created.toUTC().toString("yyyyMMddThhmmssZ");
	QCOMPARE(iFolder.encodeDateTime(folderData.iCreated),str);
	byte = iFolder.writeFolderData(folderData);
	//qDebug()<<"Byte is"<<byte;
	QVERIFY(byte.contains("created"));
	const QChar *str1 = str.constData();
	QVERIFY(byte.contains((const char *)str1));
	
	QDateTime parse = iFolder.parseDateTime(str);
	QCOMPARE(parse.date(), created.date());
	QCOMPARE(iFolder.readFolderData(byte,fldrData), true);
	QCOMPARE(fldrData.iCreated.date(),folderData.iCreated.date());
	
	QDateTime modified;
	modified = QDateTime::currentDateTime();
	folderData.iModified = modified;
	QCOMPARE(folderData.iCreated.isNull(), false);
	QCOMPARE(folderData.iModified.isNull(), false);
	QString string;
	string = modified.toUTC().toString("yyyyMMddThhmmssZ");
	QCOMPARE(iFolder.encodeDateTime(folderData.iModified),string);
	byte = iFolder.writeFolderData(folderData);
	QVERIFY(byte.contains("modified"));
	const QChar *str2 = string.constData();
	QVERIFY(byte.contains((const char *)str2));
	
	QCOMPARE(iFolder.parseDateTime(string).date(), modified.date());
	QCOMPARE(iFolder.readFolderData(byte,fldrData), true);
	QCOMPARE(fldrData.iModified.date(),folderData.iModified.date());
}
void FolderItemParserTest::testReadFolderData()
{
}