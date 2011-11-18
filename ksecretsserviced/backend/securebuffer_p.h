/*
 * Copyright (C) 2010  Michael Leupold <lemma@confuego.org>
 *
 * Large portions of the design and implementation have been copied
 * from QBuffer (qbuffer.h):
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

#ifndef SECUREBUFFER_PRIVATE_H
#define SECUREBUFFER_PRIVATE_H

#include <QtCore/QObject>

class SecureBuffer;

class SecureBufferPrivate : public QObject
{
    Q_OBJECT

public:
    SecureBufferPrivate(SecureBuffer *parent)
        : q(parent), buf(0), ioIndex(0), writtenSinceLastEmit(0), signalConnectionCount(0),
          signalsEmitted(false)
    {}
    ~SecureBufferPrivate() {}

    SecureBuffer *q;

    QCA::SecureArray *buf;
    QCA::SecureArray defaultBuf;
    int ioIndex;

    qint64 writtenSinceLastEmit;
    int signalConnectionCount;
    bool signalsEmitted;

public Q_SLOTS:
    void _q_emitSignals() {
        emit q->bytesWritten(writtenSinceLastEmit);
        writtenSinceLastEmit = 0;
        emit q->readyRead();
        signalsEmitted = false;
    }
};

#endif
