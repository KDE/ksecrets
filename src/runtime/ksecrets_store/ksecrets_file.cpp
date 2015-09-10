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
    , eof_(false)
{
    memset(&fileHead_, 0, sizeof(fileHead_));
}

KSecretsFile::~KSecretsFile()
{
    if (readFile_ != -1) {
        closeFile(readFile_);
    }
    if (writeFile_ != -1) {
        closeFile(writeFile_);
    }
}

void KSecretsFile::closeFile(int& f) noexcept
{
    auto r = close(f);
    if (r == -1) {
        syslog(KSS_LOG_ERR, "ksecrets: system return erro upon secrets file close: %d", errno);
        syslog(KSS_LOG_ERR, "ksecrets: the secrets file might now be corrupt because of the previous error");
    }
    f = -1;
}

int KSecretsFile::create(const std::string& path) noexcept
{
    int fd = ::creat(path.c_str(), S_IRUSR | S_IWUSR);
    if (fd == -1) {
        return errno;
    }

    MAC mac;
    FileHeadStruct emptyFileData;
    memcpy(emptyFileData.magic_, fileMagic, fileMagicLen);

    // FIXME should we put this kind of call in gcrypt-dedicated file?
    gcry_randomize(emptyFileData.salt_, SALT_SIZE, GCRY_STRONG_RANDOM);
    gcry_randomize(emptyFileData.iv_, IV_SIZE, GCRY_STRONG_RANDOM);

    int res = 0;
    if (::write(fd, &emptyFileData, sizeof(emptyFileData)) != sizeof(emptyFileData)) {
        res = errno;
    }
    long count = 0; // this file has 0 items in it
    if (::write(fd, &count, sizeof(count)) != sizeof(count)) {
        res = errno;
    }
    ::close(fd);
    return res;
}

bool KSecretsFile::save() noexcept
{
    assert(writeFile_ == -1);

    if (!openSaveTempFile()) {
        return false;
    }

    if (!writeHeader()) {
        syslog(KSS_LOG_ERR, "ksecrets: cannot write file header errno=%d", errno);
        closeFile(writeFile_);
        return false;
    }

    size_t count = std::count_if(entities_.cbegin(), entities_.cend(), [](SecretsEntityPtr) { return true; });
    if (!base_class::template write(count)) {
        syslog(KSS_LOG_ERR, "ksecrets: cannot write entity count errno=%d", errno);
        closeFile(writeFile_);
        return false;
    }

    for (SecretsEntityPtr entity : entities_) {
        if (!saveEntity(entity)) {
            closeFile(writeFile_);
            return false;
        }
    }

    if (!saveMac()) return false;

    char lnpath[PATH_MAX];
    snprintf(lnpath, PATH_MAX, "/proc/self/fd/%d", writeFile_);
    char tempWrittenFile[PATH_MAX];
    auto rlink = readlink(lnpath, tempWrittenFile, PATH_MAX - 1);
    if (rlink == -1) {
        syslog(KSS_LOG_ERR, "ksecrets: cannot get written temp file path! errno=%d", errno);
        return false;
    }

    closeFile(writeFile_);

    // OK that worked, now replace the current file with the temp file
    return backupAndReplaceWithWritten(tempWrittenFile);
}

bool KSecretsFile::openSaveTempFile() noexcept {
    const char* pattern = "ksecrets-XXXXXX";
    char* tmpfilename = new char[strlen(pattern) + 1];
    strcpy(tmpfilename, pattern);
    writeFile_ = mkostemp(tmpfilename, O_SYNC);
    if (writeFile_ == -1) {
        syslog(KSS_LOG_ERR, "ksecrets: cannot create temp file for save errno=%d", errno);
        return false;
    }
    else {
        syslog(KSS_LOG_INFO, "ksecrets: saving to temporary file %s", tmpfilename);
        delete[] tmpfilename;
    }

    if (!mac_.reset()) {
        syslog(KSS_LOG_ERR, "ksecrets: cannot setup MAC calculation");
        return false;
    }

    return true;
}

bool KSecretsFile::saveMac() noexcept {
    if (!mac_.write(*this)) {
        syslog(KSS_LOG_ERR, "ksecrets: cannot write file MAC errno=%d", errno);
        closeFile(writeFile_);
        return false;
    }
    return true;
}

bool KSecretsFile::saveEntity(SecretsEntityPtr entity)
{
    auto et = (std::uint8_t)entity->getType();
    if (!base_class::template write(et)) {
        return false;
    }
    if (!entity->write(*this)) {
        syslog(KSS_LOG_ERR, "ksecrets: cannot save entity to file errnor=%d", errno);
        return false;
    }
    else
        return true;
}

bool KSecretsFile::backupAndReplaceWithWritten(const char* tempFilePath) noexcept
{
    closeFile(readFile_);

    char backupPath[PATH_MAX];
    snprintf(backupPath, PATH_MAX, "%s.bkp", filePath_.c_str());
    auto rres = rename(filePath_.c_str(), backupPath);
    if (rres == -1) {
        syslog(KSS_LOG_ERR, "ksecrets: cannot backup the secrets file errno=%d", errno);
        return false;
    }

    rres = rename(tempFilePath, filePath_.c_str());
    if (rres == -1) {
        syslog(KSS_LOG_ERR, "ksecrets: cannot move temp file to current secrets file errno=%d", errno);
        return false;
    }

    if (openAndCheck() != OpenStatus::Ok) {
        syslog(KSS_LOG_ERR, "ksecrets: cannot reopen file");
        return false;
    }
    return true;
}

bool KSecretsFile::readEntities() noexcept
{
    assert(readFile_ != -1);
    if (!entities_.empty()) {
        entities_.clear();
    }

    size_t entityCount = 0;
    if (!base_class::template read(entityCount)) {
        syslog(KSS_LOG_ERR, "ksecrets: cannot read the secrets file errno=%d", errno);
        return false;
    }

    while (entityCount--) {
        if (!readNextEntity()) {
            return false;
        }
    }
    return true;
}

bool KSecretsFile::readCheckMac() noexcept {
    if (!mac_.check(*this)) {
        syslog(KSS_LOG_ERR, "ksecrets: MAC check error, the file is corrupted or someone tampered with it");
        return false;
    }
    return true;
}

bool KSecretsFile::readNextEntity() noexcept
{
    std::uint8_t et;
    if (!base_class::template read(et)) {
        syslog(KSS_LOG_ERR, "ksecrets: cannot read next entities type");
        return false;
    }
    SecretsEntityPtr entity = SecretsEntityFactory::createInstance((SecretsEntity::EntityType)et);
    if (!entity->read(*this)) {
        syslog(KSS_LOG_ERR, "ksecrets: cannot read next entity");
        return false;
    }
    entities_.emplace_back(entity);
    return true;
}

void KSecretsFile::setup(const std::string& path, bool readOnly) noexcept
{
    filePath_ = path;
    readOnly_ = readOnly;
}

KSecretsFile::OpenStatus KSecretsFile::openAndCheck() noexcept
{
    if (!open()) {
        syslog(KSS_LOG_ERR, "ksecrets: failed to open file %s", filePath_.c_str());
        return OpenStatus::CannotOpenFile;
    }
    if (locked_ && !lock()) {
        syslog(KSS_LOG_ERR, "ksecrets: cannot lock file %s", filePath_.c_str());
        return OpenStatus::CannotLockFile;
    }
    mac_.reset();
    if (!readHeader()) {
        syslog(KSS_LOG_ERR, "ksecrets: failed to read header from file %s", filePath_.c_str());
        return OpenStatus::CannotReadHeader;
    }
    if (!checkMagic()) {
        syslog(KSS_LOG_ERR, "ksecrets: magic check failed for file %s", filePath_.c_str());
        return OpenStatus::UnknownHeader;
    }
    if (!readEntities()) {
        syslog(KSS_LOG_ERR, "ksecrets: cannot read entities from file %s", filePath_.c_str());
        return OpenStatus::EntitiesReadError;
    }
    if (!readCheckMac()) {
        syslog(KSS_LOG_ERR, "ksecrets: integrity check failed for file %s", filePath_.c_str());
        return OpenStatus::IntegrityCheckFailed;
    }
    return OpenStatus::Ok;
}

bool KSecretsFile::open() noexcept
{
    readFile_ = ::open(filePath_.c_str(), O_DSYNC | O_NOATIME | O_NOFOLLOW);
    return readFile_ != -1;
}

bool KSecretsFile::lock() noexcept
{
    bool res = flock(readFile_, LOCK_EX) != -1;
    locked_ = true;
    return res;
}

bool KSecretsFile::readHeader() noexcept { return read(&fileHead_, sizeof(fileHead_)); }

bool KSecretsFile::writeHeader() noexcept { return write(&fileHead_, sizeof(fileHead_)); }

bool KSecretsFile::checkMagic() noexcept
{
    if (memcmp(fileHead_.magic_, fileMagic, fileMagicLen) != 0) {
        return false;
    }
    return true;
}

bool KSecretsFile::write(const void* buf, size_t len) noexcept
{
    assert(writeFile_ != -1);
    auto wres = ::write(writeFile_, buf, len);
    if (wres < 0) {
        syslog(KSS_LOG_ERR, "ksecrets: cannot write to file errno=%d", errno);
        return setFailState(errno);
    }
    if (static_cast<size_t>(wres) < len) {
        // no more space left on the file system
        // FIXME we should prevent such an event by keeping versions of the secrets file
        // TODO manage secrets file versions to prevent this kind of problem
        syslog(KSS_LOG_ERR, "ksecrets: cannot write all data to file. Disk full?");
        return setFailState(ENOSPC);
    }
    syslog(KSS_LOG_INFO, "ksecrets: W offset %ld", lseek(writeFile_, 0, SEEK_CUR));
    return mac_.update(buf, len);
}

bool KSecretsFile::read(void* buf, size_t len) noexcept
{
    if (eof_)
        return false;
    auto rres = ::read(readFile_, buf, len);
    if (rres < 0)
        return setFailState(errno);
    if (static_cast<size_t>(rres) < len)
        return setEOF(); // are we @ EOF?
    syslog(KSS_LOG_INFO, "ksecrets: R offset %ld", lseek(readFile_, 0, SEEK_CUR));
    return mac_.update(buf, len);
}

bool KSecretsFile::remove_entity(SecretsEntityPtr entity)
{
    Entities::iterator pos = std::find(entities_.begin(), entities_.end(), entity);
    if (pos != entities_.end()) {
        entities_.erase(pos);
        return true;
    }
    else
        return false;
}

KSecretsFile::MAC::MAC()
    : ignore_updates_(false)
{
    auto gcryerr = gcry_mac_open(&hd_, GCRY_MAC_HMAC_SHA512, 0, 0);
    if (gcryerr) {
        syslog(KSS_LOG_ERR, "opening MAC algo failed: code 0x%0x: %s/%s", gcryerr, gcry_strsource(gcryerr), gcry_strerror(gcryerr));
        valid_ = false;
    }
    else {
        valid_ = true;
    }
}

KSecretsFile::MAC::~MAC()
{
    if (valid_) {
        gcry_mac_close(hd_);
    }
}

bool KSecretsFile::MAC::init(const char* key, size_t keyLen, const void* iv, size_t ivlen) noexcept
{
    if (!valid_) {
        syslog(KSS_LOG_ERR, "ksecrets: MAC object is not valid");
        return false;
    }
    auto gcryerr = gcry_mac_setiv(hd_, iv, ivlen);
    if (gcryerr) {
        syslog(KSS_LOG_ERR, "setting MAC IV failed: code 0x%0x: %s/%s", gcryerr, gcry_strsource(gcryerr), gcry_strerror(gcryerr));
        return false;
    }
    gcryerr = gcry_mac_setkey(hd_, key, keyLen);
    if (gcryerr) {
        syslog(KSS_LOG_ERR, "setting MAC key failed: code 0x%0x: %s/%s", gcryerr, gcry_strsource(gcryerr), gcry_strerror(gcryerr));
        return false;
    }
    return true;
}

bool KSecretsFile::MAC::reset() noexcept
{
    auto gcryerr = gcry_mac_reset(hd_);
    if (gcryerr) {
        syslog(KSS_LOG_ERR, "resetting MAC failed: code 0x%0x: %s/%s", gcryerr, gcry_strsource(gcryerr), gcry_strerror(gcryerr));
        return false;
    }

    return true;
}

bool KSecretsFile::MAC::update(const void* buffer, size_t len) noexcept
{
    if (!valid_)
        return false;
    if (ignore_updates_)
        return true;
    auto gcryerr = gcry_mac_write(hd_, buffer, len);
    if (gcryerr) {
        syslog(KSS_LOG_ERR, "updating MAC failed: code 0x%0x: %s/%s", gcryerr, gcry_strsource(gcryerr), gcry_strerror(gcryerr));
        return false;
    }

    return true;
}

bool KSecretsFile::MAC::write(KSecretsFile& file)
{
    char buffer[512]; // on purpose allocate more
    size_t len = sizeof(buffer) / sizeof(buffer[0]);
    auto gcryerr = gcry_mac_read(hd_, (void*)buffer, &len);
    if (gcryerr) {
        syslog(KSS_LOG_ERR, "obtaining resulting MAC failed: code 0x%0x: %s/%s", gcryerr, gcry_strsource(gcryerr), gcry_strerror(gcryerr));
        return false;
    }
    // NOTE this will also update the MAC but we're no longer interested
    if (!file.write(&len, sizeof(len))) {
        syslog(KSS_LOG_ERR, "Cannot write calculated MAC length");
        return false;
    }
    if (!file.write(buffer, len)) {
        syslog(KSS_LOG_ERR, "Cannot write calculated MAC value");
        return false;
    }

    return true;
}

template <bool INIT>
struct AutoBool {
    explicit AutoBool(bool& b)
        : b_(b)
    {
        b_ = INIT;
    }
    ~AutoBool() { b_ = !INIT; }
    bool& b_;
};

bool KSecretsFile::MAC::check(KSecretsFile& file)
{
    AutoBool<true> ignoreUpdates(ignore_updates_); // prevent reading MAC from the file updating us, if not we'll be unable to check
    size_t len = 0;
    if (!file.read(&len, sizeof(len))) {
        syslog(KSS_LOG_ERR, "Cannot read MAC len");
        return false;
    }
    char* buffer = new char[len];
    if (buffer == nullptr) {
        syslog(KSS_LOG_ERR, "Cannot allocate MAC buffer");
        return false;
    }
    if (!file.read(buffer, len)) {
        syslog(KSS_LOG_ERR, "Cannot read MAC value");
        return false;
    }
    auto gcryerr = gcry_mac_verify(hd_, buffer, len);
    delete[] buffer;
    if (gcryerr) {
        syslog(KSS_LOG_ERR, "MAC check failed: code 0x%0x: %s/%s", gcryerr, gcry_strsource(gcryerr), gcry_strerror(gcryerr));
        return false;
    }

    return true;
}

// vim: tw=220:ts=4
