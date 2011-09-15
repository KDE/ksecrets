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

#ifndef TEMPBLOCKINGCOLLECTION_H
#define TEMPBLOCKINGCOLLECTION_H

#include "backend/backendcollection.h"

// implement a temporary collection that blocks every call.
class TempBlockingCollection : public BackendCollection
{
    Q_OBJECT

public:
    TempBlockingCollection(const QString &id, BackendCollectionManager *parent);
    virtual ~TempBlockingCollection();
    virtual QString id() const;
    virtual BackendReturn<QString> label() const;
    virtual BackendReturn<void> setLabel(const QString &label);
    virtual QDateTime created() const;
    virtual QDateTime modified() const;
    virtual bool isLocked() const;
    virtual BackendReturn<QList<BackendItem*> > items() const;
    virtual BackendReturn<QList<BackendItem*> > searchItems(const QMap<QString, QString> &attributes) const;
    virtual UnlockCollectionJob *createUnlockJob(const CollectionUnlockInfo &unlockInfo);
    virtual LockCollectionJob *createLockJob();
    virtual DeleteCollectionJob *createDeleteJob(const CollectionDeleteInfo& deleteJobInfo);
    virtual CreateItemJob *createCreateItemJob(const ItemCreateInfo& createInfo);
    virtual ChangeAuthenticationCollectionJob *createChangeAuthenticationJob(const Peer&);
    virtual ApplicationPermission applicationPermission(const QString& path) const;
    virtual bool setApplicationPermission(const QString& path, ApplicationPermission perm);
    virtual bool setCreatorApplication(QString exePath);
    virtual QString creatorApplication() const {
        return m_creator;
    }

protected:
    BackendReturn<BackendItem*> createItem(const QString &label,
                                           const QMap<QString, QString> &attributes,
                                           const QCA::SecureArray &secret, 
                                           const QString& contentType,
                                           bool replace,
                                           bool locked);

private Q_SLOTS:
    void slotItemDeleted(BackendItem *item);
    void deleteCollectionJobResult(KJob *job);

private:
    friend class TempBlockingCreateItemJob;

    QString m_id;
    QString m_label;
    QDateTime m_created;
    QDateTime m_modified;
    QString m_creator;

    QList<BackendItem*> m_items;
};

#endif
