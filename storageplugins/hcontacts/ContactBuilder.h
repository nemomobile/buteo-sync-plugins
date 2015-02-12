/*
 * This file is part of buteo-sync-plugins package
 *
 * Copyright (C) 2015 Jolla Ltd.
 * Contact: Chris Adams <chris.adams@jollamobile.com>
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

#ifndef CONTACTBUILDER_H
#define CONTACTBUILDER_H

#include <seasidecontactbuilder.h>

#include <QString>

class ContactBuilder : public SeasideContactBuilder
{
public:
    enum MatchFilterMode {
        NoFilterRequiredMode = 0,
        FilterRequiredMode
    };

    ContactBuilder(QContactManager *mgr, const QString &syncTarget, const QString &originId, MatchFilterMode mode);
    ~ContactBuilder();

    QContactFilter mergeSubsetFilter() const;
    bool mergeLocalIntoImport(QContact &import, const QContact &local, bool *erase);

    // no-op functions:
    bool mergeImportIntoImport(QContact &, QContact &, bool *erase);
    int previousDuplicateIndex(QList<QContact> &, QContact &, int);
};

#endif
