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
    bool isReady() const noexcept;

public:
    static CryptingEngine& instance();

    bool isValid() noexcept;
    bool setIV(const void* iv, size_t liv) noexcept;
    /**
     * @brief This allow to specify a password instead of letting this engine get it from the kernel keyring
     *
     * @param password the new password
     * @param salt the salt to be used for deriving the keys
     *
     * @return
     */
    bool setCredentials(const std::string& password, const char* salt) noexcept;
    bool encrypt(void* out, size_t lout, const void* in, size_t lin) noexcept;
    bool decrypt(void* out, size_t lout, const void* in, size_t lin) noexcept;

private:
    static CryptingEngine* instance_;
    bool valid_;
    bool has_iv_;
    bool has_credentials_;
    gcry_cipher_hd_t hd_;
};

#endif
