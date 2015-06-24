/* This file is part of the KDE project
 *
 * Copyright (C) 2011 Valentin Rusu <kde@rusu.info>
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

#ifndef KSECRETSSERVICECOLLECTION_H
#define KSECRETSSERVICECOLLECTION_H

#include "ksecretsservicesecret.h"
#include "ksecretsservicecollectionjobs.h"
#include "ksecretsserviceglobals.h"
#include "ksecretsservicemacros.h"

#include <QObject>
#include <QMap>
#include <QSharedPointer>
#include <kcompositejob.h>
#include <qwindowdefs.h>

namespace KSecretsService {

class SecretItem;
class CollectionPrivate;
class CreateCollectionItemJob;
class SearchCollectionSecretsJob;
class SearchCollectionItemsJob;
class ReadCollectionItemsJob;
class ReadCollectionPropertyJob;
class WriteCollectionPropertyJob;
class ListCollectionsJob;
class ChangeCollectionPasswordJob;
class LockCollectionJob;

/**
 * This is the main KSecretsService entry class, used by the applications that need to store secrets,
 * like passwords or other sensitive data, in a secured and encrypted storage.
 *
 * All the methods of this class are performing asynchronously. That is, calling them immediatley return
 * a KJob descendant class. The actual action will be asynchronously done by this job only when it's start()
 * or exec() method is called. Note that exec() method usage is discouraged though.
 *
 * The client application is responsible for KJob start() or exec() method calling. Upon job finish, the
 * client application should check KJob error() status to know if the requested operation was successufully done.
 *
 * The KJob descendant classes returned by the methods also provide custom members, depending on the operation
 * they are intented to execute. Upon successuful execution, these members hold the corresponding return values.
 *
 * Please note that all the jobs returned by this class autodelete themselbes when done. If you application
 * need to access the returned items, then it should copy them away before returning from the job's done
 * signal handling method.
 *
 * @see KJob
 */
class KSECRETSSERVICE_EXPORT Collection : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(Collection)
public:
    virtual ~Collection();

    /**
     * Options used when findCollection method is called
     */
    enum FindCollectionOptions {
        OpenOnly         =0,    /// this will only try to open the collection without creating it if not found
        CreateCollection =1     /// the collection will be created if not found
    };

    /**
     * @see status()
     * @see statusChanged()
     */
    enum Status {
        Invalid         =0,     /// the collection objet is freshly initialized and none of it's methods have been called
        Pending         =1,     /// one of the collection methods was called but this object is yet to be connected to the backed
        FoundExisting   =2,     /// this object is connected to an existing backend connection
        NewlyCreated    =3,     /// this object is connected to a newly created connection
        NotFound        =4,     /// the collection was not found
        Deleted         =5      /// the collection was deleted. Calling methods in such a collection would lead to unpredictable results
    };

    /**
     * This will try to find a collection given its name. If not found, it'll create it depending on the
     * options given. Please note that for the sake of asynchronous behaviour, this actual collection finding
     * or creation will be postponed until you'll call one of the methods of the returned object.
     * @param collectionName collection name to be found
     * @param options @see FindCollectionOptions
     * @param collectionProperties specify collection properties and it's semantics depends on the options parameter
     *      if options == CreateCollection, then the newly created collection will receive these properties
     *      if options == OpenOnly, then the properties are used to match the existing collection si be careful
     *                              not to specify a property not given when creation a collection or you'll not be
     *                              able to find it with this method
     * @param promptParentWindowId identifies the applications window to be used as a parent for prompt windows
     * @return Collection instance to be used by the client application to further manipulated it's secrets
     * @note Please note that the collection returned by this method is not yet connected to the secret storing
     * infrastructure. As such, a NotFound status would not be immediatley known. Application should be prepared
     * to get such an error upon the execution of the first KJob returned by one of the other methods of this class.
     */
    static Collection * findCollection( const QString &collectionName,
                                        FindCollectionOptions options = CreateCollection,
                                        const QVariantMap collectionProperties = QVariantMap(),
                                        const WId &promptParentWindowId = 0 );

    /**
     * Use this method to find out the names of all known secret service collections on the running system
     */
    static ListCollectionsJob * listCollections();

    /**
     * This will get the actual findStatus of this collection
     * @return Status
     */
    Status status() const;

    /**
     * Try to delete this collection. The user might be prompted to confirm that
     * and as such he/she may choose not to confirm that operation.
     *
     * Please note that after successufully calling this method, this object
     * is no longer valid and calling other methods on it would lead to undefined
     * behaviour
     *
     * @return KJob. Check it's error() member to confirm that the collection hase been deleted
     */
    KJob * deleteCollection();

    /**
     * Change the name of this collection
     * @param newName is the new collection's name
     * @return KJob. Check it's error() member to confirm that the rename was effectively done.
     */
    KJob * renameCollection( const QString& newName );


    /**
     * Search for the items matching the specified attributes
     * KSecretsService uses overall the following standard attributes:
     * "Label" : item's or collection's label
     *
     * @param attributes hold the searched items attributes
     * You may want to initialize the map followin one of the followin cases:
     * @li put in empty strings such as
     * @code
     * StrinStringMap attrs;
     * attrs["Key"] = "";
     * @endcode
     * This will match all attributes having the key "Key"
     * @li use search string as values,
     * @code
     * StrinStringMap attrs;
     * attrs["Key"] = "string";
     * @endcode
     * This will try to exactly match "string" when finding items having "key" attributes
     * @li use regular expressions
     * @code
     * StrinStringMap attrs;
     * attrs["Key"] = "regexp:expr";
     * @endcode
     * This will find items having "Key" attribute, then will use the expr to do a QRegExp match
     * against attribute values
     *
     * @return SearchCollectionItemsJob which is a KJob inheritor
     */
    SearchCollectionItemsJob * searchItems( const StringStringMap &attributes );

    /**
     * Use this method to get several secrets without getting through getting items
     */
    SearchCollectionSecretsJob * searchSecrets( const StringStringMap &attributes );

    /**
     * Create a new item inside the current collection
     * @param label holds the item's label; this label will be automatically added to the attributes map under the index "Label" and it'll eventually replace an existing item on that slot
     * @param attributes holds an map of property names / property values
     * @param secret the secret the newly created item should hold
     * @param replace true if the and eventually existing secret is to be replaced by the on specified here
     * @return CreateCollectionItemJob instance used to start the actual item creation operation in the storage infrastructure
     *
     * @see SecretItem
     */
    CreateCollectionItemJob * createItem( const QString& label, const StringStringMap &attributes, const Secret &secret, CreateItemOptions options = DoNotReplaceExistingItem );

    /**
     * Retrieve items stored inside this collection
     * @return ReadItemJob instance that will hold the returned items upon successuful execution
     */
    ReadCollectionItemsJob * items() const;

    /**
     * Retrieve the lock status of this collection
     * @Note returned ReadCollectionPropertyJob::propertyValue is boolean
     */
    ReadCollectionPropertyJob* isLocked() const;

    /**
     * Retrieve this collection's label
     * @Note returned ReadCollectionPropertyJob::propertyValue is QString
     */
    ReadCollectionPropertyJob* label() const;

    /**
     * Get the creation timestamps of this collection
     * @Note returned ReadCollectionPropertyJob::propertyValue is a time_t
     */
    ReadCollectionPropertyJob* createdTime() const;

    /**
     * Get the last modified timestamp of this collection
     * @Note returned ReadCollectionPropertyJob::propertyValue is a time_t
     */
    ReadCollectionPropertyJob* modifiedTime() const;

    /**
     * Change this collection's label
     * @return WriteCollectionPropertyJob instance. Check it's error() status after executing it to confirm collection label change.
     */
    WriteCollectionPropertyJob* setLabel( const QString &label );

    /**
     * @return ReadCollectionPropertyJob instance. It's propertyValue() holds the status (true or false) upon job execution finish.
     */
    ReadCollectionPropertyJob* isValid();

    /**
     * Request current collection's password change. A dialog box will pop-up during the returned job execution
     * @return change password job
     */
    ChangeCollectionPasswordJob* changePassword();

    /**
     * Request collection lock. Locked collection's contents cannot be changed.
     * @note Accessing the other methods will trigger collection unlocking and as such the user may be prompted for the password
     * @return LockCollectionJob
     */
    LockCollectionJob* lock( const WId =0 );

Q_SIGNALS:
    /**
     * This signal is emmited with any collection status change.
     * Please cast the given integer to the Status type
     * @see Status
     */
    void statusChanged( int );
    /**
     * This signal is emmited when one of the collection's attributes changed or when one
     * of the contained items changed
     */
    void contentsChanged();
    /**
     * This signal is emmited when the collection was effectively deleted by the delete job
     * Calling other methods on this collection instance after this signal was emmited is undefined and
     * may lead to unpredictable results.
     */
    void deleted();
    /**
     * TODO: not yet implemented
     */
    void itemCreated( const KSecretsService::SecretItem& );
    /**
     * TODO: not yet implemented
     */
    void itemDeleted( const QString& itemLabel );
    /**
     * TODO: not yet implemented
     */
    void itemChanged( const KSecretsService::SecretItem& );


protected:
    explicit Collection();

private:
    explicit Collection( CollectionPrivate* );

private:
    friend class CollectionPrivate;
    friend class CollectionJob; // to give access to Private class
    friend class FindCollectionJob;
    friend class ListCollectionsJob;
    friend class DeleteCollectionJob;
    friend class RenameCollectionJob;
    friend class CreateCollectionItemJob;
    friend class SearchCollectionSecretsJob;
    friend class SearchCollectionItemsJob;
    friend class ReadCollectionItemsJob;
    friend class ReadCollectionPropertyJob;
    friend class WriteCollectionPropertyJob;
    friend class ChangeCollectionPasswordJob;
    friend class LockCollectionJob;
    friend class UnlockCollectionJob;

    /** @internal */
    void readIsValid( ReadCollectionPropertyJob* );
    /** @internal */
    void emitStatusChanged();
    /** @internal */
    void emitContentsChanged();
    /** @internal */
    void emitDeleted();

    QSharedPointer< CollectionPrivate > d;
};


}; // namespace

#endif // KSECRETSSERVICECOLLECTION_H
