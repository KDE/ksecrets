/*
 * Copyright 2010, Michael Leupold <lemma@confuego.org>
 * Copyright 2011, Valentin Rusu <kde@rusu.info>
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

#ifndef KSECRETJOBS_H
#define KSECRETJOBS_H

#include "ksecretcollectionmanager.h"
#include "ksecretcollection.h"
#include "ksecretitem.h"

#include <backend/backendjob.h>
#include <ui/abstractuijobs.h>

#include <QtCore/QPointer>

/**
 * Job for creating a new ksecret collection.
 */
class KSecretCreateCollectionJob : public CreateCollectionJob
{
    Q_OBJECT

public:
    KSecretCreateCollectionJob(const CollectionCreateInfo &createCollectionInfo,
                               KSecretCollectionManager *manager);
    virtual bool isImmediate() const;

protected:
    virtual void start();

private Q_SLOTS:
    void newPasswordJobResult(KJob *job);
    void askAclPrefsJobResult(KJob *job);

private:
    KSecretCollectionManager *m_manager;
};

/**
 * Job for unlocking a ksecret collection.
 */
class KSecretUnlockCollectionJob : public UnlockCollectionJob
{
    Q_OBJECT

public:
    KSecretUnlockCollectionJob(const CollectionUnlockInfo &unlockInfo, KSecretCollection *coll);
    virtual bool isImmediate() const;

protected:
    virtual void start();

private Q_SLOTS:
    void askPasswordJobResult(KJob *job);
    void askAclPrefsJobResult(KJob *job);

private:
    void createAskPasswordJob();

    bool    m_firstTry;
    bool    m_passwordAsked;
    ApplicationPermission   
            m_collectionPerm;
};

/**
 * Job for locking a ksecret collection.
 */
class KSecretLockCollectionJob : public LockCollectionJob
{
    Q_OBJECT

public:
    KSecretLockCollectionJob(KSecretCollection *coll);
    virtual void start();

private:
    KSecretCollection *m_collection;
};

/**
 * Job for deleting a ksecret collection.
 */
class KSecretDeleteCollectionJob : public DeleteCollectionJob
{
    Q_OBJECT

public:
    KSecretDeleteCollectionJob(const CollectionDeleteInfo& deleteInfo);
    virtual void start();

private:
    KSecretCollection *m_collection;
};

/**
 * Job for creating a new item inside a ksecret collection.
 */
class KSecretCreateItemJob : public CreateItemJob
{
    Q_OBJECT

public:
    KSecretCreateItemJob(const ItemCreateInfo& createInfo,
                         KSecretCollection* collection);
    virtual void start();

private:
    KSecretCollection *m_collection;
};

/**
 * Job for changing a ksecret collection's authentication, that is change the password
 */
class KSecretChangeAuthenticationCollectionJob : public ChangeAuthenticationCollectionJob
{
    Q_OBJECT

public:
    KSecretChangeAuthenticationCollectionJob(BackendCollection *coll, const Peer& peer);
    virtual void start();
    
private Q_SLOTS:
    void slotUnlockResult(KJob*);
    void slotNewPasswordFinished(KJob*);
};

/**
 * Job for unlocking an item inside a ksecret collection.
 */
class KSecretUnlockItemJob : public UnlockItemJob
{
    Q_OBJECT

public:
    KSecretUnlockItemJob(const ItemUnlockInfo& unlockInfo, KSecretCollection *collection);
    virtual ~KSecretUnlockItemJob();
    virtual bool isImmediate() const;

protected:
    virtual void start();

private Q_SLOTS:
    void handleSubJobResult(KJob *job);

private:
    QPointer<UnlockCollectionJob> m_subJob;
};

/**
 * Job for locking an item inside a ksecret collection.
 */
class KSecretLockItemJob : public LockItemJob
{
    Q_OBJECT

public:
    KSecretLockItemJob(KSecretItem *item, KSecretCollection *collection);
    virtual ~KSecretLockItemJob();
    virtual bool isImmediate() const;

protected:
    virtual void start();

private Q_SLOTS:
    void handleSubJobResult(KJob *job);

private:
    QPointer<LockCollectionJob> m_subJob;
};

/**
 * Job for deleting an item inside a ksecret collection.
 */
class KSecretDeleteItemJob : public DeleteItemJob
{
    Q_OBJECT

public:
    KSecretDeleteItemJob(const ItemDeleteInfo &deleteInfo);
    virtual void start();

private:
    KSecretItem *m_item;
};

/**
 * Job for changing a ksecret collection's authentication.
 */
class KSecretChangeAuthenticationItemJob : public ChangeAuthenticationItemJob
{
    Q_OBJECT

public:
    KSecretChangeAuthenticationItemJob(BackendItem *item);
    virtual void start();
};

#endif
