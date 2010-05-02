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
#include "collection.h"
#include "prompt.h"
#include "session.h"
#include "secret.h"
#include "item.h"

#include <backend/backendcollection.h>
#include <backend/backenditem.h>

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMetaType>
#include <QtDBus/QDBusMessage>

Service::Service(BackendMaster *master, QObject *parent)
 : QObject(parent), m_master(master), m_basePath("/org/freedesktop/secrets")
{
   Q_ASSERT(master);
   
   new orgFreedesktopSecret::ServiceAdaptor(this);
   QDBusConnection::sessionBus().registerObject(m_basePath.path(), this);

   // TODO: make master singleton so we can get a KWallet-compatible interface
   //       on top of it as well.

   connect(m_master, SIGNAL(collectionCreated(BackendCollection*)),
                     SLOT(slotCollectionCreated(BackendCollection*)));
   connect(m_master, SIGNAL(collectionDeleted(BackendCollection*)),
                     SLOT(slotCollectionDeleted(BackendCollection*)));
   connect(m_master, SIGNAL(collectionChanged(BackendCollection*)),
                     SLOT(slotCollectionChanged(BackendCollection*)));

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
   QVariant output;
   QString peer;
   if (calledFromDBus()) {
      peer = message().service();
   }
   Session *session = Session::create(algorithm, input, output, peer, this);
   if (session) {
      result = session->objectPath();
   } else {
      result = QDBusObjectPath("/");
      if (calledFromDBus()) {
         sendErrorReply("org.freedesktop.Secret.Error.NotSupported");
      }
   }
   return output;
}

QDBusObjectPath Service::createCollection(const QMap<QString, QVariant> &properties,
                                          QDBusObjectPath &prompt)
{
   QString label;
   bool locked = false;

   if (properties.contains("Label")) {
      label = properties["Label"].toString();
   }
   if (properties.contains("Locked")) {
      locked = properties["Locked"].toBool();
   }
   
   if (m_master->isCallImmediate(AsyncCall::AsyncCreateCollectionMaster)) {
      BackendReturn<BackendCollection*> rc = m_master->createCollection(label, locked);
      if (rc.isError()) {
         // TODO: error creating the collection
      }

      prompt.setPath("/");
      QDBusObjectPath collPath(m_basePath.path() + "/collection/" + rc.value()->id());
      return collPath;
   } else {
      PromptBase *p = new PromptServiceCreateCollection(label, locked, m_master, this, this);
      prompt = p->objectPath();
      return QDBusObjectPath("/");
   }
}

QList<QDBusObjectPath> Service::searchItems(const QMap<QString, QString> &attributes,
                                            QList<QDBusObjectPath> &locked)
{
   // TODO: check if session exists
   // TODO: should this rather be implemented using Daemon::Collection?
   QList<QDBusObjectPath> unlocked;
   Q_FOREACH(BackendCollection* collection, m_master->collections()) {
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
   // TODO: check is session exists
   // TODO: bypass prompt

   // objects already unlocked
   QList<QDBusObjectPath> rc;
   // objects to unlock
   QMap<BackendBase*, QDBusObjectPath> unlockObjects;
   QObject *object;
   Item *item;
   Collection *collection;
   BackendItem *bi;
   BackendCollection *bc;
   
   Q_FOREACH(const QDBusObjectPath &path, objects) {
      object = QDBusConnection::sessionBus().objectRegisteredAt(path.path());
      if (!object) {
         continue;
      }
      if ((collection = qobject_cast<Collection*>(object))) {
         if ((bc = collection->backendCollection())) {
            if (!bc->isLocked()) {
               rc.append(path);
            } else {
               if (bc->isCallImmediate(AsyncCall::AsyncUnlock)) {
                  // unlock without prompt?
                  BackendReturn<bool> br = bc->unlock();
                  if (!br.isError() && br.value()) {
                     rc.append(path);
                  } else {
                     // fallback: try async unlocking
                     unlockObjects.insert(bc, path);
                  }
               } else {
                  unlockObjects.insert(bc, path);
               }
            }
         }
      } else if ((item = qobject_cast<Item*>(object))) {
         if ((bi = item->backendItem())) {
            if (!bi->isLocked()) {
               rc.append(path);
            } else {
               if (bi->isCallImmediate(AsyncCall::AsyncUnlock)) {
                  // unlock without prompt?
                  BackendReturn<bool> br = bi->unlock();
                  if (!br.isError() && br.value()) {
                     rc.append(path);
                  } else {
                     // fallback: try async unlocking
                     unlockObjects.insert(bi, path);
                  }
               } else {
                  unlockObjects.insert(bi, path);
               }
            }
         }
      }
      // NOTE: objects which either don't exist or whose type is wrong are silently ignored.
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
   // TODO: check is session exists
   // TODO: bypass prompt
   
   // objects already locked
   QList<QDBusObjectPath> rc;
   // objects to lock
   QMap<BackendBase*, QDBusObjectPath> lockObjects;
   QObject *object;
   Item *item;
   Collection *collection;
   BackendItem *bi;
   BackendCollection *bc;
   
   Q_FOREACH(const QDBusObjectPath &path, objects) {
      object = QDBusConnection::sessionBus().objectRegisteredAt(path.path());
      if (!object) {
         continue;
      }
      if ((collection = qobject_cast<Collection*>(object))) {
         if ((bc = collection->backendCollection())) {
            if (bc->isLocked()) {
               rc.append(path);
            } else {
               if (bc->isCallImmediate(AsyncCall::AsyncLock)) {
                  // lock without prompt?
                  BackendReturn<bool> br = bc->lock();
                  if (!br.isError() && br.value()) {
                     rc.append(path);
                  } else {
                     // fallback: try async unlocking
                     lockObjects.insert(bc, path);
                  }
               } else {
                  lockObjects.insert(bc, path);
               }
            }
         }
      } else if ((item = qobject_cast<Item*>(object))) {
         if ((bi = item->backendItem())) {
            if (bi->isLocked()) {
               rc.append(path);
            } else {
               if (bi->isCallImmediate(AsyncCall::AsyncLock)) {
                  // unlock without prompt?
                  BackendReturn<bool> br = bi->lock();
                  if (!br.isError() && br.value()) {
                     rc.append(path);
                  } else {
                     // fallback: try async unlocking
                     lockObjects.insert(bi, path);
                  }
               } else {
                  lockObjects.insert(bi, path);
               }
            }
         }
      }
      // NOTE: objects which either don't exist or whose type is wrong are silently ignored.
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
   QMap<QDBusObjectPath, Secret> rc;
   QObject *object;
   Session *sessionObj;
   Item *item;
   bool ok;
   Secret secret;
   
   object = QDBusConnection::sessionBus().objectRegisteredAt(session.path());
   if (!object || !(sessionObj = qobject_cast<Session*>(object))) {
      if (calledFromDBus()) {
         sendErrorReply("org.freedesktop.Secret.Error.NoSession");
      }
      return rc;
   }
   
   Q_FOREACH(const QDBusObjectPath &path, items) {
      object = QDBusConnection::sessionBus().objectRegisteredAt(path.path());
      if (object && (item = qobject_cast<Item*>(object))) {
         BackendItem *bi = item->backendItem();
         if (bi && !bi->isLocked()) {
            BackendReturn<QCA::SecureArray> be = bi->secret();
            // TODO: what should this do if getting the secret failed?
            if (!be.isError()) {
               secret = sessionObj->encrypt(be.value(), ok);
               // TODO: what should this do if encrypting failed?
               if (ok) {
                  rc.insert(path, secret);
               }
            }
         }
      }
   }

   return rc;
}

void Service::slotCollectionCreated(BackendCollection *collection)
{
   Q_ASSERT(collection);
   Collection *coll = new Collection(collection, this);
   m_collections.append(coll->objectPath());
   emit collectionCreated(coll->objectPath());
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

#include "service.moc"
