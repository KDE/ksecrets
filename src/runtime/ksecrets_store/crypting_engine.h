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
#ifndef CRYPTING_ENGINE_H
#define CRYPTING_ENGINE_H

#include <stddef.h>
#define GCRPYT_NO_DEPRECATED
#include <gcrypt.h>
#include <memory>

class CryptingEngine {
    CryptingEngine();
    void setup() noexcept;
    bool isReady() noexcept;

public:
    static CryptingEngine& instance();

    static void randomize(unsigned char* buffer, size_t length);
    static void create_nonce(unsigned char* buffer, size_t length);
    static void setKeyNameEncrypting(const char*) noexcept;
    static void setKeyNameMac(const char*) noexcept;

    constexpr static auto IV_SIZE = 8;
    constexpr static auto SALT_SIZE = 56;

    bool isValid() noexcept;
    static bool setIV(const unsigned char* iv, size_t liv) noexcept;
    static unsigned char* getIV() noexcept;
    /**
     * @brief This allow to specify a password instead of letting this engine get it from the kernel keyring
     *
     * @param password the new password
     * @param salt the salt to be used for deriving the keys
     *
     * @return
     */
    bool setCredentials(const std::string& password, const unsigned char* salt) noexcept;
    bool encrypt(void* out, size_t lout, const void* in, size_t lin) noexcept;
    bool decrypt(void* out, size_t lout, const void* in, size_t lin) noexcept;

    struct Buffer {
        Buffer();
        explicit Buffer(size_t len);
        ~Buffer();
        unsigned char* bytes_;
        size_t len_;
    };
    using BufferPtr = std::shared_ptr<Buffer>;

    struct MAC {
        MAC();
        ~MAC();
        bool reset() noexcept;
        bool update(const void* buffer, size_t len) noexcept;
        void stop() noexcept;
        BufferPtr read() noexcept;
        bool verify(unsigned char* buffer, size_t len) noexcept;

    private:
        gcry_mac_hd_t hd_;
        bool valid_;
        bool need_init_;
        bool ignore_updates_;
    };

private:
    static CryptingEngine* instance_;
    static unsigned char iv_[IV_SIZE];
    static bool has_iv_;
    bool valid_;
    bool has_credentials_;
    gcry_cipher_hd_t hd_;
};

#endif
// vim: tw=220:ts=4
