/* This file is part of the KDE project
 *
 * Copyright (C) 2011 Valentin Rusu <kde@rusu.info>
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

#ifndef KSECRETSITEM_H
#define KSECRETSITEM_H

#include "ksecretsservicesecret.h"
#include "ksecretsservicejob.h"

class KJob;

namespace KSecretsService {

class SecretsJobBase;
   
/**
 */
class SecretItem {
public:
    class DeleteItemJob;
    
    /**
     */
    KJob * deleteItem();

    class GetSecretJob;

    /**
     * Read the data held by the SecretItem
     */
    GetSecretJob * getSecret() const;

    /**
     * Modify the item's stored data
     */
    KJob* setSecret( const Secret &secret );

    /**
     * FIXME: This methods accesses a dbus property. should it be asynchronous ?
     */
    QMap< QString, QString> attributes() const;
    
    /**
     * FIXME: This methods accesses a dbus property. should it be asynchronous ?
     */
    void setAttributes( const QMap< QString, QString > &attributes );

    bool isLocked() const;
    QString label() const;
    QDateTime createdTime() const;
    QDateTime modifiedTime() const;
    
    void setLabel( const QString &label );
    
private:
    
    class Private;
    QSharedDataPointer< Private > d;
};

class SecretItem::GetSecretJob : public SecretsJobBase {
    Q_OBJECT
    Q_DISABLE_COPY(GetSecretJob)
public:
    
    Secret secret() const;
    
private:
    class Private;
    QSharedDataPointer< Private > *d;
};


};

#endif // KSECRETSITEM_H
