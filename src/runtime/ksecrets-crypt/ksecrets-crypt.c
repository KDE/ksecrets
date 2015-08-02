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

#include "ksecrets-crypt.h"

#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <string.h>
#include <keyutils.h>

#define GCRPYT_NO_DEPRECATED
#include <gcrypt.h>

#define GCRYPT_VERSION "1.6.0"

#define KSS_LOG_DEBUG (LOG_AUTH | LOG_DEBUG)
#define KSS_LOG_INFO (LOG_AUTH | LOG_INFO)
#define KSS_LOG_ERR (LOG_AUTH | LOG_ERR)

#define KSECRETS_SALTSIZE 56
#define KSECRETS_KEYSIZE 256
#define KSECRETS_ITERATIONS 50000

#define KSS_KEY_TYPE_ENCRYPT "ksecrets:encrypting"
#define KSS_KEY_TYPE_MAC "ksecrets:mac"

int mkpath(char* path, struct passwd* user_info)
{
    struct stat sb;
    char* slash;
    int done = 0;

    slash = path;

    while (!done) {
        slash += strspn(slash, "/");
        slash += strcspn(slash, "/");

        done = (*slash == '\0');
        *slash = '\0';

        if (stat(path, &sb)) {
            if (errno != ENOENT || (mkdir(path, 0777) && errno != EEXIST)) {
                syslog(LOG_ERR, "Couldn't create directory: %s because: %d-%s", path,
                    errno, strerror(errno));
                return (-1);
            }
            else {
                if (chown(path, user_info->pw_uid, user_info->pw_gid) == -1) {
                    syslog(LOG_INFO, "Couldn't change ownership of: %s", path);
                }
            }
        }
        else if (!S_ISDIR(sb.st_mode)) {
            return (-1);
        }

        *slash = '/';
    }

    return (0);
}

bool kss_init_gcry()
{
    syslog(KSS_LOG_DEBUG, "setting-up grypt library");
    if (!gcry_check_version(GCRYPT_VERSION)) {
        syslog(KSS_LOG_ERR, "kwalletd: libcrypt version is too old");
        return false;
    }

    gcry_error_t gcryerr;
    gcryerr = gcry_control(GCRYCTL_INIT_SECMEM, 32768, 0);
    if (gcryerr != 0) {
        syslog(KSS_LOG_ERR, "ksecrets: cannot get secure memory: %d", gcryerr);
        return false;
    }

    gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);
    syslog(KSS_LOG_DEBUG, "gcrypt library now set-up");
    return true;
}

const char* secrets_file_path();

/**
 * This function reads the crypting salt from the main ksecrets data file
 * If the file is not present, then it is created. This case should happen
 * when user opens the session for the first time. That is possible because
 * ksecrets file format is SALT-IV-CIPHERTEXT-MAC. So in our case we'll create
 * file containing the SALT. It'll be extended by the ksecrets daemon upon
 * first usage.
 *
 * NOTE This function has code from pam-kwallet
 * TODO adapt this code to enable configuration of the salt file path from the
 * PAM module command line
 */
bool kss_get_salt(const char* username, char** salt)
{
    if (0 == username) {
        syslog(KSS_LOG_ERR, "no username given, salt could not been retrieved");
        return false;
    }

    struct passwd* user_info;
    user_info = getpwnam(username);
    if (!user_info) {
        syslog(KSS_LOG_ERR, "pam_kwallet: Couldn't get user info (passwd) info");
        return false;
    }

    /* FIXME this path should be configurable in a future version */
    char* fixpath = secrets_file_path();
    char* path = (char*)malloc(
        strlen(user_info->pw_dir) + strlen(fixpath) + 2); /* 2 == / and \0 */
    sprintf(path, "%s/%s", user_info->pw_dir, fixpath);

    struct stat info;
    *salt = NULL;
    if (stat(path, &info) != 0 || info.st_size == 0) {
        unlink(path); /* in case the file already exists and it has size of 0 */

        const char* dir = dirname(path);
        mkpath(dir, user_info); /* create the path in case it does not exists */

        *salt = gcry_random_bytes(KSECRETS_SALTSIZE, GCRY_STRONG_RANDOM);
        FILE* fd = fopen(path, "w");

        /* If the file can't be created */
        if (fd == NULL) {
            syslog(KSS_LOG_ERR, "Couldn't open file: %s because: %d-%s", path,
                errno, strerror(errno));
            return false;
        }

        fwrite(*salt, KSECRETS_SALTSIZE, 1, fd);
        fclose(fd);

        if (chown(path, user_info->pw_uid, user_info->pw_gid) == -1) {
            syslog(
                KSS_LOG_ERR, "Couldn't change ownership of the created salt file");
            return false;
        }
        syslog(KSS_LOG_INFO, "ksecrets: created secrets file path : %s", path);
    }
    else {
        FILE* fd = fopen(path, "r");
        if (fd == NULL) {
            syslog(KSS_LOG_ERR, "Couldn't open file: %s because: %d-%s", path,
                errno, strerror(errno));
            return 1;
        }
        *salt = (char*)malloc(sizeof(char) * KSECRETS_SALTSIZE);
        memset(*salt, '\0', KSECRETS_SALTSIZE);
        fread(*salt, KSECRETS_SALTSIZE, 1, fd);
        fclose(fd);
        syslog(KSS_LOG_INFO, "ksecrets: read salt from secrets file : %s", path);
    }
    if (salt == NULL) {
        syslog(KSS_LOG_ERR, "ksecrets: Couldn't create or read the salt file");
        return false;
    }
    return true;
}

bool kss_derive_keys(const char* user_name, const char* password,
    char* encryption_key, char* mac_key)
{
    gpg_error_t gcryerr;

    syslog(KSS_LOG_INFO, "kss_set_credentials: attempting keys generation");
    if (0 == password) {
        syslog(
            KSS_LOG_INFO, "NULL password given. ksecrets will not be available.");
        return false;
    }

    if (!kss_init_gcry())
        return false;

    const char* salt;
    salt = 0;
    if (!kss_get_salt(user_name, &salt))
        return false;

    /* generate both encryption and MAC key in one go */
    char keys[2 * KSECRETS_KEYSIZE];
    gcryerr = gcry_kdf_derive(password, strlen(password),
        GCRY_KDF_ITERSALTED_S2K, GCRY_MD_SHA512, salt, 8, KSECRETS_ITERATIONS,
        2 * KSECRETS_KEYSIZE, keys);
    if (gcryerr) {
        syslog(KSS_LOG_ERR, "key derivation failed: code 0x%0x: %s/%s", gcryerr,
            gcry_strsource(gcryerr), gcry_strerror(gcryerr));
        return false;
    }

    memcpy(encryption_key, keys, KSECRETS_KEYSIZE);
    memcpy(mac_key, keys + KSECRETS_KEYSIZE, KSECRETS_KEYSIZE);
    syslog(KSS_LOG_INFO,
        "successuflly generated ksecrets keys from user password.");

    return true;
}

bool kss_store_keys(const char* encryption_key, const char* mac_key)
{
    key_serial_t ks;
    ks = add_key("user", KSS_KEY_TYPE_ENCRYPT, encryption_key, KSECRETS_KEYSIZE,
        KEY_SPEC_SESSION_KEYRING);
    if (-1 == ks) {
        syslog(KSS_LOG_ERR,
            "ksecrets: cannot store encryption key in kernel keyring: errno=%d",
            errno);
        return false;
    }
    syslog(KSS_LOG_DEBUG,
        "ksecrets: encrpyting key now in kernel keyring with id %d", ks);

    ks = add_key("user", KSS_KEY_TYPE_MAC, mac_key, KSECRETS_KEYSIZE,
        KEY_SPEC_SESSION_KEYRING);
    if (-1 == ks) {
        syslog(KSS_LOG_ERR,
            "ksecrets: cannot store mac key in kernel keyring: errno=%d", errno);
        return false;
    }
    syslog(KSS_LOG_DEBUG, "ksecrets: mac key now in kernel keyring with id %d",
        ks);
    return true;
}

bool kss_keys_already_there()
{
    struct key* key;
    key = request_key(KSS_KEY_TYPE_ENCRYPT, 0, 0, KEY_SPEC_SESSION_KEYRING);
    if (-1 == key) {
        syslog(KSS_LOG_DEBUG, "request_key failed with errno %d", errno);
        return false;
    }
    syslog(KSS_LOG_DEBUG, "ksecrets: keys already in keyring");
    return true;
}

bool kss_set_credentials(const char* user_name, const char* password)
{
    if (kss_keys_already_there())
        return true;

    char encryption_key[KSECRETS_KEYSIZE];
    char mac_key[KSECRETS_KEYSIZE];
    if (!kss_derive_keys(user_name, password, encryption_key, mac_key))
        return false;

    if (!kss_store_keys(encryption_key, mac_key))
        return false;

    return true;
}

void kss_delete_credentials()
{
    syslog(KSS_LOG_INFO, "kss_delete_credentials");
}

bool kss_can_change_password()
{
    /* nothing to do for the moment */
    syslog(KSS_LOG_INFO, "kss_can_change_password");
    return true;
}

bool kss_change_password(const char* password)
{
    syslog(LOG_INFO, "kss_change_password");
    return true;
}

const char* secrets_file_path()
{
    return ".local/share/ksecretsd/ksecrets.data";
}
