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

#include "crypt_buffer.h"

#include <cstdint>
#include <sys/types.h>
#include <memory>
#include <deque>
#include <ostream>
#include <istream>

class KSecretsFile;

/**
 * @brief Elementary secret element
 *
 * This base class encapsulates the details of the data serialization into the @ref KSecretsFile. It uses
 * ASCII serialization in order to avoid binary endian problems.
 *
 * The actual serialization is taken care of by the inheritor classes. The protocol is enforced by this class
 * being an abstract base class.
 *
 * @todo in the future, if the need arises to add another file format, this code
 * could be easily refactored to use the "visitor" pattern and convert the
 * read and write methods to use a generic type which would then be
 * implemented by the KSecretsFile. For now, no need of such generalization,
 * as I cannot foresee the introduction of another file format (why should I?)A
 *
 */
class SecretsEntity {
public:
    SecretsEntity();
    virtual ~SecretsEntity();

    virtual bool hasNext() const noexcept { return true; }

    enum class EntityType : std::uint8_t {
        SecretsEntityType, /// this is never used and never should be encountered
        CollectionDirectoryType,
        SecretsItemType,
        SecretsCollectionType,
        SecretsEOFType
    };

    virtual EntityType getType() const = 0;

    bool write(KSecretsFile&) noexcept;
    bool read(KSecretsFile&) noexcept;

    virtual bool serialize(std::ostream&) noexcept = 0;
    virtual bool deserialize(std::istream&) noexcept = 0;

private:
    virtual bool deserializeChildren(std::istream&) noexcept;
    virtual bool doBeforeRead() noexcept { return true; }
    virtual void onReadError() noexcept;

    virtual bool serializeChildren(std::ostream&) noexcept;
    virtual void onWriteError() noexcept;

private:
    CryptBuffer buffer_;
};

using SecretsEntityPtr = std::shared_ptr<SecretsEntity>;

class SecretsEntityFactory {
public:
    static SecretsEntityPtr createInstance(SecretsEntity::EntityType);
};

class CollectionDirectory : public SecretsEntity {
public:
    using Entries = std::deque<std::string>;

    CollectionDirectory();
    void addCollection(const std::string&) noexcept;
    bool hasEntry(const std::string&) const noexcept;
    virtual EntityType getType() const noexcept { return EntityType::CollectionDirectoryType; }
    const Entries& entries() const noexcept { return entries_; }

    virtual bool serialize(std::ostream&) noexcept override;
    virtual bool deserialize(std::istream&) noexcept override;

private:
    Entries entries_;
};

using CollectionDirectoryPtr = std::shared_ptr<CollectionDirectory>;

class SecretsItem : public SecretsEntity {
public:
    virtual bool serialize(std::ostream&) noexcept override;
    virtual bool deserialize(std::istream&) noexcept override;
private:
    virtual EntityType getType() const noexcept { return EntityType::SecretsItemType; }
};

using SecretsItemPtr = std::shared_ptr<SecretsItem>;

class SecretsCollection : public SecretsEntity {
public:
    void setName(const std::string&) noexcept;

    virtual bool serialize(std::ostream&) noexcept override;
    virtual bool deserialize(std::istream&) noexcept override;
private:
    virtual bool deserializeChildren(std::istream&) noexcept override;
    virtual bool serializeChildren(std::ostream&) noexcept override;
    virtual EntityType getType() const noexcept { return EntityType::SecretsCollectionType; }

    using Items = std::deque<SecretsItemPtr>;

    std::string name_;
    Items items_;
    size_t itemsCount_; // used during serialization
};

using SecretsCollectionPtr = std::shared_ptr<SecretsCollection>;

class SecretsEOF : public SecretsEntity {
    public:
    virtual bool serialize(std::ostream&) noexcept override;
    virtual bool deserialize(std::istream&) noexcept override;
private:
    bool hasNext() const noexcept override { return false; }
    virtual EntityType getType() const noexcept { return EntityType::SecretsEOFType; }
};

using SecretsEOFPtr = std::shared_ptr<SecretsEOF>;

#endif
// vim: tw=220:ts=4
