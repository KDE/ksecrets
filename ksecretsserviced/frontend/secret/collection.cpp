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

#include "collection.h"
#include "adaptors/collectionadaptor.h"
#include "service.h"
#include "item.h"
#include "session.h"
#include "prompt.h"

#include <backend/backendcollection.h>
#include <backend/backenditem.h>
#include <peer.h>
#include <kdebug.h>
#include <QtDBus/QDBusConnection>
#include <QtCore/QDebug>
#include <klocalizedstring.h>

Collection::Collection(BackendCollection *collection, Service *service)
    : QObject(service), m_service(service), m_collection(collection), m_deleted(false)
{
    Q_ASSERT(collection);
    m_objectPath.setPath(service->objectPath().path() + "/collection/" + collection->id());

    new orgFreedesktopSecret::CollectionAdaptor(this);
    QDBusConnection::sessionBus().registerObject(m_objectPath.path(), this);

    connect(collection, SIGNAL(itemCreated(BackendItem*)),
            SLOT(slotItemCreated(BackendItem*)));
    connect(collection, SIGNAL(itemDeleted(BackendItem*)),
            SLOT(slotItemDeleted(BackendItem*)));
    connect(collection, SIGNAL(itemChanged(BackendItem*)),
            SLOT(slotItemChanged(BackendItem*)));
}

const QDBusObjectPath &Collection::objectPath() const
{
    return m_objectPath;
}

const QList<QDBusObjectPath> &Collection::items() const
{
    return m_itemPaths;
}

void Collection::setLabel( const QString &label )
{
    if ( !m_deleted && m_collection ) {
        BackendReturn<void> rc = m_collection->setLabel( label );
        if ( rc.isError() ) {
            sendErrorReply( QDBusError::Failed, rc.errorMessage() );
        }
    }
    else {
        sendErrorReply( QDBusError::InvalidObjectPath, ki18n("The collection has been deleted").toString() );
    }
}

QString Collection::label() const
{
    QString result;
    if ( !m_deleted && m_collection ) {
        BackendReturn<QString> rc = m_collection->label();
        if(rc.isError()) {
            sendErrorReply( QDBusError::Failed, rc.errorMessage() );
        }
        result = rc.value();
    }
    else {
        sendErrorReply( QDBusError::InvalidObjectPath, ki18n("The collection has been deleted").toString() );
    }
    return result;
}

bool Collection::locked() const
{
    bool result = true;
    if ( !m_deleted && m_collection ) {
        result = m_collection->isLocked();
    }
    return result;
}

qulonglong Collection::created() const
{
    if ( !m_deleted && m_collection ) {
        return m_collection->created().toTime_t();
    }
    return 0;
}

qulonglong Collection::modified() const
{
    if ( !m_deleted && m_collection ) {
        return m_collection->modified().toTime_t();
    }
    return 0;
}

QDBusObjectPath Collection::deleteCollection()
{
    QDBusObjectPath result("/");
    if ( m_collection && !m_deleted ) {
        CollectionDeleteInfo deleteInfo(getCallingPeer());
        // TODO: init peer here
        DeleteCollectionJob *dcj = m_collection->createDeleteJob(deleteInfo);
        if(dcj->isImmediate()) {
            m_deleted = true;
            dcj->exec();
        } else {
            SingleJobPrompt *p = new SingleJobPrompt(m_service, dcj, this);
            result = p->objectPath();
        }
    }
    else {
        if ( m_deleted ) {
            sendErrorReply( QDBusError::Failed, ki18n("Collection was already deleted").toString() );
        }
        else {
            sendErrorReply( QDBusError::InternalError );
        }
    }
    return result;
}

void Collection::slotCollectionDeleted(BackendCollection* coll)
{
    if ( coll == m_collection ) {
        m_collection = 0;
        m_deleted = true;
    }
}

QList<QDBusObjectPath> Collection::searchItems(const QMap<QString, QString> &attributes)
{
    QList<QDBusObjectPath> rc;
    if ( m_collection ) {
        BackendReturn<QList<BackendItem*> > br = m_collection->searchItems(attributes);
        if(br.isError()) {
            sendErrorReply( QDBusError::Failed, br.errorMessage() );
        } else {
            Q_FOREACH(BackendItem * item, br.value()) {
                rc.append(QDBusObjectPath(m_objectPath.path() + '/' + item->id()));
            }
        }
    }
    else {
        sendErrorReply( QDBusError::InternalError );
    }
    return rc;
}

// FIXME: this method needs some refactoring for the sake of it's readability
QDBusObjectPath Collection::createItem(const QMap<QString, QVariant> &properties,
                                       const SecretStruct &secret, 
                                       bool replace,
                                       QDBusObjectPath &prompt)
{
    if ( !m_collection ) {
        sendErrorReply( QDBusError::InternalError );
        return QDBusObjectPath("/");
    }
    
    // default label?
    QString label;
    QMap<QString, QString> attributes;
    bool locked = false;

    // get the session object
    QObject *object = QDBusConnection::sessionBus().objectRegisteredAt(secret.m_session.path());
    Session *session;
    if(!(session = qobject_cast<Session*>(object))) {
        kDebug() << "ERROR there is no available session";
        sendErrorReply( QDBusError::InternalError );
        return QDBusObjectPath("/");
    }

    if(properties.contains("Locked")) {
        locked = properties["Locked"].toBool();
    }
    if(properties.contains("Attributes")) {
        attributes = qdbus_cast<StringStringMap>(properties["Attributes"].value<QDBusArgument>());
    }
    if(properties.contains("Label")) {
        label = properties["Label"].toString();
        attributes["Label"] = label;
    }

    // TODO: check the parameters before creating the prompt
    QCA::SecureArray secretValue;
    if(!session->decrypt(secret.m_value, secret.m_parameters, secretValue)) {
        // TODO: invalid session
        kDebug() << "ERROR Attempting to create an item without a valid session";
    }
    ItemCreateInfo createInfo(label, attributes, secretValue, secret.m_contentType, replace, locked, getCallingPeer());
    CreateItemJob *cij = m_collection->createCreateItemJob(createInfo);
    if(cij->isImmediate()) {
        cij->exec();
        if(cij->error() != BackendNoError || !cij->item()) {
            // TODO: error creating the item
            kDebug() << "ERROR creating the item";
        }

        // the Item is already created inside slotItemCreated()
        prompt.setPath("/");
        QDBusObjectPath itemPath(m_objectPath.path() + '/' + cij->item()->id());
        return itemPath;
    } else {
        SingleJobPrompt *p = new SingleJobPrompt(m_service, cij, this);
        prompt = p->objectPath();
        return QDBusObjectPath("/");
    }
}

QDBusObjectPath Collection::changePassword()
{
    if ( m_collection ) {
        ChangeAuthenticationCollectionJob *cpj = m_collection->createChangeAuthenticationJob( getCallingPeer() );
        SingleJobPrompt *pj = new SingleJobPrompt(m_service, cpj, this );
        return pj->objectPath();
    }
    else {
        sendErrorReply( QDBusError::InternalError );
        return QDBusObjectPath("/");
    }
}

BackendCollection *Collection::backendCollection() const
{
    return m_collection;
}

void Collection::slotItemCreated(BackendItem *item)
{
    Q_ASSERT(item);
    Item *itm = new Item(item, this);
    m_itemPaths.append(itm->objectPath());
    m_items.insert( itm->objectPath(), itm );
    emit itemCreated(itm->objectPath());
}

void Collection::slotItemDeleted(BackendItem *item)
{
    Q_ASSERT(item);
    QDBusObjectPath itmPath( m_objectPath.path() + '/' + item->id() );
    m_itemPaths.removeAll( itmPath );
    m_items.take( itmPath );
    emit itemDeleted(itmPath);
}

void Collection::slotItemChanged(BackendItem *item)
{
    Q_ASSERT(item);
    QDBusObjectPath itmPath(m_objectPath.path() + '/' + item->id());
    if ( !m_itemPaths.contains( itmPath ) ) {
        m_itemPaths.append( itmPath );
        m_items.insert( itmPath, new Item( item, this ) );
    }
    emit itemChanged(itmPath);
}

#include "collection.moc"
