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

#ifndef KSECRETSSERVICEITEM_H
#define KSECRETSSERVICEITEM_H

#include "ksecretsservicesecret.h"
#include "ksecretsserviceitemjobs.h"
#include "ksecretsservicemacros.h"

#include <QSharedData>


namespace KSecretsService {

typedef QMap< QString, QString > QStringStringMap;

class GetSecretItemSecretJob;
class SetSecretItemSecretJob;
class SecretItemDeleteJob;
class ReadItemPropertyJob;
class WriteItemPropertyJob;
class SecretItemPrivate;
class CreateItemJobPrivate;
   
/**
 * KSecretsService aims to let application store sensitive pieces of information as SecretItem(s).
 * The central part of a SecretItem is the secret it holds. The secret is actually a structure named @ref SecretStruct["(SecretStruct)"]
 * SecretItems can be qualified using attributes. These attributes are used internally by KSecretsService to uniquely identify them inside the collection.
 * The attributes list always contain at least one item, named "Label". It's content is up to the client application.
 * The "Label" attribute can also be read by calling the @ref attribute method and set by @ref setLabel method.
 * 
 * Please note that all the jobs returned by this class autodelete themselbes when done. If you application
 * need to access the returned items, then it should copy them away before returning from the job's done 
 * signal handling method.
 */
class KSECRETSSERVICE_EXPORT SecretItem : public QSharedData {
    SecretItem( SecretItemPrivate * );
public:
    SecretItem();
    SecretItem( const SecretItem& );
    virtual ~SecretItem();
    
    /**
     * @return SecretItemJob wich will attempt to delete this item upon it's start() method call
     */
    SecretItemDeleteJob * deleteItem( const WId &promptParentWindowId =0 );

    /**
     * Read the data held by the SecretItem
     */
    GetSecretItemSecretJob * getSecret() const;

    /**
     * Modify the item's stored data
     */
    SetSecretItemSecretJob* setSecret( const Secret &secret );

    /**
     * @note returned ReadItemPropertyJob::value is a QMap< QString, QString>
     */
    ReadItemPropertyJob * attributes() const;
    
    /**
     * @param attributes a map containing the new attributes; it must contain at least one attribute, under the name "Label"
     */
    WriteItemPropertyJob * setAttributes( const QMap< QString, QString > &attributes );

    /**
     * @note returned ReadItemPropertyJob::value is a bool
     */
    ReadItemPropertyJob * isLocked() const;
    
    /**
     * @note returned ReadItemPropertyJob::value is a QString
     */
    ReadItemPropertyJob * label() const;
    
    /**
     * @note returned ReadItemPropertyJob::value is a time_t
     */
    ReadItemPropertyJob * createdTime() const;
    
    /**
     * @note returned ReadItemPropertyJob::value is a time_t
     */
    ReadItemPropertyJob * modifiedTime() const;
    
    /**
     * Sets the item's label
     */
    WriteItemPropertyJob * setLabel( const QString &label );
    
private:
    friend class SecretItemPrivate;
    friend class GetSecretItemSecretJob;
    friend class GetSecretItemSecretJobPrivate;
    friend class SetSecretItemSecretJob;
    friend class SecretItemDeleteJob;
    friend class ReadItemPropertyJob;
    friend class WriteItemPropertyJob;
    friend class CreateItemJobPrivate;
    friend class SearchCollectionItemsJob;
    friend class CreateCollectionItemJobPrivate;
    friend class ReadCollectionItemsJob;
    
    QSharedDataPointer< SecretItemPrivate > d;
};

};

Q_DECLARE_METATYPE( KSecretsService::SecretItem )

#endif // KSECRETSSERVICEITEM_H
