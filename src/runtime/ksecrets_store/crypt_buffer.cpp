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

#include "defines.h"
#include "crypt_buffer.h"
#include "ksecrets_file.h"
#include "crypting_engine.h"

#include <sys/types.h>
#include <errno.h>
#include <memory>
#include <cassert>


CryptBuffer::CryptBuffer()
    : len_(0)
    , encrypted_(nullptr)
    , decrypted_(nullptr)
{
    setg(nullptr, nullptr, nullptr);
    setp(nullptr, nullptr);
}

CryptBuffer::~CryptBuffer()
{
    delete[] encrypted_, encrypted_ = nullptr;
    delete[] decrypted_, decrypted_ = nullptr;
}

void CryptBuffer::empty() noexcept
{
    delete[] encrypted_, encrypted_ = nullptr;
    delete[] decrypted_, decrypted_ = nullptr;
    len_ = 0;
    setg(nullptr, nullptr, nullptr);
    setp(nullptr, nullptr);
}

bool CryptBuffer::read(KSecretsDevice& file) noexcept
{
    empty();

    if (!file.read(len_))
        return false;

    try {
        encrypted_ = new unsigned char[len_];
    }
    catch (std::bad_alloc) {
        syslog(KSS_LOG_ERR, "ksecrets: got a std::bad_alloc and that means the file is corrupt");
        return false;
    }
    if (encrypted_ == nullptr) {
        len_ = 0;
        return false;
    }

    if (len_ > 0) {
        if (!file.read(encrypted_, len_))
            return false;
    }
    return true;
}

bool CryptBuffer::write(KSecretsDevice& file) noexcept
{
    if (file.write(len_)) {
        syslog(KSS_LOG_DEBUG, "ksecrets: write: |%s|", decrypted_);
        encrypt();
        return file.write(encrypted_, len_);
    }
    else
        return false;
}

bool CryptBuffer::decrypt() noexcept
{
    if (len_ == 0)
        return false;
    assert(encrypted_ != nullptr);
    decrypted_ = new unsigned char[len_];
    auto dres = CryptingEngine::instance().decrypt(decrypted_, len_, encrypted_, len_);
    if (dres) {
        syslog(KSS_LOG_DEBUG, "ksecrets: read decrypted: |%s|", decrypted_);
        setg((char*)decrypted_, (char*)decrypted_, (char*)decrypted_ + len_);
        setp((char*)decrypted_, (char*)decrypted_ + len_);
        return true;
    }
    else {
        empty();
        return false;
    }
}

bool CryptBuffer::encrypt() noexcept
{
    if (len_ == 0)
        return false;
    assert(decrypted_ != nullptr);

    delete[] encrypted_;
    encrypted_ = new unsigned char[len_];
    if (encrypted_ == nullptr) {
        return false;
    }
    CryptingEngine::create_nonce(encrypted_, len_);

    auto eres = CryptingEngine::instance().encrypt(encrypted_, len_, decrypted_, len_);
    if (!eres)
        return false;
    delete[] decrypted_, decrypted_ = nullptr;
    setp(nullptr, nullptr);
    return true;
}

CryptBuffer::int_type CryptBuffer::underflow()
{
    if (gptr() == nullptr) {
        if (!decrypt())
            return traits_type::eof();
    }
    if (gptr() < egptr()) {
        return traits_type::to_int_type(*gptr());
    }
    else {
        return traits_type::eof();
    }
}

CryptBuffer::int_type CryptBuffer::overflow(int_type c)
{
    if (c == traits_type::eof())
        return c;

    if (pptr() == epptr()) {
        size_t oldLen = len_;
        size_t oldgpos = gptr() - eback();

        len_ += cipherBlockLen_;
        if (decrypted_ == nullptr) {
            decrypted_ = (unsigned char*)std::malloc(len_);
        }
        else {
            decrypted_ = (unsigned char*)std::realloc(decrypted_, len_);
            if (decrypted_ == nullptr) {
                syslog(KSS_LOG_ERR, "ksecrets: cannot extend crypt buffer");
                setp(nullptr, nullptr);
                return traits_type::eof();
            }
        }
        // fill the not yet used area with random data
        gcry_create_nonce((unsigned char*)decrypted_ + oldLen, cipherBlockLen_);

        setp((char*)decrypted_ + oldLen, (char*)decrypted_ + len_);
        setg((char*)decrypted_, (char*)decrypted_ + oldgpos, (char*)decrypted_ + len_);

        *pptr() = c;
        pbump(sizeof(char_type));
    }

    return traits_type::to_int_type(c);
}

std::ostream& operator << (std::ostream& os, const std::string& str) {
    os << ' ' << str.length() << ":";
    os.write(str.c_str(), str.length());
    return os;
}

std::istream& operator >> (std::istream& is, std::string& str) {
    std::string::size_type len;
    is >> len;
    char c;
    is >> c;
    if (c != ':') {
        is.fail();
        return is;
    }
    str.resize(len, ' ');
    char *bytes = &*str.begin();
    is.read(bytes, len);
    return is;
}
// vim: tw=220:ts=4
