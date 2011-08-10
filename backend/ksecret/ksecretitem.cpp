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

KSecretItem::KSecretItem(const QString &id, KSecretCollection *parent)
    : BackendItem(parent), m_collection(parent), m_id(id)
{
    Q_ASSERT(parent);
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
        return m_label;
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
        return m_secret;
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
        return m_contentType;
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
        return m_attributes;
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
    return new KSecretUnlockItemJob(unlockInfo, m_collection);
}

LockItemJob *KSecretItem::createLockJob()
{
    Q_ASSERT(m_collection);
    return new KSecretLockItemJob(this, m_collection);
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
    return true;
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

bool KSecretItem::deserializeUnlocked(KSecretFile &file)
{
    Q_ASSERT(file.isValid());

    // deserialize everything to temporary variables/objects and write it
    // into the members in one go once everything could be read successfully.

    // file currently points at item-label
    QString itemLabel;
    QDateTime itemCreated;
    QDateTime itemModified;
    quint32 numAttribs;
    if(!file.readString(&itemLabel) || !file.readDatetime(&itemCreated) ||
            !file.readDatetime(&itemModified) || !file.readUint(&numAttribs)) {
        return false;
    }

    // read the attributes
    QMap<QString, QString> attributes;
    for(quint32 i = 0; i < numAttribs; ++i) {
        QString attribKey;
        QString attribValue;
        if(!file.readString(&attribKey) || !file.readString(&attribValue)) {
            return false;
        }
        attributes.insert(attribKey, attribValue);
    }

    // read the secret
    QCA::SecureArray secret;
    if(!file.readSecret(&secret)) {
        return false;
    }
    
    // read content type
    QString *contentType =0;
    if (!file.readString(contentType)) {
        return false;
    }
    Q_ASSERT(contentType!=0);

    m_label = itemLabel;
    m_created = itemCreated;
    m_modified = itemModified;
    m_attributes = attributes;
    m_secret = secret;
    m_contentType = *contentType;

    return true;
}

bool KSecretItem::serializeUnlocked(KSecretFile &file)
{
    Q_ASSERT(file.isValid());

    if(!file.writeString(m_id)) {
        return false;
    }

    // serialize data to a temporary array
    SecureBuffer device;
    KSecretFile tempFile(&device, KSecretFile::Write);
    if(!tempFile.isValid()) {
        return false;
    }

    if(!tempFile.writeString(m_label) || !tempFile.writeDatetime(m_created) ||
            !tempFile.writeDatetime(m_modified) || !tempFile.writeUint(m_attributes.size())) {
        return false;
    }

    // serialize attributes
    QMap<QString, QString>::const_iterator it = m_attributes.constBegin();
    const QMap<QString, QString>::const_iterator end = m_attributes.constEnd();
    for(; it != end; ++it) {
        if(!tempFile.writeString(it.key()) || !tempFile.writeString(it.value())) {
            return false;
        }
    }

    // serialize secret
    if(!tempFile.writeSecret(m_secret)) {
        return false;
    }
    
    // serialize contentType
    if(!tempFile.writeString(m_contentType)) {
        return false;
    }

    // now take the tempFile and write its contents to the actual file
    if(!file.writeSecret(device.buffer())) {
        return false;
    }
    return true;
}

QSet<QByteArray> KSecretItem::createHashes(const QMap<QString, QString> &attributes,
        QCA::Hash *hash)
{
    Q_ASSERT(hash);

    QSet<QByteArray> hashSet;
    QMap<QString, QString>::const_iterator it = attributes.constBegin();
    QMap<QString, QString>::const_iterator end = attributes.constEnd();
    for(; it != end; ++it) {
        hash->clear();
        hash->update(it.key().toUtf8());
        hash->update(it.value().toUtf8());
        hashSet.insert(hash->final().toByteArray());
    }

    return hashSet;
}

QSet<QByteArray> KSecretItem::createAttributeHashes(QCA::Hash *hash) const
{
    return createHashes(m_attributes, hash);
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

#include "ksecretitem.moc"
