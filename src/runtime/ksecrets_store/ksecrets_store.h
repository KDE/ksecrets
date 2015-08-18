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
class KSecretsItemPrivate;
class KSecretsCollectionPrivate;

using KSecretsItemPrivatePtr = std::shared_ptr<KSecretsItemPrivate>;
using KSecretsCollectionPrivatePtr = std::shared_ptr<KSecretsCollectionPrivate>;


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
 * Before using a collection, the application should setup it.
 * It's possible to indicate if readonly mode is possible. That would be the prefered way of
 * accessing the store, as usually applications only need some previously entered password.
 * The setup operation fails if the readonly flag is given and if the secrets file is not found.
 *
 * When setting-up without readonly flag, the file is created if not found, then the file is exclusively locked. The lock is
 * released when the class is destroyed. You should keep the file locked as shortly as
 * possible, in order to avoid deadlocks between applications that also need to read the
 * secrets. For more information @see setup().
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
public:
    using AttributesMap = std::map<std::string, std::string>;

    struct ItemValue {
        std::string contentType;
        std::vector<char> contents;
        bool operator==(const ItemValue& that) const noexcept { return contentType == that.contentType && contents == that.contents; }
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
    public:
        Item(const Item&) = default;
        Item& operator=(const Item&) = default;

        std::string label() const noexcept;
        bool setLabel(const char*) noexcept;

        AttributesMap attributes() const;
        /**
         * @brief
         *
         * @note This method uses C++11 move semantics so the AttributesMap local variable used to call this member
         * will no longer be valid upon call return.
         *
         * @param AttributesMap
         */
        bool setAttributes(AttributesMap) noexcept;

        ItemValue value() const noexcept;
        bool setValue(ItemValue) noexcept;

        std::time_t createdTime() const noexcept;
        std::time_t modifiedTime() const noexcept;

    protected:
        Item();
        friend class KSecretsStore;

    private:
        KSecretsItemPrivatePtr d;
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
    public:
        std::string label() const noexcept;
        bool setLabel(const char*) noexcept;

        std::time_t createdTime() const noexcept;
        std::time_t modifiedTime() const noexcept;

        using ItemList = std::vector<ItemPtr>;
        ItemList dirItems() const noexcept;
        ItemList searchItems(const AttributesMap&) const noexcept;
        ItemList searchItems(const char*) const noexcept;
        ItemList searchItems(const char*, const AttributesMap&) const noexcept;

        /**
         * Creates an item in the collection in one go. This is more efficient than
         * initializing en empty than setting the label, eventually the parameters and
         * the value. The returned item may still be modified, keep in mind that each
         * method call will trigger an store file update.
         *
         * @note This member uses C++11 move semantics for the AttributesMap and ItemValue
         *
         * @return ItemPtr which can be empty if creating the item was not
         * possible. So please check it via it's operator bool() before using
         * it. Open possible failure reason is that an item with the same label already
         * exists in the collection.
         */
        ItemPtr createItem(const char*, AttributesMap, ItemValue) noexcept;
        /**
         * Convenience method for creating items without supplemental
         * attributes.
         *
         * @return ItemPtr which can be empty if creating the item was not
         * possible. So please check it via it's operator bool() before using
         * it.
         */
        ItemPtr createItem(const char* label, ItemValue) noexcept;

        bool deleteItem(ItemPtr) noexcept;

        Collection(KSecretsCollectionPrivatePtr dptr);
    protected:
        Collection();
        Collection(const Collection&) = default;
        Collection& operator=(const Collection&) = default;
        friend class KSecretsStore;
        friend class KSecretsStorePrivate;

    private:
        KSecretsCollectionPrivatePtr d;
    };
    using CollectionPtr = std::shared_ptr<Collection>;

    /**
     * Default constructor.
     *
     * This constructor only initializes the store class. You should call
     * the setup() method right after the initialization and before any other
     * methods of this API. If the password was not set by the pam_ksecrets module
     * then you have to also call setCredentials()
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
        PrematureEndOfFileEncountered,
        UnknownError,
        SystemError
    };

    /**
     * @brief Small structure returned by KSecretsStore API calls
     *
     * It introduces a bool() operator client applications could use to check the correct
     * issue of the respective API call.
     */
    template <StoreStatus G> struct CallResult {
        CallResult()
            : status_(StoreStatus::SystemError)
            , errno_(-1)
        {
        }
        explicit CallResult(StoreStatus s, int err = -1)
            : status_(s)
            , errno_(err)
        {
        }
        StoreStatus status_;
        int errno_;
        operator bool() const { return status_ == G; }
    };

    template <typename T> struct AlwaysGoodPred {
        bool operator()(const T&) const noexcept { return true; }
    };
    /**
     * @brief Small structure returned by API calls that create things. It's possible to check the returned
     *        value by giving this template a custom OK_PRED of type equivalent to std::function<bool(const R&)>
     */
    template <StoreStatus G, typename R, typename OK_PRED = AlwaysGoodPred<R> > struct CallResultWithValue : public CallResult<G> {
        CallResultWithValue()
            : CallResult<G>()
        {
        }
        explicit CallResultWithValue(StoreStatus s, int err = -1)
            : CallResult<G>(s, err)
        {
        }
        R result_;
        operator bool() const noexcept { return CallResult<G>::operator bool() && OK_PRED()(result_); }
    };

    using SetupResult = CallResult<StoreStatus::Good>;

    /**
     * Before usage, the store must be setup, that is, it must know its file path.
     * This call creates the file if it's not found and the readOnly flag is set to false.
     * The file is not created when the readOnly flag is set to false in order to prevent
     * unintended side effects. If the file don't exists, the SetupResult.statu_ is set
     * to StoreStatus::SystemError and errno should be 2
     *
     * @return SetupResult whose operator bool could be used to check the error condition
     */
    std::future<SetupResult> setup(const char* path, bool readOnly = true);

    using CredentialsResult = CallResult<StoreStatus::CredentialsSet>;

    /**
     * Set the system-wide credentials for the secrets store
     *
     * This method is typically called from the pam module but it can also be
     * called from another program after prompting user for the password, for example.
     * Client applications only need to call this once per user session.
     */
    std::future<CredentialsResult> setCredentials(const char* password = nullptr, const char* keyNameEcrypting = "ksecrets:encrypting", const char* keyNameMac = "ksecrets:mac");

    bool isGood() const noexcept;

    using CollectionNames = std::vector<std::string>;
    using DirCollectionsResult = CallResultWithValue<StoreStatus::Good, CollectionNames>;
    DirCollectionsResult dirCollections() const noexcept;

    template <typename P> struct IsGoodSmartPtr {
        bool operator()(const P& p) { return p.get() != nullptr; }
    };
    using CreateCollectionResult = CallResultWithValue<StoreStatus::Good, CollectionPtr, IsGoodSmartPtr<CollectionPtr> >;
    /**
     * @return CollectionPtr which can empty if the call did not succeed
     *         Please check that with operator bool()
     *         If it fails, have you already called setup()?
     *
     */
    CreateCollectionResult createCollection(const char*) noexcept;

    using ReadCollectionResult = CallResultWithValue<StoreStatus::Good, CollectionPtr, IsGoodSmartPtr<CollectionPtr> >;
    /**
     * @return CollectionPtr which can empty if the call did not succeed, e.g.
     * the collection was not found
     *         Please check that with operator bool()
     */
    ReadCollectionResult readCollection(const char*) const noexcept;

    /**
     * @brief Return value for deleteCollection method variants. Please note the operator bool() can be used to both
     * check the collection's status after the deleteCollection call, and the result of the delete operation, as it's
     * using the `bool` specialized version of the IsGoodPred
     */
    using DeleteCollectionResult = CallResultWithValue<StoreStatus::Good, bool, AlwaysGoodPred<bool> >;
    DeleteCollectionResult deleteCollection(CollectionPtr) noexcept;
    DeleteCollectionResult deleteCollection(const char*) noexcept;

private:
    std::unique_ptr<KSecretsStorePrivate> d;
};

template <> struct KSecretsStore::AlwaysGoodPred<bool> {
    bool operator()(const bool& b) const noexcept { return b; }
};

#endif
// vim: tw=220:ts=4
