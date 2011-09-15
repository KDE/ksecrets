/* This file is part of the KDE project
 *
 * Copyright (C) 2011 Valentin Rusu <kde@rusu.info>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KSECRETDEVICE_H
#define KSECRETDEVICE_H


/**
 * @def CURRENT_FILE_VERSION This macro controls serialization operations.
 * Care should be taken to increment it's value with each file format change.
 * The modified code should be prepared to handle older version file formats.
 */
#define CURRENT_FILE_VERSION        1

#define KSECRET_MAGIC "KSECRET\n\r\r\n"
#define KSECRET_MAGIC_LEN 11
#include <QIODevice>
#include <kdebug.h>
#include <algorithm>

#include "ksecretencryptionfilter.h"

template <class B>
class KSecretDevice : public B
{
    /**
    * Increment this number with each significant change in file format
    * Care should be taken to let the code recognize old file formats
    * @see CURRENT_FILE_VERSION
    */
    static const quint32 FILE_FORMAT_VERSION = 1; 
    
public:
    KSecretDevice( const QString &path, KSecretEncryptionFilter *encryptionFilter ) :
        B( path ),
        m_encryptionFilter( encryptionFilter ),
        m_valid( false ),
        m_insideHeader( false )
    {
    }
    virtual ~KSecretDevice() {}

    virtual bool open( QIODevice::OpenMode flags ) {
        // NOTE: this device is not designed to work in buffered mode so we'll add here the unbuffered mode
        flags |= QIODevice::Unbuffered; 
        
        bool result = B::open( flags );
        if ( result ) {
            m_valid = true;
            m_insideHeader = true;
            if ( flags & QIODevice::WriteOnly ) {
                result = writeMagic();
            }
            else {
                result = readMagic();
            }
            m_insideHeader = false;
        }
        return result;
    }
    bool isAtDataStart() const;
    bool seekDataStart();
    
private:
    /**
     * Read the ksecret file's magic value.
     *
     * @return true if the magic indicates this file is a ksecret file,
     *         false else
     */
    bool readMagic();

    /**
     * Write the ksecret file's magic value.
     *
     * @return true if the magic was written successful, false else
     */
    bool writeMagic();
    
    virtual qint64  readData ( char * data, qint64 maxSize ) {
        qint64 result = -1;
        static bool readingChunk = false;
        if ( !readingChunk && !m_insideHeader ) {
            QByteArray chunk;
            QDataStream stream( this );
            readingChunk = true;
            stream >> chunk; // do recursion
            readingChunk = false;
            QByteArray decrypted = m_encryptionFilter->decryptData( chunk );
            result = std::min( (qint64)decrypted.length(), maxSize );
            memcpy( data, decrypted.constData(), decrypted.length() );
        }
        else {
            result = B::readData( data, maxSize );
        }
        return result;
    }

    // TODO: implement a buffering technique to allow for larger chunks to be encrypted, as to reduce redundancy
    virtual qint64  writeData ( const char * data, qint64 maxSize ) {
        qint64 result = -1;
        static bool writingChunk = false;
        if ( !writingChunk && !m_insideHeader ) {
            QByteArray chunk = m_encryptionFilter->encryptData( data, maxSize );
            QDataStream stream( this );
            writingChunk = true;
            stream << chunk; // do recursion
            writingChunk = false;
            result = maxSize;
        }
        else {
            result = B::writeData( data, maxSize );
        }
        
        return result; // take care not to return chunk.lenght here 
    }
    
private:
    KSecretEncryptionFilter *
                    m_encryptionFilter;
    qint64          m_dataPos;
    QByteArray      m_readBuffer;
    bool            m_valid;
    bool            m_insideHeader;
};

// template <class B>
// quint32 KSecretDevice<B>::FILE_FORMAT_VERSION = 1;

template <class B>
bool KSecretDevice<B>::readMagic()
{
    if ( !m_valid ) {
        return false;
    }
    
    m_readBuffer.resize(KSECRET_MAGIC_LEN);
    if(B::read(m_readBuffer.data(), KSECRET_MAGIC_LEN) != KSECRET_MAGIC_LEN) {
        m_valid = false;
        return false;
    }
    m_dataPos = B::pos();
    QString strMagic( m_readBuffer );
    return strMagic == KSECRET_MAGIC;
}

template <class B>
bool KSecretDevice<B>::writeMagic()
{
    if(!m_valid) {
        return false;
    }

    if( B::write(KSECRET_MAGIC, KSECRET_MAGIC_LEN) != KSECRET_MAGIC_LEN) {
        m_valid = false;
    }
    m_dataPos = B::pos(); 
    return m_valid;
}

#endif // KSECRETDEVICE_H

