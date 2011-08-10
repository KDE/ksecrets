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

#ifndef TEMPORARYITEM_H
#define TEMPORARYITEM_H

#include "../backenditem.h"

class TemporaryCollection;

class TemporaryItem : public BackendItem
{
    Q_OBJECT

public:
    friend class TemporaryCollection;

    /**
     * Constructor.
     *
     * @param id unique identifier of the new item
     * @param collection collection that created this item
     */
    TemporaryItem(const QString &id, TemporaryCollection *parent);

    /**
     * Destructor.
     */
    ~TemporaryItem();

    /**
     * The unique identifer for this item.
     */
    virtual QString id() const;

    /**
     * The human-readable label for this item.
     * @todo error
     */
    virtual BackendReturn<QString> label() const;

    /**
     * Set the human-readable label for this item.
     *
     * @param label the new label for this item
     * @todo error
     */
    virtual BackendReturn<void> setLabel(const QString &label);

    /**
     * Get the secret stored inside this item.
     *
     * @return the secret
     * @todo this will most likely become non-const as that might make sense
     *       with certain backends me reckons.
     */
    virtual BackendReturn<QCA::SecureArray> secret() const;

    /**
     * Set the secret stored inside this item.
     *
     * @param secret the secret to store
     */
    virtual BackendReturn<void> setSecret(const QCA::SecureArray &secret);

    /**
     * The attributes of the item.
     *
     * @return the item's attributes
     */
    virtual BackendReturn<QMap<QString, QString> > attributes() const;

    /**
     * Set the attributes of this item.
     *
     * @param attributes attributes to assign to this item
     */
    virtual BackendReturn<void> setAttributes(const QMap<QString, QString> &attributes);

    virtual BackendReturn<QString> contentType() const;
    virtual BackendReturn<void> setContentType(const QString&);
    
    /**
     * The time this item was created.
     */
    virtual QDateTime created() const;

    /**
     * The time this item was last modified.
     */
    virtual QDateTime modified() const;

    /**
     * Check whether this item is locked.
     *
     * @return always returns false as temporary items can't be locked.
     */
    virtual bool isLocked() const;

    /**
     * Create a job for unlocking this item.
     */
    virtual UnlockItemJob *createUnlockJob(const ItemUnlockInfo& unlockInfo);

    /**
     * Create a job for locking this item.
     */
    virtual LockItemJob *createLockJob();

    /**
     * Create a job for deleting this item.
     */
    virtual DeleteItemJob *createDeleteJob(const ItemDeleteInfo& deleteJobInfo);

    /**
     * Create a job for changing this item's authentication.
     */
    virtual ChangeAuthenticationItemJob *createChangeAuthenticationJob();

    /**
     * Check whether this item matches the attributes given.
     *
     * @param attributes attributes to match against
     * @return true if this item matches the attributes, false
     *         if the item doesn't match the attributes.
     */
    bool matches(const QMap<QString, QString> &attributes);

private Q_SLOTS:
    /**
     * Called when a DeleteItemJob signals its result.
     *
     * @param job the job that finished
     */
    void deleteItemJobResult(KJob *job);

private:
    /**
     * Mark this item as modified and emit the \sa itemChanged signal.
     */
    void markAsModified();

    QString m_id;
    QString m_label;
    QDateTime m_created;
    QDateTime m_modified;
    QMap<QString, QString> m_attributes;

    QCA::SecureArray m_secret;
    QString m_contentType;
};

#endif
