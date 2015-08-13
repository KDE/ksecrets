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
#ifndef KSECRETS_STORE_H
#define KSECRETS_STORE_H

#include <ksecrets_store_export.h>

#include <memory>
#include <ctime>
#include <map>
#include <vector>
#include <array>
#include <future>

class KSecretsStorePrivate;

/**
 * Secrets storage for KSecrets Service.
 *
 * This class would store the secrets into an underlying custom formated file.
 *
 * Each API call is stateless. That is, the secrets file will always be left in a consistent
 * state between calls. So, even if your application crashes, the file won't get corrupted.
 *   FIXME is that OK? Further tests should be confirm if a background sync
 *   should be introduced in order to get operations faster. However, today computing power do
 *   not justify à priori optimizations, so this first version would modify the file
 *   with each API call.
 * That guarantees client applications that edits are always synced to disk/storage.
 *
 * The API calls are organized in classes, following the structure of data in the store.
 * Applications will first work with a Collection, the search or insert Items into it.
 * The Item class holds, sure enough, the secret value but also let applications associate
 * the secret value with metadata, such as the label or other custom properties.
 *
 * Before using a collection, the application should open it.
 * Upon opening, it's possible to indicate if readonly mode is possible.
 *
 * When opening without readonly flag, then the file is exclusively locked. The lock is
 * released when the class is destroyed. You should keep the file locked as shortly as
 * possible, in order to avoid deadlocks between applications that also need to read the
 * secrets. For more information @see open().
 *
 * The data are encrypted using libgcypt and the algorythm Twofish which is the fasted for this library.
 *
 * TODO give here a code example once the API stabilizes
 *
 * @note All const members in this interface are thread-safe.
 *
 * @note Why is this a class and not a namespace?
 *       The underlying storage should be kept locked as briefly as possible.
 *       By providing a class, one could use local variables and the class
 *       would be destroyed, releasing the file, upon block exit.
 */
class KSECRETS_STORE_EXPORT KSecretsStore {
    class ItemPrivate;
    class CollectionPrivate;

public:
    using AttributesMap = std::map<std::string, std::string>;

    struct ItemValue {
        std::string contentType;
        std::vector<char> contents;
    };

    /* Holds a secret value.
     *
     * The Item class let applications associate metadata with secret values.
     * These metadata could simply be the label or other custom attributes.
     * Items are organized in Collections.
     *
     * @see Collection
     */
    class Item {
        Item(const Item&) = default;
        Item& operator=(const Item&) = default;

        std::string label() const noexcept;
        bool setLabel(const char*) noexcept;

        AttributesMap attributes() const;
        void setAttributes(AttributesMap&&) noexcept;

        ItemValue value() const noexcept;
        bool setValue(ItemValue&&) noexcept;

        std::time_t createdTime() const noexcept;
        std::time_t modifiedTime() const noexcept;
    protected:
        Item();
        friend class KSecretsStore;

    private:
        std::shared_ptr<ItemPrivate> d;
    };
    using ItemPtr = std::shared_ptr<Item>;

    /**
     * Each application organizes it's secrets in collections.
     *
     * Typical applications will only use one collection. A collection can store
     * an arbitrary amount of Items. Each Item has a label, custom attributes and
     * a secret value.
     *
     * The custom attributes are application-defined. This API would store these
     * attributes as they are provided.
     *
     * Search methods are provided to let application locate items by specifying
     * only a subset of these custom attributes. When searching, partial matching is
     * used, so you could only provide part of the value of an attribute and get all
     * the items having attribute value containing that partially specified value.
     */
    class Collection {
        Collection(const Collection&) = default;
        Collection& operator=(const Collection&) = default;

        std::string label() const noexcept;
        bool setLabel(const char*) noexcept;

        std::time_t createdTime() const noexcept;
        std::time_t modifiedTime() const noexcept;

        using ItemList = std::vector<ItemPtr>;
        ItemList searchItems(AttributesMap&&) noexcept;
        ItemList searchItems(const char*) noexcept;
        ItemList searchItems(const char*, AttributesMap&&) noexcept;

        /**
         * Creates an item in the collection in one go. This is more efficient than
         * initializing en empty than setting the label, eventually the parameters and
         * the value. The returned item may still be modified, keep in mind that each
         * method call will trigger an store file update.
         *
         * @return ItemPtr which can be empty if creating the item was not
         * possible. So please check it via it's operator bool() before using
         * it.
         */
        ItemPtr createItem(const char*, AttributesMap&&, ItemValue&&) noexcept;
        /*
         * Convenience method for creating items without supplemental
         * attributes.
         *
         * @return ItemPtr which can be empty if creating the item was not
         * possible. So please check it via it's operator bool() before using
         * it.
         */
        ItemPtr createItem(const char *label, ItemValue&&) noexcept;

    protected:
        Collection();
        friend class KSecretsStore;

    private:
        std::shared_ptr<CollectionPrivate> d;
    };
    using CollectionPtr = std::shared_ptr<Collection>;

    /*
     * Default constructor.
     *
     * This constructor only initializes the store class. You should call
     * the open() method right after the initialization and before any other
     * methods of this API.
     *
     * @see open()
     */
    KSecretsStore();
    KSecretsStore(const KSecretsStore&) = delete;
    virtual ~KSecretsStore();

    enum class StoreStatus {
        Good,
        JustCreated,
        SetupShouldBeCalledFirst,
        CredentialsSet,
        IncorrectState,
        CannotInitGcrypt,
        CannotDeriveKeys,
        CannotStoreKeys,
        SetupError,
        NoPathGiven,
        InvalidFile, // the file format was not recognized. Is this a ksecrets file?
        CannotOpenFile,
        CannotLockFile,
        CannotReadFile,
        SystemError
    };

    template <StoreStatus G>
    struct OpResult {
        StoreStatus status_;
        int errno_;
        operator bool() const { return status_ == G; }
    };

    using SetupResult = OpResult<StoreStatus::Good>;
    // struct SetupResult {
    //     StoreStatus status_;
    //     int errno_;
    //     operator bool() const { return status_ == StoreStatus::Good; }
    // };

    /*
     * Before usage, the store must be setup, that is, it must know its file path.
     * This call creates the file if it's not found and the readOnly flag is set to false.
     * The file is not created when the readOnly flag is set to false in order to prevent
     * unintended side effects. If the file don't exists, the SetupResult.statu_ is set
     * to StoreStatus::SystemError and errno should be 2
     *
     * @return SetupResult whose operator bool could be used to check the error condition
     */
    std::future<SetupResult> setup(const char* path, bool readOnly =true);

    using CredentialsResult = OpResult<StoreStatus::CredentialsSet>;
    // struct CredentialsResult {
    //     StoreStatus status_;
    //     int errno_;
    //     operator bool() const { return status_ == StoreStatus::Good; }
    // };
    /**
     * Set the system-wide credentials for the secrets store
     *
     * This method is typically called from the pam module but it can also be
     * called from another program after prompting user for the password, for example.
     * Client applications only need to call this once per user session.
     */
    std::future<CredentialsResult> setCredentials(const char* password = nullptr, const char* keyNameEcrypting = "ksecrets:encrypting", const char* keyNameMac = "ksecrets:mac");

    bool isGood() const noexcept;

    constexpr static auto SALT_SIZE = 56;
    /**
     * @return pointer to the salt structure inside the internal structure of this object. The buffer has SALT_SIZE length.
     */
    const char* salt() const;

    using CollectionNames = std::vector<std::string>;
    CollectionNames dirCollections() const noexcept;
    /*
     * @return CollectionPtr which can empty if the call did not succeed.
     * Please check that with operator bool().
     * If it fails, have you already called open()?
     */
    CollectionPtr createCollection(const char*) noexcept;
    /*
     * @return CollectionPtr which can empty if the call did not succeed, e.g.
     * the collection was not found.
     *         Please check that with operator bool()
     */
    CollectionPtr readCollection(const char*) const noexcept;

    bool deleteCollection(CollectionPtr) noexcept;
    bool deleteCollection(const char*) noexcept;

private:
    std::unique_ptr<KSecretsStorePrivate> d;
};

#endif
// vim: tw=220:ts=4
