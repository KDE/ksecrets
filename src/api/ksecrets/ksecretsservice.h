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
#ifndef KSECRETSSERVICE
#define KSECRETSSERVICE

#include <QObject>
#include <QFuture>
#include <ksecrets_export.h>

#include "ksecretscollection.h"

namespace KSecrets {

class ServicePrivate;

/**
 * KSecrets Service entry point.
 *
 * KDE applications use this class to actually handle their secrets.
 * This class would do the magic of connecting to the right backend
 * and store or retrieve the secrets.
 *
 * We encourage you to first read the Free Desktop draft about
 * <a href="http://standards.freedesktop.org/secret-service/">Secret Service</a>
 */
class KSECRETS_EXPORT Service : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(Service)
public:
    virtual ~Service();

    /**
     * Options used when findCollection method is called
     */
    enum FindCollectionOptions {
        OpenOnly = 0,         /// this will only try to open the collection without
                              /// creating it if not found
        CreateCollection = 1, /// the collection will be created if not found
        LookIntoKWallet = 2   /// Specify this if your application was ported from KWallet to KSecrets
    };

    /**
    */
    static QFuture<CollectionPtr> findCollection(const QString& collectionName,
        FindCollectionOptions options = CreateCollection,
        const QVariantMap& collectionProperties = QVariantMap(),
        QWidget* promptParent = 0);

    /**
     * Use this method to find out the names of all known secret service
     * collections on the running system
     */
    static QFuture<QList<CollectionPtr> > listCollections();

private:
    QSharedPointer<ServicePrivate> d;
};

} // namespace

#endif
