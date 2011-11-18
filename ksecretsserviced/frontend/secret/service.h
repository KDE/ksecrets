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

#ifndef DAEMON_SERVICE_H
#define DAEMON_SERVICE_H

#include <backend/backendmaster.h>

#include <QtCore/QObject>
#include <QtCore/QMultiMap>
#include <QtDBus/QDBusObjectPath>
#include <QtDBus/QDBusConnectionInterface>

#include "ksecretobject.h"
#include "adaptors/secretstruct.h"
#include <QDBusContext>

class Collection;


/**
 * Main entry point of the secret service D-Bus daemon implementing the
 * org.freedesktop.Secret.Service interface.
 *
 * @todo Implement proper session handling
 */
class Service : public QObject, public QDBusContext, public KSecretObject<Service>
{
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * @param master Backend master to use
     * @param parent Parent object
     */
    explicit Service(BackendMaster *master, QObject *parent = 0);

    /**
     * Return the service's path on the D-Bus.
     *
     * @return the Service object's path
     */
    const QDBusObjectPath &objectPath() const;

    /**
     * Accessor for the list of known collections.
     *
     * @return all collections known to the service.
     */
    const QList<QDBusObjectPath> &collections() const;

    /**
     * Open a unique session for the caller application.
     *
     * @param algorithm the algorithm the caller wishes to use
     * @param input input arguments for the algorithm
     * @param result the object path of the session, if session was created
     * @return output of the session algorithm negotiation
     */
    QVariant openSession(const QString &algorithm, const QVariant &input, QDBusObjectPath &result);

    /**
     * Create a new collection with the specified properties.
     *
     * @param properties properties for the new collection
     * @param prompt a prompt object if prompting is necessary, or "/" if no prompt was needed
     * @return the new collection object or "/" if prompting is necessary
     */
    QDBusObjectPath createCollection(const QMap<QString, QVariant> &properties,
				     const QString& alias,
                                     QDBusObjectPath &prompt);
    
    /**
     * Find items in any collection.
     *
     * @param attributes the attributes an item has to match in order to be found
     * @param locked items found that require authentication
     * @return items found that are already unlocked
     */
    QList<QDBusObjectPath> searchItems(const QMap<QString, QString> &attributes,
                                       QList<QDBusObjectPath> &locked);

    /**
     * Unlock the specified items or collections.
     *
     * @param objects objects to unlock
     * @param prompt a prompt object which can be used to unlock the remaining objects or
     *               "/" if no prompt is necessary.
     * @return objects that were unlocked without a prompt
     */
    QList<QDBusObjectPath> unlock(const QList<QDBusObjectPath> &objects, QDBusObjectPath &prompt);

    /**
     * Lock the specified items or collections.
     *
     * @param objects objects to lock
     * @param prompt a prompt to lock the objects or "/" if no prompt is necessary
     * @return objects that were locked without a prompt
     */
    QList<QDBusObjectPath> lock(const QList<QDBusObjectPath> &objects, QDBusObjectPath &prompt);

    /**
     * Retrieve multiple secrets from different items.
     *
     * @param items items to get secrets for
     * @param session the session to use to encode the secrets
     * @return the secrets for the items.
     */
    QMap<QDBusObjectPath, SecretStruct> getSecrets(const QList<QDBusObjectPath> &items,
            const QDBusObjectPath &session);

    BackendMaster *master() const { return m_master; }
    
Q_SIGNALS:
    /**
     * Emitted when a new collection is discovered or created.
     *
     * @param collection Objectpath of the collection that was discovered/created
     */
    void collectionCreated(const QDBusObjectPath &collection);

    /**
     * Emitted when an existing collection was deleted or disappeared.
     *
     * @param collection Objectpath of the collection that was deleted
     */
    void collectionDeleted(const QDBusObjectPath &collection);

    /**
     * Emitted when an existing collection changed.
     *
     * @param collection Objectpath of the collection that changed
     */
    void collectionChanged(const QDBusObjectPath &collection);

private Q_SLOTS:
    // handle collectionCreated() calls from master
    void slotCollectionCreated(BackendCollection *collection);
    // handle collectionDeleted() calls from master
    void slotCollectionDeleted(BackendCollection *collection);
    // handle collectionChanged() calls from master
    void slotCollectionChanged(BackendCollection *collection);

    void onDbusDisconnected(QString path);


private:
    BackendMaster *m_master;
    QList<QDBusObjectPath> m_collections; // cache object paths of collections
    const QDBusObjectPath m_basePath;
};

#endif
