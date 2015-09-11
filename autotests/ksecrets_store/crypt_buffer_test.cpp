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

#include "crypt_buffer_test.h"

#include <crypt_buffer.h>
#include <crypting_engine.h>
#include <QtTest/QtTest>
#define GCRPYT_NO_DEPRECATED
#include <gcrypt.h>
#include <cassert>
#include <iomanip>

QTEST_GUILESS_MAIN(CryptBufferTest)

CryptBufferTest::CryptBufferTest() {}
CryptBufferTest::~CryptBufferTest() {}

void CryptBufferTest::initTestCase()
{
    unsigned char salt[CryptingEngine::SALT_SIZE];
    CryptingEngine::create_nonce(salt, CryptingEngine::SALT_SIZE);

    unsigned char iv[CryptingEngine::IV_SIZE];
    CryptingEngine::create_nonce(iv, CryptingEngine::IV_SIZE);


    CryptingEngine &crengine = CryptingEngine::instance();
    crengine.setKeyNameEncrypting("ksecrets-test:encrypting");
    crengine.setKeyNameMac("ksecrets-test:mac");
    crengine.setIV(iv, CryptingEngine::IV_SIZE);
    crengine.setCredentials("test", salt);
}

void CryptBufferTest::cleanupTestCase()
{
    // Nothing to do here for the moment
}

class TestDevice : public KSecretsDevice {
public:
    TestDevice()
    {
        iv_ = new unsigned char[CryptingEngine::IV_SIZE];
        gcry_create_nonce(iv_, CryptingEngine::IV_SIZE);
        len_ = 1024;
        buffer_ = (char*)std::malloc(len_);
        gptr_ = buffer_;
        pptr_ = buffer_;
    }
    ~TestDevice()
    {
        if (buffer_)
            std::free(buffer_);
    }
    virtual unsigned const char* iv() const noexcept { return iv_; }
    virtual bool read(void* buf, size_t count) noexcept
    {
        assert(count < (len_ - (gptr_ - buffer_)));
        memcpy(buf, gptr_, count);
        gptr_ += count;
        return true;
    }
    virtual bool write(const void* buf, size_t count) noexcept
    {
        auto oldppos = pptr_ - buffer_;
        if ((len_ - oldppos) < count) {
            auto oldgpos = gptr_ - buffer_;
            len_ += 1024;
            buffer_ = (char*)std::realloc(buffer_, len_);
            assert(buffer_ != nullptr);
            pptr_ = buffer_ + oldppos;
            gptr_ = buffer_ + oldgpos;
        }
        memcpy(pptr_, buf, count);
        pptr_ += count;
        return true;
    }

private:
    unsigned char* iv_;
    size_t len_;
    char* buffer_;
    char* gptr_;
    char* pptr_;
};

void CryptBufferTest::testEncryptDecryptStream()
{
    TestDevice theDevice;

    // put in some data
    std::string testString = "test data";
    {
        CryptBuffer theBuffer;
        std::ostream os(&theBuffer);
        os << testString;
        theBuffer.write(theDevice);
    }

    // then check if it gets back
    {
        CryptBuffer theBuffer;
        theBuffer.read(theDevice);
        std::istream is(&theBuffer);

        std::string::size_type len;
        is >> len;
        char c;
        is >> c;
        QVERIFY(c == ':');

        std::string readString;
        is >> readString;

        QVERIFY(readString.compare(readString) == 0);
    }
}

// vim: tw=220:ts=4
