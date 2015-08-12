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
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <unistd.h>
#include <stdio.h>
#include <syslog.h>
#define GCRPYT_NO_DEPRECATED
#include <gcrypt.h>

#define KSECRETS_SALTSIZE 56
#define KSECRETS_KEYSIZE 256
#define KSS_LOG_ERR (LOG_AUTH | LOG_ERR)

const char* keyNameEncrypting = nullptr;
const char* keyNameMac = nullptr;

extern "C" {
bool kss_init_gcry();
bool kss_derive_keys(const char* salt, const char* password, char* encryption_key, char* mac_key, size_t);
bool kss_store_keys(const char* encryption_key, const char* mac_key, size_t keySize);
const char* get_keyname_encrypting() { return keyNameEncrypting; }
const char* get_keyname_mac() { return keyNameMac; }
}

KSecretsStorePrivate::KSecretsStorePrivate(KSecretsStore* b)
    : b_(b)
{
    status_ = KSecretsStore::StoreStatus::JustCreated;
}

KSecretsStore::KSecretsStore()
    : d(new KSecretsStorePrivate(this))
{
}

KSecretsStore::~KSecretsStore() = default;

std::future<KSecretsStore::SetupResult> KSecretsStore::setup(const char* path, const char* password, const char* keyNameEcrypting, const char* keyNameMac)
{
    // sanity checks
    if (d->status_ != StoreStatus::JustCreated) {
        return std::async(std::launch::deferred, []() { return SetupResult{ StoreStatus::IncorrectState, -1 }; });
    }
    if (path == nullptr || strlen(path) == 0) {
        return std::async(std::launch::deferred, []() { return SetupResult{ StoreStatus::NoPathGiven, 0 }; });
    }

    bool shouldCreateFile = false;
    struct stat buf;
    if (stat(path, &buf) != 0) {
        auto err = errno;
        if (ENOENT == err) {
            shouldCreateFile = true;
        }
        else {
            return std::async(std::launch::deferred, [err]() { return SetupResult{ StoreStatus::SystemError, errno }; });
        }
    }
    else {
        if (buf.st_size == 0) {
            unlink(path);
            shouldCreateFile = true; // recreate if empty file was found
        }
    }

    ::keyNameEncrypting = keyNameEcrypting;
    ::keyNameMac = keyNameMac;

    auto localThis = this;
    std::string filePath = path;
    std::string pwd = password;
    return std::async(std::launch::async, [localThis, filePath, shouldCreateFile, pwd]() { return localThis->d->setup(filePath, shouldCreateFile, pwd); });
}

KSecretsStore::SetupResult KSecretsStorePrivate::setup(const std::string& path, bool shouldCreateFile, const std::string& password)
{
    if (shouldCreateFile) {
        auto createres = createFile(path);
        if (createres != 0) {
            return setStoreStatus(KSecretsStore::SetupResult({ KSecretsStore::StoreStatus::SystemError, createres }));
        }
    }
    if (!kss_init_gcry()) {
        return setStoreStatus(KSecretsStore::SetupResult({ KSecretsStore::StoreStatus::CannotInitGcrypt, -1 }));
    }
    // FIXME this should be adjusted on platforms where kernel keyring is not available and store the keys elsewhere
    char encryption_key[KSECRETS_KEYSIZE];
    char mac_key[KSECRETS_KEYSIZE];
    if (!kss_derive_keys(salt(), password.c_str(), encryption_key, mac_key, KSECRETS_KEYSIZE)) {
        return setStoreStatus(KSecretsStore::SetupResult({ KSecretsStore::StoreStatus::CannotDeriveKeys, -1 }));
    }

    if (!kss_store_keys(encryption_key, mac_key, KSECRETS_KEYSIZE)) {
        return setStoreStatus(KSecretsStore::SetupResult({ KSecretsStore::StoreStatus::CannotStoreKeys, -1 }));
    }
    secretsFile_.filePath_ = path;
    return setStoreStatus(KSecretsStore::SetupResult({ KSecretsStore::StoreStatus::SetupDone, 0 }));
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

std::future<KSecretsStore::OpenResult> KSecretsStore::open(bool readonly /* =true */) noexcept
{
    // check the state first
    if (d->status_ == StoreStatus::JustCreated) {
        return std::async(std::launch::deferred, []() { return OpenResult{ StoreStatus::SetupShouldBeCalledFirst, -1 }; });
    }
    if (d->status_ != StoreStatus::SetupDone) {
        // open could not be called two times or perhaps the setup call left this store in an invalid state because of some error
        return std::async(std::launch::deferred, []() { return OpenResult{ StoreStatus::IncorrectState, -1 }; });
    }
    // now we can proceed
    auto localThis = this;
    if (!readonly) {
        return std::async([localThis]() { return localThis->d->open(true); });
    }
    else {
        return std::async([localThis]() { return localThis->d->open(false); });
    }
}

const char* KSecretsStore::salt() const { return d->salt(); }

const char* KSecretsStorePrivate::salt() const { return fileHead_.salt; }

KSecretsStore::OpenResult KSecretsStorePrivate::open(bool lockFile)
{
    using OpenResult = KSecretsStore::OpenResult;
    secretsFile_.file_ = ::open(secretsFile_.filePath_.c_str(), O_DSYNC | O_NOATIME | O_NOFOLLOW);
    if (secretsFile_.file_ == -1) {
        return setStoreStatus(OpenResult({ KSecretsStore::StoreStatus::CannotOpenFile, errno }));
    }
    if (lockFile) {
        if (flock(secretsFile_.file_, LOCK_EX) == -1) {
            return KSecretsStore::OpenResult{ KSecretsStore::StoreStatus::CannotLockFile, errno };
        }
        secretsFile_.locked_ = true;
    }
    auto r = read(secretsFile_.file_, &fileHead_, sizeof(fileHead_));
    if (r == -1) {
        return setStoreStatus(OpenResult({ KSecretsStore::StoreStatus::CannotReadFile, errno }));
    }
    if ((size_t)r < sizeof(fileHead_) || memcmp(fileHead_.magic, fileMagic, fileMagicLen) != 0) {
        return setStoreStatus(OpenResult({ KSecretsStore::StoreStatus::InvalidFile, -1 }));
    }
    // decrypting will occur upon collection request
    return setStoreStatus(OpenResult({ KSecretsStore::StoreStatus::Good, 0 }));
}

KSecretsStorePrivate::SecretsFile::~SecretsFile(){
    if (file_ != -1) {
        auto r = close(file_);
        if (r == -1) {
            syslog(KSS_LOG_ERR, "ksecrets: system return erro upon secrets file close: %d (%m)", errno);
            syslog(KSS_LOG_ERR, "ksecrets: the secrets file might now be corrup because of the previous error");
        }
        file_ = -1;
    }
}

KSecretsStore::CollectionNames KSecretsStore::dirCollections() const noexcept
{
    // TODO
    return CollectionNames();
}

KSecretsStore::CollectionPtr KSecretsStore::createCollection(const char*) noexcept
{
    // TODO
    return CollectionPtr();
}

KSecretsStore::CollectionPtr KSecretsStore::readCollection(const char*) const noexcept
{
    // TODO
    return CollectionPtr();
}

bool KSecretsStore::deleteCollection(CollectionPtr) noexcept
{
    // TODO
    return false;
}

bool KSecretsStore::deleteCollection(const char*) noexcept
{
    // TODO
    return false;
}
// vim: tw=220:ts=4
