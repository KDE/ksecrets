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

#ifndef DAEMON_ITEM_H
#define DAEMON_ITEM_H

#include "adaptors/daemonsecret.h"

#include <QtCore/QObject>

#include "ksecretobject.h"

class BackendItem;
class Collection;

/**
 * Represents an item on the D-Bus implementing the org.freedesktop.Secret.Item
 * interface.
 */
class Item : public QObject, public QDBusContext, public KSecretObject<Item>
{
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * @param item Backend item object
     * @param collection Parent collection
     */
    Item(BackendItem *item, Collection *collection);

    /**
     * Return the item's path on the D-Bus.
     *
     * @return the Item object's path
     */
    const QDBusObjectPath &objectPath() const;

public: // called by D-Bus adaptors

    /**
     * Check whether this item is locked.
     *
     * @return true if the item is locked, false if it is unlocked
     */
    bool locked() const;

    /**
     * Set the item's attributes.
     *
     * @param attributes the attributes to set for the item
     */
    void setAttributes(const QMap<QString, QString> &attributes);

    /**
     * Get the item's attributes.
     *
     * @return the attributes stored for this item
     */
    QMap<QString, QString> attributes() const;

    /**
     * Set the item's human-readable label.
     *
     * @param label label to assign to this item
     */
    void setLabel(const QString &label);

    /**
     * Get the human-readable label assigned to this item.
     *
     * @return the item assigned to this item
     */
    QString label() const;

    /**
     * Retrieve the item's time of creation as a unix timestamp.
     *
     * @return item's time of creation
     */
    qulonglong created() const;

    /**
     * Retrieve the item's last modification date as a unix timestamp.
     *
     * @return item's last time of modification
     */
    qulonglong modified() const;

    /**
     * Delete the item from the collection it's in.
     *
     * @return the object path of a prompt object for the operation or "/" if the
     *         item was deleted without a prompt.
     */
    QDBusObjectPath deleteItem();

    /**
     * Get the secret stored within the item.
     *
     * @param session Session to use for securing the D-Bus transport
     * @return the (possibly encrypted) DaemonSecret structure
     */
    SecretStruct getSecret(const QDBusObjectPath &session);

    /**
     * Set the secret stored within the item.
     *
     * @param secret DaemonSecret to store inside the item
     */
    void setSecret(const SecretStruct &secret);

public:
    /**
     * Get the backend item associated with this frontend item.
     *
     * @return the backend item belonging to this frontend item
     * @remarks called by other frontend objects
     */
    BackendItem *backendItem() const;

private:
    BackendItem *m_item;
    QDBusObjectPath m_objectPath;
};

#endif
