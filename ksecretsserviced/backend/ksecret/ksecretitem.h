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

#ifndef KSECRETITEM_H
#define KSECRETITEM_H

#include "../backenditem.h"

#include <QtCore/QSet>

class KSecretStream;
class KSecretCollection;
class KSecretDeleteItemJob;

/**
 * Represents an item stored inside a ksecret file.
 */
class KSecretItem : public BackendItem
{
    Q_OBJECT

    KSecretItem();
    
public:
    /**
     * Constructor.
     *
     * @param id unique identifier of the new item
     * @param collection collection that created this item
     */
    KSecretItem(const QString &id, KSecretCollection *parent);

    /**
     * Destructor.
     */
    ~KSecretItem();

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
    
    virtual BackendReturn<QString> contentType() const;
    virtual BackendReturn<void> setContentType(const QString& contentType);

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
     * @return true if the item is locked, false else
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

protected:

    /**
     * Delete the item.
     *
     * @remarks this is called by KSecretDeleteItemJob
     */
    BackendReturn<bool> deleteItem();

Q_SIGNALS:
    /**
     * Emitted when the item has been "used" ie. one of its values has been
     * read or written.
     *
     * @param item the item that was used
     * @remarks this is used internally to implement the "close-if-unused" timer
     */
    void itemUsed(BackendItem *item);

    /**
     * This signal is emitted when an item's attributes change so the
     * collection can rebuild the attribute lookup hashes related to this
     * item.
     *
     * @param item Item whose attributes changed
     */
    void attributesChanged(KSecretItem *item);

private:
    friend class KSecretCollection;
    friend class KSecretDeleteItemJob;
    
    friend KSecretStream& operator << ( KSecretStream &out, KSecretItem* );
    friend KSecretStream& operator >> ( KSecretStream &in, KSecretItem*& );

    /**
     * Mark this item as modified and emit the \sa itemChanged signal.
     */
    void markAsModified();

    /**
     * Mark this item as used and emit the \sa itemUsed signal.
     */
    void markAsUsed() const;

//     KSecretCollection *m_collection;

    QString m_id;
    QString m_label;
    QDateTime m_created;
    QDateTime m_modified;
    QMap<QString, QString> m_attributes;

    QCA::SecureArray m_secret;
    QString m_contentType;
};

KSecretStream& operator << ( KSecretStream &out, KSecretItem* );
KSecretStream& operator >> ( KSecretStream &in, KSecretItem*& );


#endif
