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

#include "temporarycollection.h"
#include "temporaryitem.h"
#include "temporaryjobs.h"

#include "../lib/secrettool.h"

#include <klocalizedstring.h>
#include <QtCore/QTimer>

TemporaryCollection::TemporaryCollection(const QString &id,
        BackendCollectionManager *parent)
    : BackendCollection(parent)
{
    m_id = id;
    QDateTime now = QDateTime::currentDateTime();
    m_created = now;
    m_modified = now;
}

TemporaryCollection::~TemporaryCollection()
{
}

QString TemporaryCollection::id() const
{
    return m_id;
}

BackendReturn<QString> TemporaryCollection::label() const
{
    return BackendReturn< QString >( m_label );
}

BackendReturn<void> TemporaryCollection::setLabel(const QString &label)
{
    m_label = label;
    emit collectionChanged(this);
    return BackendReturn<void>();
}

QDateTime TemporaryCollection::created() const
{
    return m_created;
}

QDateTime TemporaryCollection::modified() const
{
    return m_modified;
}

bool TemporaryCollection::isLocked() const
{
    // temporary collection can't be locked
    return false;
}

BackendReturn<QList<BackendItem*> > TemporaryCollection::items() const
{
    return BackendReturn<QList<BackendItem*> >( m_items );
}

BackendReturn<QList<BackendItem*> > TemporaryCollection::searchItems(
    const QMap<QString, QString> &attributes) const
{
    TemporaryItem *titem;
    QList<BackendItem*> foundItems;
    Q_FOREACH(BackendItem * item, m_items) {
        titem = qobject_cast<TemporaryItem*>(item);
        if(titem && titem->matches(attributes)) {
            foundItems.append(item);
        }
    }
    return BackendReturn<QList<BackendItem*> >( foundItems );
}

UnlockCollectionJob *TemporaryCollection::createUnlockJob(const CollectionUnlockInfo &unlockInfo)
{
    return new TemporaryUnlockCollectionJob(unlockInfo, this);
}

LockCollectionJob *TemporaryCollection::createLockJob()
{
    return new TemporaryLockCollectionJob(this);
}

DeleteCollectionJob *TemporaryCollection::createDeleteJob(const CollectionDeleteInfo& deleteJobInfo)
{
    deleteJobInfo.m_collection = this;
    TemporaryDeleteCollectionJob *job = new TemporaryDeleteCollectionJob(deleteJobInfo);
    connect(job, SIGNAL(result(KJob*)),
            SLOT(deleteCollectionJobResult(KJob*)));
    return job;
}

CreateItemJob *TemporaryCollection::createCreateItemJob(const ItemCreateInfo& createInfo)
{
    return new TemporaryCreateItemJob(createInfo, this);
}

BackendReturn<BackendItem*> TemporaryCollection::createItem(const QString &label,
        const QMap<QString, QString> &attributes,
        const QCA::SecureArray &secret,
        const QString& contentType,
        bool replace, bool locked)
{
    Q_UNUSED(locked);

    TemporaryItem *item = 0;
    bool replacing = false;

    // check for duplicates
    BackendReturn<QList<BackendItem*> > foundItems = searchItems(attributes);
    if(!foundItems.isError() && foundItems.value().size() > 0) {
        QList<BackendItem*> oldlist = foundItems.value();
        Q_FOREACH(BackendItem * olditem, oldlist) {
            if(olditem->attributes().value() == attributes) {
                if(replace) {
                    // replacing an existing item
                    item = qobject_cast<TemporaryItem*>(olditem);
                    replacing = true;
                } else {
                    // item existing but should not be replaced
                    return BackendReturn<BackendItem*>(0, BackendErrorAlreadyExists);
                }
                break;
            }
        }
    }

    if(!item) {
        item = new TemporaryItem(createId(), this);
    }
    // block signals while changing label, secret and attributes
    item->blockSignals(true);
    item->setLabel(label);
    item->setAttributes(attributes);
    item->setSecret(secret);
    item->setContentType(contentType);
    item->blockSignals(false);

    if(replacing) {
        emit itemChanged(item);
    } else {
        m_items.append(item);
        // new item, signals need to be wired
        connect(item, SIGNAL(itemDeleted(BackendItem*)), SLOT(slotItemDeleted(BackendItem*)));
        connect(item, SIGNAL(itemChanged(BackendItem*)), SIGNAL(itemChanged(BackendItem*)));
        emit itemCreated(item);
    }

    return BackendReturn<BackendItem*>( item );
}

ChangeAuthenticationCollectionJob *TemporaryCollection::createChangeAuthenticationJob( const Peer& peer )
{
    return new TemporaryChangeAuthenticationCollectionJob(this, peer);
}

void TemporaryCollection::slotItemDeleted(BackendItem *item)
{
    m_items.removeAll(item);
    emit itemDeleted(item);
}

void TemporaryCollection::deleteCollectionJobResult(KJob *job)
{
    TemporaryDeleteCollectionJob *dcj = qobject_cast<TemporaryDeleteCollectionJob*>(job);
    Q_ASSERT(dcj);
    if(!dcj->result()) {
        return;
    }
    emit collectionDeleted(this);
}

#include "temporarycollection.moc"
