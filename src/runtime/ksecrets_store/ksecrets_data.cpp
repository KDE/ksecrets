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
#include "ksecrets_data.h"
#include "ksecrets_file.h"

#include <unistd.h>

SecretsEntity::SecretsEntity()
    : size_(0)
    , state_(Empty)
    , encrypted_(nullptr)
    , unencrypted_(nullptr)
{
}
SecretsEntity::~SecretsEntity()
{
    if (encrypted_) {
        ::free(encrypted_);
        encrypted_ = nullptr;
    }
    if (unencrypted_) {
        ::free(unencrypted_);
        unencrypted_ = nullptr;
    }
}
bool SecretsEntity::read(KSecretsFile& file)
{
    if (!file.read(size_)) {
        return false;
    }

    encrypted_ = ::malloc(res);
    if (encrypted_ == nullptr) {
        return false;
    }
}
