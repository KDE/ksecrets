/* This file is part of the KDE project
 *
 * Copyright (C) 2011 Valentin Rusu <valir@kde.org>
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

#ifndef KSECRETSCOLLECTION_H
#define KSECRETSCOLLECTION_H

#include "ksecretsvalue.h"
#include "ksecretsglobals.h"
#include "ksecretsitem.h"

#include <QObject>
#include <QMap>
#include <QSharedPointer>
#include <QFuture>
#include <QDateTime>
#include <qwindowdefs.h>

#include <ksecrets_export.h>

namespace KSecrets {

class SecretItem;
class CollectionPrivate;
class Collection;

typedef QSharedPointer<Collection> CollectionPtr;

/**
 * This is the main KSecrets entry class, used by the applications that
 * need to store secrets, like passwords or other sensitive data, in a secured
 * and encrypted storage.
 */
class KSECRETS_EXPORT Collection : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(Collection)
public:
    virtual ~Collection();

    /**
     * @see status()
     * @see statusChanged()
     */
    enum Status {
        Invalid
        = 0, /// the collection objet is freshly initialized and none of
             /// it's methods have been called
        Pending
        = 1, /// one of the collection methods was called but this object
             /// is yet to be connected to the backed
        FoundExisting
        = 2, /// this object is connected to an existing backend connection
        NewlyCreated
        = 3, /// this object is connected to a newly created connection
        NotFound = 4, /// the collection was not found
        Deleted = 5 /// the collection was deleted. Calling methods in such a
                    /// collection would lead to unpredictable results
    };

    /**
     * Use this method to find out the names of all known secret service
     * collections on the running system
     */
    static QFuture<QList<CollectionPtr> > listCollections();

    /**
     * This will get the actual findStatus of this collection
     * @return Status
     */
    Status status() const;

    /**
     * Try to delete this collection. The user might be prompted to confirm
     *that
     * and as such he/she may choose not to confirm that operation.
     *
     * Please note that after successufully calling this method, this object
     * is no longer valid and calling other methods on it would lead to
     *undefined
     * behaviour
     */
    QFuture<bool> deleteCollection();

    /**
     * Change the name of this collection
     * @param newName is the new collection's name
     */
    QFuture<bool> renameCollection(const QString& newName);

    /**
     * Search for the items matching the specified attributes
     * KSecrets uses overall the following standard attributes:
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
     * This will try to exactly match "string" when finding items having "key"
     *attributes
     * @li use regular expressions
     * @code
     * StrinStringMap attrs;
     * attrs["Key"] = "regexp:expr";
     * @endcode
     * This will find items having "Key" attribute, then will use the expr to
     *do
     *a QRegExp match
     * against attribute values
     *
     * @return QFuture whose method results() will bring all the items found
     *
     */
    QFuture<QList<SecretItemPtr> > searchItems(
        const AttributesMap& attributes);

    QFuture<QList<SecretItemPtr> > searchItems(const QString& label);

    QFuture<QList<SecretItemPtr> > searchItems(
        const QString& label, const AttributesMap& attributes);

    /**
     * Use this method to get several secrets without getting through getting
     * items
     */
    QFuture<QList<SecretPtr> > searchSecrets(
        const StringStringMap& attributes);

    /**
     * Create a new item inside the current collection
     * @param label holds the item's label
     * this label will be automatically
     * added to the attributes map under the index "Label" and it'll
     *eventually
     * replace an existing item on that slot
     * @param attributes holds an map of property names / property values
     * @param secret the secret the newly created item should hold
     * @param replace true if the and eventually existing secret is to be
     *replaced by the on specified here
     *
     * @see SecretItem
     */
    QFuture<bool> createItem(const QString& label, const Secret& secret,
        const AttributesMap& attributes = AttributesMap(),
        CreateItemOptions options = DoNotReplaceExistingItem);

    /**
     * Retrieve items stored inside this collection
     */
    QFuture<QList<SecretItemPtr> > items() const;

    /**
     * Retrieve the lock status of this collection
     */
    QFuture<bool> isLocked() const;

    /**
     * Retrieve this collection's label
     */
    QFuture<QString> label() const;

    /**
     * Get the creation timestamps of this collection
     */
    QFuture<QDateTime> createdTime() const;

    /**
     * Get the last modified timestamp of this collection
     */
    QFuture<QDateTime> modifiedTime() const;

    /**
     * Change this collection's label
     */
    QFuture<bool> setLabel(const QString& label);

    /**
     * Applications should check this before attempting other operations on
     *this
     * collection.
     *
     * @return true if the collection is actually connected to its backend
     */
    QFuture<bool> isValid();

    /**
     * Request collection lock. Locked collection's contents cannot be
     * changed.
     */
    QFuture<bool> lock();

Q_SIGNALS:
    /**
     * This signal is emmited with any collection status change.
     * Please cast the given integer to the Status type
     * @see Status
     */
    void statusChanged(int);
    /**
     * This signal is emmited when one of the collection's attributes changed
     * or
     * when one
     * of the contained items changed
     */
    void contentsChanged();
    /**
     * This signal is emmited when the collection was effectively deleted by
     * the
     * delete job
     * Calling other methods on this collection instance after this signal was
     * emmited is undefined and
     * may lead to unpredictable results.
     */
    void deleted();
    /**
     */
    void itemCreated(const KSecrets::SecretItem&);
    /**
     */
    void itemDeleted(const QString& itemLabel);
    /**
     */
    void itemChanged(const KSecrets::SecretItem&);

protected:
    friend class ServicePrivate; // collections are instantiated from the
                                 // service only
    explicit Collection();

private:
    explicit Collection(CollectionPrivate*);

private:
    friend class CollectionPrivate;

    /** @internal */
    void emitStatusChanged();
    /** @internal */
    void emitContentsChanged();
    /** @internal */
    void emitDeleted();

    QSharedPointer<CollectionPrivate> d;
};

}; // namespace

#endif // KSECRETSCOLLECTION_H
