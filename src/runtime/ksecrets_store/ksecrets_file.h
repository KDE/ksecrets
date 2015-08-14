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

#include <memory>

class KSecretsFile {
public:
    KSecretsFile();
    ~KSecretsFile();

    constexpr static auto IV_SIZE = 32;
    constexpr static auto SALT_SIZE = 56;
    struct FileHeadStruct {
        char magic[9];
        char salt[SALT_SIZE];
        char iv[IV_SIZE];
    };

    int create(const std::string &path);
    void setup(const std::string &path, bool readOnly) noexcept;
    bool open() noexcept;
    bool lock() noexcept;
    bool readHeader() noexcept;
    bool checkMagic() noexcept;
    const char *salt() const noexcept { return fileHead_.salt; }

private:
    std::string filePath_;
    int file_;
    bool locked_;
    bool readOnly_;
    FileHeadStruct fileHead_;
};

#endif
// vim: tw=220:ts=4
