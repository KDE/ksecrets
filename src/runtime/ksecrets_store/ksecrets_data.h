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
#ifndef KSECRETS_DATA_H
#define KSECRETS_DATA_H

#include "ksecrets_crypt.h"

#include <cstdint>
#include <sys/types.h>

class KSecretsFile;

/**
 * @brief Elementary secret element
 *
 * TODO this class uses routines from ksecrets_crypt.cpp file to handle
 * encrypting and decrypting of the files. It would be better to define some
 * plugin architecture, allowing users specify different encryption methods.
 */
struct SecretsEntity {
    SecretsEntity();
    SecretsEntity(const SecretsEntity&) = delete;
    SecretsEntity(SecretsEntity&&) = delete;
    virtual ~SecretsEntity();

    enum class State : std::uint8_t {
        Empty = 0,
        Encrypted = 0x01,
        Decrypted = 0x02
    };

    bool isEmpty() const noexcept { return state_ == State::Empty; }
    bool isDecrypted() const noexcept
    {
        return (static_cast<std::uint8_t>(state_)
                   & static_cast<std::uint8_t>(State::Decrypted)) != 0;
    }

    virtual bool decrypt() noexcept;
    virtual bool encrypt() noexcept;

    virtual bool read(KSecretsFile&) noexcept;
    virtual bool write(KSecretsFile&) const noexcept;

    State state_;
    CryptBuffer encrypted_;
    CryptBuffer unencrypted_;
};

struct SecretsCollection : public SecretsEntity {
};

struct CollectionDirectory : public SecretsEntity {
};

#endif
