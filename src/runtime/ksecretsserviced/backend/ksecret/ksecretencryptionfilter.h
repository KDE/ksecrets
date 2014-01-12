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


#ifndef KSECRETENCRYPTIONFILTER_H
#define KSECRETENCRYPTIONFILTER_H

#include <qca_tools.h>
#include <qca_basic.h>
#include <QSet>

class QIODevice;

class KSecretEncryptionFilter {
public:
    /**
     * Known hashing and MAC algorithms.
     */
    enum AlgorithmHash {
        SHA256 = 0 /// SHA256
    };

    /**
     * Known encryption algorithms.
     */
    enum AlgorithmEncryption {
        AES256 = 0 /// AES256 using CBC and default padding
    };


    explicit KSecretEncryptionFilter( const QCA::SecureArray &password );
    virtual ~KSecretEncryptionFilter();
    
    bool setPassword( const QCA::SecureArray &password );
    bool attachFile( QIODevice* );
    
    QCA::Hash *hash() const { return m_hash; }

    QCA::SecureArray encryptData( const QByteArray & );
    
    QByteArray decryptData( QByteArray );
    
    /**
     * Create a list of hashes out of some attributes.
     *
     * @param attributes the attributes to create the hashes for
     * @param hash the hash function to use
     * @returns a list of hashes for each of the attributes
     */
    QSet<QByteArray> createHashes(const QMap<QString, QString> &attributes);

private:
    /**
     * Set-up the encryption to be used by the secret collection.
     * This creates hash and cipher functors as configured.
     *
     * @param errorMessage set in case of an error
     * @return true if setting up the encryption worked, false if
     *         there were errors (ie. unsupported encryption methods.
     */
    bool setupAlgorithms();
    bool setupForReading();
    bool setupForWriting();
    
    QCA::Hash           *m_hash;                // hashing algorithm
    quint32             m_algoHash;             // hashing/mac algorithm identifier
    quint32             m_algoCipher;           // encryption algorithm identifier
    QCA::SymmetricKey   m_cryptKey;            // key derived from the password
    QCA::InitializationVector   
                        m_initVector;           // initialization vector of the verifier
    QCA::SecureArray    m_verEncryptedRandom;   // encrypted random data of the verifier
    QCA::MessageAuthenticationCode 
                        *m_mac;                 // message authentication code algorithm
    QCA::Cipher         *m_cipher;              // encryption algorithm
    
    QIODevice           *m_file;
    QString             m_errorMessage;
};

#endif // KSECRETENCRYPTIONFILTER_H
