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

#include "service.h"
#include "dbus/serviceadaptor.h"
#include "dbus/attributemap.h"
#include "collection.h"
#include "prompt.h"
#include "secret.h"

#include <backend/backendcollection.h>
#include <backend/backenditem.h>

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMetaType>

Service::Service(QObject *parent)
 : QObject(parent), m_master(this), m_basePath("/org/freedesktop/secrets")
{
   new orgFreedesktopSecret::ServiceAdaptor(this);
   QDBusConnection::sessionBus().registerObject(m_basePath.path(), this);

   // TODO: make master singleton so we can get a KWallet-compatible interface
   //       on top of it as well.

   connect(&m_master, SIGNAL(collectionCreated(Collection*)),
                      SLOT(slotCollectionCreated(Collection*)));
   connect(&m_master, SIGNAL(collectionDeleted(Collection*)),
                      SLOT(slotCollectionDeleted(Collection*)));
   connect(&m_master, SIGNAL(collectionChanged(Collection*)),
                      SLOT(slotCollectionChanged(Collection*)));

   // TODO: add managers to master
}

const QDBusObjectPath &Service::objectPath() const
{
   return m_basePath;
}

const QList<QDBusObjectPath> &Service::collections() const
{
   return m_collections;
}

QVariant Service::openSession(const QString &algorithm, const QVariant &input,
                              QDBusObjectPath &result)
{
   Q_UNUSED(algorithm);
   Q_UNUSED(input);
   Q_UNUSED(result);
   
   // TODO: Session management
   return 0;
}

QDBusObjectPath Service::createCollection(const QMap<QString, QVariant> &properties,
                                          QDBusObjectPath &prompt)
{
   // TODO: bypass prompt
   PromptBase *p = new PromptServiceCreateCollection(properties["Label"].toString(),
                                                     properties["Locked"].toBool(),
                                                     this, this);
   prompt = p->objectPath();
   return QDBusObjectPath("/");
}

QList<QDBusObjectPath> Service::searchItems(const QMap<QString, QString> &attributes,
                                            QList<QDBusObjectPath> &locked)
{
   // TODO: should this rather be implemented using Daemon::Collection?
   QList<QDBusObjectPath> unlocked;
   Q_FOREACH(BackendCollection* collection, m_master.collections()) {
      QString collPath = m_basePath.path() + "/collection/" + collection->id();
      BackendReturn<QList<BackendItem*> > rc = collection->searchItems(attributes);
      if (!rc.isError()) {
         QList<BackendItem*> items = rc.value();
         Q_FOREACH(BackendItem *item, items) {
            if (item->isLocked()) {
               locked.append(QDBusObjectPath(collPath + "/" + item->id()));
            } else {
               unlocked.append(QDBusObjectPath(collPath + "/" + item->id()));
            }
         }
      }
   }
   return unlocked;
}

QList<QDBusObjectPath> Service::unlock(const QList<QDBusObjectPath> &objects,
                                       QDBusObjectPath &prompt)
{
   // TODO: bypass prompt

   // objects already unlocked
   QList<QDBusObjectPath> rc;
   // objects to unlock
   QMap<BackendBase*, QDBusObjectPath> unlockObjects;
   
   Q_FOREACH(const QDBusObjectPath &path, objects) {
      QObject *object = QDBusConnection::sessionBus().objectRegisteredAt(path.path());
      BackendItem *item = qobject_cast<BackendItem*>(object);
      BackendCollection *collection = qobject_cast<BackendCollection*>(object);
      if (collection) {
         if (!collection->isLocked()) {
            rc.append(path);
         } else {
            unlockObjects.insert(collection, path);
         }
      } else if (item) {
         if (!item->isLocked()) {
            rc.append(path);
         } else {
            unlockObjects.insert(item, path);
         }
      } else {
         // TODO: error - object to unlock is neither item nor collection (maybe it doesn't
         //       even exist).
         return QList<QDBusObjectPath>();
      }
   }

   if (unlockObjects.size() > 0) {
      PromptServiceUnlock *p = new PromptServiceUnlock(unlockObjects, this, this);
      prompt = p->objectPath();
   } else {
      prompt.setPath("/");
   }

   return rc;
}

QList<QDBusObjectPath> Service::lock(const QList<QDBusObjectPath> &objects,
                                     QDBusObjectPath &prompt)
{
   // TODO: bypass prompt

   // objects already locked
   QList<QDBusObjectPath> rc;
   // objects to lock
   QMap<BackendBase*, QDBusObjectPath> lockObjects;

   Q_FOREACH(const QDBusObjectPath &path, objects) {
      QObject *object = QDBusConnection::sessionBus().objectRegisteredAt(path.path());
      BackendItem *item = qobject_cast<BackendItem*>(object);
      BackendCollection *collection = qobject_cast<BackendCollection*>(object);
      if (collection) {
         if (collection->isLocked()) {
            rc.append(path);
         } else {
            lockObjects.insert(collection, path);
         }
      } else if (item) {
         if (item->isLocked()) {
            rc.append(path);
         } else {
            lockObjects.insert(item, path);
         }
      } else {
         // TODO: error - object to unlock is neither item nor collection (maybe it doesn't
         //       even exist).
         return QList<QDBusObjectPath>();
      }
   }

   if (lockObjects.size() > 0) {
      PromptServiceLock *p = new PromptServiceLock(lockObjects, this, this);
      prompt = p->objectPath();
   } else {
      prompt.setPath("/");
   }

   return rc;
}

QMap<QDBusObjectPath, Secret> Service::getSecrets(const QList<QDBusObjectPath> &items,
                                                  const QDBusObjectPath &session)
{
   Q_UNUSED(items);
   Q_UNUSED(session);
   // TODO: implement
   return QMap<QDBusObjectPath, Secret>();
}

void Service::slotCollectionCreated(BackendCollection *collection)
{
   Q_ASSERT(collection);
   Collection *coll = new Collection(collection, this);
   m_collections.append(coll->objectPath());
   emit collectionChanged(coll->objectPath());
}

void Service::slotCollectionDeleted(BackendCollection *collection)
{
   Q_ASSERT(collection);
   QDBusObjectPath collPath(m_basePath.path() + "/collection/" + collection->id());
   m_collections.removeAll(collPath);
   // TODO: make sure Daemon::Collection gets destroyed
   emit collectionDeleted(collPath);
}

void Service::slotCollectionChanged(BackendCollection *collection)
{
   Q_ASSERT(collection);
   QDBusObjectPath collPath(m_basePath.path() + "/collection/" + collection->id());
   emit collectionChanged(collPath);
}

class _TypeRegistrar
{
public:
   _TypeRegistrar() {
      qDBusRegisterMetaType<Secret>();
      qDBusRegisterMetaType<orgFreedesktopSecret::AttributeMap>();
   }
};

_TypeRegistrar Service::_reg;

#include "service.moc"
