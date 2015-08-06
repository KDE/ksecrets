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

#include "ksecrets_backend.h"
#include "ksecrets_backend_p.h"

#include <future>
#include <thread>
#include <sys/stat.h>

KSecretsBackendPrivate::KSecretsBackendPrivate(KSecretsBackend* b)
    : b_(b)
{
}

KSecretsBackend::KSecretsBackend()
    : d(new KSecretsBackendPrivate(this))
{
}

KSecretsBackend::~KSecretsBackend() = default;

std::future<KSecretsBackend::OpenResult> KSecretsBackend::open(
    std::string&& path, bool readonly /* =true */) noexcept
{
    // sanity checks
    if (path.empty()) {
        return std::async(std::launch::deferred, []() {
            return OpenResult{ OpenResult::OpenStatus::NoPathGiven, 0 };
        });
    }

    struct stat buf;
    if (stat(path.c_str(), &buf) != 0) {
        auto err = errno;
        return std::async(std::launch::deferred, [err]() {
            return OpenResult{ OpenResult::OpenStatus::SystemError, errno };
        });
    }

    // now we can proceed
    auto localThis = this;
    if (!readonly) {
        return std::async(std::launch::async,
            [localThis, path]() { return localThis->d->lock_open(path); });
    }
    else {
        return std::async(std::launch::deferred,
            [localThis, path]() { return localThis->d->open(path); });
    }
}

KSecretsBackend::OpenResult KSecretsBackendPrivate::lock_open(
    const std::string& path)
{
    // TODO
    return { KSecretsBackend::OpenResult::OpenStatus::Good, 0};
}

KSecretsBackend::OpenResult KSecretsBackendPrivate::open(
    const std::string& path)
{
    // TODO
    return { KSecretsBackend::OpenResult::OpenStatus::Good, 0};
}
