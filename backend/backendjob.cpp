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

#include "backendjob.h"
#include "backendmaster.h"
#include "backendcollectionmanager.h"
#include "backendcollection.h"
#include "backenditem.h"

#include <klocalizedstring.h>

BackendJob::BackendJob(JobType type, JobQueue *queue)
    : QueuedJob(queue), m_type(type), m_dismissed(false), m_error(NoError)
{
}

bool BackendJob::isImmediate() const
{
    return true;
}

BackendJob::JobType BackendJob::type() const
{
    return m_type;
}

bool BackendJob::isDismissed() const
{
    return m_dismissed;
}

void BackendJob::dismiss()
{
    m_dismissed = true;
    deleteLater();
    emitResult();
}

ErrorType BackendJob::error() const
{
    return m_error;
}

const QString &BackendJob::errorMessage() const
{
    return m_errorMessage;
}

void BackendJob::start()
{
    exec();
}

void BackendJob::setError(ErrorType error, const QString &errorMessage)
{
    m_error = error;
    m_errorMessage = errorMessage;
}

CreateCollectionJob::CreateCollectionJob(const CollectionCreateInfo &createCollectionInfo,
        BackendCollectionManager *manager)
    : BackendJob(BackendJob::TypeCreateCollection, manager),
      m_createCollectionInfo(createCollectionInfo),
      m_manager(manager), m_collection(0)
{
}

const QString &CreateCollectionJob::label() const
{
    return m_createCollectionInfo.m_label;
}

bool CreateCollectionJob::locked() const
{
    return m_createCollectionInfo.m_locked;
}

BackendCollectionManager *CreateCollectionJob::manager() const
{
    return m_manager;
}

void CreateCollectionJob::setCollection(BackendCollection *collection)
{
    m_collection = collection;
}

CreateCollectionMasterJob::CreateCollectionMasterJob(const CollectionCreateInfo& createCollectionInfo,
        BackendMaster *master)
    : BackendJob(BackendJob::TypeCreateCollectionMaster, master),
      m_createCollectionInfo(createCollectionInfo),
      m_master(master), m_subJob(0), m_collection(0)
{
}

CreateCollectionMasterJob::~CreateCollectionMasterJob()
{
    // FIXME: do something about a possible subJob!
}

bool CreateCollectionMasterJob::isImmediate() const
{
    if(m_master->managers().count() == 0) {
        // no manager => job fails
        return true;
    } else {
        // It's safe to assume that the call is not immediate if there are backends
        // available, as only the most basic ones are synchronous.
        return false;
    }
}

void CreateCollectionMasterJob::exec()
{
    if(m_master->managers().count() == 0) {
        setError(ErrorOther, i18n("No backend to create the collection was found."));
        emitResult();
    } else {
        // can't call synchronous if there's a backend.
        Q_ASSERT(false);
    }
}

void CreateCollectionMasterJob::start()
{
    if(m_master->managers().count() == 0) {
        setError(ErrorOther, i18n("No backend to create the collection was found."));
        emitResult();
    } else if(m_master->managers().count() == 1) {
        createSubJob(m_master->managers().first());
        connect(m_subJob, SIGNAL(result(QueuedJob*)),
                SLOT(createCollectionJobFinished(QueuedJob*)));
        m_subJob->enqueue();
    } else {
        setError(ErrorOther, i18n("Not implemented."));
        emitResult();
    }
}

BackendCollection *CreateCollectionMasterJob::collection() const
{
    return m_collection;
}

void CreateCollectionMasterJob::createCollectionJobFinished(QueuedJob *subJob)
{
    // copy the subjob's result
    Q_ASSERT(m_subJob == subJob);
    CreateCollectionJob *ccj = qobject_cast<CreateCollectionJob*>(subJob);
    Q_ASSERT(ccj);
    if(ccj->error() != NoError) {
        setError(ccj->error(), ccj->errorMessage());
    } else {
        m_collection = ccj->collection();
    }
    emitResult();
}

void CreateCollectionMasterJob::createSubJob(BackendCollectionManager *manager)
{
    Q_ASSERT(!m_subJob);
    m_subJob = manager->createCreateCollectionJob(m_createCollectionInfo);
}

BooleanResultJob::BooleanResultJob(BackendJob::JobType type, JobQueue* queue)
    : BackendJob(type, queue)
{
}

bool BooleanResultJob::result() const
{
    return m_result;
}

void BooleanResultJob::setResult(bool result)
{
    m_result = result;
}

LockCollectionJob::LockCollectionJob(BackendCollection *collection)
    : BooleanResultJob(BackendJob::TypeLockCollection, collection),
      m_collection(collection)
{
}

BackendCollection *LockCollectionJob::collection()
{
    return m_collection;
}

LockItemJob::LockItemJob(BackendItem *item)
    : BooleanResultJob(BackendJob::TypeLockItem, item), m_item(item)
{
}

BackendItem *LockItemJob::item()
{
    return m_item;
}

UnlockCollectionJob::UnlockCollectionJob(const CollectionUnlockInfo &unlockInfo, BackendCollection *collection)
    : BooleanResultJob(BackendJob::TypeUnlockCollection, collection),
      m_collection(collection), m_unlockInfo(unlockInfo)
{
}

UnlockItemJob::UnlockItemJob(const ItemUnlockInfo& unlockInfo)
    : BooleanResultJob(BackendJob::TypeUnlockItem, unlockInfo.m_item), m_unlockInfo(unlockInfo)
{
}

BackendItem *UnlockItemJob::item()
{
    return m_unlockInfo.m_item;
}

DeleteCollectionJob::DeleteCollectionJob(const CollectionDeleteInfo& deleteInfo)
    : BooleanResultJob(BackendJob::TypeDeleteCollection, deleteInfo.m_collection),
      m_deleteInfo(deleteInfo)
{
}

BackendCollection *DeleteCollectionJob::collection()
{
    return m_deleteInfo.m_collection;
}

DeleteItemJob::DeleteItemJob(const ItemDeleteInfo& deleteInfo)
    : BooleanResultJob(BackendJob::TypeDeleteItem, deleteInfo.m_item), m_deleteInfo(deleteInfo)
{
}

BackendItem *DeleteItemJob::item()
{
    return m_deleteInfo.m_item;
}

ChangeAuthenticationCollectionJob::ChangeAuthenticationCollectionJob(BackendCollection* collection)
    : BooleanResultJob(BackendJob::TypeChangeAuthenticationCollection, collection),
      m_collection(collection)
{
}

BackendCollection *ChangeAuthenticationCollectionJob::collection()
{
    return m_collection;
}

ChangeAuthenticationItemJob::ChangeAuthenticationItemJob(BackendItem *item)
    : BooleanResultJob(BackendJob::TypeChangeAuthenticationItem, item), m_item(item)
{
}

BackendItem *ChangeAuthenticationItemJob::item()
{
    return m_item;
}

CreateItemJob::CreateItemJob(const ItemCreateInfo& createInfo, BackendCollection *collection)
    : BackendJob(BackendJob::TypeCreateItem, collection), m_collection(collection),
      m_createInfo(createInfo), m_item(0)
{
}

BackendItem *CreateItemJob::item() const
{
    return m_item;
}

const QString &CreateItemJob::label() const
{
    return m_createInfo.m_label;
}

const QMap<QString, QString> &CreateItemJob::attributes() const
{
    return m_createInfo.m_attributes;
}

const QCA::SecureArray &CreateItemJob::secret() const
{
    return m_createInfo.m_secret;
}

bool CreateItemJob::locked() const
{
    return m_createInfo.m_locked;
}

bool CreateItemJob::replace() const
{
    return m_createInfo.m_replace;
}

BackendCollection *CreateItemJob::collection()
{
    return m_collection;
}

void CreateItemJob::setItem(BackendItem *item)
{
    m_item = item;
}

#include "backendjob.moc"
