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

#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/limits.h>
#include <pwd.h>
#include <string.h>
#include <keyutils.h>

#define GCRPYT_NO_DEPRECATED
#include <gcrypt.h>

#define GCRYPT_REQUIRED_VERSION "1.6.0"

#define KSS_LOG_DEBUG (LOG_AUTH | LOG_DEBUG)
#define KSS_LOG_INFO (LOG_AUTH | LOG_INFO)
#define KSS_LOG_ERR (LOG_AUTH | LOG_ERR)

#define KSECRETS_ITERATIONS 50000

/* these functions are implemented in config.cpp next to this file */
extern const char* prepare_secret_file_location(const char*);
extern const char* get_keyname_encrypting();
extern const char* get_keyname_mac();

#define false 0
#define true 1

int kss_init_gcry()
{
    syslog(KSS_LOG_DEBUG, "ksecrets: setting-up grypt library");
    if (!gcry_check_version(GCRYPT_REQUIRED_VERSION)) {
        syslog(KSS_LOG_ERR, "ksecrets_store: libcrypt version is too old");
        return 0;
    }

    gcry_error_t gcryerr;
    gcryerr = gcry_control(GCRYCTL_INIT_SECMEM, 32768, 0);
    if (gcryerr != 0) {
        syslog(KSS_LOG_ERR, "ksecrets_store: cannot get secure memory: %d", gcryerr);
        return 0;
    }

    gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);
    syslog(KSS_LOG_DEBUG, "gcrypt library now set-up");
    return 1;
}

int kss_derive_keys(const char* salt, const char* password, char* encryption_key, char* mac_key, size_t keySize)
{
    gpg_error_t gcryerr;

    syslog(KSS_LOG_INFO, "kss_set_credentials: attempting keys generation");
    if (0 == password) {
        syslog(KSS_LOG_INFO, "NULL password given. ksecrets will not be available.");
        return false;
    }

    /* generate both encryption and MAC key in one go */
    char keys[2 * keySize];
    gcryerr = gcry_kdf_derive(password, strlen(password), GCRY_KDF_ITERSALTED_S2K, GCRY_MD_SHA512, salt, 8, KSECRETS_ITERATIONS, 2 * keySize, keys);
    if (gcryerr) {
        syslog(KSS_LOG_ERR, "key derivation failed: code 0x%0x: %s/%s", gcryerr, gcry_strsource(gcryerr), gcry_strerror(gcryerr));
        return false;
    }

    memcpy(encryption_key, keys, keySize);
    memcpy(mac_key, keys + keySize, keySize);
    syslog(KSS_LOG_INFO, "successuflly generated ksecrets keys from user password.");

    return true;
}

int kss_store_keys(const char* encryption_key, const char* mac_key, size_t keySize)
{
    key_serial_t ks;
    const char* key_name = get_keyname_encrypting();
    ks = add_key("user", key_name, encryption_key, keySize, KEY_SPEC_SESSION_KEYRING);
    if (-1 == ks) {
        syslog(KSS_LOG_ERR, "ksecrets: cannot store encryption key in kernel keyring: errno=%d (%m)", errno);
        return false;
    }
    syslog(KSS_LOG_DEBUG, "ksecrets: encrpyting key now in kernel keyring with id %d and desc %s", ks, key_name);

    key_name = get_keyname_mac();
    ks = add_key("user", key_name, mac_key, keySize, KEY_SPEC_SESSION_KEYRING);
    if (-1 == ks) {
        syslog(KSS_LOG_ERR, "ksecrets: cannot store mac key in kernel keyring: errno=%d (%m)", errno);
        return false;
    }
    syslog(KSS_LOG_DEBUG, "ksecrets: mac key now in kernel keyring with id %d and desc %s", ks, key_name);
    return true;
}

int kss_keys_already_there()
{
    key_serial_t key;
    key = request_key("user", get_keyname_encrypting(), 0, KEY_SPEC_SESSION_KEYRING);
    if (-1 == key) {
        syslog(KSS_LOG_DEBUG, "request_key failed with errno %d (%m), so assuming ksecrets not yet loaded", errno);
        return false;
    }
    syslog(KSS_LOG_DEBUG, "ksecrets: keys already in keyring");
    return true;
}

int kss_set_credentials(const char* user_name, const char* password)
{
    syslog(KSS_LOG_DEBUG, "kss_set_credentials for %s", user_name);
    if (kss_keys_already_there())
        return true;


    return true;
}

int kss_delete_credentials()
{
    syslog(KSS_LOG_INFO, "kss_delete_credentials");
    key_serial_t key;
    key = request_key("user", get_keyname_encrypting(), 0, KEY_SPEC_SESSION_KEYRING);
    if (-1 == key) {
        syslog(KSS_LOG_DEBUG, "request_key failed with errno %d (%m), cannot purge encrypting key", errno);
        return false;
    }
    long res = keyctl(KEYCTL_REVOKE, key);
    if (-1 == res) {
        syslog(KSS_LOG_DEBUG, "removing key failed with errno %d (%m), cannot purge encrypting key", errno);
        return false;
    }

    key = request_key("user", get_keyname_mac(), 0, KEY_SPEC_SESSION_KEYRING);
    if (-1 == key) {
        syslog(KSS_LOG_DEBUG, "request_key failed with errno %d (%m), cannot purge mac key", errno);
        return false;
    }
    res = keyctl(KEYCTL_REVOKE, key);
    if (-1 == res) {
        syslog(KSS_LOG_DEBUG, "removing key failed with errno %d (%m), cannot purge mac key", errno);
        return false;
    }
    return true;
}

int kss_can_change_password()
{
    /* nothing to do for the moment */
    syslog(KSS_LOG_INFO, "kss_can_change_password");
    return true;
}

int kss_change_password(const char* new_password)
{
    syslog(LOG_INFO, "kss_change_password");
    return true;
}
/* vim: tw=220 ts=4
*/
