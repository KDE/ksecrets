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

#include "ksecretitem.h"
#include "ksecretcollection.h"
#include "ksecretjobs.h"
#include "../securebuffer.h"

#include <QtCore/QPointer>
#include <QtCore/QTimer>

KSecretItem::KSecretItem() :
    BackendItem( 0 )
{
    m_created = QDateTime::currentDateTimeUtc();
    m_modified = m_created;
}

KSecretItem::KSecretItem(const QString &id, KSecretCollection *parent)
    : BackendItem(parent), m_id(id)
{
    Q_ASSERT(parent);
    m_created = QDateTime::currentDateTimeUtc();
    m_modified = m_created;
}

KSecretItem::~KSecretItem()
{
}

QString KSecretItem::id() const
{
    return m_id;
}

BackendReturn<QString> KSecretItem::label() const
{
    if(isLocked()) {
        return BackendReturn<QString>(QString(), BackendErrorIsLocked);
    } else {
        markAsUsed();
        return BackendReturn<QString>( m_label );
    }
}

BackendReturn<void> KSecretItem::setLabel(const QString &label)
{
    if(isLocked()) {
        return BackendReturn<void>(BackendErrorIsLocked);
    } else {
        m_label = label;
        markAsModified();
        return BackendReturn<void>();
    }
}

BackendReturn<QCA::SecureArray> KSecretItem::secret() const
{
    if(isLocked()) {
        return BackendReturn<QCA::SecureArray>(QCA::SecureArray(), BackendErrorIsLocked);
    } else {
        markAsUsed();
        return BackendReturn<QCA::SecureArray>( m_secret );
    }
}

BackendReturn<void> KSecretItem::setSecret(const QCA::SecureArray &secret)
{
    if(isLocked()) {
        return BackendReturn<void>(BackendErrorIsLocked);
    } else {
        m_secret = secret;
        markAsModified();
        return BackendReturn<void>();
    }
}

BackendReturn< QString > KSecretItem::contentType() const
{
    if (isLocked()) {
        return BackendReturn<QString>(QString(), BackendErrorIsLocked);
    }
    else {
        markAsUsed();
        return BackendReturn< QString >( m_contentType );
    }
}

BackendReturn< void > KSecretItem::setContentType(const QString& contentType)
{
    if(isLocked()) {
        return BackendReturn<void>(BackendErrorIsLocked);
    } else {
        m_contentType = contentType;
        markAsModified();
        return BackendReturn<void>();
    }
}

BackendReturn<QMap<QString, QString> > KSecretItem::attributes() const
{
    if(isLocked()) {
        return BackendReturn<QMap<QString, QString> >(QMap<QString, QString>(), BackendErrorIsLocked);
    } else {
        markAsUsed();
        return BackendReturn<QMap<QString, QString> >( m_attributes );
    }
}

BackendReturn<void> KSecretItem::setAttributes(const QMap<QString, QString> &attributes)
{
    if(isLocked()) {
        return BackendReturn<void>(BackendErrorIsLocked);
    } else {
        m_attributes = attributes;
        markAsModified();
        emit attributesChanged(this);
        return BackendReturn<void>();
    }
}

QDateTime KSecretItem::created() const
{
    markAsUsed();
    return m_created;
}

QDateTime KSecretItem::modified() const
{
    markAsUsed();
    return m_modified;
}

bool KSecretItem::isLocked() const
{
    return m_collection->isLocked();
}

UnlockItemJob *KSecretItem::createUnlockJob(const ItemUnlockInfo& unlockInfo)
{
    Q_ASSERT(m_collection);
    Q_ASSERT(unlockInfo.m_item == 0); // FIXME: what to do when item information is already present ?
    unlockInfo.m_item = this;
    return new KSecretUnlockItemJob(unlockInfo, qobject_cast< KSecretCollection* >( m_collection ) );
}

LockItemJob *KSecretItem::createLockJob()
{
    Q_ASSERT(m_collection);
    return new KSecretLockItemJob(this, qobject_cast< KSecretCollection* >( m_collection) );
}

DeleteItemJob *KSecretItem::createDeleteJob(const ItemDeleteInfo& deleteJobInfo)
{
    deleteJobInfo.m_item = this;
    return new KSecretDeleteItemJob(deleteJobInfo);
}

BackendReturn<bool> KSecretItem::deleteItem()
{
    emit itemDeleted(this);
    deleteLater();
    return BackendReturn<bool>( true );
}

ChangeAuthenticationItemJob *KSecretItem::createChangeAuthenticationJob()
{
    return new KSecretChangeAuthenticationItemJob(this);
}

bool KSecretItem::matches(const QMap<QString, QString> &attributes)
{
    QMap<QString, QString>::const_iterator it = attributes.constBegin();
    const QMap<QString, QString>::const_iterator end = attributes.constEnd();
    for(; it != end; ++it) {
        if(!m_attributes.contains(it.key()) ||
                m_attributes.value(it.key()) != it.value()) {
            return false;
        }
    }
    return true;
}

void KSecretItem::markAsModified()
{
    m_modified = QDateTime::currentDateTime();
    emit itemUsed(this);
    emit itemChanged(this);
}

void KSecretItem::markAsUsed() const
{
    // TODO: figure out what to do. this method exists so a "close-if-unused" timer
    //       can be implemented.
}

KSecretStream& operator<<(KSecretStream& out, KSecretItem* item )
{
    out << item->m_id;
    out << item->m_label;
    out << item->m_created;
    out << item->m_modified;
    out << item->m_attributes;
    out << item->m_secret;
    out << item->m_contentType;
    return out;
}

KSecretStream& operator>>(KSecretStream& in, KSecretItem* &item )
{
    item = new KSecretItem();
    in >> item->m_id;
    in >> item->m_label;
    in >> item->m_created;
    in >> item->m_modified;
    in >> item->m_attributes;
    in >> item->m_secret;
    in >> item->m_contentType;
    return in;
}

#include "ksecretitem.moc"
