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

#include "temporaryitem.h"
#include "temporarycollection.h"
#include "temporaryjobs.h"

#include <QtCore/QTimer>

TemporaryItem::TemporaryItem(const QString &id, TemporaryCollection *parent)
    : BackendItem(parent), m_id(id)
{
    QDateTime now = QDateTime::currentDateTime();
    m_created = now;
    m_modified = now;
}

TemporaryItem::~TemporaryItem()
{
}

QString TemporaryItem::id() const
{
    return m_id;
}

BackendReturn<QString> TemporaryItem::label() const
{
    return BackendReturn<QString>(m_label);
}

BackendReturn<void> TemporaryItem::setLabel(const QString &label)
{
    m_label = label;
    markAsModified();
    return BackendReturn<void>();
}

BackendReturn<QCA::SecureArray> TemporaryItem::secret() const
{
    return BackendReturn<QCA::SecureArray>(m_secret);
}

BackendReturn<void> TemporaryItem::setSecret(const QCA::SecureArray &secret)
{
    m_secret = secret;
    markAsModified();
    return BackendReturn<void>();
}

BackendReturn< QString > TemporaryItem::contentType() const
{
    return BackendReturn< QString >( m_contentType );
}

BackendReturn< void > TemporaryItem::setContentType(const QString& contentType)
{
    m_contentType = contentType;
    markAsModified();
    return BackendReturn<void>();
}

BackendReturn<QMap<QString, QString> > TemporaryItem::attributes() const
{
    return BackendReturn<QMap<QString, QString> >( m_attributes );
}

BackendReturn<void> TemporaryItem::setAttributes(const QMap<QString, QString> &attributes)
{
    m_attributes = attributes;
    markAsModified();
    return BackendReturn<void>();
}

QDateTime TemporaryItem::created() const
{
    return m_created;
}

QDateTime TemporaryItem::modified() const
{
    return m_modified;
}

bool TemporaryItem::isLocked() const
{
    return false;
}

UnlockItemJob *TemporaryItem::createUnlockJob(const ItemUnlockInfo& unlockInfo)
{
    Q_ASSERT(unlockInfo.m_item == 0);
    unlockInfo.m_item = this;
    return new TemporaryUnlockItemJob(unlockInfo);
}

LockItemJob *TemporaryItem::createLockJob()
{
    return new TemporaryLockItemJob(this);
}

DeleteItemJob *TemporaryItem::createDeleteJob(const ItemDeleteInfo& deleteJobInfo)
{
    deleteJobInfo.m_item = this; //FIXME: what if m_item was already != NULL here ?
    TemporaryDeleteItemJob *job = new TemporaryDeleteItemJob(deleteJobInfo);
    connect(job, SIGNAL(result(KJob*)), SLOT(deleteItemJobResult(KJob*)));
    return job;
}

ChangeAuthenticationItemJob *TemporaryItem::createChangeAuthenticationJob()
{
    return new TemporaryChangeAuthenticationItemJob(this);
}

bool TemporaryItem::matches(const QMap<QString, QString> &attributes)
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

void TemporaryItem::deleteItemJobResult(KJob *job)
{
    TemporaryDeleteItemJob *dij = qobject_cast<TemporaryDeleteItemJob*>(job);
    Q_ASSERT(dij);
    if(!dij->result()) {
        return;
    }
    emit itemDeleted(this);
}

void TemporaryItem::markAsModified()
{
    m_modified = QDateTime::currentDateTime();
    emit itemChanged(this);
}

#include "temporaryitem.moc"
