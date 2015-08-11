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

#include "ksecrets_store.h"
#include "ksecrets_store_p.h"

#include <future>
#include <thread>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#define GCRPYT_NO_DEPRECATED
#include <gcrypt.h>

KSecretsStorePrivate::KSecretsStorePrivate(KSecretsStore* b)
    : b_(b)
{
    openStatus_.status_ = KSecretsStore::OpenStatus::NotYetOpened;
}

KSecretsStore::KSecretsStore()
    : d(new KSecretsStorePrivate(this))
{
}

KSecretsStore::~KSecretsStore() = default;

std::future<KSecretsStore::OpenResult> KSecretsStore::open(std::string&& path, bool readonly /* =true */) noexcept
{
    // sanity checks
    if (path.empty()) {
        return std::async(std::launch::deferred, []() { return OpenResult{ OpenStatus::NoPathGiven, 0 }; });
    }

    bool shouldCreateFile = false;
    struct stat buf;
    if (stat(path.c_str(), &buf) != 0) {
        auto err = errno;
        if (ENOENT == err) {
            shouldCreateFile = true;
        }
        else {
            return std::async(std::launch::deferred, [err]() { return OpenResult{ OpenStatus::SystemError, errno }; });
        }
    }
    else {
        if (buf.st_size == 0) {
            unlink(path.c_str());
            shouldCreateFile = true; // recreate if empty file was found
        }
    }

    // now we can proceed
    auto localThis = this;
    if (!readonly) {
        return std::async(
            std::launch::async, [localThis, path, shouldCreateFile]() { return localThis->d->createFileIfNeededThenDo(path, shouldCreateFile, std::bind(&KSecretsStorePrivate::lock_open, localThis->d.get(), path)); });
    }
    else {
        return std::async(
            std::launch::deferred, [localThis, path, shouldCreateFile]() { return localThis->d->createFileIfNeededThenDo(path, shouldCreateFile, std::bind(&KSecretsStorePrivate::open, localThis->d.get(), path)); });
    }
}

KSecretsStore::OpenResult KSecretsStorePrivate::createFileIfNeededThenDo(const std::string& path, bool shouldCreateFile, open_action action)
{
    if (shouldCreateFile) {
        auto createRes = createFile(path);
        if (createRes != 0) {
            return setOpenStatus({ KSecretsStore::OpenStatus::SystemError, createRes });
        }
    }
    return action(path);
}

char fileMagic[] = { 'k', 's', 'e', 'c', 'r', 'e', 't', 's' };
constexpr auto fileMagicLen = sizeof(fileMagic) / sizeof(fileMagic[0]);

int KSecretsStorePrivate::createFile(const std::string& path)
{
    FILE* f = fopen(path.c_str(), "w");
    if (f == nullptr) {
        return errno;
    }

    FileHeadStruct emptyFileData;
    memcpy(emptyFileData.magic, fileMagic, fileMagicLen);
    gcry_randomize(emptyFileData.salt, KSecretsStore::SALT_SIZE, GCRY_STRONG_RANDOM);
    gcry_randomize(emptyFileData.iv, IV_SIZE, GCRY_STRONG_RANDOM);

    int res = 0;
    if (fwrite(&emptyFileData, sizeof(emptyFileData), 1, f) != sizeof(emptyFileData)) {
        res = -1; // FIXME is errno available here?
    }
    fclose(f);
    return res;
}

const char* KSecretsStore::salt() const { return d->salt(); }

const char* KSecretsStorePrivate::salt() const { return fileHead_.salt; }

KSecretsStore::OpenResult KSecretsStorePrivate::lock_open(const std::string& path)
{
    file_ = fopen(path.c_str(), "w+");
    if (file_ == nullptr) {
        return { KSecretsStore::OpenStatus::SystemError, errno };
    }
    // TODO perform the lock here
    return open(path);
}

KSecretsStore::OpenResult KSecretsStorePrivate::setOpenStatus(KSecretsStore::OpenResult openStatus)
{
    openStatus_ = openStatus;
    return openStatus;
}

KSecretsStore::OpenResult KSecretsStorePrivate::open(const std::string& path)
{
    if (file_ == nullptr) {
        file_ = fopen(path.c_str(), "r");
    }
    if (file_ == nullptr) {
        return setOpenStatus({ KSecretsStore::OpenStatus::SystemError, errno });
    }
    if (fread(&fileHead_, sizeof(fileHead_), 1, file_) != sizeof(fileHead_)) {
        return setOpenStatus({ KSecretsStore::OpenStatus::SystemError, ferror(file_) });
    }
    if (memcmp(fileHead_.magic, fileMagic, fileMagicLen) != 0) {
        return setOpenStatus({ KSecretsStore::OpenStatus::InvalidFile, 0 });
    }
    // decrypting will occur upon collection request
    return setOpenStatus({ KSecretsStore::OpenStatus::Good, 0 });
}

KSecretsStore::CollectionNames KSecretsStore::dirCollections() const noexcept
{
    // TODO
    return CollectionNames();
}

KSecretsStore::CollectionPtr KSecretsStore::createCollection(std::string&&) noexcept
{
    // TODO
    return CollectionPtr();
}

KSecretsStore::CollectionPtr KSecretsStore::readCollection(std::string&&) const noexcept
{
    // TODO
    return CollectionPtr();
}

bool KSecretsStore::deleteCollection(CollectionPtr) noexcept
{
    // TODO
    return false;
}

bool KSecretsStore::deleteCollection(std::string&&) noexcept
{
    // TODO
    return false;
}
// vim: tw=220:ts=4
