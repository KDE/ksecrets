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
#ifndef KSECRETS_FILE_H
#define KSECRETS_FILE_H

#include "ksecrets_data.h"
#include "ksecrets_device.h"
#include "crypting_engine.h"

#include <memory>
#include <deque>
#include <algorithm>
#define GCRPYT_NO_DEPRECATED
#include <gcrypt.h>

/**
 * @brief This is the secrets file format handling class
 *
 * The file format strives to be multiplatform compatible. The aim is to be able to just copy this file from one
 * platform to another and just open it with the corresponding library or application.
 *
 * The file has three main sections:
 *   FileHead
 *   Encrypted items
 *   Checksum
 *
 * The file header is described by the @ref FileHeadStruct. This structure contains the file format magic string
 * followed by the salt and the initialization vector needed during libgcrypt setup.
 *
 * The actual data follows the file header and is encrypted with libgcrypt using a pair of keys derived by from user's
 * password using libgcrypt. The encryption details are handled by the @ref CryptingEngine. The serialization of the
 * items is taken care of by the @ref SecretsItem class and it's inheritors. The serialization is done in ASCII in
 * order to avoid endian issues.
 *
 * The end of the file containa the checksum. That'a also handled by the @ref SecretsItem base class.
 *
 * @sa SecretsItem, CryptingEngine
 */
class KSecretsFile : public KSecretsDevice {
    using base_class = KSecretsDevice;

public:
    KSecretsFile();
    ~KSecretsFile();

    struct FileHeadStruct {
        char magic_[9];
        unsigned char salt_[CryptingEngine::SALT_SIZE];
        unsigned char iv_[CryptingEngine::IV_SIZE];
    };

    enum class OpenStatus { Ok, CannotOpenFile, CannotLockFile, CannotReadHeader, UnknownHeader, CryptEngineError, EntitiesReadError, IntegrityCheckFailed };

    int create(const std::string& path) noexcept;
    void setup(const std::string& path, bool readOnly) noexcept;
    OpenStatus openAndCheck() noexcept;
    bool open() noexcept;
    bool openSaveTempFile() noexcept;
    bool saveMac() noexcept;
    bool readEntities() noexcept;
    bool readCheckMac() noexcept;
    bool readNextEntity() noexcept;
    bool save() noexcept;
    bool saveEntity(SecretsEntityPtr);
    bool lock() noexcept;
    bool readHeader() noexcept;
    bool writeHeader() noexcept;
    bool checkMagic() noexcept;
    const unsigned char* salt() const noexcept { return fileHead_.salt_; }
    virtual const unsigned char* iv() const noexcept override { return fileHead_.iv_; }
    virtual bool read(void* buf, size_t count) noexcept override;
    int errnumber() const noexcept { return errno_; }
    bool eof() const noexcept { return eof_; }
    virtual bool write(const void* buf, size_t count) noexcept override;

    template <class E> bool emplace_entity(E&& e) noexcept
    {
        entities_.emplace_back(e);
        return save();
    }
    bool remove_entity(SecretsEntityPtr);
    template <class P> SecretsEntityPtr find_entity(P pred)
    {
        Entities::iterator pos = std::find_if(entities_.begin(), entities_.end(), pred);
        if (pos != entities_.end()) {
            return *pos;
        }
        else
            return SecretsEntityPtr();
    }

private:
    bool setFailState(int err, bool retval = false) noexcept
    {
        errno_ = err;
        return retval; // wo do like this so this function could end other methods with an elegant return setFailState(errno);
    }
    bool setEOF() noexcept
    {
        eof_ = true;
        return false; // this work the same as setFailState
    }
    bool decryptEntity(SecretsEntity&) noexcept;
    void closeFile(int&) noexcept;
    bool backupAndReplaceWithWritten(const char*) noexcept;

    using Entities = std::deque<SecretsEntityPtr>;

    std::string filePath_;
    int readFile_;
    int writeFile_;
    bool locked_;
    bool readOnly_;
    FileHeadStruct fileHead_;
    Entities entities_;
    int errno_;
    bool eof_;
    CryptingEngine::MAC mac_;
};

#endif
// vim: tw=220:ts=4
