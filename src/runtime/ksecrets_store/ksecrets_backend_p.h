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
#ifndef KSECRETSBACKEND_P_H
#define KSECRETSBACKEND_P_H

#include "ksecrets_backend.h"

class KSecretsBackendPrivate {
public:
    KSecretsBackendPrivate() = delete;
    explicit KSecretsBackendPrivate(KSecretsBackend*);

    KSecretsBackend::OpenResult lock_open(const std::string&);
    KSecretsBackend::OpenResult open(const std::string&);
    using open_action
        = std::function<KSecretsBackend::OpenResult(const std::string&)>;
    KSecretsBackend::OpenResult createFileIfNeededThenDo(
        const std::string&, bool, open_action);
    int createFile(const std::string&);
    const char* salt() const;

    constexpr static auto IV_SIZE = 32;
    struct FileHeadStruct {
        char magic[9];
        char salt[KSecretsBackend::SALT_SIZE];
        char iv[IV_SIZE];
    };

    KSecretsBackend::OpenResult setOpenStatus(KSecretsBackend::OpenResult);
    bool isOpen() const noexcept { return KSecretsBackend::OpenStatus::Good == openStatus_.status_; }

    KSecretsBackend* b_;
    FILE* file_;
    FileHeadStruct fileHead_;
    KSecretsBackend::OpenResult openStatus_;
};

#endif
// vim: tw=220:ts=4
