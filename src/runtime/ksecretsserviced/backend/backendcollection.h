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

#ifndef BACKENDCOLLECTION_H
#define BACKENDCOLLECTION_H

#include "backendreturn.h"
#include "backendjob.h"

#include <jobinfostructs.h>
#include <acl.h>

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QDateTime>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCrypto>

class BackendCollectionManager;
class BackendItem;

/**
 * Abstract base class for backend collections.
 *
 * When inheriting from BackendCollection it's usually sufficient to define the pure
 * virtual methods. The inheriting class is responsible for emitting the
 * \sa itemCreated, \sa itemDeleted, \sa itemChanged, \sa collectionDeleted and
 * \sa collectionChanged signals.
 */
class BackendCollection : public QObject
{
    Q_OBJECT

public:

    /**
     * Constructor
     */
    BackendCollection(BackendCollectionManager *manager = 0);

    /**
     * Destructor
     */
    virtual ~BackendCollection();

    /**
     * The unique identifier for this collection.
     */
    virtual QString id() const = 0;

    /**
     * The human-readable label for this collection.
     */
    virtual BackendReturn<QString> label() const = 0;

    /**
     * Set this collection's label human-readable label.
     *
     * @param label the new label for this collection
     */
    virtual BackendReturn<void> setLabel(const QString &label) = 0;

    /**
     * The time this collection was created.
     */
    virtual QDateTime created() const = 0;

    /**
     * The time this collection was last modified.
     */
    virtual QDateTime modified() const = 0;

    /**
     * Check whether this collection is locked.
     *
     * @return true if the collection is locked, false if the collection is
     *         unlocked.
     */
    virtual bool isLocked() const = 0;

    /**
     * List all items inside this backend.
     *
     * @return a list containing all items inside this backend.
     */
    virtual BackendReturn<QList<BackendItem*> > items() const = 0;

    /**
     * Return all items whose attributes match the search terms.
     *
     * @param attributes attributes against which the items should be matched
     * @return a list of items matching the attributes. An empty list either means that
     *         no items were found or that an error occurred (eg. collection needs
     *         unlocking before listing the items).
     */
    virtual BackendReturn<QList<BackendItem*> > searchItems(
        const QMap<QString, QString> &attributes) const = 0;

    /**
     * Create a job for unlocking this collection.
     */
    virtual UnlockCollectionJob *createUnlockJob(const CollectionUnlockInfo &unlockInfo) = 0;

    /**
     * Create a job for locking this collection.
     */
    virtual LockCollectionJob *createLockJob() = 0;

    /**
     * Create a job for deleting this collection.
     */
    virtual DeleteCollectionJob *createDeleteJob(const CollectionDeleteInfo& deleteJobInfo) = 0;

    /**
     * Create a job for creating an item.
     *
     * @param label label to assign to the new item
     * @param attributes attributes to store for the new item
     * @param secret the secret to store
     * @param replace if true, an existing item with the same attributes
     *                will be replaced, if false no item will be created
     *                if one with the same attributes already exists
     * @param locked if true, the item will be locked after creation
     */
    virtual CreateItemJob *createCreateItemJob(const ItemCreateInfo& createInfo) = 0;

    /**
     * Create a job for changing this collection's authentication.
     */
    virtual ChangeAuthenticationCollectionJob *createChangeAuthenticationJob( const Peer& ) = 0;

    /**
     * Check if the given DBus peer has access to this collection
     * Inheritors may want to implement doCheckPeerAclAllowed.
     * Depending on the ApplicationPermission stored for the given  peer, his
     * method may pop-up a dialog box asking user for permission.
     * NOTE This method is not virtual to prevent accidental or incorrect overloading.
     * @param Peer the peer information or NULL if not called over DBus
     * @return true if the given peer has access or if peer == NULL
     */
    bool isPeerAclAllowed(const Peer* peer) const;

    /**
     * Get the permission an executable file has for this collection
     * @param path Path to the executable file
     * @return one of the ApplicationPermission enumeration values
     */
    virtual ApplicationPermission applicationPermission(const QString& path) const = 0;

    /**
     * Sets the permission an executable application has for this collection.
     * Note that the permissions are stored as per executable path to prevent application
     * identify spoofing. User should always carefully read the path to the executable when
     * giving access to this collection
     * @param path the complete path to the exectuable
     * @param perm one of the values from the ApplicationPermission enumeration
     * @return true if the permission have been retained. The return value is false if
     * the givent executable file does not exist on disc.
     */
    virtual bool setApplicationPermission(const QString& path, ApplicationPermission perm) = 0;
    
    /**
     * This member is called during collection creation to store the creator application path.
     * If the collection already has a creator set, then this method has no effect
     * @param exePath Path to the application that created this collection
     * @return false if the collection already has a creator set
     */
    virtual bool setCreatorApplication(QString exePath) =0;
    
    /**
     * This method returns the executable path of the application which originally
     * created this collection.
     */
    virtual QString creatorApplication() const =0;

protected:
    /**
     * This member is called by isPeerAclAllowed when peer != NULL and when
     * the application permission for this peer is set to PermissionAsk. An
     * UI job is used to get ask the user if access is to be allowed or not.
     * @param peer the dbus peer information
     * @return always true, allowing access to this collection
     */
    bool askPeerAclAllowed(const Peer*) const;

Q_SIGNALS:
    /**
     * Emitted when a new item within this collection has been created.
     *
     * @param item item that has been created
     */
    void itemCreated(BackendItem *item);

    /**
     * Emitted when an item within this collection has been deleted.
     *
     * @param item item that is being deleted
     * @remarks The slot connected to this signal is the last place to safely
     *          access the item before it is actually removed.
     */
    void itemDeleted(BackendItem *item);

    /**
     * Emitted when an item within this collection has been changed.
     *
     * @param item item that has been changed
     */
    void itemChanged(BackendItem *item);

    /**
     * Emitted right before a collection is being deleted in response to a deleteCollection
     * call.
     *
     * @param collection the collection being deleted
     */
    void collectionDeleted(BackendCollection *collection);

    /**
     * Emitted when the collection has been changed.
     *
     * @param collection the collection that changed
     */
    void collectionChanged(BackendCollection *collection);
};

#endif
