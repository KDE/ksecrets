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

SecretsEntity::SecretsEntity() {}

SecretsEntity::~SecretsEntity() {}

bool SecretsEntity::write(KSecretsFile& file)
{
    std::ostream os(&buffer_);
    if (!doBeforeWrite(os))
        return false;

    if (!buffer_.write(file))
        return false;

    return doAfterWrite();
}

bool SecretsEntity::read(KSecretsFile& file)
{
    if (!doBeforeRead())
        return false;

    if (buffer_.read(file)) {
        std::istream is(&buffer_);
        return doAfterRead(is);
    }
    else
        return false;
}

CollectionDirectory::CollectionDirectory() {}

bool CollectionDirectory::hasEntry(const std::string& collName) const
{
    auto pos = std::find(entries_.begin(), entries_.end(), collName);
    return pos != entries_.end();
}

bool CollectionDirectory::doBeforeWrite(std::ostream&)
{
    // TODO
    return false;
}

bool CollectionDirectory::doAfterRead(std::istream&)
{
    // TODO
    return false;
}

void SecretsCollection::setName(const std::string& name) { name_ = name; }

bool SecretsCollection::doBeforeWrite(std::ostream&)
{
    // TODO
    return false;
}

bool SecretsCollection::doAfterRead(std::istream&)
{
    // TODO
    return false;
}

bool SecretsItem::doBeforeWrite(std::ostream&)
{
    // TODO
    return false;
}

bool SecretsItem::doAfterRead(std::istream&)
{
    // TODO
    return false;
}

bool SecretsEOF::doBeforeWrite(std::ostream&)
{
    // TODO
    return false;
}

bool SecretsEOF::doAfterRead(std::istream&)
{
    // TODO
    return false;
}

// vim: tw=220:ts=4
