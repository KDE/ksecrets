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
#include "dbus/collectionadaptor.h"
#include "service.h"
#include "item.h"
#include "session.h"
#include "prompt.h"

#include <backend/backendcollection.h>
#include <backend/backenditem.h>

#include <QtDBus/QDBusConnection>

Collection::Collection(BackendCollection *collection, Service *service)
 : QObject(service), m_service(service), m_collection(collection)
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
   return m_items;
}

void Collection::setLabel(const QString &label)
{
   BackendReturn<void> rc = m_collection->setLabel(label);
   if (rc.isError()) {
      // TODO: generate D-Bus error
   }
}

QString Collection::label() const
{
   BackendReturn<QString> rc = m_collection->label();
   if (rc.isError()) {
      // TODO: generate D-Bus error
   }
   return rc.value();
}

bool Collection::locked() const
{
   return m_collection->isLocked();
}

qulonglong Collection::created() const
{
   return m_collection->created().toTime_t();
}

qulonglong Collection::modified() const
{
   return m_collection->modified().toTime_t();
}

QDBusObjectPath Collection::deleteCollection()
{
   // bypass prompt?
   if (m_collection->isCallImmediate(AsyncCall::AsyncDeleteCollection)) {
      m_collection->deleteCollection();
      return QDBusObjectPath("/");
   } else {
      PromptCollectionDelete *p = new PromptCollectionDelete(m_collection, m_service);
      return p->objectPath();
   }
}

QList<QDBusObjectPath> Collection::searchItems(const QMap<QString, QString> &attributes)
{
   QList<QDBusObjectPath> rc;
   BackendReturn<QList<BackendItem*> > br = m_collection->searchItems(attributes);
   if (br.isError()) {
      // TODO: generate D-Bus error
   }
   else {
      Q_FOREACH(BackendItem *item, br.value()) {
         rc.append(QDBusObjectPath(m_objectPath.path() + "/" + item->id()));
      }
   }
   return rc;
}

QDBusObjectPath Collection::createItem(const QMap<QString, QVariant> &properties,
                                       const Secret &secret, bool replace,
                                       QDBusObjectPath &prompt)
{
   // TODO: bypass prompt
   QString label;
   QMap<QString, QString> attributes;
   bool locked = false;

   // get the session object
   QObject *object = QDBusConnection::sessionBus().objectRegisteredAt(secret.session().path());
   Session *session;
   if (!(session = qobject_cast<Session*>(object))) {
      // TODO: error, requires session
   }
   
   if (properties.contains("Label")) {
      label = properties["Label"].toString();
   }
   if (properties.contains("Locked")) {
      locked = properties["Locked"].toBool();
   }
   if (properties.contains("Attributes")) {
      QMap<QString, QVariant> tempMap = properties["Attributes"].toMap();
      QMap<QString, QVariant>::const_iterator it = tempMap.constBegin();
      QMap<QString, QVariant>::const_iterator end = tempMap.constEnd();
      for ( ; it != end; ++it) {
         attributes.insert(it.key(), it.value().toString());
      }
   }

   // TODO: check the parameters before creating the prompt
   bool ok;
   QCA::SecureArray secretValue = session->decrypt(secret, ok);
   if (!ok) {
      // TODO: invalid session
   }
   if (m_collection->isCallImmediate(AsyncCall::AsyncCreateItem)) {
      BackendReturn<BackendItem*> rc = m_collection->createItem(label, attributes, secretValue,
                                                                replace, locked);
      if (rc.isError()) {
         // TODO: error creating the item
      }
      
      // the item is already created inside slotItemCreated()
      prompt.setPath("/");
      QDBusObjectPath itemPath(m_objectPath.path() + "/" + rc.value()->id());
      return itemPath;
   } else {
      PromptBase *p = new PromptCollectionCreateItem(m_collection, label, attributes, locked,
                                                   secret.value(), replace, m_service, this);
                                                   
      prompt = p->objectPath();
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
   m_items.append(itm->objectPath());
   emit itemCreated(itm->objectPath());
}

void Collection::slotItemDeleted(BackendItem *item)
{
   Q_ASSERT(item);
   QDBusObjectPath itmPath(m_objectPath.path() + "/" + item->id());
   m_items.removeAll(itmPath);
   emit itemDeleted(itmPath);
}

void Collection::slotItemChanged(BackendItem *item)
{
   Q_ASSERT(item);
   QDBusObjectPath itmPath(m_objectPath.path() + "/" + item->id());
   emit itemChanged(itmPath);
}

#include "collection.moc"
