/*
 * Copyright (C) 2010  Michael Leupold <lemma@confuego.org>
 *
 * Large portions of the design and implementation have been copied
 * from QBuffer (qbuffer.cpp):
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

#include "securebuffer.h"
#include "securebuffer_p.h"

SecureBuffer::SecureBuffer(QObject *parent)
    : QIODevice(parent), d(new SecureBufferPrivate(this))
{
    d->buf = &d->defaultBuf;
}

SecureBuffer::SecureBuffer(QCA::SecureArray *buf, QObject *parent)
    : QIODevice(parent), d(new SecureBufferPrivate(this))
{
    d->buf = buf ? buf : &d->defaultBuf;
}

SecureBuffer::~SecureBuffer()
{
}

QCA::SecureArray &SecureBuffer::buffer()
{
    return *d->buf;
}

void SecureBuffer::setBuffer(QCA::SecureArray *secureArray) const
{
    if(isOpen()) {
        qWarning("SecureBuffer::setBuffer: Buffer is open");
        return;
    }
    if(secureArray) {
        d->buf = secureArray;
    } else {
        d->buf = &d->defaultBuf;
    }
    d->defaultBuf.clear();
    d->ioIndex = 0;
}

const QCA::SecureArray &SecureBuffer::data() const
{
    return *d->buf;
}

void SecureBuffer::setData(const QCA::SecureArray &data)
{
    if(isOpen()) {
        qWarning("SecureBuffer::setData: Buffer is open");
        return;
    }
    *d->buf = data;
    d->ioIndex = 0;
}

void SecureBuffer::setData(const QByteArray &data)
{
    if(isOpen()) {
        qWarning("SecureBuffer::setData: Buffer is open");
        return;
    }
    *d->buf = data;
    d->ioIndex = 0;
}

void SecureBuffer::setData(const char *data, int size)
{
    setData(QByteArray(data, size));
}

bool SecureBuffer::open(QIODevice::OpenMode mode)
{
    // always operate in Unbuffered mode for security reasons.
    mode |= QIODevice::Unbuffered;

    if((mode & QIODevice::Append) == Append) {
        mode |= QIODevice::WriteOnly;
    }
    setOpenMode(mode);
    if(!(isReadable() || isWritable())) {
        qWarning("SecureBuffer::open: File access not specified");
        return false;
    }

    if((mode & QIODevice::Truncate) == QIODevice::Truncate) {
        d->buf->clear();
    }
    if((mode & QIODevice::Append) == QIODevice::Append) {
        seek(d->buf->size());
    } else {
        seek(0);
    }

    return true;
}

void SecureBuffer::close()
{
    QIODevice::close();
}

qint64 SecureBuffer::size() const
{
    return qint64(d->buf->size());
}

qint64 SecureBuffer::pos() const
{
    return QIODevice::pos();
}

bool SecureBuffer::seek(qint64 pos)
{
    if(pos > d->buf->size() && isWritable()) {
        if(seek(d->buf->size())) {
            const qint64 gapSize = pos - d->buf->size();
            if(write(QByteArray(gapSize, 0)) != gapSize) {
                qWarning("SecureBuffer::seek: Unable to fill gap");
                return false;
            }
        } else {
            return false;
        }
    } else if(pos > d->buf->size() || pos < 0) {
        qWarning("SecureBuffer::seek: Invalid pos: %d", int(pos));
        return false;
    }
    d->ioIndex = int(pos);
    return QIODevice::seek(pos);
}

bool SecureBuffer::atEnd() const
{
    return QIODevice::atEnd();
}

bool SecureBuffer::canReadLine() const
{
    if(!isOpen()) {
        return false;
    }

    return ::memchr(d->buf->constData() + int(pos()), '\n', int(size())) != 0 || QIODevice::canReadLine();
}

void SecureBuffer::connectNotify(const char *signal)
{
    if(strcmp(signal + 1, "readyRead()") == 0 ||
            strcmp(signal + 1, "bytesWritten(qint64)") == 0) {
        d->signalConnectionCount++;
    }
}

void SecureBuffer::disconnectNotify(const char *signal)
{
    if(!signal || strcmp(signal + 1, "readyRead()") == 0 ||
            strcmp(signal + 1, "bytesWritten(qint64)") == 0) {
        d->signalConnectionCount--;
    }
}

qint64 SecureBuffer::readData(char *data, qint64 maxlen)
{
    if((maxlen = qMin(maxlen, qint64(d->buf->size()) - d->ioIndex)) <= 0) {
        return qint64(0);
    }
    memcpy(data, d->buf->constData() + d->ioIndex, maxlen);
    d->ioIndex += int(maxlen);
    return maxlen;
}

qint64 SecureBuffer::writeData(const char *data, qint64 len)
{
    int extraBytes = d->ioIndex + len - d->buf->size();
    if(extraBytes > 0) {  // exceeds current buffer size
        int newSize = d->buf->size() + extraBytes;
        d->buf->resize(newSize);
        if(d->buf->size() != newSize) {  // error resizing
            qWarning("SecureBuffer::writeData: Memory allocation error");
            return -1;
        }
    }

    memcpy(d->buf->data() + d->ioIndex, (uchar *)data, int(len));
    d->ioIndex += int(len);

    d->writtenSinceLastEmit += len;
    if(d->signalConnectionCount && !d->signalsEmitted && !signalsBlocked()) {
        d->signalsEmitted = true;
        QMetaObject::invokeMethod(d, "_q_emitSignals", Qt::QueuedConnection);
    }

    return len;
}

#include "securebuffer.moc"
#include "securebuffer_p.moc"
