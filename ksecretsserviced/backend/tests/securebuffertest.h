/*
 * Copyright (C) 2010  Michael Leupold <lemma@confuego.org>
 *
 * Large portions of the design and implementation have been copied
 * from the QBuffer unit-test (tst_qbuffer.cpp):
 *   Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *   All rights reserved.
 *   Contact: Nokia Corporation (qt-info@nokia.com)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */

#ifndef SECUREBUFFERTEST_H
#define SECUREBUFFERTEST_H

#include <QtCore/QObject>
#include <QtCore/QMetaType>
#include <QtCrypto/QtCrypto>

Q_DECLARE_METATYPE(QCA::SecureArray)

class SecureBufferTest : public QObject
{
    Q_OBJECT

public:
    SecureBufferTest();

private slots:
    void getSetCheck();
    void readBlock();
    void readBlockPastEnd();
    void writeBlock_data();
    void writeBlock();
    void seek();
    void seekTest_data();
    void seekTest();
    void read_rawdata();
    void isSequential();
    void signalTest_data();
    void signalTest();
    void isClosedAfterClose();
    void readLine_data();
    void readLine();
    void canReadLine_data();
    void canReadLine();
    void atEnd();
    void readLineBoundaries();
    void writeAfterSecureArrayResize();

protected slots:
    void readyReadSlot();
    void bytesWrittenSlot(qint64 written);

private:
    qint64 totalBytesWritten;
    bool gotReadyRead;

    QCA::Initializer init;
};

#endif
