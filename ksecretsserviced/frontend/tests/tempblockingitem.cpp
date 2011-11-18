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

#include "tempblockingitem.h"
#include "tempblockingcollection.h"
#include "tempblockingjobs.h"

TempBlockingItem::TempBlockingItem(const QString &id, TempBlockingCollection *parent)
    : BackendItem(parent), m_id(id)
{
    QDateTime now = QDateTime::currentDateTime();
    m_created = now;
    m_modified = now;
}

TempBlockingItem::~TempBlockingItem()
{
}

QString TempBlockingItem::id() const
{
    return m_id;
}

BackendReturn<QString> TempBlockingItem::label() const
{
    return BackendReturn<QString>( m_label );
}

BackendReturn<void> TempBlockingItem::setLabel(const QString &label)
{
    m_label = label;
    markAsModified();
    return BackendReturn<void>();
}

BackendReturn<QCA::SecureArray> TempBlockingItem::secret() const
{
    return BackendReturn<QCA::SecureArray>( m_secret );
}

BackendReturn<void> TempBlockingItem::setSecret(const QCA::SecureArray &secret)
{
    m_secret = secret;
    markAsModified();
    return BackendReturn<void>();
}

BackendReturn< QString > TempBlockingItem::contentType() const
{
    return BackendReturn< QString >( m_contentType );
}

BackendReturn< void > TempBlockingItem::setContentType(const QString& contentType)
{
    m_contentType = contentType;
    markAsModified();
    return BackendReturn<void>();
}


BackendReturn<QMap<QString, QString> > TempBlockingItem::attributes() const
{
    return BackendReturn<QMap<QString, QString> >( m_attributes );
}

BackendReturn<void> TempBlockingItem::setAttributes(const QMap<QString, QString> &attributes)
{
    m_attributes = attributes;
    markAsModified();
    return BackendReturn<void>();
}

QDateTime TempBlockingItem::created() const
{
    return m_created;
}

QDateTime TempBlockingItem::modified() const
{
    return m_modified;
}

bool TempBlockingItem::isLocked() const
{
    return false;
}

UnlockItemJob *TempBlockingItem::createUnlockJob(const ItemUnlockInfo &unlockInfo)
{
    unlockInfo.m_item = this;
    return new TempBlockingUnlockItemJob(unlockInfo);
}

LockItemJob *TempBlockingItem::createLockJob()
{
    return new TempBlockingLockItemJob(this);
}

DeleteItemJob *TempBlockingItem::createDeleteJob(const ItemDeleteInfo& deleteJobInfo)
{
    deleteJobInfo.m_item = this;
    TempBlockingDeleteItemJob *job = new TempBlockingDeleteItemJob(deleteJobInfo);
    connect(job, SIGNAL(result(KJob*)), SLOT(deleteItemJobResult(KJob*)));
    return job;
}

ChangeAuthenticationItemJob *TempBlockingItem::createChangeAuthenticationJob()
{
    return new TempBlockingChangeAuthenticationItemJob(this);
}

bool TempBlockingItem::matches(const QMap<QString, QString> &attributes)
{
    QMap<QString, QString>::const_iterator it = attributes.constBegin();
    QMap<QString, QString>::const_iterator end = attributes.constEnd();
    for(; it != end; ++it) {
        if(!m_attributes.contains(it.key()) ||
                m_attributes.value(it.key()) != it.value()) {
            return false;
        }
    }

    return true;
}

void TempBlockingItem::deleteItemJobResult(KJob *job)
{
    TempBlockingDeleteItemJob *dij = qobject_cast<TempBlockingDeleteItemJob*>(job);
    Q_ASSERT(dij);
    if(!dij->result()) {
        return;
    }
    emit itemDeleted(this);
}

void TempBlockingItem::markAsModified()
{
    m_modified = QDateTime::currentDateTime();
    emit itemChanged(this);
}

#include "tempblockingitem.moc"
