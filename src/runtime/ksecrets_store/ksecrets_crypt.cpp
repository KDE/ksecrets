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

#include "defines.h"
#include "ksecrets_crypt.h"
#include "ksecrets_file.h"

#include <sys/types.h>
#include <errno.h>
#include <memory>
#include <cassert>

extern "C" {
#include <keyutils.h>
}

#define GCRPYT_NO_DEPRECATED
#include <gcrypt.h>

#define GCRYPT_REQUIRED_VERSION "1.6.0"

#define KSECRETS_SALTSIZE 56
#define KSECRETS_KEYSIZE 256

const char* get_keyname_encrypting();
const char* get_keyname_mac();

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

int kss_keys_already_there()
{
    key_serial_t key;
    key = request_key("user", get_keyname_encrypting(), 0, KEY_SPEC_USER_SESSION_KEYRING);
    if (-1 == key) {
        syslog(KSS_LOG_DEBUG, "request_key failed with errno %d, so assuming ksecrets not yet loaded", errno);
        return FALSE;
    }
    syslog(KSS_LOG_DEBUG, "ksecrets: keys already in keyring");
    return TRUE;
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

#define ERRNO(cryres) gcry_err_code_to_errno(gcry_err_code(cryres))

long kss_cipher_setup(gcry_cipher_hd_t* hd, const void* iv, size_t liv)
{
    // FIXME perhaps all this initialization stuff could only be done once,
    // when password is setup
    auto cryres = gcry_cipher_open(hd, GCRY_CIPHER_BLOWFISH, GCRY_CIPHER_MODE_CBC, 0);
    if (cryres) {
        syslog(KSS_LOG_ERR, "ksecrets: gcry_cipher_open returned error %d", cryres);
        return ERRNO(cryres);
    }
    cryres = gcry_cipher_setiv(*hd, iv, liv);
    if (cryres) {
        syslog(KSS_LOG_ERR, "ksecrets: gcry_cipher_setif returned error %d", cryres);
        return ERRNO(cryres);
    }
    char encryptingKey[KSECRETS_KEYSIZE];
    auto keyres = kss_read_encrypting_key(encryptingKey, sizeof(encryptingKey) / sizeof(encryptingKey[0]));
    assert(keyres <= 0); // if positive result, then the handed buffer size is not sufficient
    if (keyres < 0) {
        syslog(KSS_LOG_ERR, "ksecrets: encrypting key not found in the keyring");
        return keyres;
    }
    cryres = gcry_cipher_setkey(*hd, encryptingKey, sizeof(encryptingKey) / sizeof(encryptingKey[0]));
    if (cryres) {
        syslog(KSS_LOG_ERR, "ksecrets: gcry_cipher_setkey returned %d", cryres);
        return ERRNO(cryres);
    }
    return 0;
}

long kss_encrypt_buffer(unsigned char* out, size_t lout, const void* iv, size_t liv, const unsigned char* in, size_t lin)
{
    gcry_cipher_hd_t hd;
    auto cryres = kss_cipher_setup(&hd, iv, liv);
    if (cryres)
        return cryres; // error already logged
    cryres = gcry_cipher_encrypt(hd, out, lout, in, lin);
    if (cryres) {
        syslog(KSS_LOG_ERR, "ksecrets: gcry_cipher_encrypt returned %ld", cryres);
        return ERRNO(cryres);
    }
    return 0;
}

long kss_decrypt_buffer(unsigned char* out, size_t lout, const void* iv, size_t liv, const unsigned char* in, size_t lin)
{
    gcry_cipher_hd_t hd;
    auto cryres = kss_cipher_setup(&hd, iv, liv);
    if (cryres)
        return cryres; // error already logged
    cryres = gcry_cipher_decrypt(hd, out, lout, in, lin);
    if (cryres) {
        syslog(KSS_LOG_ERR, "ksecrets: gcry_cipher_decrypt returned %ld", cryres);
        return ERRNO(cryres);
    }
    return 0;
}

CryptBuffer::CryptBuffer()
    : len_(0)
    , encrypted_(nullptr)
    , decrypted_(nullptr)
{
    setg(nullptr, nullptr, nullptr);
    setp(nullptr, nullptr);
}

CryptBuffer::~CryptBuffer()
{
    delete[] encrypted_, encrypted_ = nullptr;
    delete[] decrypted_, decrypted_ = nullptr;
}

void CryptBuffer::empty() noexcept
{
    delete[] encrypted_, encrypted_ = nullptr;
    delete[] decrypted_, decrypted_ = nullptr;
    len_ = 0;
    setg(nullptr, nullptr, nullptr);
    setp(nullptr, nullptr);
}

bool CryptBuffer::read(KSecretsDevice& file)
{
    empty();
    iv_ = file.iv();

    if (!file.read(len_))
        return false;

    try {
        encrypted_ = new char[len_];
    }
    catch (std::bad_alloc) {
        syslog(KSS_LOG_ERR, "ksecrets: got a std::bad_alloc and that means the file is corrupt");
        return false;
    }
    if (encrypted_ == nullptr) {
        len_ = 0;
        return false;
    }

    if (len_ > 0) {
        if (!file.read(encrypted_, len_))
            return false;
    }
    return true;
}

bool CryptBuffer::write(KSecretsDevice& file)
{
    if (file.write(len_)) {
        syslog(KSS_LOG_DEBUG, "ksecrets: write: |%s|", decrypted_);
        iv_ = file.iv();
        encrypt();
        return file.write(encrypted_, len_);
    }
    else
        return false;
}

bool CryptBuffer::decrypt() noexcept
{
    if (len_ == 0)
        return false;
    assert(encrypted_ != nullptr);
    decrypted_ = new char[len_];
    auto dres = kss_decrypt_buffer((unsigned char*)decrypted_, len_, iv_, KSecretsFile::IV_SIZE, (const unsigned char*)encrypted_, len_);
    if (dres == 0) {
        syslog(KSS_LOG_DEBUG, "ksecrets: read decrypted: |%s|", decrypted_);
        setg(decrypted_, decrypted_, decrypted_ + len_);
        setp(decrypted_, decrypted_ + len_);
        return true;
    }
    else {
        empty();
        return false;
    }
}

bool CryptBuffer::encrypt() noexcept
{
    if (len_ == 0)
        return false;
    assert(decrypted_ != nullptr);

    delete[] encrypted_;
    encrypted_ = new char[len_];
    if (encrypted_ == nullptr) {
        return false;
    }
    gcry_create_nonce(encrypted_, len_);

    auto eres = kss_encrypt_buffer((unsigned char*)encrypted_, len_, iv_, KSecretsFile::IV_SIZE, (const unsigned char*)decrypted_, len_);
    if (eres != 0)
        return false;
    delete[] decrypted_, decrypted_ = nullptr;
    setp(nullptr, nullptr);
    return true;
}

CryptBuffer::int_type CryptBuffer::underflow()
{
    if (gptr() == nullptr) {
        if (!decrypt())
            return traits_type::eof();
    }
    if (gptr() < egptr()) {
        return traits_type::to_int_type(*gptr());
    }
    else {
        return traits_type::eof();
    }
}

CryptBuffer::int_type CryptBuffer::overflow(int_type c)
{
    if (c == traits_type::eof())
        return c;

    if (pptr() == epptr()) {
        size_t oldLen = len_;
        size_t oldgpos = gptr() - eback();

        len_ += cipherBlockLen_;
        if (decrypted_ == nullptr) {
            decrypted_ = (char*)std::malloc(len_);
        }
        else {
            decrypted_ = (char*)std::realloc(decrypted_, len_);
            if (decrypted_ == nullptr) {
                syslog(KSS_LOG_ERR, "ksecrets: cannot extend crypt buffer");
                setp(nullptr, nullptr);
                return traits_type::eof();
            }
        }
        // fill the not yet used area with random data
        gcry_create_nonce((unsigned char*)decrypted_ + oldLen, cipherBlockLen_);

        setp(decrypted_ + oldLen, decrypted_ + len_);
        setg(decrypted_, decrypted_ + oldgpos, decrypted_ + len_);

        *pptr() = c;
        pbump(sizeof(char_type));
    }

    return traits_type::to_int_type(c);
}

std::ostream& operator << (std::ostream& os, const std::string& str) {
    os << ' ' << str.length() << ":";
    os.write(str.c_str(), str.length());
    return os;
}

std::istream& operator >> (std::istream& is, std::string& str) {
    std::string::size_type len;
    is >> len;
    char c;
    is >> c;
    if (c != ':') {
        is.fail();
        return is;
    }
    str.resize(len, ' ');
    char *bytes = &*str.begin();
    is.read(bytes, len);
    return is;
}
// vim: tw=220:ts=4
