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

long kss_encrypt_buffer(unsigned char* out, size_t lout, const void* iv,
    size_t liv, const unsigned char* in, size_t lin);
long kss_decrypt_buffer(unsigned char* out, size_t lout, const void* iv,
    size_t liv, const unsigned char* in, size_t lin);
char* kss_alloc_crypt_buffer(size_t rlen);

SecretsEntity::SecretsEntity()
    : state_(State::Empty)
{
}

SecretsEntity::~SecretsEntity() {}

// TODO refactor this when encrypting plugins will be put in place
// the KSecretsFile should place the IV in the plugin structure instead of
// this class
const char* iv = nullptr;
size_t liv = KSecretsFile::IV_SIZE;

bool SecretsEntity::read(KSecretsFile& file)
{
    if (iv == nullptr) {
        iv = file.iv();
    }

    size_t s;
    if (!file.read(s)) {
        return false;
    }
    if (!encrypted_.allocate(s)) {
        return false;
    }

    return file.read(encrypted_.data_,
        encrypted_.len_); // beware not to specify encrypted.size_ here
}

bool SecretsEntity::decrypt()
{
    if (isEmpty())
        return false;
    if (unencrypted_.len_ > 0)
        return true; // already decrpyted
    if (encrypted_.len_ == 0)
        return false; // what to decrypt?
    unencrypted_.allocate(encrypted_.len_);
    auto dres = kss_decrypt_buffer(unencrypted_.data_, unencrypted_.len_, iv,
        liv, encrypted_.data_, encrypted_.len_);
    return dres == 0;
}

bool SecretsEntity::encrypt()
{
    // TODO
    return false;
}

bool SecretsEntity::write(KSecretsFile &) const
{
    // TODO
    return false;
}
// vim: tw=220:ts=4
