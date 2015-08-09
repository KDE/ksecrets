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

#include "ksecrets_backend.h"
#include "ksecrets_backend_p.h"

#include <future>
#include <thread>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#define GCRPYT_NO_DEPRECATED
#include <gcrypt.h>

KSecretsBackendPrivate::KSecretsBackendPrivate(KSecretsBackend* b)
    : b_(b)
{
    openStatus_.status_ = KSecretsBackend::OpenStatus::NotYetOpened;
}

KSecretsBackend::KSecretsBackend()
    : d(new KSecretsBackendPrivate(this))
{
}

KSecretsBackend::~KSecretsBackend() = default;

std::future<KSecretsBackend::OpenResult> KSecretsBackend::open(std::string&& path, bool readonly /* =true */) noexcept
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
            std::launch::async, [localThis, path, shouldCreateFile]() { return localThis->d->createFileIfNeededThenDo(path, shouldCreateFile, std::bind(&KSecretsBackendPrivate::lock_open, localThis->d.get(), path)); });
    }
    else {
        return std::async(
            std::launch::deferred, [localThis, path, shouldCreateFile]() { return localThis->d->createFileIfNeededThenDo(path, shouldCreateFile, std::bind(&KSecretsBackendPrivate::open, localThis->d.get(), path)); });
    }
}

KSecretsBackend::OpenResult KSecretsBackendPrivate::createFileIfNeededThenDo(const std::string& path, bool shouldCreateFile, open_action action)
{
    if (shouldCreateFile) {
        auto createRes = createFile(path);
        if (createRes != 0) {
            return setOpenStatus({ KSecretsBackend::OpenStatus::SystemError, createRes });
        }
    }
    return action(path);
}

char fileMagic[] = { 'k', 's', 'e', 'c', 'r', 'e', 't', 's' };
constexpr auto fileMagicLen = sizeof(fileMagic) / sizeof(fileMagic[0]);

int KSecretsBackendPrivate::createFile(const std::string& path)
{
    FILE* f = fopen(path.c_str(), "w");
    if (f == nullptr) {
        return errno;
    }

    FileHeadStruct emptyFileData;
    memcpy(emptyFileData.magic, fileMagic, fileMagicLen);
    gcry_randomize(emptyFileData.salt, KSecretsBackend::SALT_SIZE, GCRY_STRONG_RANDOM);
    gcry_randomize(emptyFileData.iv, IV_SIZE, GCRY_STRONG_RANDOM);

    int res = 0;
    if (fwrite(&emptyFileData, sizeof(emptyFileData), 1, f) != sizeof(emptyFileData)) {
        res = -1; // FIXME is errno available here?
    }
    fclose(f);
    return res;
}

const char* KSecretsBackend::salt() const { return d->salt(); }

const char* KSecretsBackendPrivate::salt() const { return fileHead_.salt; }

KSecretsBackend::OpenResult KSecretsBackendPrivate::lock_open(const std::string& path)
{
    file_ = fopen(path.c_str(), "w+");
    if (file_ == nullptr) {
        return { KSecretsBackend::OpenStatus::SystemError, errno };
    }
    // TODO perform the lock here
    return open(path);
}

KSecretsBackend::OpenResult KSecretsBackendPrivate::setOpenStatus(KSecretsBackend::OpenResult openStatus)
{
    openStatus_ = openStatus;
    return openStatus;
}

KSecretsBackend::OpenResult KSecretsBackendPrivate::open(const std::string& path)
{
    if (file_ == nullptr) {
        file_ = fopen(path.c_str(), "r");
    }
    if (file_ == nullptr) {
        return setOpenStatus({ KSecretsBackend::OpenStatus::SystemError, errno });
    }
    if (fread(&fileHead_, sizeof(fileHead_), 1, file_) != sizeof(fileHead_)) {
        return setOpenStatus({ KSecretsBackend::OpenStatus::SystemError, ferror(file_) });
    }
    if (memcmp(fileHead_.magic, fileMagic, fileMagicLen) != 0) {
        return setOpenStatus({ KSecretsBackend::OpenStatus::InvalidFile, 0 });
    }
    // decrypting will occur upon collection request
    return setOpenStatus({ KSecretsBackend::OpenStatus::Good, 0 });
}

KSecretsBackend::CollectionNames KSecretsBackend::dirCollections() const noexcept
{
    // TODO
    return CollectionNames();
}

KSecretsBackend::CollectionPtr KSecretsBackend::createCollection(std::string&&) noexcept
{
    // TODO
    return CollectionPtr();
}

KSecretsBackend::CollectionPtr KSecretsBackend::readCollection(std::string&&) const noexcept
{
    // TODO
    return CollectionPtr();
}

bool KSecretsBackend::deleteCollection(CollectionPtr) noexcept
{
    // TODO
    return false;
}

bool KSecretsBackend::deleteCollection(std::string&&) noexcept
{
    // TODO
    return false;
}
// vim: tw=220:ts=4
