/*
* This file is part of buteo-sync-plugins package
*
* Copyright (C) 2013 Jolla Ltd. and/or its subsidiary(-ies).
*
* Author: Sateesh Kavuri <sateesh.kavuri@gmail.com>
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
*/

#include <LogMacros.h>

#include "BTConnection.h"

BTConnection::BTConnection() :
    mFd (-1), mMutex (QMutex::Recursive), mDisconnected (true)
{
    FUNCTION_CALL_TRACE;
}

int
BTConnection::connect ()
{
    FUNCTION_CALL_TRACE;
    
    return mFd;
}

bool
BTConnection::isConnected () const
{
    FUNCTION_CALL_TRACE;

    if (mFd == -1)
        return false;
    else
        return true;
}

void
BTConnection::disconnect ()
{
    FUNCTION_CALL_TRACE;
    
}

int
BTConnection::openBTSocket ()
{
    FUNCTION_CALL_TRACE;

    return mFd;
}

void
BTConnection::closeBTSocket ()
{
    FUNCTION_CALL_TRACE;
}

bool
BTConnection::addServiceRecords (const QByteArray &sr, quint32 srId)
{
    FUNCTION_CALL_TRACE;
    
    return true;
}

bool
BTConnection::removeServiceRecords (const quint32 srId)
{
    FUNCTION_CALL_TRACE;
    
    return true;
}

bool
BTConnection::readSRFromFile (const QString filename, QByteArray &record)
{
    FUNCTION_CALL_TRACE;
    
    return true;
}
