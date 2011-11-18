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

#include "tempblockingjobs.h"

#include "../lib/secrettool.h"

#include <QtCore/QTimer>

TempBlockingCreateCollectionJob::TempBlockingCreateCollectionJob(const CollectionCreateInfo &createCollectionInfo,
        TempBlockingCollectionManager *manager)
    : CreateCollectionJob(createCollectionInfo, manager)
{
}

void TempBlockingCreateCollectionJob::perform()
{
    TempBlockingCollection *coll = new TempBlockingCollection(createId(), manager());
    coll->setLabel(label());

    setCollection(coll);
    emitResult();
}

TempBlockingUnlockCollectionJob::TempBlockingUnlockCollectionJob(const CollectionUnlockInfo &unlockInfo,
        BackendCollection* coll)
    : UnlockCollectionJob(unlockInfo, coll)
{
}

void TempBlockingUnlockCollectionJob::perform()
{
    setResult(true);
    emitResult();
}

TempBlockingLockCollectionJob::TempBlockingLockCollectionJob(BackendCollection* coll)
    : LockCollectionJob(coll)
{
}

void TempBlockingLockCollectionJob::perform()
{
    setError(BackendErrorNotSupported);
    setResult(false);
    emitResult();
}

TempBlockingDeleteCollectionJob::TempBlockingDeleteCollectionJob(const CollectionDeleteInfo& deleteInfo)
    : DeleteCollectionJob(deleteInfo)
{
}

void TempBlockingDeleteCollectionJob::perform()
{
    setResult(true);
    collection()->deleteLater();
    emitResult();
}

TempBlockingChangeAuthenticationCollectionJob::TempBlockingChangeAuthenticationCollectionJob(BackendCollection* coll, const Peer& peer)
    : ChangeAuthenticationCollectionJob(coll, peer)
{
}

void TempBlockingChangeAuthenticationCollectionJob::perform()
{
    setError(BackendErrorNotSupported);
    setResult(false);
    emitResult();
}

TempBlockingCreateItemJob::TempBlockingCreateItemJob(const ItemCreateInfo& createInfo,
        TempBlockingCollection* collection)
    : CreateItemJob(createInfo, collection),
      m_tempColl(collection)
{
}

void TempBlockingCreateItemJob::perform()
{
    BackendReturn<BackendItem*> rc = m_tempColl->createItem(label(), attributes(),
                                     secret(), contentType(), replace(), locked() );

    if(rc.isError()) {
        setError(rc.error(), rc.errorMessage());
    } else {
        setItem(rc.value());
    }
    emitResult();
}

TempBlockingUnlockItemJob::TempBlockingUnlockItemJob(const ItemUnlockInfo& unlockInfo)
    : UnlockItemJob(unlockInfo)
{
}

void TempBlockingUnlockItemJob::perform()
{
    setResult(true);
    emitResult();
}

TempBlockingLockItemJob::TempBlockingLockItemJob(BackendItem* item)
    : LockItemJob(item)
{
}

void TempBlockingLockItemJob::perform()
{
    setError(BackendErrorNotSupported);
    setResult(false);
    emitResult();
}

TempBlockingDeleteItemJob::TempBlockingDeleteItemJob(const ItemDeleteInfo &deleteInfo)
    : DeleteItemJob(deleteInfo)
{
}

void TempBlockingDeleteItemJob::perform()
{
    setResult(true);
    item()->deleteLater();
    emitResult();
}

TempBlockingChangeAuthenticationItemJob::TempBlockingChangeAuthenticationItemJob(BackendItem* item)
    : ChangeAuthenticationItemJob(item)
{
}

void TempBlockingChangeAuthenticationItemJob::perform()
{
    setError(BackendErrorNotSupported);
    setResult(false);
    emitResult();
}

#include "tempblockingjobs.moc"
