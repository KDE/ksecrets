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

#include "ksecretjobs.h"

#include <backend/backendmaster.h>
#include <ui/abstractuimanager.h>
#include <secrettool.h>
#include <backendcollection.h>

#include <QtCore/QTimer>
#include <klocalizedstring.h>

#include <QtCore/QDebug>

KSecretCreateCollectionJob::KSecretCreateCollectionJob(const CollectionCreateInfo &createCollectionInfo,
        KSecretCollectionManager *manager)
    : CreateCollectionJob(createCollectionInfo, manager), m_manager(manager)
{
}

bool KSecretCreateCollectionJob::isImmediate() const
{
    // TODO: depending on available authentiation methods this may return
    //       true.
    return false;
}

void KSecretCreateCollectionJob::exec()
{
    Q_ASSERT(false);
}

void KSecretCreateCollectionJob::start()
{
    // start a job for getting a new password for the collection from the user.
    AbstractUiManager *uiManager = BackendMaster::instance()->uiManager();
    AbstractNewPasswordJob *subJob = uiManager->createNewPasswordJob(label());
    connect(subJob, SIGNAL(result(QueuedJob*)),
            SLOT(newPasswordJobResult(QueuedJob*)));
    subJob->enqueue();

    // then create a job asking user preferences for handling application permission
    AbstractAskAclPrefsJob *aclSubjob = uiManager->createAskAclPrefsJob(createCollectionInfo());
    connect(aclSubjob, SIGNAL(result(QueuedJob*)),
            SLOT(askAclPrefsJobResult(QueuedJob*)));
    aclSubjob->enqueue();
}

void KSecretCreateCollectionJob::newPasswordJobResult(QueuedJob *job)
{
    AbstractNewPasswordJob *npj = qobject_cast<AbstractNewPasswordJob*>(job);
    Q_ASSERT(npj);

    if(npj->cancelled()) {
        setCollection(0);
        setError(ErrorOther, i18n("Creating the collection was cancelled by the user."));
        dismiss(); // this will also emitResult()
        return;
    }

    // TODO: collection needs authentication methods, filenames, ...
    QString errorMessage;
    KSecretCollection *coll = KSecretCollection::create(createId(), npj->password(),
                              manager(), errorMessage);
    if(!coll) {
        setCollection(0);
        setError(ErrorOther, errorMessage);
        dismiss(); // this will also emitResult()
        return;
    }
    coll->setLabel(label());

    // NOTE: coll has to be added to m_collections before serializing it for the
    //       first time, so when slotDirectoryChanged is called, the collection
    //       is already known.
    m_manager->addCollection(coll);

    if(locked()) {
        coll->lock();
    }

    setCollection(coll);
    // not yet time for  emitResult(); 
    // let the ACL dialog to pop before 
}

void KSecretCreateCollectionJob::askAclPrefsJobResult(QueuedJob* job)
{
    AbstractAskAclPrefsJob *aclJob = qobject_cast< AbstractAskAclPrefsJob* >(job);
    Q_ASSERT(aclJob);

    if (!collection()->setApplicationPermission(
        createCollectionInfo().m_peer.exePath(),
        aclJob->permission() ) ) {
            setError(ErrorAclSetPermission, i18n("Cannot store application ACL policy into the back-end.") );
            // FIXME: should we remove the freshly created collection ?
            emitResult();
    }
    collection()->setCreatorApplication( createCollectionInfo().m_peer.exePath() );
    
    emitResult();
}


KSecretUnlockCollectionJob::KSecretUnlockCollectionJob(const CollectionUnlockInfo &unlockInfo,
        KSecretCollection *coll)
    : UnlockCollectionJob(unlockInfo, coll), m_firstTry(true)
{
}

bool KSecretUnlockCollectionJob::isImmediate() const
{
    // unlocked collections don't need a job to open
    if(!collection()->isLocked()) {
        return true;
    } else {
        return false;
    }
}

void KSecretUnlockCollectionJob::exec()
{
    Q_ASSERT(!collection()->isLocked());
    setResult(true);
    emitResult();
}

void KSecretUnlockCollectionJob::start()
{
    if(!collection()->isLocked()) {
        setResult(true);
        emitResult();
        return;
    }

    // default is always to ask the user
    if(unlockInfo().m_peer.isValid()) {
        m_collectionPerm = collection()->applicationPermission(unlockInfo().m_peer.exePath());
        switch(m_collectionPerm) {
        case PermissionDeny:
            // the application was explicitly denied access by the user in the past
            setResult(false);
            emitResult();
            break;
        case PermissionAsk:
            // the permission found was set to "ask each time for the password"
            createAskPasswordJob();
            break;
        case PermissionAllow: {
                KSecretCollection *ksecretColl = dynamic_cast< KSecretCollection* >(collection());
                if ( !ksecretColl->tryUnlock() ) {
                    createAskPasswordJob();
                }
                else {
                    setResult(true);
                    emitResult();
                }
            }
            break;
        case PermissionUndefined:
            // this is the first time the calling application tries to open this collection
            // first, prompt for the collection password, then ask for ACL preferences
            createAskPasswordJob();
            break;
        default:
            Q_ASSERT(0); // unknown case detected !!
            setResult(false);
            emitResult();
        }
    }
    else {
        setResult(false); // FIXME: should we give a message here or let it silently deny access ?
        emitResult();
    }
}

void KSecretUnlockCollectionJob::createAskPasswordJob()
{
    AbstractUiManager *uiManager = BackendMaster::instance()->uiManager();
    // start a job for getting the password for the collection from the user.
    AbstractAskPasswordJob *subJob = uiManager->createAskPasswordJob(collection()->label().value(),
                                     !m_firstTry);
    connect(subJob, SIGNAL(result(QueuedJob*)), SLOT(askPasswordJobResult(QueuedJob*)));

    // if this is not the first try, enqueue in front so the dialog gets shown again
    // right away.
    subJob->enqueue(m_firstTry);
    m_firstTry = false;
}

void KSecretUnlockCollectionJob::askAclPrefsJobResult(QueuedJob *job)
{
    AbstractAskAclPrefsJob *apj = qobject_cast<AbstractAskAclPrefsJob*>(job);
    Q_ASSERT(apj);
    if(apj->denied()) {
        setResult(false);
        setError(ErrorOther, i18n("Unlocking the collection was denied."));
        emitResult();
    } else 
    if (apj->cancelled() ) {
        setResult(false);
        setError(ErrorOther, i18n("Unlocking the collection was canceled by the user."));
        emitResult();
    }
    else {
        // now that the access to this collection is allowed, store user choice and go further and ask the password
        if ( !collection()->setApplicationPermission( 
            unlockInfo().m_peer.exePath(),
            apj->permission() ) )
        {
            setResult(false);
            setError(ErrorAclSetPermission, i18n("Cannot store application ACL policy into the back-end.") );
            emitResult();
        }
        else {
            createAskPasswordJob();
        }
    }
}

void KSecretUnlockCollectionJob::askPasswordJobResult(QueuedJob *job)
{
    AbstractAskPasswordJob *apj = qobject_cast<AbstractAskPasswordJob*>(job);
    Q_ASSERT(apj);

    if(apj->cancelled()) {
        setResult(false);
        setError(ErrorOther, i18n("Unlocking the collection was canceled by the user."));
        emitResult();
        return;
    }

    KSecretCollection *ksecretColl = dynamic_cast< KSecretCollection* >(collection());
    BackendReturn<bool> rc = ksecretColl->tryUnlockPassword(apj->password());
    if(rc.isError()) {
        setResult(false);
        setError(rc.error(), rc.errorMessage());
        emitResult();
    } else if(!rc.value()) {
        // try again the password
        createAskPasswordJob();
    } else {
        if ( m_collectionPerm == PermissionUndefined ) {
            // ask for the ACL preference if the application is unknown by this collection
            AbstractUiManager *uiManager = BackendMaster::instance()->uiManager();
            AbstractAskAclPrefsJob* askAclPrefsJob = uiManager->createAskAclPrefsJob(unlockInfo());
            connect(askAclPrefsJob, SIGNAL(result(QueuedJob*)), SLOT(askAclPrefsJobResult(QueuedJob*)));
            askAclPrefsJob->enqueue();        
        }
        else {
            setResult(true);
            emitResult();
        }
    }
}

KSecretLockCollectionJob::KSecretLockCollectionJob(KSecretCollection *coll)
    : LockCollectionJob(coll), m_collection(coll)
{
}

void KSecretLockCollectionJob::exec()
{
    // nothing to do if already locked
    if(collection()->isLocked()) {
        setResult(true);
        emitResult();
        return;
    }

    BackendReturn<bool> rc = m_collection->lock();
    if(rc.isError()) {
        setError(rc.error(), rc.errorMessage());
        setResult(false);
    } else {
        setResult(true);
    }
    emitResult();
}

KSecretDeleteCollectionJob::KSecretDeleteCollectionJob(const CollectionDeleteInfo& deleteInfo)
    : DeleteCollectionJob(deleteInfo)
{
    Q_ASSERT(deleteInfo.m_collection != 0);
    m_collection = qobject_cast< KSecretCollection* >(deleteInfo.m_collection);
}

void KSecretDeleteCollectionJob::exec()
{
    BackendReturn<bool> rc = m_collection->deleteCollection();
    if(rc.isError()) {
        setError(rc.error(), rc.errorMessage());
        setResult(false);
    } else {
        setResult(true);
    }
    emitResult();
}

KSecretCreateItemJob::KSecretCreateItemJob(const ItemCreateInfo& createInfo,
        KSecretCollection* collection)
    : CreateItemJob(createInfo, collection),
      m_collection(collection)
{
}

void KSecretCreateItemJob::exec()
{
    BackendReturn<BackendItem*> rc = m_collection->createItem(label(), attributes(),
                                     secret(), locked(), replace());
    if(rc.isError()) {
        setError(rc.error(), rc.errorMessage());
        setItem(0);
    } else {
        setItem(rc.value());
    }
    emitResult();
}

KSecretChangeAuthenticationCollectionJob::KSecretChangeAuthenticationCollectionJob(BackendCollection *coll)
    : ChangeAuthenticationCollectionJob(coll)
{
}

void KSecretChangeAuthenticationCollectionJob::exec()
{
    setError(ErrorOther, "Not implemented.");
    setResult(false);
    emitResult();
}

KSecretUnlockItemJob::KSecretUnlockItemJob(const ItemUnlockInfo& unlockInfo,
        KSecretCollection *collection)
    : UnlockItemJob(unlockInfo)
{
    Q_ASSERT(collection != NULL);
    // delegate to an unlock job on the collection
    CollectionUnlockInfo collUnlockInfo(unlockInfo.m_peer);
    m_subJob = collection->createUnlockJob(collUnlockInfo);
}

KSecretUnlockItemJob::~KSecretUnlockItemJob()
{
    // delete the subjob if it wasn't executed.
    if(!m_subJob.isNull()) {
        m_subJob->deleteLater();
    }
}

bool KSecretUnlockItemJob::isImmediate() const
{
    Q_ASSERT(!m_subJob.isNull());
    return m_subJob->isImmediate();
}

void KSecretUnlockItemJob::exec()
{
    Q_ASSERT(!m_subJob.isNull());
    m_subJob->exec();
    handleSubJobResult(m_subJob);
}

void KSecretUnlockItemJob::start()
{
    connect(m_subJob, SIGNAL(result(QueuedJob*)),
            SLOT(handleSubJobResult(QueuedJob*)));
    m_subJob->enqueue();
}

void KSecretUnlockItemJob::handleSubJobResult(QueuedJob *job)
{
    Q_UNUSED(job);
    Q_ASSERT(!m_subJob.isNull());
    Q_ASSERT(m_subJob == job);
    Q_ASSERT(m_subJob->isFinished());
    setError(m_subJob->error(), m_subJob->errorMessage());
    setResult(m_subJob->result());
    emitResult();
}

KSecretLockItemJob::KSecretLockItemJob(KSecretItem *item,
                                       KSecretCollection *collection)
    : LockItemJob(item)
{
    // delegate to a lock job on the collection
    m_subJob = collection->createLockJob();
}

KSecretLockItemJob::~KSecretLockItemJob()
{
    // delete the subjob if it wasn't executed.
    if(!m_subJob.isNull()) {
        m_subJob->deleteLater();
    }
}

bool KSecretLockItemJob::isImmediate() const
{
    Q_ASSERT(!m_subJob.isNull());
    return m_subJob->isImmediate();
}

void KSecretLockItemJob::exec()
{
    Q_ASSERT(!m_subJob.isNull());
    m_subJob->exec();
    handleSubJobResult(m_subJob);
}

void KSecretLockItemJob::start()
{
    connect(m_subJob, SIGNAL(result(QueuedJob*)),
            SLOT(handleSubJobResult(QueuedJob*)));
    m_subJob->enqueue();
}

void KSecretLockItemJob::handleSubJobResult(QueuedJob *job)
{
    Q_UNUSED(job);
    Q_ASSERT(!m_subJob.isNull());
    Q_ASSERT(m_subJob == job);
    Q_ASSERT(m_subJob->isFinished());
    setError(m_subJob->error(), m_subJob->errorMessage());
    setResult(m_subJob->result());
    emitResult();
}

KSecretDeleteItemJob::KSecretDeleteItemJob(const ItemDeleteInfo &deleteInfo)
    : DeleteItemJob(deleteInfo)
{
    Q_ASSERT(deleteInfo.m_item != 0);
    m_item = qobject_cast< KSecretItem* >(deleteInfo.m_item);
}

void KSecretDeleteItemJob::exec()
{
    // TODO: this could easily be delegated to the collection
    BackendReturn<bool> rc = m_item->deleteItem();
    if(rc.isError()) {
        setError(rc.error(), rc.errorMessage());
        setResult(false);
    } else {
        setResult(true);
    }
    emitResult();
}

KSecretChangeAuthenticationItemJob::KSecretChangeAuthenticationItemJob(BackendItem *item)
    : ChangeAuthenticationItemJob(item)
{
}

void KSecretChangeAuthenticationItemJob::exec()
{
    setError(ErrorNotSupported);
    setResult(false);
    emitResult();
}


#include "ksecretjobs.moc"
