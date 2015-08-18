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

    enum class State : std::uint8_t {
        Empty = 0,
        Encrypted = 0x01,
        Decrypted = 0x02
    };

    bool isEmpty() const noexcept { return state_ == State::Empty; }
    virtual bool hasNext() const noexcept { return true; }
    bool isDecrypted() const noexcept
    {
        return (static_cast<std::uint8_t>(state_)
                   & static_cast<std::uint8_t>(State::Decrypted)) != 0;
    }

    virtual bool decrypt() noexcept;
    virtual bool encrypt() noexcept;

    bool read(KSecretsFile&) noexcept;
    virtual bool doBeforeRead() noexcept { return true; }
    virtual bool doAfterRead() noexcept = 0;

    bool write(KSecretsFile&) noexcept;
    virtual bool doBeforeWrite() noexcept = 0;
    virtual bool doAfterWrite() noexcept { return true; }

    State state_;
    CryptBuffer encrypted_;
    CryptBuffer unencrypted_;
};

using SecretsEntityPtr = std::shared_ptr<SecretsEntity>;

class SecretsCollection : public SecretsEntity {
public:
    void setName(const std::string&) noexcept;

private:
    virtual bool doBeforeWrite() noexcept override;
    virtual bool doAfterRead() noexcept override;

    std::string name_;
};

using SecretsCollectionPtr = std::shared_ptr<SecretsCollection>;

class CollectionDirectory : public SecretsEntity {
public:
    CollectionDirectory();
    bool hasEntry(const std::string&) const noexcept;

private:
    virtual bool doBeforeWrite() noexcept override;
    virtual bool doAfterRead() noexcept override;

    std::list<std::string> entries_;
};

using CollectionDirectoryPtr = std::shared_ptr<CollectionDirectory>;

class SecretsItem : public SecretsEntity {
public:
private:
    virtual bool doBeforeWrite() noexcept override;
    virtual bool doAfterRead() noexcept override;
};

using SecretsItemPtr = std::shared_ptr<SecretsItem>;

class SecretsEOF : public SecretsEntity {
private:
    bool hasNext() const noexcept override { return false; }
    virtual bool doBeforeWrite() noexcept override;
    virtual bool doAfterRead() noexcept override;
};

using SecretsEOFPtr = std::shared_ptr<SecretsEOF>;

#endif
// vim: tw=220:ts=4
