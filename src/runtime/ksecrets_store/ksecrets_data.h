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
#include <memory>
#include <list>
#include <ostream>
#include <istream>

class KSecretsFile;

/**
 * @brief Elementary secret element
 *
 * TODO this class uses routines from ksecrets_crypt.cpp file to handle
 * encrypting and decrypting of the files. It would be better to define some
 * plugin architecture, allowing users specify different encryption methods.
 *
 * TODO in the future, if the need arises to add another file format, this
 *code
 * could be easily refactored to use the "visitor" pattern and convert the
 * read and write methods to use a generic type which would then be
 * implemented by the KSecretsFile. For now, no need of such generalization,
 * as I cannot foresee the introduction of another file format (why should I?)
 */
class SecretsEntity {
public:
    SecretsEntity();
    virtual ~SecretsEntity();

    virtual bool hasNext() const noexcept { return true; }

    enum class EntityType {
        SecretsEntityType, /// this is never used and never should be encountered
        CollectionDirectoryType,
        SecretsItemType,
        SecretsCollectionType,
        SecretsEOFType
    };

    virtual EntityType getType() const = 0;

    bool read(KSecretsFile&) noexcept;
    virtual bool doBeforeRead() noexcept { return true; }
    virtual bool doAfterRead(std::istream&) noexcept = 0;
    virtual void doOnReadError() noexcept;

    bool write(KSecretsFile&) noexcept;
    virtual bool doBeforeWrite(std::ostream&) noexcept = 0;
    virtual bool doAfterWrite() noexcept { return true; }
    virtual void doOnWriteError() noexcept;

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
    CollectionDirectory();
    bool hasEntry(const std::string&) const noexcept;
    virtual EntityType getType() const noexcept { return EntityType::CollectionDirectoryType; }

private:
    virtual bool doBeforeWrite(std::ostream&) noexcept override;
    virtual bool doAfterRead(std::istream&) noexcept override;

    using Entries = std::list<std::string>;
    Entries entries_;
};

using CollectionDirectoryPtr = std::shared_ptr<CollectionDirectory>;

class SecretsItem : public SecretsEntity {
public:
private:
    virtual bool doBeforeWrite(std::ostream&) noexcept override;
    virtual bool doAfterRead(std::istream&) noexcept override;
    virtual EntityType getType() const noexcept { return EntityType::SecretsItemType; }
};

using SecretsItemPtr = std::shared_ptr<SecretsItem>;

class SecretsCollection : public SecretsEntity {
public:
    void setName(const std::string&) noexcept;

private:
    virtual bool doBeforeWrite(std::ostream&) noexcept override;
    virtual bool doAfterRead(std::istream&) noexcept override;
    virtual EntityType getType() const noexcept { return EntityType::SecretsCollectionType; }

    using Items = std::list<SecretsItemPtr>;

    std::string name_;
    Items items_;
};

using SecretsCollectionPtr = std::shared_ptr<SecretsCollection>;

class SecretsEOF : public SecretsEntity {
private:
    bool hasNext() const noexcept override { return false; }
    virtual bool doBeforeWrite(std::ostream&) noexcept override;
    virtual bool doAfterRead(std::istream&) noexcept override;
    virtual EntityType getType() const noexcept { return EntityType::SecretsEOFType; }
};

using SecretsEOFPtr = std::shared_ptr<SecretsEOF>;

#endif
// vim: tw=220:ts=4
