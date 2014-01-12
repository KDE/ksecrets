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

#ifndef TEMPORARYJOBS_H
#define TEMPORARYJOBS_H

#include "temporarycollectionmanager.h"
#include "temporarycollection.h"
#include "temporaryitem.h"
#include "../backendjob.h"

#include <QtCore/QString>

/**
 * Job for creating a new temporary collection.
 */
class TemporaryCreateCollectionJob : public CreateCollectionJob
{
    Q_OBJECT

public:
    TemporaryCreateCollectionJob(const CollectionCreateInfo &createCollectionInfo,
                                 TemporaryCollectionManager *manager);
    virtual void start();
};

/**
 * Job for unlocking a temporary collection.
 */
class TemporaryUnlockCollectionJob : public UnlockCollectionJob
{
    Q_OBJECT

public:
    TemporaryUnlockCollectionJob(const CollectionUnlockInfo &unlockInfo, BackendCollection *coll);
    virtual void start();
};

/**
 * Job for locking a temporary collection.
 */
class TemporaryLockCollectionJob : public LockCollectionJob
{
    Q_OBJECT

public:
    TemporaryLockCollectionJob(BackendCollection *coll);
    virtual void start();
};

/**
 * Job for deleting a temporary collection.
 */
class TemporaryDeleteCollectionJob : public DeleteCollectionJob
{
    Q_OBJECT

public:
    TemporaryDeleteCollectionJob(const CollectionDeleteInfo& deleteInfo);
    virtual void start();
};

/**
 * Job for changing a temporary collection's authentication.
 */
class TemporaryChangeAuthenticationCollectionJob : public ChangeAuthenticationCollectionJob
{
    Q_OBJECT

public:
    TemporaryChangeAuthenticationCollectionJob(BackendCollection *coll, const Peer& );
    virtual void start();
};

/**
 * Job for creating an item inside a temporary collection.
 */
class TemporaryCreateItemJob : public CreateItemJob
{
    Q_OBJECT

public:
    TemporaryCreateItemJob(const ItemCreateInfo& createInfo,
                           TemporaryCollection *collection);
    virtual void start();

private:
    TemporaryCollection *m_tempColl;
};

/**
 * Job for unlocking an item inside a temporary collection.
 */
class TemporaryUnlockItemJob : public UnlockItemJob
{
    Q_OBJECT

public:
    TemporaryUnlockItemJob(const ItemUnlockInfo& unlockInfo);
    virtual void start();
};

/**
 * Job for locking an item inside a temporary collection.
 */
class TemporaryLockItemJob : public LockItemJob
{
    Q_OBJECT

public:
    TemporaryLockItemJob(BackendItem *item);
    virtual void start();
};

/**
 * Job for deleting an item inside a temporary collection.
 */
class TemporaryDeleteItemJob : public DeleteItemJob
{
    Q_OBJECT

public:
    TemporaryDeleteItemJob(const ItemDeleteInfo& deleteInfo);
    virtual void start();
};

/**
 * Job for changing a temporary item's authentication.
 */
class TemporaryChangeAuthenticationItemJob : public ChangeAuthenticationItemJob
{
    Q_OBJECT

public:
    TemporaryChangeAuthenticationItemJob(BackendItem *item);
    virtual void start();
};

#endif
