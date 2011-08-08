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

#ifndef TEMPBLOCKINGJOBS_H
#define TEMPBLOCKINGJOBS_H

#include "tempblockingcollectionmanager.h"
#include "tempblockingcollection.h"
#include "tempblockingitem.h"

#include <QtCore/QTimer>

class TempBlockingCreateCollectionJob : public CreateCollectionJob
{
    Q_OBJECT

public:
    TempBlockingCreateCollectionJob(const CollectionCreateInfo& createCollectionInfo,
                                    TempBlockingCollectionManager *manager);
    virtual bool isImmediate() const {
        return false;
    }
    virtual void exec() {
        Q_ASSERT(false);
    }
    virtual void start() {
        QTimer::singleShot(0, this, SLOT(perform()));
    }

private Q_SLOTS:
    void perform();
};

class TempBlockingUnlockCollectionJob : public UnlockCollectionJob
{
    Q_OBJECT

public:
    TempBlockingUnlockCollectionJob(const CollectionUnlockInfo &unlockInfo, BackendCollection *coll);
    virtual bool isImmediate() const {
        return false;
    }
    virtual void exec() {
        Q_ASSERT(false);
    }
    virtual void start() {
        QTimer::singleShot(0, this, SLOT(perform()));
    }

private Q_SLOTS:
    void perform();
};

class TempBlockingDeleteCollectionJob : public DeleteCollectionJob
{
    Q_OBJECT

public:
    TempBlockingDeleteCollectionJob(const CollectionDeleteInfo& deleteInfo);
    virtual bool isImmediate() const {
        return false;
    }
    virtual void exec() {
        Q_ASSERT(false);
    }
    virtual void start() {
        QTimer::singleShot(0, this, SLOT(perform()));
    }

private Q_SLOTS:
    void perform();
};

class TempBlockingLockCollectionJob : public LockCollectionJob
{
    Q_OBJECT

public:
    TempBlockingLockCollectionJob(BackendCollection *coll);
    virtual bool isImmediate() const {
        return false;
    }
    virtual void exec() {
        Q_ASSERT(false);
    }
    virtual void start() {
        QTimer::singleShot(0, this, SLOT(perform()));
    }

private Q_SLOTS:
    void perform();
};

class TempBlockingChangeAuthenticationCollectionJob : public ChangeAuthenticationCollectionJob
{
    Q_OBJECT

public:
    TempBlockingChangeAuthenticationCollectionJob(BackendCollection *coll, const Peer& peer );
    virtual bool isImmediate() const {
        return false;
    }
    virtual void exec() {
        Q_ASSERT(false);
    }
    virtual void start() {
        QTimer::singleShot(0, this, SLOT(perform()));
    }

private Q_SLOTS:
    void perform();
};

class TempBlockingCreateItemJob : public CreateItemJob
{
    Q_OBJECT

public:
    TempBlockingCreateItemJob(const ItemCreateInfo& createInfo,
                              TempBlockingCollection *collection);
    virtual bool isImmediate() const {
        return false;
    }
    virtual void exec() {
        Q_ASSERT(false);
    }
    virtual void start() {
        QTimer::singleShot(0, this, SLOT(perform()));
    }

private Q_SLOTS:
    void perform();

private:
    TempBlockingCollection* m_tempColl;
};

class TempBlockingUnlockItemJob : public UnlockItemJob
{
    Q_OBJECT

public:
    TempBlockingUnlockItemJob(const ItemUnlockInfo& unlockInfo);
    virtual bool isImmediate() const {
        return false;
    }
    virtual void exec() {
        Q_ASSERT(false);
    }
    virtual void start() {
        QTimer::singleShot(0, this, SLOT(perform()));
    }

private Q_SLOTS:
    void perform();
};

class TempBlockingLockItemJob : public LockItemJob
{
    Q_OBJECT

public:
    TempBlockingLockItemJob(BackendItem *item);
    virtual bool isImmediate() const {
        return false;
    }
    virtual void exec() {
        Q_ASSERT(false);
    }
    virtual void start() {
        QTimer::singleShot(0, this, SLOT(perform()));
    }

private Q_SLOTS:
    void perform();
};

class TempBlockingDeleteItemJob : public DeleteItemJob
{
    Q_OBJECT

public:
    TempBlockingDeleteItemJob(const ItemDeleteInfo &deleteInfo);
    virtual bool isImmediate() const {
        return false;
    }
    virtual void exec() {
        Q_ASSERT(false);
    }
    virtual void start() {
        QTimer::singleShot(0, this, SLOT(perform()));
    }

private Q_SLOTS:
    void perform();
};

class TempBlockingChangeAuthenticationItemJob : public ChangeAuthenticationItemJob
{
    Q_OBJECT

public:
    TempBlockingChangeAuthenticationItemJob(BackendItem *item);
    virtual bool isImmediate() const {
        return false;
    }
    virtual void exec() {
        Q_ASSERT(false);
    }
    virtual void start() {
        QTimer::singleShot(0, this, SLOT(perform()));
    }

private Q_SLOTS:
    void perform();
};

#endif
