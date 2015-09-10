/*
    This file is part of the KDE Libraries

    Copyright (C) 2015 Valentin Rusu (valir@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB. If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef KSECRETS_CRYPT_H
#define KSECRETS_CRYPT_H

#include "ksecrets_device.h"

#include <sys/types.h>
#include <streambuf>
#include <iostream>

class KSecretsFile;

/**
 * @brief The CryptBuffer class is responsible for holding a serialization buffer allowing SecretEntity serialization then encryption
 *
 * The serialization should be done in text format. Even if numeric fields will take more space, this will provide several
 * advandages. First, debugging is easier, as we could easily see that the items get correctly decrypted right away. Then,
 * text serialization is not subject to the problems related to CPU endian architecture.
 *
 * The buffer implementation is quite simple and it provides two buffers. The encrypted_ buffer holds the stuff as it should
 * be put on disk. Decrypting occurs only when reading from the buffer. As such, the CPU is preserved and only used when
 * client application actually reads the corresponding SecretEntity. The data is then available in the decrypted_ buffer.
 *
 * After allocation, both encrypted_ and decrypted_ buffers are filled with "garbage", that is random data, provided by the libgrypt
 * library's gcry_create_nonce function. That's intended to make it harder to detect patterns into the encrypted file.
 *
 * The encryption algorythm is GCRY_CIPHER_BLOWFISH with the mode flag GCRY_CIPHER_MODE_CBC. This ensures pretty good privacy
 * without too much strain on the CPU.
 *
 * Several streaming operators are provided, to help string serialization in text mode.
 */
class CryptBuffer : public std::streambuf {
public:
    CryptBuffer();
    CryptBuffer(CryptBuffer&&) = default;
    ~CryptBuffer();


    void empty() noexcept;

    bool read(KSecretsDevice&) noexcept;
    bool write(KSecretsDevice&) noexcept;

private:
    int_type underflow() override;
    int_type overflow(int_type) override;

    bool decrypt() noexcept;
    bool encrypt() noexcept;

private:
    static constexpr size_t cipherBlockLen_ = 8; /// blowfish block len is 8
    size_t len_; /// the length of both encrypted_ and decrypted_ buffers is the same
    char* encrypted_;
    char* decrypted_;
    const char* iv_;
};

// operators for text-mode serialization
std::ostream& operator << (std::ostream& os, const std::string& str);
std::istream& operator >> (std::istream& is, std::string& str);

#endif
// vim: tw=220:ts=4
