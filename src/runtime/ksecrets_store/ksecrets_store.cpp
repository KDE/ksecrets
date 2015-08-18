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
#include "ksecrets_file.h"
#include "ksecrets_data.h"

#include <future>
#include <thread>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <syslog.h>
#define GCRPYT_NO_DEPRECATED
#include <gcrypt.h>

#define KSS_LOG_ERR (LOG_AUTH | LOG_ERR)

const char* keyNameEncrypting = nullptr;
const char* keyNameMac = nullptr;

bool kss_init_gcry();
int kss_set_credentials(const std::string& password, const char* salt);
const char* get_keyname_encrypting() { return keyNameEncrypting; }
const char* get_keyname_mac() { return keyNameMac; }

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

std::future<KSecretsStore::SetupResult> KSecretsStore::setup(const char* path, bool readOnly /* = true */)
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
        // when asking read only access, creating file is not possible
        if (ENOENT == err && !readOnly) {
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

    auto localThis = this;
    std::string filePath = path;
    return std::async(std::launch::async, [localThis, filePath, shouldCreateFile, readOnly]() { return localThis->d->setup(filePath, shouldCreateFile, readOnly); });
}

KSecretsStore::SetupResult KSecretsStorePrivate::setup(const std::string& path, bool shouldCreateFile, bool readOnly)
{
    if (shouldCreateFile) {
        auto createres = createFile(path);
        if (createres != 0) {
            return setStoreStatus(KSecretsStore::SetupResult(KSecretsStore::StoreStatus::SystemError, createres));
        }
    }
    secretsFile_.setup(path, readOnly);
    return open(!readOnly);
}

std::future<KSecretsStore::CredentialsResult> KSecretsStore::setCredentials(const char* password, const char* keyNameEncrypting, const char* keyNameMac)
{
    ::keyNameEncrypting = keyNameEncrypting;
    ::keyNameMac = keyNameMac;

    std::string pwd = password;
    auto localThis = this;
    return std::async(std::launch::async, [localThis, pwd]() { return localThis->d->setCredentials(pwd); });
}

KSecretsStore::CredentialsResult KSecretsStorePrivate::setCredentials(const std::string& password)
{
    using Result = KSecretsStore::CredentialsResult;
    if (!kss_init_gcry()) {
        return setStoreStatus(Result(KSecretsStore::StoreStatus::CannotInitGcrypt, -1));
    }
    auto res = kss_set_credentials(password, salt());
    if (-1 == res) {
        return setStoreStatus(Result(KSecretsStore::StoreStatus::CannotDeriveKeys, res));
    }
    return setStoreStatus(Result(KSecretsStore::StoreStatus::CredentialsSet, 0));
}

int KSecretsStorePrivate::createFile(const std::string& path) { return secretsFile_.create(path); }

bool KSecretsStore::isGood() const noexcept { return d->status_ == StoreStatus::Good; }

const char* KSecretsStorePrivate::salt() const { return secretsFile_.salt(); }

KSecretsStore::SetupResult KSecretsStorePrivate::open(bool lockFile)
{
    using OpenResult = KSecretsStore::SetupResult;
    if (!secretsFile_.open()) {
        return setStoreStatus(OpenResult(KSecretsStore::StoreStatus::CannotOpenFile, errno));
    }
    if (lockFile) {
        if (!secretsFile_.lock()) {
            return setStoreStatus(OpenResult(KSecretsStore::StoreStatus::CannotLockFile, errno));
        }
    }
    if (!secretsFile_.readHeader()) {
        return setStoreStatus(OpenResult(KSecretsStore::StoreStatus::CannotReadFile, errno));
    }
    if (!secretsFile_.checkMagic()) {
        return setStoreStatus(OpenResult(KSecretsStore::StoreStatus::InvalidFile, -1));
    }
    // TODO add here MAC integrity check
    // decrypting will occur upon collection request
    return setStoreStatus(OpenResult(KSecretsStore::StoreStatus::Good, 0));
}

KSecretsStore::DirCollectionsResult KSecretsStore::dirCollections() const noexcept { return d->dirCollections(); }

KSecretsStore::DirCollectionsResult KSecretsStorePrivate::dirCollections()
{
    KSecretsStore::DirCollectionsResult res(KSecretsStore::StoreStatus::InvalidFile);
    auto dir = secretsFile_.dirCollections();
    if (dir.first) {
    }
    return res;
}

KSecretsStore::CreateCollectionResult KSecretsStore::createCollection(const char* collName) noexcept
{
    return d->createCollection(collName);
}

template <class R>
R mapSecretsFileFailure(KSecretsFile &file, R &&r)
{
    if (file.errnumber()) {
        r.errno_ = file.errnumber();
        r.status_ = KSecretsStore::StoreStatus::SystemError;
    } else {
        if (file.eof()) {
            r.status_ = KSecretsStore::StoreStatus::PrematureEndOfFileEncountered;
        } else {
            r.status_ = KSecretsStore::StoreStatus::UnknownError; // really, we should get here very seldom
        }
    }

    return r;
}

KSecretsStore::CreateCollectionResult KSecretsStorePrivate::createCollection(const std::string &collName) noexcept
{
    KSecretsStore::CreateCollectionResult res;
    auto cptr = std::make_shared<KSecretsCollectionPrivate>();
    if (!cptr->createCollection(secretsFile_, collName)) {
        return mapSecretsFileFailure(secretsFile_, res);
    }
    res.result_ = std::make_shared<KSecretsStore::Collection>(cptr);
    return res;
}

bool KSecretsCollectionPrivate::createCollection(KSecretsFile &file, const std::string &collName)
{
    bool res = false;
    auto dir =  file.dirCollections();
    if (dir.first) {
        if (!dir.second->hasEntry(collName)) {
            collection_data_ = file.createCollection(collName);
            if (collection_data_ != nullptr) {
                res = true;
            }
        }
    }
    return res;
}

KSecretsStore::Collection::Collection(KSecretsCollectionPrivatePtr dptr) : d(dptr) {}

KSecretsStore::ReadCollectionResult KSecretsStore::readCollection(const char*) const noexcept
{
    // TODO
    return ReadCollectionResult();
}

KSecretsStore::DeleteCollectionResult KSecretsStore::deleteCollection(CollectionPtr) noexcept
{
    // TODO
    return DeleteCollectionResult();
}

KSecretsStore::DeleteCollectionResult KSecretsStore::deleteCollection(const char*) noexcept
{
    // TODO
    return DeleteCollectionResult();
}

std::time_t KSecretsStore::Collection::createdTime() const
{
    // TODO
    return std::time_t();
}

std::time_t KSecretsStore::Collection::modifiedTime() const
{
    // TODO
    return std::time_t();
}

std::string KSecretsStore::Collection::label() const
{
    // TODO
    return "";
}

KSecretsStore::Collection::ItemList KSecretsStore::Collection::dirItems() const
{
    // TODO
    return ItemList();
}

KSecretsStore::Collection::ItemList KSecretsStore::Collection::searchItems(const AttributesMap&) const
{
    // TODO
    return ItemList();
}

KSecretsStore::Collection::ItemList KSecretsStore::Collection::searchItems(const char*, const AttributesMap&) const
{
    // TODO
    return ItemList();
}

KSecretsStore::Collection::ItemList KSecretsStore::Collection::searchItems(const char*) const
{
    // TODO
    return ItemList();
}

KSecretsStore::ItemPtr KSecretsStore::Collection::createItem(const char*, AttributesMap, ItemValue)
{
    // TODO
    return ItemPtr();
}

bool KSecretsStore::Collection::deleteItem(ItemPtr)
{
    // TODO
    return false;
}

KSecretsStore::ItemPtr KSecretsStore::Collection::createItem(const char*, ItemValue)
{
    // TODO
    return ItemPtr();
}

std::time_t KSecretsStore::Item::createdTime() const
{
    // TODO
    return std::time_t();
}

std::time_t KSecretsStore::Item::modifiedTime() const
{
    // TODO
    return std::time_t();
}

std::string KSecretsStore::Item::label() const
{
    // TODO
    return "";
}

bool KSecretsStore::Item::setLabel(const char*)
{
    // TODO
    return false;
}

KSecretsStore::ItemValue KSecretsStore::Item::value() const
{
    // TODO
    return ItemValue();
}

bool KSecretsStore::Item::setValue(ItemValue)
{
    // TODO
    return false;
}

KSecretsStore::AttributesMap KSecretsStore::Item::attributes() const
{
    // TODO
    return AttributesMap();
}

bool KSecretsStore::Item::setAttributes(AttributesMap)
{
    // TODO
    return false;
}

// vim: tw=220:ts=4
