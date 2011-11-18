/*
 * Copyright 2010, Michael Leupold <lemma@confuego.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DAEMON_COLLECTION_H
#define DAEMON_COLLECTION_H

#include "adaptors/daemonsecret.h"

#include <QtCore/QObject>
#include <QtDBus/QDBusObjectPath>

#include "ksecretobject.h"
#include <QDBusContext>

class BackendCollection;
class BackendItem;

class Service;
class Item;

/**
 * Represents a collection on the D-Bus implementing the org.freedesktop.Secret.Collection
 * interface.
 */
class Collection : public QObject, public QDBusContext, public KSecretObject<Collection>
{
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * @param collection Backend collection object
     * @param service Parent service
     */
    Collection(BackendCollection *collection, Service *service);

    /**
     * Return the collection's path on the D-Bus.
     *
     * @return the Collection object's path
     */
    const QDBusObjectPath &objectPath() const;

    /**
     * Get the items stored inside this collection.
     *
     * @return a list of object paths of items inside the collection
     */
    const QList<QDBusObjectPath> &items() const;

    /**
     * Set the collection's label.
     *
     * @param label label to set for the collection
     */
    void setLabel(const QString &label);

    /**
     * Get the collection's label.
     *
     * @return the collection's label
     */
    QString label() const;

    /**
     * Check whether this collection is locked.
     *
     * @return true if the collection is locked, false if it is unlocked
     */
    bool locked() const;

    /**
     * Get the collection's creation time as a unix timestamp.
     *
     * @return collection's creation time
     */
    qulonglong created() const;

    /**
     * Get the collection's last modification time as a unix timestamp.
     *
     * @return collection's modification time
     */
    qulonglong modified() const;

    /**
     * Delete this collection.
     *
     * @return the object path of a Prompt object or special value "/" if the
     *         collection was deleted without prompting
     */
    QDBusObjectPath deleteCollection();

    /**
     * Search items inside the collection.
     *
     * @param attributes search attributes to match items against
     */
    QList<QDBusObjectPath> searchItems(const QMap<QString, QString> &attributes);

    /**
     * Create a new item inside the collection.
     *
     * @param properties Properties for the new item
     * @param secret DaemonSecret for the new item
     * @param replace if true, an existing item with the same attributes will be replaced,
     *                if false and an item with the same attributes exists, no new item
     *                will be created
     * @param prompt object path of a prompt object if prompting is necessary or "/" if
     *               the action could be completed without a prompt
     * @return object path of the new item or "/" if prompting is necessary
     */
    QDBusObjectPath createItem(const QMap<QString, QVariant> &properties,
                               const SecretStruct &secret, 
                               bool replace, QDBusObjectPath &prompt);

    /**
     * Get the backend collection linked to this object.
     */
    BackendCollection *backendCollection() const;
    
    /**
     * Requests for current collection's password change
     * @return object path of the prompt that'll collect the new password
     */
    QDBusObjectPath changePassword();

Q_SIGNALS:
    /**
     * Signals the creation of a new item.
     *
     * @param item object path of the item that was created
     */
    void itemCreated(const QDBusObjectPath &item);

    /**
     * Signals the deletion of an item.
     *
     * @param item object path of the item that was deleted
     */
    void itemDeleted(const QDBusObjectPath &item);

    /**
     * Signals that an item inside the collection was changed.
     *
     * @param item object path of the item that was changed
     */
    void itemChanged(const QDBusObjectPath &item);

private Q_SLOTS:
    // handle itemCreated() calls from a backend collection
    void slotItemCreated(BackendItem *item);
    // handle itemDeleted() calls from a backend collection
    void slotItemDeleted(BackendItem *item);
    // handle itemChanged() calls from a backend collection
    void slotItemChanged(BackendItem *item);
    void slotCollectionDeleted(BackendCollection *coll);

private:
    Service *m_service; // parent service
    BackendCollection *m_collection;
    QDBusObjectPath m_objectPath;
    QList<QDBusObjectPath> m_itemPaths; // cache for items' object paths
    QMap< QDBusObjectPath, Item*>   m_items;
    bool    m_deleted;
};

#endif
