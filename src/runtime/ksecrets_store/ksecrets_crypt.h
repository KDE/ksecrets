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

#include <sys/types.h>
#include <streambuf>

class KSecretsFile;

class CryptBuffer : public std::streambuf {
public:
    CryptBuffer();
    CryptBuffer(CryptBuffer&&) = default;
    ~CryptBuffer();


    void empty() noexcept;

    bool read(KSecretsFile&);
    bool write(KSecretsFile&);

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
    bool  dirty_;
};

#endif
// vim: tw=220:ts=4
