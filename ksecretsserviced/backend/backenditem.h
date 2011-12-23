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

#ifndef BACKENDITEM_H
#define BACKENDITEM_H

#include "backendreturn.h"
#include "backendjob.h"

#include "jobinfostructs.h"

#include <QtCore/QMap>
#include <QtCore/QDateTime>
#include <QtCrypto>

class BackendCollection;

/**
 * Abstract base for items inside a backend collection.
 *
 * When inheriting from BackendItem it's usually sufficient to define the pure
 * virtual methods. The inheriting class is responsible for emitting the
 * \sa itemDeleted and \sa itemChanged signals.
 */
class BackendItem : public QObject
{
    Q_OBJECT

public:
    /**
     * Constructor
     */
    BackendItem(BackendCollection *collection = 0);

    /**
     * Destructor
     */
    virtual ~BackendItem();

    /**
     * The unique identifer for this item.
     */
    virtual QString id() const = 0;

    /**
     * The human-readable label for this item.
     */
    virtual BackendReturn<QString> label() const = 0;

    /**
     * Set the human-readable label for this item.
     *
     * @param label the new label for this item
     */
    virtual BackendReturn<void> setLabel(const QString &label) = 0;

    /**
     * Get the secret stored inside this item.
     *
     * @return the secret
     * @todo this will most likely become non-const as that might make sense
     *       with certain backends me reckons.
     */
    virtual BackendReturn<QCA::SecureArray> secret() const = 0;

    /**
     * Set the secret stored inside this item.
     *
     * @param secret the secret to store
     */
    virtual BackendReturn<void> setSecret(const QCA::SecureArray &secret) = 0;

    /**
     * The attributes of the item.
     *
     * @return the item's attributes
     */
    virtual BackendReturn<QMap<QString, QString> > attributes() const = 0;

    /**
     * Set the attributes of this item.
     *
     * @param attributes attributes to assign to this item
     */
    virtual BackendReturn<void> setAttributes(const QMap<QString, QString> &attributes) = 0;

    /**
     * The time this item was created.
     */
    virtual QDateTime created() const = 0;

    /**
     * The time this item was last modified.
     */
    virtual QDateTime modified() const = 0;

    /**
     * Check whether this item is locked.
     *
     * @return true if the item is locked, false if the item is
     *         unlocked.
     */
    virtual bool isLocked() const = 0;

    /**
     * Create a call for unlocking this item.
     */
    virtual UnlockItemJob *createUnlockJob(const ItemUnlockInfo& unlockInfo) = 0;

    /**
     * Create a call for locking this item.
     */
    virtual LockItemJob *createLockJob() = 0;

    /**
     * Create a call for deleting this item.
     */
    virtual DeleteItemJob *createDeleteJob(const ItemDeleteInfo& deleteJobInfo) = 0;

    /**
     * Create a job for changing this item's authentication.
     */
    virtual ChangeAuthenticationItemJob *createChangeAuthenticationJob() = 0;

    /**
     * Returns the collection which holds this item or NULL if this item is not (yet)
     * contained in a collection
     */
    BackendCollection *collection() const {
        return m_collection;
    }
    
    void setCollection( BackendCollection* parent ) {
        m_collection = parent;
    }
    
    virtual BackendReturn<QString> contentType() const =0;
    virtual BackendReturn<void> setContentType(const QString&) =0;

Q_SIGNALS:
    /**
     * Emitted right before the item is about to be deleted.
     *
     * @param item the item being deleted
     * @remarks this is only used internally and connected to collection's itemDeleted
     *          signal.
     */
    void itemDeleted(BackendItem *item);

    /**
     * This signal must be emitted whenever the item changes.
     *
     * @param item the item that changed
     * @remarks this is only used internally and connected to collection's itemChanged
     *          signal.
     */
    void itemChanged(BackendItem *item);

protected:
    BackendCollection *m_collection;
};

#endif
