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

#include <ksecrets_backend.h>

#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QLoggingCategory>

QLoggingCategory logCat("pam_ksecrets");
KSharedConfigPtr sharedConfig;

#define CONFIGNAME "ksecretsrc"

extern "C" {
/*
 * @note even if you could use QDir::home() to retrieve user's home directory, this function
 * may be called in contexts where current user information is not yet available
 */
const char* prepare_secret_file_location(const char* home_dir)
{
    qCDebug(logCat) << "prepare_secret_file_location(" << home_dir << ")";
    const char* defaultDataPath = QStandardPaths::isTestModeEnabled() ? ".qttest/ksecrets.data" : ".config/kde.org/ksecrets.data";
    sharedConfig = KSharedConfig::openConfig(QLatin1String(CONFIGNAME));
    QString secretsLocation = sharedConfig->group(QLatin1String("General")).readEntry("SecretsPath", defaultDataPath);

    QDir userDir(home_dir); // reasonably assume user dir always exist
    QString secretsPath = userDir.absoluteFilePath(secretsLocation);
    QFileInfo secretsInfo(secretsPath);
    if (!secretsInfo.exists()) {
        qCDebug(logCat) << "secrets file not found, attempting parent directory creation";
        QString basePath = secretsInfo.dir().absolutePath();
        if (!userDir.mkpath(basePath)) {
            qCDebug(logCat) << "error attempting path creation: " << basePath << " error=" << errno << " (" << strerror(errno) << ")";
            return NULL;
        }
    }

    return secretsPath.toUtf8().constData();
}

const char* get_keyname_encrypting()
{
    return sharedConfig
        ->group(QLatin1String("keyring"))
        .readEntry("keyname_encrypt", "ksecrets:encrypting")
        .toUtf8()
        .constData();
}

const char* get_keyname_mac()
{
    return sharedConfig
        ->group(QLatin1String("keyring"))
        .readEntry("keyname_mac", "ksecrets:mac")
        .toUtf8()
        .constData();
}

bool get_salt(const char* path, char* buf, size_t len)
{
    bool res= false;
    KSecretsBackend backend;
    auto openfut = backend.open(path);
    if (openfut.get().status_ == KSecretsBackend::OpenStatus::Good) {

    }
    return res;
}
}

