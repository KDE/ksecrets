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

#include <memory>
#include <deque>
#include <algorithm>
#define GCRPYT_NO_DEPRECATED
#include <gcrypt.h>

/**
 * @brief This is the secrets file format handling class
 */
class KSecretsFile : public KSecretsDevice {
    using base_class = KSecretsDevice;

public:
    KSecretsFile();
    ~KSecretsFile();

    struct FileHeadStruct {
        char magic_[9];
        char salt_[SALT_SIZE];
        char iv_[IV_SIZE];
    };

    enum class OpenStatus { Ok, CannotOpenFile, CannotLockFile, CannotReadHeader, UnknownHeader, EntitiesReadError, IntegrityCheckFailed };

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
    const char* salt() const noexcept { return fileHead_.salt_; }
    virtual const char* iv() const noexcept override { return fileHead_.iv_; }
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

    struct MAC {
        MAC();
        ~MAC();
        bool init(const char* key, size_t keyLen, const void* iv, size_t ivlen) noexcept;
        bool reset() noexcept;
        bool update(const void* buffer, size_t len) noexcept;
        bool write(KSecretsFile&);
        bool check(KSecretsFile&);

        bool valid_;
        gcry_mac_hd_t hd_;

    private:
        bool ignore_updates_;
    };

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
    MAC mac_;
};

#endif
// vim: tw=220:ts=4
