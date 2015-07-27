/* This file is part of the KDE project
 *
 * Copyright (C) 2011 Valentin Rusu <kde@rusu.info>
 * Copyright (C) 2015 Valentin Rusu <kde@rusu.info>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "ksecretscollection.h"
#include "ksecretscollection_p.h"
#include "dbusbackend.h"
#include "collection_interface.h"

#include <QDateTime>
#include <QtDBus/QDBusPendingReply>
#include <QTimer>
#include <QDebug>
#include <QtConcurrentRun>

namespace KSecrets {

Collection::Collection()
    : QObject()
    , d(new CollectionPrivate(this))
{
  // nothing to do
}

Collection::~Collection() {}

QFuture< QList<CollectionPtr> > Collection::listCollections()
{
  return QtConcurrent::run(&CollectionPrivate::listCollections);
}

Collection::Status Collection::status() const { return d->collectionStatus; }

QFuture<bool> Collection::deleteCollection()
{
  return QtConcurrent::run(d, &CollectionPrivate::deleteCollection);
}

QFuture<bool> Collection::renameCollection(const QString& newName)
{
  return QtConcurrent::run(&d, &CollectionPrivate::renameCollection, newName);
}

QFuture<SecretItemPtr> Collection::searchItems(
    const QStringStringMap& attributes)
{
  return QtConcurrent::run(&d, &CollectionPrivate::searchItems, attributes);
}

QFuture<SecretPtr> Collection::searchSecrets(
    const QStringStringMap& attributes)
{
  return QtConcurrent::run(&d, &CollectionPrivate::searchSecrets, attributes);
}

QFuture<bool> Collection::createItem(const QString& label,
    const QMap<QString, QString>& attributes, const Secret& secret,
    CreateItemOptions options /* = DoNotReplaceExistingItem */)
{
  return QtConcurrent::run(
      &d, &CollectionPrivate::createItem(label, attributes, secret, options));
}

QFuture<SecretItemPtr> Collection::items() const
{
  return QtConcurrent::run(&d, &CollectionPrivate::items);
}

QFuture<bool> Collection::isLocked() const
{
  return QtConcurrent::run(&d, &CollectionPrivate::isLocked);
}

QFuture<QString> Collection::label() const
{
  return QtConcurrent::run(&d, &CollectionPrivate::label);
}

QFuture<QDateTime> Collection::createdTime() const
{
  return QtConcurrent::run(&d, &CollectionPrivate::createdTime);
}

QFuture<QDateTime> Collection::modifiedTime() const
{
  return QtConcurrent::run(&d, &CollectionPrivate::modifiedTime);
}

QFuture<bool> Collection::setLabel(const QString& label)
{
  return QtConcurrent::run(&d, &CollectionPrivate::writeProperty,
      QString("Label"), QVariant(label));
}

QFuture<bool> Collection::lock()
{
  return QtConcurrent::run(&d, &CollectionPrivate::lock);
}

void Collection::emitStatusChanged()
{
  emit statusChanged(d->collectionStatus);
}

void Collection::emitContentsChanged() { emit contentsChanged(); }

void Collection::emitDeleted() { emit deleted(); }

QMap<QDBusObjectPath, CollectionPrivate*> CollectionPrivate::collectionMap;

CollectionPrivate::CollectionPrivate(Collection* coll)
    : collection(coll)
    , collectionStatus(Collection::Invalid)
    , collectionIf(0)
{
}

CollectionPrivate::~CollectionPrivate()
{
  if (collectionMap.contains(dbusPath)) {
    collectionMap.remove(dbusPath);
  }
}

void CollectionPrivate::setStatus(Collection::Status newStatus)
{
  collectionStatus = newStatus;
  collection->emitStatusChanged();
}

bool CollectionPrivate::isValid()
{
  // NOTE: do not call collectionInterface() to get the interface pointer, if
  // not you'll get an infinite recursive call
  return collectionIf && collectionIf->isValid()
      && (collectionStatus == Collection::FoundExisting
             || collectionStatus == Collection::NewlyCreated);
}

void CollectionPrivate::setDBusPath(const QDBusObjectPath& path)
{
  collectionIf = DBusSession::createCollectionIf(path);
  if (collectionIf->isValid()) {
    qDebug() << "SUCCESS opening collection " << path.path();
    collectionMap.insert(path, this);
    dbusPath = path;
  }
  else {
    setStatus(Collection::NotFound);
    qDebug() << "ERROR opening collection " << path.path();
  }
}

OrgFreedesktopSecretCollectionInterface*
CollectionPrivate::collectionInterface()
{
  if ((collectionIf == 0) || (!collectionIf->isValid())) {
  }
  return collectionIf;
}

QFuture<bool> Collection::isValid()
{
  return QtConcurrent::run(&d, &CollectionPrivate::isValid);
}

void CollectionPrivate::notifyCollectionChanged(const QDBusObjectPath& path)
{
  if (collectionMap.contains(path)) {
    CollectionPrivate* cp = collectionMap[path];
    cp->collection->emitContentsChanged();
  }
  else {
    qDebug() << "Ignoring notifyCollectionChanged for " << path.path();
  }
}

void CollectionPrivate::notifyCollectionDeleted(const QDBusObjectPath& path)
{
  if (collectionMap.contains(path)) {
    CollectionPrivate* cp = collectionMap[path];
    cp->collection->emitDeleted();
  }
  else {
    qDebug() << "Ignoring notifyCollectionDeleted for " << path.path();
  }
}

bool CollectionPrivate::writeProperty(const QString& propName, const QVariant& propVal) {
  // TODO
  return false;
}
#include "ksecretscollection.moc"
} // namespace
