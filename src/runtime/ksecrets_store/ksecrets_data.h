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

#include <cstdint>
#include <sys/types.h>

class KSecretsFile;

struct SecretsEntity {
    SecretsEntity();
    SecretsEntity(const SecretsEntity&) = delete;
    SecretsEntity(SecretsEntity&&) = delete;
    virtual ~SecretsEntity();

    enum class State: std::uint8_t {
        Empty = 0,
        Encrypted = 0x01,
        Decrypted = 0x02
    };

    size_t size_;
    State state_;
    char* encrypted_;
    char* unencrypted_;

    bool decrypt() noexcept;
    bool encrypt() noexcept;

    bool read(KSecretsFile&) noexcept;
    bool write(KSecretsFile&) const noexcept;
};

struct SecretsCollection : public SecretsEntity {
};

struct CollectionDirectory : public SecretsEntity {
};

#endif
