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
    /**
     * While writing, this device accumulates cunks of data before sending
     * them to the encryption filter. This constant adjusts the minimum size
     * of these chunks
     */
    static const int BUFFER_CHUNK_SIZE = 128;
    
public:
    KSecretDevice( const QString &path, KSecretEncryptionFilter *encryptionFilter ) :
        B( path ),
        m_encryptionFilter( encryptionFilter ),
        m_valid( false ),
        m_bufferPos( 0 ),
        m_closing( false ),
        m_encrypting( false )
    {
    }
    virtual ~KSecretDevice() {}

    
    virtual bool open( QIODevice::OpenMode flags ) {
        // NOTE: this device is not designed to work in buffered mode so we'll add here the unbuffered mode
        flags |= QIODevice::Unbuffered;

        m_buffer.clear();
        m_bufferPos = 0;
        m_closing = false;

        bool result = B::open( flags );
        if ( result ) {
            m_valid = true;
            if ( flags & QIODevice::WriteOnly ) {
                result = writeMagic();
            }
            else {
                result = readMagic();
            }
            
            if ( result ) {
                result = m_encryptionFilter->attachFile( this );
            }
        }
        return result;
    }
    
    bool isAtDataStart() const;
    bool seekDataStart();
    
    void startEncrypting() {
        m_encrypting = true;
    }
    
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
        if ( !readingChunk && m_encrypting ) {
            if ( (m_buffer.size() - m_bufferPos) < maxSize ) {
                QDataStream stream( this );
                readingChunk = true;
                QByteArray chunk;
                stream >> chunk; // do recursion
                readingChunk = false;
                
                m_buffer = m_encryptionFilter->decryptData( chunk );
                m_bufferPos = 0;
            }
            
            result = std::min( (qint64)( m_buffer.size() - m_bufferPos ), maxSize );
            memcpy( data, m_buffer.constData() + m_bufferPos, maxSize );
            m_bufferPos += maxSize;
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
        if ( !writingChunk && m_encrypting && !m_closing ) {
            m_buffer.append( data, maxSize );
            if ( m_buffer.size() >= BUFFER_CHUNK_SIZE ) {
                QCA::SecureArray arr = m_encryptionFilter->encryptData( m_buffer );
                QByteArray chunk = arr.toByteArray();
//                 kDebug() << "buffer size " << m_buffer.size() << " encrypted chunk size " << arr.size();
                m_buffer.clear();
                QDataStream stream( this );
                writingChunk = true;
                stream << chunk; // do recursion
                writingChunk = false;
            }
            result = maxSize;
        }
        else {
            result = B::writeData( data, maxSize );
        }
        
        return result; // take care not to return chunk.length here 
    }

public:
    virtual void close() {
        Q_ASSERT( !m_closing );
        // write remaining data before closing
        if ( !m_buffer.isEmpty() ) {
            if ( m_buffer.size() < 16 ) {
                // ensure QCA has the minimal amount of data to do the encryption
                m_buffer.append("randomDataToMakeQCAHappy");
            }
            QByteArray chunk = m_encryptionFilter->encryptData( m_buffer ).toByteArray();
            m_buffer.clear();
            QDataStream stream( this );
            m_closing = true;
            stream << chunk; // do recursion
        }
        
        B::close();
    }
    
    
private:
    KSecretEncryptionFilter *
                    m_encryptionFilter;
    qint64          m_dataPos;
    QByteArray      m_readBuffer;
    bool            m_valid;
    QByteArray      m_buffer;
    int             m_bufferPos;
    bool            m_closing;
    bool            m_encrypting;
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

