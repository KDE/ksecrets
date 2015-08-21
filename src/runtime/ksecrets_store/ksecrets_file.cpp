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
#include <string.h>
#define GCRPYT_NO_DEPRECATED
#include <gcrypt.h>

char fileMagic[] = { 'k', 's', 'e', 'c', 'r', 'e', 't', 's' };
constexpr auto fileMagicLen = sizeof(fileMagic) / sizeof(fileMagic[0]);

KSecretsFile::KSecretsFile()
    : file_(-1)
    , locked_(false)
    , empty_(true)
    , eof_(false)
{
    memset(&fileHead_, 0, sizeof(fileHead_));
}

KSecretsFile::~KSecretsFile()
{
    if (file_ != -1) {
        auto r = close(file_);
        if (r == -1) {
            syslog(KSS_LOG_ERR, "ksecrets: system return erro upon secrets "
                                "file close: %d (%m)",
                errno);
            syslog(KSS_LOG_ERR, "ksecrets: the secrets file might now be "
                                "corrup because of the previous error");
        }
        file_ = -1;
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

void KSecretsFile::setup(const std::string& path, bool readOnly)
{
    filePath_ = path;
    readOnly_ = readOnly;
}

bool KSecretsFile::open()
{
    file_ = ::open(filePath_.c_str(), O_DSYNC | O_NOATIME | O_NOFOLLOW);
    return file_ != -1;
}

bool KSecretsFile::lock()
{
    return flock(file_, LOCK_EX) != -1;
    locked_ = true;
}

bool KSecretsFile::readHeader()
{
    auto rres = ::read(file_, &fileHead_, sizeof(fileHead_));
    char dummyBuffer[4];
    auto bytes = sizeof(dummyBuffer) / sizeof(dummyBuffer[0]);
    auto eoftest = ::read(file_, dummyBuffer, bytes);
    if (eoftest == 0) {
        // we are at EOF already, so file is empty
        empty_ = true;
        eof_ = true;
    }
    else {
        if (-1 == lseek(file_, -bytes, SEEK_CUR)) {
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

int KSecretsFile::checkMAC() const
{
    if (empty_)
        return 0; // MAC of empty files is always OK
    // const char *macKey = kss_read_mac_key();
    // gcry_error_t err;
    // gcry_mac_hd_t hd;
    // err = gcry_mac_open(&hd, GCRY_MAC_HMAC_SHA512, 0, NULL);
    // if (!err) return gcry_err_code_to_errno(gcry_err_code(err));
    //
    // err = gcry_mac_setkey(hd, );
    // TODO
    return -1;
}

bool KSecretsFile::write(size_t s) { return write(&s, sizeof(size_t)); }

bool KSecretsFile::write(const void* buf, size_t len)
{
    auto wres = ::write(file_, buf, len);
    if (wres < 0) {
        return setFailState(errno);
    }
    if (static_cast<size_t>(wres) < len) {
        // no more space left on the file system
        // FIXME we should prevent such an event by keeping versions of the secrets file
        // TODO manage secrets file versions to prevent this kind of problem
        return setFailState(ENOSPC);
    }
    return true;
}

bool KSecretsFile::read(size_t& s) { return read(&s, sizeof(s)); }

bool KSecretsFile::read(void* buf, size_t len)
{
    if (eof_)
        return false;
    auto rres = ::read(file_, buf, len);
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
        return false; // file is empty, don't event attempt read
    return directory_.read(*this);
}

SecretsCollectionPtr KSecretsFile::createCollection(const std::string& collName) noexcept
{
    auto newColl = std::make_shared<SecretsCollection>();
    newColl->setName(collName);
    entities_.emplace_front(newColl);
    return std::dynamic_pointer_cast<SecretsCollection>(entities_.front());
}

// vim: tw=220:ts=4
