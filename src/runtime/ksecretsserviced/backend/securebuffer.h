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

#ifndef SECUREBUFFER_H
#define SECUREBUFFER_H

#include <QtCore/QIODevice>
#include <QtCrypto>

class SecureBufferPrivate;

/**
 * SecureBuffer allows to access a QCA::SecureArray using the QIODevice
 * interface.
 */
class SecureBuffer : public QIODevice
{
    Q_OBJECT

public:
    /**
       Construct a new secure buffer with the given \a parent
       allocating a new SecureArray as its internal storage.

       \param parent the parent object of this SecureBuffer
     */
    explicit SecureBuffer(QObject *parent = 0);

    /**
        Construct a new secure buffer using \a buf as its internal
        storage.

        \param buf SecureArray to use as internal storage
        \param parent the parent object of this SecureBuffer
      */
    explicit SecureBuffer(QCA::SecureArray *buf, QObject *parent = 0);

    /**
        Destroys the secure buffer.
     */
    ~SecureBuffer();

    /**
        Returns a reference to the SecureBuffer's internal buffer.

        \return a reference to the internal buffer
        \sa setBuffer(), data()
     */
    QCA::SecureArray &buffer();

    /**
        Make the SecureBuffer use the SecureArray pointed to by \a
        secureArray as its internal storage. The SecureBuffer does
        not take ownership of the SecureArray. The caller is responsible
        that the SecureArray remains valid as long as the SecureBuffer
        is not destroyed.

        Does nothing if isOpen() is true.

        If you write something into the buffer, \a secureArray will
        be modified (size as well as contents).

        \param secureArray the SecureArray to use as this buffer's
        internal storage
     */
    void setBuffer(QCA::SecureArray *secureArray) const;

    /**
        Returns the data contained in the buffer.

        \return the data contained the buffer
        \sa buffer(), setBuffer(), setData()
     */
    const QCA::SecureArray &data() const;

    /**
        Copy the contents of \a data to the internal buffer replacing all
        previous contents. This does nothing if isOpen() is true.

        \param data data to copy to the internal buffer
     */
    void setData(const QCA::SecureArray &data);

    /**
        Copy the contents of \a data to the internal buffer replacing all
        previous contents. This does nothing if isOpen() is true.

        \param data data to copy to the internal buffer
     */
    void setData(const QByteArray &data);

    /**
        Copy the first \a size bytes of the contents of \a data to the
        internal buffer replacing all previous contents. This does nothing if
        isOpen() is true.

        \param data array to copy the data from
        \param size number of bytes to copy from the array
     */
    void setData(const char *data, int size);

    /**
        Open the SecureBuffer and set its \a OpenMode to mode.

        \param openMode the mode to open the SecureBuffer with
        \return true if opening succeeded, false else
     */
    bool open(QIODevice::OpenMode mode);

    /**
        Closes the SecureBuffer.
     */
    void close();

    /**
        Returns the size of the underlying SecureArray.

        \return the size of the internal buffer
     */
    qint64 size() const;

    /**
        Return the position the data is written to or read from.

        \return the current position inside the internal buffer
     */
    qint64 pos() const;

    /**
        Set the current read/write position to \a pos.

        \param pos the position to set
     */
    bool seek(qint64 pos);

    /**
        Check if the current position is at the end of the buffer.

        \return true if the current position is at the end of the
        internal buffer, false if it isn't
     */
    bool atEnd() const;

    /**
        Check if a complete line can be read from the buffer.

        \return true if a complete line can be read from the buffer,
        false if not
     */
    bool canReadLine() const;

protected:
    /**
        Called when a connection is made to one of the buffer's signals.
      */
    void connectNotify(const char*);

    /**
        Called when a slot is disconnected from one of the buffer's signals.
      */
    void disconnectNotify(const char*);

    /**
        Read data from the buffer.

        \param data array to store the data read into
        \param maxlen maximum number of bytes to read
        \return actual number of bytes read or -1 in case of an error
     */
    qint64 readData(char *data, qint64 maxlen);

    /**
        Write data into the buffer.

        \param data array to read the data to be written from
        \param len number of bytes to write
        \return actual number of bytes written or -1 in case of an error
     */
    qint64 writeData(const char *data, qint64 len);

private:
    friend class SecureBufferPrivate;
    SecureBufferPrivate *d;

    Q_DISABLE_COPY(SecureBuffer)
};

#endif
