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

#include "item.h"
#include "collection.h"
#include "adaptors/itemadaptor.h"
#include "session.h"
#include "adaptors/daemonsecret.h"
#include "prompt.h"

#include <backend/backenditem.h>

#include <QtDBus/QDBusConnection>

Item::Item(BackendItem *item, Collection *collection)
    : QObject(collection), m_item(item)
{
    Q_ASSERT(item);
    m_objectPath.setPath(collection->objectPath().path() + '/' + item->id());

    new orgFreedesktopSecret::ItemAdaptor(this);
    QDBusConnection::sessionBus().registerObject(m_objectPath.path(), this);
}

const QDBusObjectPath &Item::objectPath() const
{
    return m_objectPath;
}

bool Item::locked() const
{
    return m_item->isLocked();
}

void Item::setAttributes(const QMap<QString, QString> &attributes)
{
    BackendReturn<void> rc = m_item->setAttributes(attributes);
    if(rc.isError()) {
        // TODO: set D-Bus error
    }
}

QMap<QString, QString> Item::attributes() const
{
    BackendReturn<QMap<QString, QString> > rc = m_item->attributes();
    if(rc.isError()) {
        // TODO: set D-Bus error
    }
    return rc.value();
}

void Item::setLabel(const QString &label)
{
    BackendReturn<void> rc = m_item->setLabel(label);
    if(rc.isError()) {
        // TODO: set D-Bus error
    }
}

QString Item::label() const
{
    BackendReturn<QString> rc = m_item->label();
    if(rc.isError()) {
        // TODO: set D-Bus error
    }
    return rc.value();
}

qulonglong Item::created() const
{
    return m_item->created().toTime_t();
}

qulonglong Item::modified() const
{
    return m_item->modified().toTime_t();
}

QDBusObjectPath Item::deleteItem()
{
    ItemDeleteInfo deleteInfo(getCallingPeer());
    DeleteItemJob *dij = m_item->createDeleteJob(deleteInfo);
    if(dij->isImmediate()) {
        dij->exec();
        return QDBusObjectPath("/");
    } else {
        // FIXME: needs the service!
        SingleJobPrompt *p = new SingleJobPrompt(0, dij, this);
        return p->objectPath();
    }
}

SecretStruct Item::getSecret(const QDBusObjectPath &session)
{
    SecretStruct result;
    
    if(m_item->isLocked()) {
        // TODO: error, requires unlocking
        Q_ASSERT(0);
    }
    else {
        QObject *object = QDBusConnection::sessionBus().objectRegisteredAt(session.path());
        Session *sessionObj;
        if(object && (sessionObj = qobject_cast<Session*>(object))) {
            BackendReturn< QCA::SecureArray > secretRet = m_item->secret();
            QCA::SecureArray encryptedValue;
            QByteArray encryptedParams;
            if ( secretRet.error() == BackendNoError && 
                 sessionObj->encrypt( secretRet.value(), encryptedValue, encryptedParams ) ) {
                result.m_session.setPath( session.path() );
                result.m_value = encryptedValue.toByteArray();
                result.m_parameters = encryptedParams;
                result.m_contentType = m_item->contentType().value();
            
                BackendReturn<QString> contentTypeRet = m_item->contentType();
                if ( !contentTypeRet.isError() ) {
                    result.m_contentType = contentTypeRet.value();
                }
                else {
                    // TODO: handle error
                    Q_ASSERT(0);
                }
            }
            
        } else {
            // TODO: error, requires session
            // FIXME: this really needs fixing as we get here in the followin scenario:
            // kwallet manager editor is open
            // ksecretsserviced is killed
            // navigation occurs in kwallet editor
            // the collection is unlocked then CRASH here
            Q_ASSERT(0);
        }
    }
    return result;
}

void Item::setSecret(const SecretStruct &secret)
{
    if(m_item->isLocked()) {
        // TODO: error, requires unlocking
        return;
    }

    QObject *object = QDBusConnection::sessionBus().objectRegisteredAt(secret.m_session.path());
    Session *sessionObj;
    if(object && (sessionObj = qobject_cast<Session*>(object))) {
        QCA::SecureArray secretValue;
        if( !sessionObj->decrypt(secret.m_value, secret.m_parameters, secretValue)) {
            // TODO: invalid session
            Q_ASSERT(0);
            return;
        }
        BackendReturn<void> rc = m_item->setSecret(secretValue);
        if(rc.isError()) {
            // TODO: handle error
            Q_ASSERT(0);
            return;
        }
        BackendReturn< void > rc2 = m_item->setContentType(secret.m_contentType);
        if (rc2.isError()) {
            // TODO: handle error
            Q_ASSERT(0);
            return;
        }
    }
}

BackendItem *Item::backendItem() const
{
    return m_item;
}

#include "item.moc"
