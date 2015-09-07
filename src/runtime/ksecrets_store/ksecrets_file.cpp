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

#include "ksecrets_file.h"
#include "defines.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/param.h>
#include <string.h>
#include <cassert>
#include <stdlib.h>
#include <algorithm>

char fileMagic[] = { 'k', 's', 'e', 'c', 'r', 'e', 't', 's' };
constexpr auto fileMagicLen = sizeof(fileMagic) / sizeof(fileMagic[0]);

KSecretsFile::KSecretsFile()
    : readFile_(-1)
    , writeFile_(-1)
    , locked_(false)
    , empty_(true)
    , eof_(false)
{
    memset(&fileHead_, 0, sizeof(fileHead_));
}

KSecretsFile::~KSecretsFile()
{
    if (readFile_ != -1) {
        closeFile(readFile_);
        readFile_ = -1;
    }
    if (writeFile_ != -1) {
        closeFile(writeFile_);
        writeFile_ = -1;
    }
}

void KSecretsFile::closeFile(int f)
{
    auto r = close(f);
    if (r == -1) {
        syslog(KSS_LOG_ERR, "ksecrets: system return erro upon secrets "
                            "file close: %d (%m)",
            errno);
        syslog(KSS_LOG_ERR, "ksecrets: the secrets file might now be "
                            "corrup because of the previous error");
    }
}

int KSecretsFile::create(const std::string& path)
{
    // TODO modify this to use POSIX functions write and open instead of fwrite and fopen
    // TODO add the SecretsEOF structure at the end of the empty file
    FILE* f = fopen(path.c_str(), "w");
    if (f == nullptr) {
        return errno;
    }

    FileHeadStruct emptyFileData;
    memcpy(emptyFileData.magic_, fileMagic, fileMagicLen);

    // FIXME should we put this kind of call in gcrypt-dedicated file?
    gcry_randomize(emptyFileData.salt_, SALT_SIZE, GCRY_STRONG_RANDOM);
    gcry_randomize(emptyFileData.iv_, IV_SIZE, GCRY_STRONG_RANDOM);

    int res = 0;
    if (fwrite(&emptyFileData, 1, sizeof(emptyFileData), f) != sizeof(emptyFileData)) {
        res = ferror(f);
    }
    fclose(f);
    return res;
}

bool KSecretsFile::save()
{
    assert(writeFile_ == -1);
    assert(readFile_ != -1);

    const char* pattern = "ksecrets-XXXXXX";
    writeFile_ = mkostemp((char*)pattern, O_SYNC);
    if (writeFile_ == -1) {
        syslog(KSS_LOG_ERR, "ksecrets: cannot create temp file for save errno=%d (%m)", errno);
        return false;
    }

    if (!mac_.reset()) {
        syslog(KSS_LOG_ERR, "ksecrets: cannot setup MAC calculation");
        return false;
    }

    if (write(&fileHead_, sizeof(fileHead_))) {
        syslog(KSS_LOG_ERR, "ksecrets: cannot write file header errno=%d (%m)", errno);
        return false;
    }

    bool res = true;
    long count = std::count_if(entities_.cbegin(), entities_.cend(), [](SecretsEntityPtr) { return true; });
    if (!write(&count, sizeof(count))) {
        syslog(KSS_LOG_ERR, "ksecrets: cannot write entity count errno=%d (%m)", errno);
        return false;
    }

    for (SecretsEntityPtr entity : entities_) {
        if (!saveEntity(entity)) {
            return false;
        }
    }

    if (!mac_.write(*this)) {
        syslog(KSS_LOG_ERR, "ksecrets: cannot write file MAC errno=%d (%m)", errno);
        return false;
    }

    char lnpath[PATH_MAX];
    snprintf(lnpath, PATH_MAX, "/proc/self/fd/%d", writeFile_);
    char tempWrittenFile[PATH_MAX];
    auto rlink = readlink(lnpath, tempWrittenFile, PATH_MAX - 1);
    if (rlink == -1) {
        syslog(KSS_LOG_ERR, "ksecrets: cannot get written temp file path! errno=%d (%m)", errno);
        return false;
    }

    if (!res) {
        closeFile(writeFile_);
        writeFile_ = -1;
        return false;
    }

    // OK that worked, now replace the current file with the temp file
    return backupAndReplaceWithWritten(tempWrittenFile);
}

bool KSecretsFile::saveEntity(SecretsEntityPtr entity)
{
    SecretsEntity::EntityType et = entity->getType();
    if (!write(&et, sizeof(et))) {
        return false;
    }
    if (!entity->write(*this)) {
        syslog(KSS_LOG_ERR, "ksecrets: cannot save entity to file errnor=%d (%m)", errno);
        return false;
    }
    else
        return true;
}

bool KSecretsFile::backupAndReplaceWithWritten(const char* tempFilePath)
{
    closeFile(readFile_);
    char backupPath[PATH_MAX];
    snprintf(backupPath, PATH_MAX, "%s.bkp", filePath_.c_str());
    auto rres = rename(filePath_.c_str(), backupPath);
    if (rres == -1) {
        syslog(KSS_LOG_ERR, "ksecrets: cannot backup the secrets file errno=%d (%m)", errno);
        return false;
    }
    // FIXME this code is the same sequence as in KSecretsStorePrivate when opening so factor it somehow

    return true;
}

bool KSecretsFile::readAndCheck()
{
    assert(readFile_ != -1);
    if (empty_)
        return true; // MAC of empty files is always OK
    if (!entities_.empty()) {
        entities_.clear();
    }
    mac_.reset();

    long entityCount = 0;
    if (::read(readFile_, &entityCount, sizeof(entityCount)) < (ssize_t)sizeof(entityCount)) {
        syslog(KSS_LOG_ERR, "ksecrets: cannot read the secrets file errno=%d (%m)", errno);
        return false;
    }

    while (entityCount--) {
        if (!readNextEntity()) {
            return false;
        }
    }

    return true;
}

bool KSecretsFile::readNextEntity()
{
    SecretsEntity::EntityType et;
    if (!read(&et, sizeof(et))) {
        syslog(KSS_LOG_ERR, "ksecrets: cannot read next entities type");
        return false;
    }
    SecretsEntityPtr entity = SecretsEntityFactory::createInstance(et);
    if (!entity->read(*this)) {
        syslog(KSS_LOG_ERR, "ksecrets: cannot read next entity");
        return false;
    }
    entities_.emplace_back(entity);
    return true;
}

void KSecretsFile::setup(const std::string& path, bool readOnly)
{
    filePath_ = path;
    readOnly_ = readOnly;
}

bool KSecretsFile::open()
{
    readFile_ = ::open(filePath_.c_str(), O_DSYNC | O_NOATIME | O_NOFOLLOW);
    return readFile_ != -1;
}

bool KSecretsFile::lock()
{
    return flock(readFile_, LOCK_EX) != -1;
    locked_ = true;
}

bool KSecretsFile::readHeader()
{
    auto rres = ::read(readFile_, &fileHead_, sizeof(fileHead_));
    char dummyBuffer[4];
    auto bytes = sizeof(dummyBuffer) / sizeof(dummyBuffer[0]);
    auto eoftest = ::read(readFile_, dummyBuffer, bytes);
    if (eoftest == 0) {
        // we are at EOF already, so file is empty
        empty_ = true;
        eof_ = true;
    }
    else {
        if (-1 == lseek(readFile_, -bytes, SEEK_CUR)) {
            setFailState(errno);
            return false;
        }
        empty_ = false;
    }
    return rres == sizeof(fileHead_);
}

bool KSecretsFile::checkMagic()
{
    if (memcmp(fileHead_.magic_, fileMagic, fileMagicLen) != 0) {
        return false;
    }
    return true;
}

bool KSecretsFile::write(size_t s) { return write(&s, sizeof(size_t)); }

bool KSecretsFile::write(const void* buf, size_t len)
{
    assert(writeFile_ != -1);
    auto wres = ::write(writeFile_, buf, len);
    if (wres < 0) {
        syslog(KSS_LOG_ERR, "ksecrets: cannot write to file errno=%d (%m)", errno);
        return setFailState(errno);
    }
    if (static_cast<size_t>(wres) < len) {
        // no more space left on the file system
        // FIXME we should prevent such an event by keeping versions of the secrets file
        // TODO manage secrets file versions to prevent this kind of problem
        syslog(KSS_LOG_ERR, "ksecrets: cannot write all data to file. Disk full?");
        return setFailState(ENOSPC);
    }
    return true;
}

bool KSecretsFile::read(size_t& s) { return read(&s, sizeof(s)); }

bool KSecretsFile::read(void* buf, size_t len)
{
    if (eof_)
        return false;
    auto rres = ::read(readFile_, buf, len);
    if (rres < 0)
        return setFailState(errno);
    if (static_cast<size_t>(rres) < len)
        return setEOF(); // are we @ EOF?
    return true;
}

KSecretsFile::DirCollectionResult KSecretsFile::dirCollections()
{
    DirCollectionResult res;
    res.first = false;
    res.second = nullptr;

    if (empty_) {
        return res;
    }

    if (readDirectory()) {
        res.first = true;
        res.second = &directory_;
    }

    return res;
}

bool KSecretsFile::readDirectory()
{
    if (empty_)
        return false; // file is empty, don't even attempt read
    return directory_.read(*this);
}

SecretsCollectionPtr KSecretsFile::createCollection(const std::string& collName) noexcept
{
    auto newColl = std::make_shared<SecretsCollection>();
    newColl->setName(collName);
    entities_.emplace_front(newColl);
    if (save())
        return std::dynamic_pointer_cast<SecretsCollection>(entities_.front());
    else
        return SecretsCollectionPtr();
}

KSecretsFile::MAC::MAC() {}
KSecretsFile::MAC::~MAC() {}
bool KSecretsFile::MAC::init(const char* key, size_t keyLen, const void* iv, size_t ivlen) noexcept
{
    // TODO
    return false;
}
bool KSecretsFile::MAC::reset() noexcept
{
    // TODO
    return false;
}
bool KSecretsFile::MAC::update(const void* buffer, size_t len) noexcept
{
    // TODO
    return false;
}
bool KSecretsFile::MAC::write(KSecretsFile&)
{
    // TODO
    return false;
}

bool KSecretsFile::MAC::check(KSecretsFile&)
{
    // TODO
    return false;
}

// vim: tw=220:ts=4
