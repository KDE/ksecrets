/* This file is part of the KDE project
 *
 * Copyright (C) 2011 Valentin Rusu <valir@kde.org>
 * Copyright (C) 2015 Valentin Rusu <valir@kde.org>
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

Collection::Status Collection::status() const { return d->collectionStatus; }

QFuture<bool> Collection::deleteCollection()
{
    return QtConcurrent::run(d.data(), &CollectionPrivate::deleteCollection);
}

QFuture<bool> Collection::renameCollection(const QString& newName)
{
    return QtConcurrent::run(
        d.data(), &CollectionPrivate::renameCollection, newName);
}

QFuture<QList<SecretItemPtr> > Collection::searchItems(
    const AttributesMap& attributes)
{
    return QtConcurrent::run(
        d.data(), &CollectionPrivate::searchItems, attributes);
}

QFuture<QList<SecretItemPtr> > Collection::searchItems(const QString&)
{
    AttributesMap attrs;
    // TODO add here the label property
    return searchItems(attrs);
}

QFuture<QList<SecretItemPtr> > Collection::searchItems(
    const QString&, const AttributesMap& initialAttrs)
{
    AttributesMap attrs(initialAttrs);
    // TODO add here the label property
    return searchItems(attrs);
}

QFuture<QList<SecretPtr> > Collection::searchSecrets(
    const AttributesMap& attributes)
{
    return QtConcurrent::run(
        d.data(), &CollectionPrivate::searchSecrets, attributes);
}

QFuture<bool> Collection::createItem(const QString& label,
    const Secret& secret, const QMap<QString, QString>& attributes,
    CreateItemOptions options /* = DoNotReplaceExistingItem */)
{
    return QtConcurrent::run(d.data(), &CollectionPrivate::createItem, label,
        attributes, secret, options);
}

QFuture<QList<SecretItemPtr> > Collection::items() const
{
    return QtConcurrent::run(d.data(), &CollectionPrivate::items);
}

QFuture<bool> Collection::isLocked() const
{
    return QtConcurrent::run(d.data(), &CollectionPrivate::isLocked);
}

QFuture<QString> Collection::label() const
{
    return QtConcurrent::run(d.data(), &CollectionPrivate::label);
}

QFuture<QDateTime> Collection::createdTime() const
{
    return QtConcurrent::run(d.data(), &CollectionPrivate::createdTime);
}

QFuture<QDateTime> Collection::modifiedTime() const
{
    return QtConcurrent::run(d.data(), &CollectionPrivate::modifiedTime);
}

QFuture<bool> Collection::setLabel(const QString& label)
{
    return QtConcurrent::run(d.data(), &CollectionPrivate::writeProperty,
        QLatin1Literal("Label"), QVariant(label));
}

QFuture<bool> Collection::lock()
{
    return QtConcurrent::run(d.data(), &CollectionPrivate::lock);
}

void Collection::emitStatusChanged()
{
    emit statusChanged(d->collectionStatus);
}

void Collection::emitContentsChanged() { emit contentsChanged(); }

void Collection::emitDeleted() { emit deleted(); }

CollectionPrivate::CollectionPrivate(Collection* coll)
    : collection(coll)
    , collectionStatus(Collection::Invalid)
{
}

CollectionPrivate::~CollectionPrivate() {}

void CollectionPrivate::setStatus(Collection::Status newStatus)
{
    collectionStatus = newStatus;
    collection->emitStatusChanged();
}

bool CollectionPrivate::isValid()
{
    return (collectionStatus == Collection::NewlyCreated)
        || (collectionStatus == Collection::FoundExisting);
}

QFuture<bool> Collection::isValid()
{
    return QtConcurrent::run(d.data(), &CollectionPrivate::isValid);
}

bool CollectionPrivate::writeProperty(
    const QString& propName, const QVariant& propVal)
{
    // TODO
    return false;
}

bool CollectionPrivate::isNewlyCreated() const
{
    // TODO
    return false;
}

bool CollectionPrivate::lock()
{
    // TODO
    return true;
}

bool CollectionPrivate::deleteCollection()
{
    // TODO
    return true;
}

bool CollectionPrivate::renameCollection(const QString&)
{
    // TODO
    return true;
}

QList<SecretItemPtr> CollectionPrivate::searchItems(const AttributesMap&)
{
    // TODO
    return QList<SecretItemPtr>();
}

QList<SecretPtr> CollectionPrivate::searchSecrets(const AttributesMap&)
{
    // TODO
    return QList<SecretPtr>();
}

bool CollectionPrivate::createItem(const QString& label,
    const AttributesMap& attributes, const Secret& secret,
    CreateItemOptions options)
{
    // TODO
    return true;
}

bool CollectionPrivate::isLocked()
{
    // TODO
    return false;
}

QString CollectionPrivate::label()
{
    // TODO
    return QLatin1Literal("");
}

bool CollectionPrivate::setLabel(const QString&)
{
    // TODO
    return true;
}

QDateTime CollectionPrivate::createdTime()
{
    // TODO
    return QDateTime();
}

QDateTime CollectionPrivate::modifiedTime()
{
    // TODO
    return QDateTime();
}

QList<SecretItemPtr> CollectionPrivate::items() const
{
    // TODO
    return QList<SecretItemPtr>();
}

} // namespace
