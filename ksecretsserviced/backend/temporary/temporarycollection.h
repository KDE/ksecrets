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

#ifndef TEMPORARYCOLLECTION_H
#define TEMPORARYCOLLECTION_H

#include "../backendcollection.h"

// forward declarations
class TemporaryCreateItemJob;

/**
 * A temporary collection stored in memory.
 */
class TemporaryCollection : public BackendCollection
{
    Q_OBJECT

public:
    /**
     * Constructor
     *
     * @param id the new collection's identifier
     * @param parent the collection manager that created this collection
     */
    TemporaryCollection(const QString &id, BackendCollectionManager *parent);

    /**
     * Destructor
     */
    virtual ~TemporaryCollection();

    /**
     * The unique identifier for this collection.
     */
    virtual QString id() const;

    /**
     * The human-readable label for this collection.
     * @todo error
     */
    virtual BackendReturn<QString> label() const;

    /**
     * Set this collection's human-readable label.
     *
     * @todo error
     * @param label the new label for this collection
     */
    virtual BackendReturn<void> setLabel(const QString &label);

    /**
     * The time this collection was created.
     */
    virtual QDateTime created() const;

    /**
     * The time this collection was last modified.
     */
    virtual QDateTime modified() const;

    /**
     * Check whether this collection is locked.
     *
     * @return true if the collection is locked, false if the collection is
     *         unlocked.
     */
    virtual bool isLocked() const;

    /**
     * List all items inside this backend.
     *
     * @return a list containing all items inside this backend. An empty list
     *         either means that no items were found or that an error occurred
     *         (eg. collection needs unlocking before listing the items).
     * @todo error
     */
    virtual BackendReturn<QList<BackendItem*> > items() const;

    /**
     * Return all items whose attributes match the search terms.
     *
     * @param attributes attributes against which the items should be matched
     * @return a list of items matching the attributes. An empty list either means that
     *         no items were found or that an error occurred (eg. collection needs
     *         unlocking before listing the items).
     * @todo error
     */
    virtual BackendReturn<QList<BackendItem*> > searchItems(
        const QMap<QString, QString> &attributes) const;

    /**
     * Create a job for unlocking this collection.
     */
    virtual UnlockCollectionJob *createUnlockJob(const CollectionUnlockInfo &unlockInfo);

    /**
     * Create a job for locking this collection.
     */
    virtual LockCollectionJob *createLockJob();

    /**
     * Create a job for deleting this collection.
     */
    virtual DeleteCollectionJob *createDeleteJob(const CollectionDeleteInfo& deleteJobInfo);

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
    virtual CreateItemJob *createCreateItemJob(const ItemCreateInfo& createInfo);

    /**
     * Create a job for changingcoll this collection's authentication.
     */
    virtual ChangeAuthenticationCollectionJob *createChangeAuthenticationJob( const Peer& );

    virtual ApplicationPermission applicationPermission(const QString&) const {
        return PermissionAllow;
    }
    virtual bool setApplicationPermission(const QString& , ApplicationPermission) {
        /* no ACL handling */
        return true;
    }

    /**
     * This member is called during collection creation to store the creator application path.
     * If the collection already has a creator set, then this method has no effect
     * @param exePath Path to the application that created this collection
     * @return false if the collection already has a creator set
     */
    virtual bool setCreatorApplication(QString exePath) {
        if (m_creator.isEmpty()) {
            m_creator = exePath;
            return true;
        }
        else
            return false;
    }
    
    /**
     * This method returns the executable path of the application which originally
     * created this collection.
     */
    virtual QString creatorApplication() const {
        return m_creator;
    }

protected:
    // Method for creating items. This is only called by TemporaryCreateItemJobs.
    BackendReturn<BackendItem*> createItem(const QString &label,
                                           const QMap<QString, QString> &attributes,
                                           const QCA::SecureArray &secret, 
                                           const QString &contentType,
                                           bool replace,
                                           bool locked);

private Q_SLOTS:
    /**
     * Remove an item from our list of known items.
     *
     * @param item Item to remove
     */
    void slotItemDeleted(BackendItem *item);

    /**
     * Called when a DeleteCollectionJob signals its result.
     *
     * @param job the job that finished
     */
    void deleteCollectionJobResult(KJob *job);

private:
    friend class TemporaryCreateItemJob;

    QString m_id;
    QString m_label;
    QDateTime m_created;
    QDateTime m_modified;
    QString m_creator;

    QList<BackendItem*> m_items;
};

#endif
