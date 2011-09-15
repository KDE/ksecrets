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
    if(!QCA::isSupported("sha1")) {
        return QCA::SymmetricKey();
    }

    QCA::PBKDF2 deriv("sha1");
    return deriv.makeKey(password, QCA::InitializationVector(), keyLength, 10000);
}


KSecretEncryptionFilter::KSecretEncryptionFilter( const QCA::SecureArray &password ) :
        m_hash(0), m_mac(0), m_cipher(0), m_symmetricKey(0), m_file(0)
{
    m_algoHash = SHA256;
    m_algoCipher = AES256;
    setupAlgorithms();
    setPassword( password );
}

KSecretEncryptionFilter::~KSecretEncryptionFilter()
{
    delete m_hash;
    delete m_cipher;
    delete m_mac;
    delete m_symmetricKey;
    qDeleteAll(m_encryptedSymKeys);
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

    m_symmetricKey = new QCA::SymmetricKey(m_cipher->keyLength().minimum());
    
    return true;
}


bool KSecretEncryptionFilter::setPassword(const QCA::SecureArray& password)
{
    bool result = true;
    // create a new symmetric key
    // TODO: is minimum() right in all cases?
    m_symmetricKey = new QCA::SymmetricKey(m_cipher->keyLength().minimum());
    int keyLength = m_cipher->keyLength().minimum();
    m_keyUnlockKey = createKeyFromPassword(password, keyLength);
    if ( m_keyUnlockKey.isEmpty() ) {
        kDebug() << "Cannot create unlock key";
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
//    Q_ASSERT( m_file && m_file->isAtDataStart() );
    
    qDeleteAll( m_encryptedSymKeys );
    
    QCA::InitializationVector iv = QCA::InitializationVector(m_cipher->keyLength().minimum());
    EncryptedKey *key = new EncryptedKey;
    key->m_type = KeyPassword;
    key->m_iv = iv.toByteArray();
    m_cipher->setup(QCA::Encode, m_keyUnlockKey, iv);
    key->m_key.append(m_cipher->update(*m_symmetricKey).toByteArray());
    key->m_key.append(m_cipher->final().toByteArray());
    m_encryptedSymKeys.append(key);

    // build the verifier
    m_verInitVector = QCA::InitializationVector(m_cipher->blockSize());
    // get random data and compute its hash
    QCA::SecureArray randomData = QCA::Random::randomArray(VERIFIER_LENGTH);
    m_hash->clear();
    m_hash->update(randomData);
    randomData.append(m_hash->final());
    // now encrypt randomData
    m_cipher->setup(QCA::Encode, *m_symmetricKey, m_verInitVector);
    m_verEncryptedRandom = m_cipher->update(randomData);
    m_verEncryptedRandom.append(m_cipher->final());
    
    KSecretStream stream( m_file );
    stream.setVersion( QDataStream::Qt_4_7 );
    stream  << m_algoHash 
            << m_algoCipher 
            << m_verInitVector 
            << m_verEncryptedRandom;
    return stream.status() == QDataStream::Ok;
}

bool KSecretEncryptionFilter::setupForReading() {
//    Q_ASSERT( m_file && m_file->isAtDataStart() );
    // before actually trying to unlock, check if the key matches by decrypting
    // the verifier.
    if ( m_symmetricKey == 0) {
        return false;
    }

    KSecretStream stream( m_file );
    (QDataStream&)stream  >> m_algoHash 
            >> m_algoCipher;
    if ( stream.status() != QDataStream::Ok ) {
        kDebug() << "Cannot read algo setup";
        return false;
    }
            
    setupAlgorithms();
    
    stream  >> m_verInitVector
            >> m_verEncryptedRandom;
    if ( stream.status() != QDataStream::Ok ) {
        kDebug() << "Cannot read init vectors";
        return false;
    }

    m_cipher->setup(QCA::Decode, *m_symmetricKey, m_verInitVector);
    QCA::SecureArray verifier = m_cipher->update(m_verEncryptedRandom);
    verifier.append(m_cipher->final());
    QCA::SecureArray randomData(VERIFIER_LENGTH);
    QCA::SecureArray verifierHash(verifier.size() - VERIFIER_LENGTH);
    int i = 0;
    for(; i < VERIFIER_LENGTH; ++i) {
        randomData[i] = verifier[i];
    }
    for(; i < verifier.size(); ++i) {
        verifierHash[i - VERIFIER_LENGTH] = verifier[i];
    }
    // rebuild the hash and compare to the decrypted one
    m_hash->clear();
    m_hash->update(randomData);
    if(verifierHash != m_hash->final()) {
        // hashes don't match, the master key is wrong.
        kDebug() << "Hashes do not match!";
        return false;
    }
    
    /**
     * Code from tryUnlockPassword
     */
    // try decrypting any of the symmetric keys using the m_keyUnlockKey
    Q_FOREACH(EncryptedKey * key, m_encryptedSymKeys) {
        if(key->m_type == KeyPassword) {
            m_cipher->setup(QCA::Decode, m_keyUnlockKey, key->m_iv);
            QCA::SecureArray unlockedKey = m_cipher->update(key->m_key);
            unlockedKey.append(m_cipher->final());
            m_symmetricKey = new QCA::SymmetricKey(unlockedKey);
        }
    }
    return true;
}

QByteArray KSecretEncryptionFilter::encryptData(const char* data, qint64 size)
{
    // FIXME: implement this (this is a NOOP for the moment)
    return QByteArray( data, size );
}

QByteArray KSecretEncryptionFilter::decryptData(QByteArray encrypted)
{
    // FIXME: implement this (this is a NOOP for the moment)
    return encrypted;
}
