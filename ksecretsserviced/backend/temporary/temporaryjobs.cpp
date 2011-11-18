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

#include "temporaryjobs.h"

#include "../lib/secrettool.h"

TemporaryCreateCollectionJob::TemporaryCreateCollectionJob(const CollectionCreateInfo &createCollectionInfo,
        TemporaryCollectionManager *manager)
    : CreateCollectionJob(createCollectionInfo, manager)
{
}

void TemporaryCreateCollectionJob::start()
{
    TemporaryCollection *coll = new TemporaryCollection(createId(), manager());
    coll->setLabel(label());

    setCollection(coll);
    emitResult();
}

TemporaryUnlockCollectionJob::TemporaryUnlockCollectionJob(const CollectionUnlockInfo &unlockInfo,
        BackendCollection *coll)
    : UnlockCollectionJob(unlockInfo, coll)
{
}

void TemporaryUnlockCollectionJob::start()
{
    // collection is always unlocked.
    setResult(true);
    emitResult();
}

TemporaryLockCollectionJob::TemporaryLockCollectionJob(BackendCollection *coll)
    : LockCollectionJob(coll)
{
}

void TemporaryLockCollectionJob::start()
{
    // collection can not be locked as that's not supported for this backend.
    setError(BackendErrorNotSupported);
    setResult(false);
    emitResult();
}

TemporaryDeleteCollectionJob::TemporaryDeleteCollectionJob(const CollectionDeleteInfo& deleteInfo)
    : DeleteCollectionJob(deleteInfo)
{
}

void TemporaryDeleteCollectionJob::start()
{
    setResult(true);
    collection()->deleteLater();
    emitResult();
}

TemporaryChangeAuthenticationCollectionJob::TemporaryChangeAuthenticationCollectionJob(BackendCollection *coll, const Peer& peer)
    : ChangeAuthenticationCollectionJob(coll, peer)
{
}

void TemporaryChangeAuthenticationCollectionJob::start()
{
    setError(BackendErrorNotSupported);
    setResult(false);
    emitResult();
}

TemporaryCreateItemJob::TemporaryCreateItemJob(const ItemCreateInfo& createInfo,
        TemporaryCollection *collection)
    : CreateItemJob(createInfo, collection),
      m_tempColl(collection)
{
}

void TemporaryCreateItemJob::start()
{
    // let the collection do all the work.
    BackendReturn<BackendItem*> rc = m_tempColl->createItem(label(), attributes(),
                                     secret(), contentType(), replace(), locked());
    if(rc.isError()) {
        setError(rc.error(), rc.errorMessage());
    } else {
        setItem(rc.value());
    }
    emitResult();
}

TemporaryUnlockItemJob::TemporaryUnlockItemJob(const ItemUnlockInfo& unlockInfo) : UnlockItemJob(unlockInfo)
{
}

void TemporaryUnlockItemJob::start()
{
    // item is always unlocked.
    setResult(true);
    emitResult();
}

TemporaryLockItemJob::TemporaryLockItemJob(BackendItem *item) : LockItemJob(item)
{
}

void TemporaryLockItemJob::start()
{
    // item can not be locked as that's not supported for this backend.
    setError(BackendErrorNotSupported);
    setResult(false);
    emitResult();
}

TemporaryDeleteItemJob::TemporaryDeleteItemJob(const ItemDeleteInfo& deleteInfo) : DeleteItemJob(deleteInfo)
{
}

void TemporaryDeleteItemJob::start()
{
    setResult(true);
    item()->deleteLater();
    emitResult();
}

TemporaryChangeAuthenticationItemJob::TemporaryChangeAuthenticationItemJob(BackendItem *item)
    : ChangeAuthenticationItemJob(item)
{
}

void TemporaryChangeAuthenticationItemJob::start()
{
    setError(BackendErrorNotSupported);
    setResult(false);
    emitResult();
}

#include "temporaryjobs.moc"
