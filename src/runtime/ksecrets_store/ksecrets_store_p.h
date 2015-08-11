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
    };

private:
    std::time_t createdTime_;
    std::time_t modifiedTime_;
};

class KSecretsItemPrivate : public TimeStamped {
};

class KSecretsCollectionPrivate : public TimeStamped {
};

class KSecretsStorePrivate {
public:
    KSecretsStorePrivate() = delete;
    explicit KSecretsStorePrivate(KSecretsStore*);

    KSecretsStore::OpenResult lock_open(const std::string&);
    KSecretsStore::OpenResult open(const std::string&);
    using open_action = std::function<KSecretsStore::OpenResult(const std::string&)>;
    KSecretsStore::OpenResult createFileIfNeededThenDo(const std::string&, bool, open_action);
    int createFile(const std::string&);
    const char* salt() const;

    constexpr static auto IV_SIZE = 32;
    struct FileHeadStruct {
        char magic[9];
        char salt[KSecretsStore::SALT_SIZE];
        char iv[IV_SIZE];
    };

    KSecretsStore::OpenResult setOpenStatus(KSecretsStore::OpenResult);
    bool isOpen() const noexcept { return KSecretsStore::OpenStatus::Good == openStatus_.status_; }

    KSecretsStore* b_;
    FILE* file_;
    FileHeadStruct fileHead_;
    KSecretsStore::OpenResult openStatus_;
};

#endif
// vim: tw=220:ts=4
