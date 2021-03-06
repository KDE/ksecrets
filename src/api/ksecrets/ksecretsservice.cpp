/*
    This file is part of the KDE Libraries

    Copyright (C) 2015 Valentin Rusu (valir@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB. If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "ksecretsservice.h"
#include "ksecretsservice_p.h"
#include "ksecretscollection.h"
#include "ksecretscollection_p.h"
#include <QtConcurrent/QtConcurrent>

namespace KSecrets {

typedef QSharedPointer<Collection> CollectionPtr;


CollectionPtr ServicePrivate::findCollection(const QString& collName,
    Service::FindCollectionOptions opts,
    const QVariantMap& collProps,
    QWidget* promptParent)
{

  // TODO
  return CollectionPtr(new Collection, &QObject::deleteLater);
}

QList<CollectionPtr> ServicePrivate::listCollections()
{
    // TODO
    return QList<CollectionPtr>();
}

Service::~Service()
{
}

QFuture<CollectionPtr> Service::findCollection(const QString& collectionName, FindCollectionOptions options /* = CreateCollection */,
    const QVariantMap& collectionProperties /* = QVariantMap() */,
    QWidget* promptParent /* =0 */)
{
  return QtConcurrent::run(&ServicePrivate::findCollection, collectionName,
      options, collectionProperties, promptParent);
}

QFuture<QList<CollectionPtr> > Service::listCollections()
{
    return QtConcurrent::run(&ServicePrivate::listCollections);
}

} // namespace
