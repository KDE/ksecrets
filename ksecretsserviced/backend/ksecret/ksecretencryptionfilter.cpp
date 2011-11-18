/* This file is part of the KDE project
 *
 * Copyright 2010, Michael Leupold <lemma@confuego.org>
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

#include "ksecretencryptionfilter.h"

#include <klocalizedstring.h>
#include <kdebug.h>
#include "ksecretstream.h"

// this must not be changed or else file compatibility is gone!
#define VERIFIER_LENGTH 64


// create a symmetric encryption key from a user-supplied password using
// PBKDF2.
// Returns a zero-length key if creating the key failed.
static QCA::SymmetricKey createKeyFromPassword(const QCA::SecureArray &password,
        int keyLength)
{
//     if(!QCA::isSupported("sha1")) {
//         return QCA::SymmetricKey();
//     }

    QCA::PBKDF2 deriv("sha1");
    return deriv.makeKey(password, QCA::InitializationVector(), keyLength, 10000);
}


KSecretEncryptionFilter::KSecretEncryptionFilter( const QCA::SecureArray &password ) :
        m_hash(0), m_mac(0), m_cipher(0), m_file(0)
{
    m_algoHash = SHA256;
    m_algoCipher = AES256;
    setupAlgorithms();
    m_initVector.resize( m_cipher->keyLength().minimum() );
    setPassword( password );
}

KSecretEncryptionFilter::~KSecretEncryptionFilter()
{
    delete m_hash;
    delete m_cipher;
    delete m_mac;
}

bool KSecretEncryptionFilter::setupAlgorithms()
{
    // figure out algorithms to use
    switch(m_algoHash) {

    case KSecretEncryptionFilter::SHA256:
        if(!QCA::isSupported("sha256") || !QCA::isSupported("hmac(sha256)")) {
            m_errorMessage = i18nc("Error message: unsupported hashing algorithm SHA256 used",
                                 "The hashing algorithm SHA256 is not supported by your installation.");
            return false;
        }
        m_hash = new QCA::Hash("sha256");
        m_mac = new QCA::MessageAuthenticationCode("hmac(sha256)", QCA::SymmetricKey());
        break;

    default:
        m_errorMessage = i18nc("Error message: unknown hashing algorithm used",
                             "The file uses an unknown hashing algorithm.");
        return false;
    }

    switch(m_algoCipher) {

    case KSecretEncryptionFilter::AES256:
        if(!QCA::isSupported("aes256-cbc-pkcs7")) {
            m_errorMessage = i18nc("Error message: unsupported encryption algorithm AES256 used",
                                 "The encryption algorithm AES256 is not supported by your installation.");
            return false;
        }
        m_cipher = new QCA::Cipher("aes256", QCA::Cipher::CBC, QCA::Cipher::PKCS7);
        break;

    default:
        m_errorMessage = i18nc("Error message: unknown encryption algorithm used",
                             "The file uses an unknown encryption algorithm.");
        return false;
    }

    return true;
}


bool KSecretEncryptionFilter::setPassword(const QCA::SecureArray& password)
{
    bool result = true;
    int keyLength = m_cipher->keyLength().minimum();
    m_cryptKey = createKeyFromPassword(password, keyLength);
    if ( m_cryptKey.isEmpty() ) {
//         kDebug() << "Cannot create key";
        result = false;
    }
    return result;
}

bool KSecretEncryptionFilter::attachFile(QIODevice* file)
{
    m_file = file;
    if ( m_file->isWritable() ) {
        return setupForWriting();
    }
    else {
        return setupForReading();
    }
}

bool KSecretEncryptionFilter::setupForWriting()
{
    KSecretStream stream( m_file );
    stream.setVersion( QDataStream::Qt_4_7 );
    (QDataStream&)stream  
            << m_algoHash 
            << m_algoCipher;
            
    stream << m_initVector;
            
    Q_ASSERT( !m_cryptKey.isEmpty() );
    return stream.status() == QDataStream::Ok;
}

bool KSecretEncryptionFilter::setupForReading() {
    KSecretStream stream( m_file );
    (QDataStream&)stream  
            >> m_algoHash 
            >> m_algoCipher;
            
    stream >> m_initVector;
    
    if ( stream.status() != QDataStream::Ok ) {
        kDebug() << "Cannot read algo setup";
        return false;
    }
            
    return setupAlgorithms();
}

QCA::SecureArray KSecretEncryptionFilter::encryptData( const QByteArray & data)
{
    Q_ASSERT( !m_cryptKey.isEmpty() );
    m_cipher->setup( QCA::Encode, m_cryptKey, m_initVector );
    QCA::SecureArray result = m_cipher->update( QCA::SecureArray( data ) );
    if ( !m_cipher->ok() ) {
        kDebug() << "Cannot encrypt data!";
    }
    result.append( m_cipher->final() );
    if ( !m_cipher->ok() ) {
        kDebug() << "Cannot encrypt: final failed!";
    }
//     kDebug() << "Encryted size " << result.size();
    return result;
}

QByteArray KSecretEncryptionFilter::decryptData(QByteArray encrypted)
{
    m_cipher->setup( QCA::Decode, m_cryptKey, m_initVector );
    QCA::MemoryRegion result = m_cipher->process( encrypted );
    if ( !m_cipher->ok() ) {
        kDebug() << "Cannot decrypt data!";
    }
    return result.toByteArray();
}

QSet<QByteArray> KSecretEncryptionFilter::createHashes(const QMap<QString, QString> &attributes)
{
    Q_ASSERT(m_hash);

    QSet<QByteArray> hashSet;
    QMap<QString, QString>::const_iterator it = attributes.constBegin();
    QMap<QString, QString>::const_iterator end = attributes.constEnd();
    for(; it != end; ++it) {
        m_hash->clear();
        m_hash->update(it.key().toUtf8());
        m_hash->update(it.value().toUtf8());
        hashSet.insert(m_hash->final().toByteArray());
    }

    return hashSet;
}
