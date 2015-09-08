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
#ifndef KSECRETSBACKEND_P_H
#define KSECRETSBACKEND_P_H

#include "ksecrets_store.h"
#include "ksecrets_file.h"

class TimeStamped {

protected:
    TimeStamped()
        : createdTime_(std::time(nullptr))
        , modifiedTime_(std::time(nullptr))
    {
    }
    virtual ~TimeStamped() = default;
    TimeStamped(const TimeStamped&) = default;
    TimeStamped& operator=(const TimeStamped&) = default;

    template <class FUNC> void modify(FUNC func)
    {
        // FIXME func may return some value so modify this to take that into account
        func();
        modifiedTime_ = std::time(nullptr);
    }

private:
    std::time_t createdTime_;
    std::time_t modifiedTime_;
};

class KSecretsItemPrivate : public TimeStamped {
};

class KSecretsCollectionPrivate : public TimeStamped {
public:
    bool createCollection(KSecretsFile &secretsFile, const std::string &collName);
    CollectionDirectoryPtr collectionsDir(KSecretsFile &secretsFile) noexcept;
private:
    SecretsCollectionPtr collection_data_;
    CollectionDirectoryPtr collections_dir_;
};

class KSecretsStorePrivate {
public:
    KSecretsStorePrivate() = delete;
    explicit KSecretsStorePrivate(KSecretsStore*);

    KSecretsStore::SetupResult setup(const std::string& path, bool, bool) noexcept;
    KSecretsStore::CredentialsResult setCredentials(const std::string&) noexcept;
    KSecretsStore::SetupResult open(bool) noexcept;
    int createFile(const std::string&) noexcept;
    const char* salt() const noexcept;
    KSecretsStore::CreateCollectionResult createCollection(const std::string&) noexcept;
    KSecretsStore::DirCollectionsResult dirCollections() noexcept;

    template <typename S> S setStoreStatus(S s) noexcept
    {
        status_ = s.status_;
        return s;
    }
    bool isOpen() const noexcept { return KSecretsStore::StoreStatus::Good == status_; }

    KSecretsStore* b_;
    KSecretsFile secretsFile_;
    KSecretsStore::StoreStatus status_;
};

#endif
// vim: tw=220:ts=4
