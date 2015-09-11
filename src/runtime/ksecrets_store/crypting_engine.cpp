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

#include "crypting_engine.h"
#include "defines.h"

#define GCRPYT_NO_DEPRECATED
#include <gcrypt.h>
#include <errno.h>
#include <memory>
#include <cassert>

extern "C" {
#include <keyutils.h>
}

#define GCRYPT_REQUIRED_VERSION "1.6.0"
#define KSECRETS_SALTSIZE 56
#define KSECRETS_KEYSIZE 256

#define ERRNO(cryres) gcry_err_code_to_errno(gcry_err_code(cryres))

const char* keyNameEncrypting = nullptr;
const char* keyNameMac = nullptr;

const char* get_keyname_encrypting() { return keyNameEncrypting; }
const char* get_keyname_mac() { return keyNameMac; }

int kss_derive_keys(const char* salt, const char* password, char* encryption_key, char* mac_key, size_t keySize)
{
    gpg_error_t gcryerr;

    syslog(KSS_LOG_INFO, "ksecrets: attempting keys generation");
    if (0 == password) {
        syslog(KSS_LOG_INFO, "NULL password given. ksecrets will not be available.");
        return FALSE;
    }

    /* generate both encryption and MAC key in one go */
    char* keys = new char[2 * keySize];
    if (keys == nullptr) {
        syslog(KSS_LOG_ERR, "ksecrets: cannot allocate memory for key buffer");
        return FALSE;
    }
    gcryerr = gcry_kdf_derive(password, strlen(password), GCRY_KDF_ITERSALTED_S2K, GCRY_MD_SHA512, salt, 8, KSECRETS_ITERATIONS, 2 * keySize, keys);
    if (gcryerr) {
        delete[] keys;
        syslog(KSS_LOG_ERR, "ksecrets: key derivation failed: code 0x%0x: %s/%s", gcryerr, gcry_strsource(gcryerr), gcry_strerror(gcryerr));
        return FALSE;
    }

    memcpy(encryption_key, keys, keySize);
    memcpy(mac_key, keys + keySize, keySize);
    delete[] keys;
    syslog(KSS_LOG_INFO, "successuflly generated ksecrets keys from user password.");

    return TRUE;
}

int kss_store_keys(const char* encryption_key, const char* mac_key, size_t keySize)
{
    key_serial_t ks;
    const char* key_name = get_keyname_encrypting();
    ks = add_key("user", key_name, encryption_key, keySize, KEY_SPEC_USER_SESSION_KEYRING);
    if (-1 == ks) {
        syslog(KSS_LOG_ERR, "ksecrets: cannot store encryption key in kernel "
                            "keyring: errno=%d",
            errno);
        return FALSE;
    }
    syslog(KSS_LOG_DEBUG, "ksecrets: encrypting key now in kernel keyring with id %d and desc %s", ks, key_name);

    key_name = get_keyname_mac();
    ks = add_key("user", key_name, mac_key, keySize, KEY_SPEC_USER_SESSION_KEYRING);
    if (-1 == ks) {
        syslog(KSS_LOG_ERR, "ksecrets: cannot store mac key in kernel keyring: errno=%d", errno);
        return FALSE;
    }
    syslog(KSS_LOG_DEBUG, "ksecrets: mac key now in kernel keyring with id %d and desc %s", ks, key_name);
    return TRUE;
}

int kss_set_credentials(const std::string& password, const char* salt)
{
    // FIXME this should be adjusted on platforms where kernel keyring is not
    // available and store the keys elsewhere
    char encryption_key[KSECRETS_KEYSIZE];
    char mac_key[KSECRETS_KEYSIZE];
    auto res = kss_derive_keys(salt, password.c_str(), encryption_key, mac_key, KSECRETS_KEYSIZE);
    if (res == FALSE)
        return res;

    return kss_store_keys(encryption_key, mac_key, KSECRETS_KEYSIZE);
}

/**
 * @brief Reads a key from the kernel keyring
 *
 * @param keyName keyname from the keyring
 * @param buffer buffer where to store the key payload
 * @param bufferSize buffer size. If unsufficient, the return value would specify the needed length
 *
 * @return -1 on error, 0 on success, >0 needed buffer length in bytes, when bufferSize was not sufficient
 */
long kss_read_key(const char* keyName, char* buffer, size_t bufferSize)
{
    key_serial_t key;
    key = request_key("user", keyName, 0, KEY_SPEC_USER_SESSION_KEYRING);
    if (-1 == key) {
        syslog(KSS_LOG_DEBUG, "request_key failed with errno %d when reading key %s", errno, keyName);
        return -1;
    }
    auto bytes = keyctl_read(key, buffer, bufferSize);
    if (bytes == -1) {
        syslog(KSS_LOG_ERR, "error reading key %s contents from the keyring", keyName);
        return -1;
    }
    if ((size_t)bytes > bufferSize) {
        return bytes;
    }
    return 0; // key contents correctly transffered into the buffer
}

long kss_read_mac_key(char* buffer, size_t bufferSize) { return kss_read_key(get_keyname_mac(), buffer, bufferSize); }

long kss_read_encrypting_key(char* buffer, size_t bufferSize) { return kss_read_key(get_keyname_encrypting(), buffer, bufferSize); }

CryptingEngine* CryptingEngine::instance_ = nullptr;
unsigned char CryptingEngine::iv_[IV_SIZE];
bool CryptingEngine::has_iv_ = false;
unsigned char* CryptingEngine::getIV() noexcept { return iv_; }

CryptingEngine& CryptingEngine::instance()
{
    if (instance_ == nullptr) {
        instance_ = new CryptingEngine();
        instance_->setup();
    }
    return *instance_;
}

void CryptingEngine::randomize(unsigned char* buffer, size_t length) { gcry_randomize(buffer, length, GCRY_STRONG_RANDOM); }
void CryptingEngine::create_nonce(unsigned char* buffer, size_t len) { gcry_create_nonce(buffer, len); }

void CryptingEngine::setKeyNameEncrypting(const char* name) noexcept { keyNameEncrypting = name; }

void CryptingEngine::setKeyNameMac(const char* name) noexcept { keyNameMac = name; }

CryptingEngine::CryptingEngine()
    : valid_(false)
    , has_credentials_(false)
{
}

void CryptingEngine::setup() noexcept
{
    syslog(KSS_LOG_DEBUG, "ksecrets: setting-up gcrypt library");
    if (!gcry_check_version(GCRYPT_REQUIRED_VERSION)) {
        syslog(KSS_LOG_ERR, "ksecrets_store: libcrypt version is too old");
        return;
    }

    gcry_error_t gcryerr;
    gcryerr = gcry_control(GCRYCTL_INIT_SECMEM, 32768, 0);
    if (gcryerr != 0) {
        syslog(KSS_LOG_ERR, "ksecrets_store: cannot get secure memory: %d", gcryerr);
        return;
    }

    gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);
    syslog(KSS_LOG_DEBUG, "gcrypt library now set-up");

    auto cryres = gcry_cipher_open(&hd_, GCRY_CIPHER_BLOWFISH, GCRY_CIPHER_MODE_CBC, 0);
    if (cryres) {
        syslog(KSS_LOG_ERR, "ksecrets: gcry_cipher_open returned error %d", cryres);
        return;
    }
    valid_ = true;
}

bool CryptingEngine::setIV(const unsigned char* iv, size_t liv) noexcept
{
    if (liv < IV_SIZE) {
        syslog(KSS_LOG_ERR, "ksecrets: setIV called with a too short buffer");
        return false;
    }
    memcpy(iv_, iv, IV_SIZE);
    has_iv_ = true;
    return true;
}

bool CryptingEngine::isValid() noexcept
{
    if (!valid_)
        setup();
    return valid_;
}

bool CryptingEngine::isReady() noexcept
{
    if (!valid_) {
        syslog(KSS_LOG_ERR, "ksecrets: crypting engine is not correctly initialized");
        return false;
    }
    if (!has_iv_) {
        syslog(KSS_LOG_ERR, "ksecrets: IV must be set before attempting encryption operations");
        return false;
    }
    if (!has_credentials_) {
        char encryptingKey[KSECRETS_KEYSIZE];
        auto keyres = kss_read_encrypting_key(encryptingKey, sizeof(encryptingKey) / sizeof(encryptingKey[0]));
        assert(keyres <= 0); // if positive result, then the handed buffer size is not sufficient
        if (keyres < 0) {
            // this situation arises when neither pam_ksecrets did not set the credentials,  nor the library user did not call setCredentials
            syslog(KSS_LOG_ERR, "ksecrets: encrypting key not found in the keyring");
            return false;
        }
        auto cryres = gcry_cipher_setkey(hd_, encryptingKey, sizeof(encryptingKey) / sizeof(encryptingKey[0]));
        if (cryres) {
            syslog(KSS_LOG_ERR, "ksecrets: gcry_cipher_setkey returned %d", cryres);
            return false;
        }
        has_credentials_ = true;
    }
    return true;
}

bool CryptingEngine::setCredentials(const std::string& password, const unsigned char* salt) noexcept
{
    if (keyNameEncrypting == nullptr) {
        syslog(KSS_LOG_ERR, "ksecrets: please set encrypting keyname first");
        return false;
    }
    if (keyNameMac == nullptr) {
        syslog(KSS_LOG_ERR, "ksecrets: please set mac keyname first");
        return false;
    }
    if (kss_set_credentials(password, (char*)salt) == FALSE) {
        return false;
    }
    has_credentials_ = true;
    return true;
}

bool CryptingEngine::encrypt(void* out, size_t lout, const void* in, size_t lin) noexcept
{
    if (!isReady())
        return false;
    auto cryres = gcry_cipher_setiv(hd_, iv_, IV_SIZE);
    if (cryres) {
        syslog(KSS_LOG_ERR, "ksecrets: gcry_cipher_setiv returned error %d", cryres);
        return false;
    }
    cryres = gcry_cipher_encrypt(hd_, out, lout, in, lin);
    if (cryres) {
        syslog(KSS_LOG_ERR, "ksecrets: gcry_cipher_encrypt returned %d", cryres);
        return false;
    }
    return true;
}

bool CryptingEngine::decrypt(void* out, size_t lout, const void* in, size_t lin) noexcept
{
    if (!isReady())
        return false;
    auto cryres = gcry_cipher_setiv(hd_, iv_, IV_SIZE);
    if (cryres) {
        syslog(KSS_LOG_ERR, "ksecrets: gcry_cipher_setiv returned error %d", cryres);
        return false;
    }
    cryres = gcry_cipher_decrypt(hd_, out, lout, in, lin);
    if (cryres) {
        syslog(KSS_LOG_ERR, "ksecrets: gcry_cipher_decrypt returned %d", cryres);
        return false;
    }
    return true;
}

CryptingEngine::MAC::MAC()
    : need_init_(true)
    , ignore_updates_(false)
{
    auto gcryerr = gcry_mac_open(&hd_, GCRY_MAC_HMAC_SHA512, 0, 0);
    if (gcryerr) {
        syslog(KSS_LOG_ERR, "opening MAC algo failed: code 0x%0x: %s/%s", gcryerr, gcry_strsource(gcryerr), gcry_strerror(gcryerr));
        valid_ = false;
    }
    else {
        valid_ = true;
    }
}

CryptingEngine::MAC::~MAC()
{
    if (valid_) {
        gcry_mac_close(hd_);
    }
}

bool CryptingEngine::MAC::reset() noexcept
{
    if (!valid_) {
        syslog(KSS_LOG_ERR, "ksecrets: MAC object is not valid");
        return false;
    }
    if (need_init_) {
        char macKey[KSECRETS_KEYSIZE];
        if (kss_read_mac_key(macKey, KSECRETS_KEYSIZE) != 0) {
            syslog(KSS_LOG_ERR, "ksecrets: cannot retrieve MAC key");
            return false;
        }
        // auto gcryerr = gcry_mac_setiv(hd_, CryptingEngine::getIV(), CryptingEngine::IV_SIZE);
        // if (gcryerr) {
        //     syslog(KSS_LOG_ERR, "setting MAC IV failed: code 0x%0x: %s/%s", gcryerr, gcry_strsource(gcryerr), gcry_strerror(gcryerr));
        //     return false;
        // }
        auto gcryerr = gcry_mac_setkey(hd_, macKey, KSECRETS_KEYSIZE);
        if (gcryerr) {
            syslog(KSS_LOG_ERR, "setting MAC key failed: code 0x%0x: %s/%s", gcryerr, gcry_strsource(gcryerr), gcry_strerror(gcryerr));
            return false;
        }
        need_init_ = false;
    }
    else {
        auto gcryerr = gcry_mac_reset(hd_);
        if (gcryerr) {
            syslog(KSS_LOG_ERR, "resetting MAC failed: code 0x%0x: %s/%s", gcryerr, gcry_strsource(gcryerr), gcry_strerror(gcryerr));
            return false;
        }
    }
    ignore_updates_ = false;

    return true;
}

void CryptingEngine::MAC::stop() noexcept { ignore_updates_ = true; }

bool CryptingEngine::MAC::update(const void* buffer, size_t len) noexcept
{
    if (!valid_)
        return false;
    if (ignore_updates_)
        return true;
    if (need_init_) {
        syslog(KSS_LOG_ERR, "ksecrets: you forgot to call reset!");
        return false;
    }
    auto gcryerr = gcry_mac_write(hd_, buffer, len);
    if (gcryerr) {
        syslog(KSS_LOG_ERR, "ksecrets: updating MAC failed: code 0x%0x: %s/%s", gcryerr, gcry_strsource(gcryerr), gcry_strerror(gcryerr));
        return false;
    }

    return true;
}

CryptingEngine::BufferPtr CryptingEngine::MAC::read() noexcept
{
    if (need_init_) {
        syslog(KSS_LOG_ERR, "ksecrets: you forgot to call reset!");
        return false;
    }
    auto res = std::make_shared<CryptingEngine::Buffer>();
    char buffer[512]; // on purpose allocate more
    size_t len = sizeof(buffer) / sizeof(buffer[0]);
    auto gcryerr = gcry_mac_read(hd_, (void*)buffer, &len);
    if (gcryerr) {
        syslog(KSS_LOG_ERR, "obtaining resulting MAC failed: code 0x%0x: %s/%s", gcryerr, gcry_strsource(gcryerr), gcry_strerror(gcryerr));
        return res;
    }

    res->bytes_ = (unsigned char*)std::malloc(len);
    if (res->bytes_ == nullptr) {
        syslog(KSS_LOG_ERR, "ksecrets: cannot allocate MAC buffer");
        return res;
    } else {
        memcpy(res->bytes_, buffer, len);
    }
    res->len_ = len;
    return res;
}

bool CryptingEngine::MAC::verify(unsigned char* buffer, size_t len) noexcept
{
    if (need_init_) {
        syslog(KSS_LOG_ERR, "ksecrets: you forgot to call reset!");
        return false;
    }
    auto gcryerr = gcry_mac_verify(hd_, buffer, len);
    delete[] buffer;
    if (gcryerr) {
        syslog(KSS_LOG_ERR, "MAC check failed: code 0x%0x: %s/%s", gcryerr, gcry_strsource(gcryerr), gcry_strerror(gcryerr));
        return false;
    }

    return true;
}

CryptingEngine::Buffer::Buffer()
    : bytes_(nullptr)
    , len_(0)
{
}
CryptingEngine::Buffer::Buffer(size_t len)
    : len_(0)
{
    bytes_ = (unsigned char*)std::malloc(len);
    if (bytes_ == nullptr)
        return;
    len_ = len;
}

CryptingEngine::Buffer::~Buffer()
{
    if (bytes_ != nullptr)
        std::free(bytes_);
}
// vim: tw=220:ts=4
