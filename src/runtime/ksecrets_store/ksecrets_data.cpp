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
#include "defines.h"

#include <unistd.h>
#include <algorithm>
#include <cassert>
#include <iomanip>

SecretsEntityPtr SecretsEntityFactory::createInstance(SecretsEntity::EntityType et)
{
    SecretsEntityPtr res;
    switch (et) {
    case SecretsEntity::EntityType::SecretsEntityType:
        assert(0);
        break;
    case SecretsEntity::EntityType::CollectionDirectoryType:
        res = std::make_shared<CollectionDirectory>();
        break;
    case SecretsEntity::EntityType::SecretsItemType:
        res = std::make_shared<SecretsItem>();
        break;
    case SecretsEntity::EntityType::SecretsEOFType:
        res = std::make_shared<SecretsEOF>();
        break;
    case SecretsEntity::EntityType::SecretsCollectionType:
        res = std::make_shared<SecretsCollection>();
        break;
    default:
        syslog(KSS_LOG_ERR, "ksecrets: unkonw entity type creation requested %ld", (long)et);
    }
    return res;
}

SecretsEntity::SecretsEntity() {}

SecretsEntity::~SecretsEntity() {}

bool SecretsEntity::write(KSecretsFile& file) noexcept
{
    std::ostream os(&buffer_);
    if (!serialize(os) || !os.good())
        return false;

    if (!serializeChildren(os) || !os.good())
        return false;

    if (!buffer_.write(file)) {
        onWriteError();
        return false;
    }

    return true;
}

bool SecretsEntity::read(KSecretsFile& file) noexcept
{
    if (!doBeforeRead())
        return false;

    bool res = false;
    if (buffer_.read(file)) {
        std::istream is(&buffer_);

        // if (!deserialize(is) || !is.good())
        if (!deserialize(is))
            return false;

        // if (!deserializeChildren(is) && !is.good())
        if (!deserializeChildren(is))
            return false;

        res = true;
    }
    else {
        onReadError();
    }
    return res;
}

bool SecretsEntity::deserializeChildren(std::istream&) noexcept { return true; }
bool SecretsEntity::serializeChildren(std::ostream&) noexcept { return true; }
void SecretsEntity::onReadError() noexcept { /* nothing to do here */}
void SecretsEntity::onWriteError() noexcept { /* nothing to do here */}

CollectionDirectory::CollectionDirectory() {}

void CollectionDirectory::addCollection(const std::string& collName) noexcept
{
    assert(!hasEntry(collName));
    entries_.emplace_back(collName);
    // FIXME should we sort this list?
}

bool CollectionDirectory::hasEntry(const std::string& collName) const noexcept
{
    // FIXME should we use some binary search algo here?
    auto pos = std::find(entries_.begin(), entries_.end(), collName);
    return pos != entries_.end();
}

bool CollectionDirectory::serialize(std::ostream& os) noexcept
{
    syslog(KSS_LOG_INFO, "ksecrets: CollectionDirectory serializing %d items", (int)entries_.size());
    os << ' ' << entries_.size();
    for (const std::string& entry : entries_) {
        syslog(KSS_LOG_INFO, "   %s", entry.c_str());
        os << entry;
    }
    return true;
}

bool CollectionDirectory::deserialize(std::istream& is) noexcept
{
    if (!is.good())
        return false;
    Entries::size_type n;
    is >> n;
    for (Entries::size_type i = 0; i < n; i++) {
        std::string entry;
        is >> entry;
        if (!is.good())
            return false;
        entries_.emplace_back(entry);
    }
    return true;
}

void SecretsCollection::setName(const std::string& name) noexcept { name_ = name; }

bool SecretsCollection::serializeChildren(std::ostream& os) noexcept
{
    bool res = true;
    for (SecretsItemPtr item : items_) {
        if (item->serialize(os) || !os.good())
            return false;
    }
    return res;
}

bool SecretsCollection::deserializeChildren(std::istream& is) noexcept
{
    bool res = true;

    return res;
}

bool SecretsCollection::serialize(std::ostream& os) noexcept
{
    os << name_;
    os << ' ' << items_.size();
    return true;
}

bool SecretsCollection::deserialize(std::istream& is) noexcept
{
    is >> name_;
    is >> itemsCount_;
    return true;
}

bool SecretsItem::serialize(std::ostream&) noexcept
{
    // TODO
    return false;
}

bool SecretsItem::deserialize(std::istream&) noexcept
{
    // TODO
    return false;
}

bool SecretsEOF::serialize(std::ostream&) noexcept
{
    // TODO
    return false;
}

bool SecretsEOF::deserialize(std::istream&) noexcept
{
    // TODO
    return false;
}

// vim: tw=220:ts=4
