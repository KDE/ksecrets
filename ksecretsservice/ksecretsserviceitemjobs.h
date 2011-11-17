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

#ifndef KSECRETSSERVICEITEMJOBS_H
#define KSECRETSSERVICEITEMJOBS_H

#include "ksecretsservicesecret.h"
#include "ksecretsservicemacros.h"

#include <QSharedPointer>
#include <kcompositejob.h>
#include <qwindowdefs.h>

namespace KSecretsService {

class SecretItem;
class SecretItemJobPrivate;
class GetSecretItemSecretJobPrivate;
class SetSecretItemSecretJobPrivate;
class SecretItemDeleteJobPrivate;
class ReadItemPropertyJobPrivate;
class WriteItemPropertyJobPrivate;

/**
 * @class SecretItemJob
 * @internal
 * 
 * Base class for the SecretItem related jobs. Your application is not supposed
 * to use this class directly.
 */
class KSECRETSSERVICE_EXPORT SecretItemJob : public KCompositeJob {
    Q_OBJECT
    Q_DISABLE_COPY(SecretItemJob)
public:
    enum ItemJobError {
        UndefinedError =-1,             /// this error should never be encountered
        NoError =0,
        InternalError,
        OperationCancelledByTheUser,    /// the user choose to cancel ther operation during a message prompt
        CollectionNotFound,
        CreateError,
        DeleteError,
        RenameError,
        MissingParameterError
    };
    
    explicit SecretItemJob( SecretItem * item );
    virtual ~SecretItemJob();

    SecretItem* secretItem() const;
    /**
     * Chaining asynchronous jobs may need you to pass data items around
     * This member let you attach arbitrary data to this job
     */
    void setCustomData( const QVariant& data );
    const QVariant& customData() const;
    
protected:
    void finished( ItemJobError, const QString& msg = QString() );
    
private:
    friend class SecretItemJobPrivate;
    QSharedPointer< SecretItemJobPrivate > d;
};

class KSECRETSSERVICE_EXPORT GetSecretItemSecretJob : public SecretItemJob {
    Q_OBJECT
    Q_DISABLE_COPY(GetSecretItemSecretJob)
public:
    explicit GetSecretItemSecretJob( SecretItem* );
    virtual ~GetSecretItemSecretJob();
    virtual void start();
    Secret secret() const;
    
private:
    friend class GetSecretItemSecretJobPrivate;
    QSharedPointer< GetSecretItemSecretJobPrivate > d;
};

class KSECRETSSERVICE_EXPORT SetSecretItemSecretJob : public SecretItemJob {
    Q_OBJECT
    Q_DISABLE_COPY(SetSecretItemSecretJob)
public:
    SetSecretItemSecretJob( SecretItem*, const Secret& );
    virtual ~SetSecretItemSecretJob();
    
    virtual void start();
    
private:
    friend class SetSecretItemSecretJobPrivate;
    QSharedPointer< SetSecretItemSecretJobPrivate > d;
};

class KSECRETSSERVICE_EXPORT SecretItemDeleteJob : public SecretItemJob {
    Q_OBJECT
    Q_DISABLE_COPY(SecretItemDeleteJob)
public:
    SecretItemDeleteJob( SecretItem*, const WId &promptParentWindowId );
    virtual ~SecretItemDeleteJob();
    
    virtual void start();
    
private:
    friend class SecretItemDeleteJobPrivate;
    QSharedPointer< SecretItemDeleteJobPrivate > d;
};

class KSECRETSSERVICE_EXPORT ReadItemPropertyJob : public SecretItemJob {
    Q_OBJECT
    Q_DISABLE_COPY(ReadItemPropertyJob)
public:
    ReadItemPropertyJob( SecretItem *, const char *propName);
    ReadItemPropertyJob( SecretItem *, void (SecretItem::*propReadMember)( ReadItemPropertyJob * ) );
    virtual ~ReadItemPropertyJob();
    
    virtual void start();
    const QVariant& propertyValue() const;
    
private:
    friend class ReadItemPropertyJobPrivate;
    QSharedDataPointer<ReadItemPropertyJobPrivate> d;
    void (SecretItem::*propertyReadMember)( ReadItemPropertyJob * );
};

class KSECRETSSERVICE_EXPORT WriteItemPropertyJob : public SecretItemJob {
    Q_OBJECT
    Q_DISABLE_COPY(WriteItemPropertyJob)
public:
    explicit WriteItemPropertyJob(SecretItem* item, const char *propName, const QVariant &value);
    virtual ~WriteItemPropertyJob();
    
    virtual void start();
    
private:
    friend class WriteItemPropertyJobPrivate;
    QSharedDataPointer<WriteItemPropertyJobPrivate> d;
};


}; // namespace

#endif // KSECRETSSERVICEITEMJOBS_H
