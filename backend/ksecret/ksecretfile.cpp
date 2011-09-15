/*
 * Copyright 2010, Michael Leupold <lemma@confuego.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ksecretfile.h"

#include <QtCore/QtEndian>
#include <kdebug.h>
#include <klocalizedstring.h>


// FIXME: currently the daemon could be tricked into reading a large portion of a forged
//        ksecret file wasting a lot of memory and making the pc unusable.

KSecretFile::KSecretFile(QIODevice *device, OpenMode mode) : 
    m_device(device), 
    m_mode(mode)
{
    m_valid = m_device->open((mode == Read) ? QIODevice::ReadOnly : QIODevice::WriteOnly);

    if ( m_valid ) {
        if ( mode == Write ) {
            m_valid &= writeHeader();
        }
        else {
            m_valid &= readHeader();
        }
    }
}

KSecretFile::~KSecretFile()
{
    m_device->close();
    // TODO: delete items?
    // TODO: make sure there's nothing else to delete
}

KSecretFile::OpenMode KSecretFile::mode() const
{
    return m_mode;
}

bool KSecretFile::writeHeader()
{
    // write the ksecret file header
    return 
        writeMagic() && 
        writeUint( FILE_FORMAT_VERSION );
}


bool KSecretFile::readHeader()
{
    quint32 fileVersion;
    bool result = 
        readMagic() &&
        readUint( &fileVersion );
    
    result &= ( fileVersion <= CURRENT_FILE_VERSION );
    return result;
}

void KSecretFile::close()
{
    m_device->close();
}

bool KSecretFile::isValid() const
{
    return m_valid;
}

qint64 KSecretFile::pos() const
{
    return m_device->pos();
}

bool KSecretFile::seek(qint64 pos)
{
    return m_device->seek(pos);
}


bool KSecretFile::isAtDataStart() const
{
    return m_dataPos == m_device->pos();
}

bool KSecretFile::seekDataStart()
{
    return m_device->seek( m_dataPos );
}

bool KSecretFile::readUint(quint32 *value)
{
    Q_ASSERT(value);
    Q_ASSERT(m_mode == Read);

    if(!m_valid) {
        return false;
    }

    m_readBuffer.resize(4);
    if(m_device->read(m_readBuffer.data(), 4) != 4) {
        m_valid = false;
        return false;
    }
    *value = qFromBigEndian<quint32>((const uchar*)m_readBuffer.constData());
    return true;
}

bool KSecretFile::writeUint(quint32 value)
{
    Q_ASSERT(m_mode == Write);

    if(!m_valid) {
        return false;
    }

    uchar outputBuffer[4];
    qToBigEndian<quint32>(value, outputBuffer);
    if(m_device->write((const char*)outputBuffer, 4) != 4) {
        m_valid = false;
    }
    return m_valid;
}

bool KSecretFile::readDatetime(QDateTime *value)
{
    Q_ASSERT(value);
    Q_ASSERT(m_mode == Read);

    if(!m_valid) {
        return false;
    }

    m_readBuffer.resize(8);
    if(m_device->read(m_readBuffer.data(), 8) != 8) {
        m_valid = false;
        return false;
    }
    value->setTime_t(qFromBigEndian<quint64>((const uchar*)m_readBuffer.constData()));
    return true;
}

bool KSecretFile::writeDatetime(const QDateTime &value)
{
    Q_ASSERT(m_mode == Write);

    if(!m_valid) {
        return false;
    }

    uchar outputBuffer[8];
    qToBigEndian<quint64>(value.toTime_t(), outputBuffer);
    if(m_device->write((const char*)outputBuffer, 8) != 8) {
        m_valid = false;
    }
    return m_valid;
}

bool KSecretFile::readBytearray(QByteArray *value)
{
    Q_ASSERT(value);
    Q_ASSERT(m_mode == Read);

    if(!m_valid) {
        return false;
    }

    quint32 length;
    if(!readUint(&length)) {
        m_valid = false;
        return false;
    }

    value->resize(length);
    if(m_device->read(value->data(), length) != length) {
        m_valid = false;
    }
    return m_valid;
}

bool KSecretFile::writeBytearray(const QByteArray &value)
{
    Q_ASSERT(m_mode == Write);

    if(!m_valid) {
        return false;
    }

    if(!writeUint(value.size()) || 
        m_device->write(value) != value.size()) {
        m_valid = false;
    }
    return m_valid;
}

bool KSecretFile::readString(QString *value)
{
    Q_ASSERT(value);
    Q_ASSERT(m_mode == Read);

    if(!m_valid) {
        return false;
    }

    QByteArray bytearray;
    if(!readBytearray(&bytearray)) {
        m_valid = false;
        return false;
    }

    // TODO: detect invalid Utf-8 strings!
    *value = QString::fromUtf8(bytearray);
    return true;
}

bool KSecretFile::writeString(const QString &value)
{
    return writeBytearray(value.toUtf8());
}

bool KSecretFile::readSecret(QCA::SecureArray *value)
{
    Q_ASSERT(value);
    Q_ASSERT(m_mode == Read);

//    kDebug() << "pos = " << pos();
    if(!m_valid) {
        return false;
    }

    quint32 length;
    if(!readUint(&length)) {
        m_valid = false;
        return false;
    }

    value->resize(length);
    if(m_device->read(value->data(), length) != length) {
        m_valid = false;
        return false;
    }
    return true;
}

bool KSecretFile::writeSecret(const QCA::SecureArray &value)
{
    Q_ASSERT(m_mode == Write);

//    kDebug() << "pos = " << pos();
    if(!m_valid) {
        return false;
    }

    if(!writeUint(value.size())) {
        m_valid = false;
    }
    else {
        if (m_device->write(value.constData(), value.size()) != value.size()) {
            m_valid = false;
        }
    }
    return m_valid;
}

