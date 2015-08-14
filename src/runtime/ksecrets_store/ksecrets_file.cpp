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
    FILE* f = fopen(path.c_str(), "w");
    if (f == nullptr) {
        return errno;
    }

    FileHeadStruct emptyFileData;
    memcpy(emptyFileData.magic, fileMagic, fileMagicLen);

    // FIXME should we put this kind of call in gcrypt-dedicated file?
    gcry_randomize(emptyFileData.salt, SALT_SIZE, GCRY_STRONG_RANDOM);
    gcry_randomize(emptyFileData.iv, IV_SIZE, GCRY_STRONG_RANDOM);

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

bool KSecretsFile::readHeader() { return read(file_, &fileHead_, sizeof(fileHead_)) == sizeof(fileHead_); }

bool KSecretsFile::checkMagic()
{
    if (memcmp(fileHead_.magic, fileMagic, fileMagicLen) != 0) {
        return false;
    }
    return true;
}
// vim: tw=220:ts=4
