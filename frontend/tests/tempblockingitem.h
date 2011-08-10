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

#ifndef TEMPBLOCKINGITEM_H
#define TEMPBLOCKINGITEM_H

#include "backend/backenditem.h"

class TempBlockingCollection;

// temporary item that blocks every call
class TempBlockingItem : public BackendItem
{
    Q_OBJECT

public:
    friend class TempBlockingCollection;

    TempBlockingItem(const QString &id, TempBlockingCollection *parent);
    ~TempBlockingItem();
    virtual QString id() const;
    virtual BackendReturn<QString> label() const;
    virtual BackendReturn<void> setLabel(const QString &label);
    virtual BackendReturn<QCA::SecureArray> secret() const;
    virtual BackendReturn<void> setSecret(const QCA::SecureArray &secret);
    virtual BackendReturn<QMap<QString, QString> > attributes() const;
    virtual BackendReturn<void> setAttributes(const QMap<QString, QString> &attributes);
    virtual BackendReturn<QString> contentType() const;
    virtual BackendReturn<void> setContentType(const QString&);
    virtual QDateTime created() const;
    virtual QDateTime modified() const;
    virtual bool isLocked() const;
    virtual UnlockItemJob *createUnlockJob(const ItemUnlockInfo &unlockInfo);
    virtual LockItemJob *createLockJob();
    virtual DeleteItemJob *createDeleteJob(const ItemDeleteInfo& deleteJobInfo);
    virtual ChangeAuthenticationItemJob *createChangeAuthenticationJob();
    bool matches(const QMap<QString, QString> &attributes);

private Q_SLOTS:
    void deleteItemJobResult(KJob *job);

private:
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
