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
#include "adaptors/serviceadaptor.h"
#include "collection.h"
#include "prompt.h"
#include "session.h"
#include "adaptors/daemonsecret.h"
#include "item.h"
#include "peer.h"
#include "jobinfostructs.h"

#include <backend/backendcollection.h>
#include <backend/backenditem.h>

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusMetaType>
#include <QtDBus/QDBusMessage>
#include <kdebug.h>
#include <klocalizedstring.h>

//#include <dbus/dbus.h>

Service::Service(BackendMaster *master, QObject *parent)
    : QObject(parent), m_master(master), m_basePath("/org/freedesktop/secrets")
{
    Q_ASSERT(master);

    new orgFreedesktopSecret::ServiceAdaptor(this);
    QDBusConnection::sessionBus().registerObject(m_basePath.path(), this);

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
    Session *session = Session::create(algorithm, input, output, getCallingPeer(), this);
    if(session) {
        result = session->objectPath();
    } else {
        result = QDBusObjectPath("/");
        if(calledFromDBus()) {
            sendErrorReply("org.freedesktop.Secret.Error.NotSupported");
        }
    }
    if(calledFromDBus()) {
        connection().connect(QLatin1String("org.freedesktop.DBus"),
                             QLatin1String("/org/freedesktop/DBus"),
                             QLatin1String("org.freedesktop.DBus.Local"),
                             QLatin1String("Disconnected"),
                             this, SLOT(onDbusDisconnected(QString)));
    }

    return output;
}

void Service::onDbusDisconnected(QString path)
{
    kDebug() << "Disconnected " << path;
}

QDBusObjectPath Service::createCollection(const QMap<QString, QVariant> &properties,
                                          const QString& alias,
                                          QDBusObjectPath &prompt)
{
    QString label;
    if ( alias.isEmpty() ) {
        // TODO: find a way to get properties lookup case insensitive
        if(properties.contains("Label")) {
            label = properties["Label"].toString();
        }
        else {
            // FIXME: shouldn't we throw an error if the collection has no name specified ?
            kDebug() << "No collection name give, giving default alias";
            // TODO: default label
            label = i18n("Default Collection");
        }
    }
    else {
        label = alias;
    }

    CollectionCreateInfo createCollectionInfo(label, getCallingPeer());
    if(properties.contains("Locked")) {
        createCollectionInfo.m_locked = properties["Locked"].toBool();
    }

    CreateCollectionMasterJob *job = m_master->createCreateCollectionMasterJob(createCollectionInfo);
    if(job->isImmediate()) {
        job->exec();
        if(job->error() != BackendNoError || !job->collection()) {
            // TODO: error creating the collection
            return QDBusObjectPath("/");
        } else {
            BackendCollection *coll = job->collection();
            prompt.setPath("/");
            QDBusObjectPath collPath(m_basePath.path() + "/collection/" + coll->id());
            return collPath;
        }
    } else {
        PromptBase *p = new SingleJobPrompt(this, job, this);
        prompt = p->objectPath();
        return QDBusObjectPath("/");
    }
}

QList<QDBusObjectPath> Service::searchItems(const StringStringMap &attributes,
        QList<QDBusObjectPath> &locked)
{
    // TODO: check if session exists
    // TODO: should this rather be implemented using Daemon::Collection?
    // TODO: should we implement ACL handling on this call ? Maybe the collection unlocking ACL handling may have been sufficient
    QList<QDBusObjectPath> unlocked;
    Q_FOREACH(BackendCollection * collection, m_master->collections()) {
        QString collPath = m_basePath.path() + "/collection/" + collection->id();
        BackendReturn<QList<BackendItem*> > rc = collection->searchItems(attributes);
        if(!rc.isError()) {
            QList<BackendItem*> items = rc.value();
            Q_FOREACH(BackendItem * item, items) {
                if(item->isLocked()) {
                    locked.append(QDBusObjectPath(collPath + '/' + item->id()));
                } else {
                    unlocked.append(QDBusObjectPath(collPath + '/' + item->id()));
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
    // jobs to call asynchronously
    QSet<BackendJob*> unlockJobs;
    QObject *object;
    Item *item;
    Collection *collection;
    BackendItem *bi;
    BackendCollection *bc;

    Q_FOREACH(const QDBusObjectPath & path, objects) {
        object = QDBusConnection::sessionBus().objectRegisteredAt(path.path());
        if(!object) {
            continue;
        }
        if((collection = qobject_cast<Collection*>(object))) {
            bc = collection->backendCollection();
            if(bc) {
                if(!bc->isLocked()) {
                    rc.append(path);
                } else {
                    CollectionUnlockInfo unlockInfo(getCallingPeer());
                    UnlockCollectionJob *ucj = bc->createUnlockJob(unlockInfo);
                    if(ucj->isImmediate()) {
                        ucj->exec();
                        if(ucj->error() != BackendNoError || !ucj->result()) {
                            // not unlocked, maybe due to an error.
                            // There's not much to do about it. Silently ignore.
                        } else {
                            rc.append(path);
                        }
                    } else {
                        unlockJobs.insert(ucj);
                    }
                }
            }
        } else if((item = qobject_cast<Item*>(object))) {
            bi = item->backendItem();
            if(bi) {
                if(!bi->isLocked()) {
                    rc.append(path);
                } else {
                    ItemUnlockInfo unlockInfo(getCallingPeer());
                    UnlockItemJob *uij = bi->createUnlockJob(unlockInfo);
                    if(uij->isImmediate()) {
                        uij->exec();
                        if(uij->error() != BackendNoError || !uij->result()) {
                            // not unlocked, maybe due to an error.
                            // There's not much to do about it. Silently ignore.
                        } else {
                            rc.append(path);
                        }
                    } else {
                        unlockJobs.insert(uij);
                    }
                }
            }
        }
        // NOTE: objects which either don't exist or whose type is wrong are silently ignored.
    }

    if(!unlockJobs.isEmpty()) {
        ServiceMultiPrompt *p = new ServiceMultiPrompt(this, unlockJobs, this);
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
    // TODO: do we need ACL handling when locking objects ?

    // objects already locked
    QList<QDBusObjectPath> rc;
    // jobs to call asynchronously
    QSet<BackendJob*> lockJobs;
    QObject *object;
    Item *item;
    Collection *collection;
    BackendItem *bi;
    BackendCollection *bc;

    Q_FOREACH(const QDBusObjectPath & path, objects) {
        object = QDBusConnection::sessionBus().objectRegisteredAt(path.path());
        if(!object) {
            continue;
        }
        if((collection = qobject_cast<Collection*>(object))) {
            bc = collection->backendCollection();
            if(bc) {
                if(bc->isLocked()) {
                    rc.append(path);
                } else {
                    LockCollectionJob *lcj = bc->createLockJob();
                    if(lcj->isImmediate()) {
                        lcj->exec();
                        if(lcj->error() != BackendNoError || !lcj->result()) {
                            // not locked, maybe due to an error.
                            // There's not much to do about it. Silently ignore.
                        } else {
                            rc.append(path);
                        }
                    } else {
                        lockJobs.insert(lcj);
                    }
                }
            }
        } else if((item = qobject_cast<Item*>(object))) {
            bi = item->backendItem();
            if(bi) {
                if(bi->isLocked()) {
                    rc.append(path);
                } else {
                    LockItemJob *lij = bi->createLockJob();
                    if(lij->isImmediate()) {
                        lij->exec();
                        if(lij->error() != BackendNoError || !lij->result()) {
                            // not locked, maybe due to an error.
                            // There's not much to do about it. Silently ignore.
                        } else {
                            rc.append(path);
                        }
                    } else {
                        lockJobs.insert(lij);
                    }
                }
            }
        }
        // NOTE: objects which either don't exist or whose type is wrong are silently ignored.
    }

    if(!lockJobs.isEmpty()) {
        ServiceMultiPrompt *p = new ServiceMultiPrompt(this, lockJobs, this);
        prompt = p->objectPath();
    } else {
        prompt.setPath("/");
    }

    return rc;
}

QMap<QDBusObjectPath, SecretStruct> Service::getSecrets(const QList<QDBusObjectPath> &items,
        const QDBusObjectPath &session)
{
    QMap<QDBusObjectPath, SecretStruct> rc;
    QObject *object;
    Session *sessionObj;
    Item *item;

    object = QDBusConnection::sessionBus().objectRegisteredAt(session.path());
    if(!object || !(sessionObj = qobject_cast<Session*>(object))) {
        if(calledFromDBus()) {
            sendErrorReply("org.freedesktop.Secret.Error.NoSession");
        }
        return rc;
    }

    Q_FOREACH(const QDBusObjectPath & path, items) {
        object = QDBusConnection::sessionBus().objectRegisteredAt(path.path());
        if(object && (item = qobject_cast<Item*>(object))) {
            BackendItem *bi = item->backendItem();
            if(bi && !bi->isLocked()) {
                SecretStruct secret = item->getSecret( session );
                // TODO: what should this do if getting the secret failed?
                rc.insert(path, secret);
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
