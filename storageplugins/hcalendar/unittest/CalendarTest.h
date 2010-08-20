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
#ifndef HCALENDARTEST_H
#define HCALENDARTEST_H

#include <QVector>
#include <QString>
#include <QObject>

#include "CalendarStorage.h"

class StorageItem;

class CalendarTest : public QObject
{
    Q_OBJECT
public:
    CalendarTest();
    ~CalendarTest();

    private slots:
        void initTestCase();
        void cleanupTestCase();

        void testSuite();

    private:


        void runTestSuite( const QByteArray& aOriginalData, const QByteArray& aModifiedData,
                           Buteo::StoragePlugin& aPlugin );

        CalendarStorage iCalendarStorage;


};

#endif // HCALENDARTEST_H
