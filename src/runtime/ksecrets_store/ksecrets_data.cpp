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
#include <algorithm>
#include <cassert>

long kss_encrypt_buffer(unsigned char* out, size_t lout, const void* iv, size_t liv, const unsigned char* in, size_t lin);
long kss_decrypt_buffer(unsigned char* out, size_t lout, const void* iv, size_t liv, const unsigned char* in, size_t lin);
char* kss_alloc_crypt_buffer(size_t rlen);

SecretsEntity::SecretsEntity() {}

SecretsEntity::~SecretsEntity() {}

bool SecretsEntity::write(KSecretsFile& file)
{
    if (!doBeforeWrite())
        return false;

    if (!encrypted_.write(file))
        return false;

    return doAfterWrite();
}

bool SecretsEntity::read(KSecretsFile& file)
{
    if (!doBeforeRead())
        return false;

    if (encrypted_.read(file))
        return doAfterRead();
    else
        return false;
}

CollectionDirectory::CollectionDirectory() {}

bool CollectionDirectory::hasEntry(const std::string& collName) const
{
    auto pos = std::find(entries_.begin(), entries_.end(), collName);
    return pos != entries_.end();
}

bool CollectionDirectory::doBeforeWrite()
{
    // TODO
    return false;
}

bool CollectionDirectory::doAfterRead()
{
    // TODO
    return false;
}

void SecretsCollection::setName(const std::string& name) { name_ = name; }

bool SecretsCollection::doBeforeWrite()
{
    // TODO
    return false;
}

bool SecretsCollection::doAfterRead()
{
    // TODO
    return false;
}

bool SecretsItem::doBeforeWrite()
{
    // TODO
    return false;
}

bool SecretsItem::doAfterRead()
{
    // TODO
    return false;
}

bool SecretsEOF::doBeforeWrite()
{
    // TODO
    return false;
}

bool SecretsEOF::doAfterRead()
{
    // TODO
    return false;
}

// vim: tw=220:ts=4
