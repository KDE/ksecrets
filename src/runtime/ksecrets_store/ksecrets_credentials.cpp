/* This file is part of the KDE project
 *
 * Copyright (C) 2015 Valentin Rusu <valir@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "ksecrets_credentials.h"
#include "ksecrets_store.h"
#include "defines.h"

#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include <pwd.h>
#include <string.h>
extern "C" {
#include <keyutils.h>
}


const char* get_keyname_encrypting();
const char* get_keyname_mac();
int kss_keys_already_there();


extern "C"
int KSECRETS_STORE_EXPORT kss_set_credentials(const char* user_name, const char* password, const char* path)
{
    UNUSED(user_name);
    if (kss_keys_already_there())
        return TRUE;

    KSecretsStore secretsStore;
    auto setupres = secretsStore.setup(path);
    if (!setupres.get()) {
        return FALSE;
    }

    auto credres = secretsStore.setCredentials(password);
    return credres.get() ? TRUE: FALSE;
}

extern "C"
int KSECRETS_STORE_EXPORT kss_delete_credentials()
{
    syslog(KSS_LOG_INFO, "kss_delete_credentials");
    key_serial_t key;
    key = request_key("user", get_keyname_encrypting(), 0, KEY_SPEC_SESSION_KEYRING);
    if (-1 == key) {
        syslog(KSS_LOG_DEBUG, "request_key failed with errno %d (%m), cannot purge encrypting key", errno);
        return FALSE;
    }
    long res = keyctl(KEYCTL_REVOKE, key);
    if (-1 == res) {
        syslog(KSS_LOG_DEBUG, "removing key failed with errno %d (%m), cannot purge encrypting key", errno);
        return FALSE;
    }

    key = request_key("user", get_keyname_mac(), 0, KEY_SPEC_SESSION_KEYRING);
    if (-1 == key) {
        syslog(KSS_LOG_DEBUG, "request_key failed with errno %d (%m), cannot purge mac key", errno);
        return FALSE;
    }
    res = keyctl(KEYCTL_REVOKE, key);
    if (-1 == res) {
        syslog(KSS_LOG_DEBUG, "removing key failed with errno %d (%m), cannot purge mac key", errno);
        return FALSE;
    }
    return TRUE;
}

extern "C"
int KSECRETS_STORE_EXPORT kss_can_change_password()
{
    /* nothing to do for the moment */
    syslog(KSS_LOG_INFO, "kss_can_change_password");
    return TRUE;
}

extern "C"
int KSECRETS_STORE_EXPORT kss_change_password(const char* new_password)
{
    UNUSED(new_password);
    syslog(LOG_INFO, "kss_change_password");
    return TRUE;
}
/* vim: tw=220 ts=4
*/
