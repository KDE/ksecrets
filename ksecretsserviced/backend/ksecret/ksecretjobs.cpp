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

#include "ksecretjobs.h"

#include <backend/backendmaster.h>
#include <ui/abstractuimanager.h>
#include "../lib/secrettool.h"
#include <backendcollection.h>

#include <QtCore/QTimer>
#include <klocalizedstring.h>

#include <QtCore/QDebug>
#include <kdebug.h>

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

void KSecretCreateCollectionJob::start()
{
    // start a job for getting a new password for the collection from the user.
    AbstractUiManager *uiManager = BackendMaster::instance()->uiManager();
    AbstractNewPasswordJob *subJob = uiManager->createNewPasswordJob(label());
    if ( addSubjob( subJob ) ) {
        connect(subJob, SIGNAL(result(KJob*)),
                SLOT(newPasswordJobResult(KJob*)));
        subJob->start();
    }
    else {
        kDebug() << "Cannot start subJob";
        setErrorText( i18n("Cannot start password job") );
        emitResult();
    }
}

void KSecretCreateCollectionJob::newPasswordJobResult(KJob *job)
{
    AbstractNewPasswordJob *npj = qobject_cast<AbstractNewPasswordJob*>(job);
    Q_ASSERT(npj);

    if(npj->cancelled()) {
        setCollection(0);
        setError(BackendErrorOther, i18n("Creating the collection was cancelled by the user."));
        dismiss(); // this will also emitResult()
        return;
    }

    // TODO: collection needs authentication methods, filenames, ...
    QString errorMessage;
    QString collId = createId();
    manager()->creatingCollection( collId );
    KSecretCollection *coll = KSecretCollection::create(collId, npj->password(),
                              manager(), errorMessage);
    if(!coll) {
        setCollection(0);
        setError(BackendErrorOther, errorMessage);
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
    AbstractUiManager *uiManager = BackendMaster::instance()->uiManager();
    AbstractAskAclPrefsJob *aclSubjob = uiManager->createAskAclPrefsJob(createCollectionInfo());
    if ( addSubjob( aclSubjob ) ) {
        connect(aclSubjob, SIGNAL(result(KJob*)),
                SLOT(askAclPrefsJobResult(KJob*)));
        aclSubjob->start();
    }
    else {
        kDebug() << "Cannot add aclSubjob";
        setErrorText( i18n("Cannot launch ACL preferences job") );
        emitResult();
    }
}

void KSecretCreateCollectionJob::askAclPrefsJobResult(KJob* job)
{
    AbstractAskAclPrefsJob *aclJob = qobject_cast< AbstractAskAclPrefsJob* >(job);
    Q_ASSERT(aclJob);

    if (!collection()->setApplicationPermission(
        createCollectionInfo().m_peer.exePath(),
        aclJob->permission() ) ) {
            setError(BackendErrorAclSetPermission, i18n("Cannot store application ACL policy into the back-end.") );
            // FIXME: should we remove the freshly created collection ?
            emitResult();
    }
    collection()->setCreatorApplication( createCollectionInfo().m_peer.exePath() );
    
    emitResult();
}


KSecretUnlockCollectionJob::KSecretUnlockCollectionJob(const CollectionUnlockInfo &unlockInfo,
        KSecretCollection *coll)
    : UnlockCollectionJob(unlockInfo, coll), 
    m_firstTry(true), 
    m_passwordAsked(false), 
    m_collectionPerm(PermissionUndefined)
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
                if ( ksecretColl->tryUnlock().isError() ) {
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
    if ( !m_passwordAsked ) {
        AbstractUiManager *uiManager = BackendMaster::instance()->uiManager();
        // start a job for getting the password for the collection from the user.
        AbstractAskPasswordJob *subJob = uiManager->createAskPasswordJob(collection()->label().value(),
                                        !m_firstTry);
        if ( addSubjob( subJob ) ) {
            connect(subJob, SIGNAL(result(KJob*)), SLOT(askPasswordJobResult(KJob*)));

            // TODO: handle severaly retries
            subJob->start();
            m_firstTry = false;
        }
        else {
            kDebug() << "Cannot add subJob";
            setErrorText(i18n("Cannot launch asking password job"));
            emitResult();
        }
    }
}

void KSecretUnlockCollectionJob::askAclPrefsJobResult(KJob *job)
{
    AbstractAskAclPrefsJob *apj = qobject_cast<AbstractAskAclPrefsJob*>(job);
    Q_ASSERT(apj);
    if(apj->denied()) {
        setResult(false);
        setError(BackendErrorOther, i18n("Unlocking the collection was denied."));
        emitResult();
    } else 
    if (apj->cancelled() ) {
        setResult(false);
        setError(BackendErrorOther, i18n("Unlocking the collection was canceled by the user."));
        emitResult();
    }
    else {
        // now that the access to this collection is allowed, store user choice and go further and ask the password
        if ( !collection()->setApplicationPermission( 
            unlockInfo().m_peer.exePath(),
            apj->permission() ) )
        {
            setResult(false);
            setError(BackendErrorAclSetPermission, i18n("Cannot store application ACL policy into the back-end.") );
            emitResult();
        }
        else {
            if ( !m_passwordAsked ) {
                createAskPasswordJob();
            }
            else {
                setResult(true);
                emitResult();
            }
        }
    }
}

void KSecretUnlockCollectionJob::askPasswordJobResult(KJob *job)
{
    AbstractAskPasswordJob *apj = qobject_cast<AbstractAskPasswordJob*>(job);
    Q_ASSERT(apj);

    if(apj->cancelled()) {
        setResult(false);
        setError(BackendErrorOther, i18n("Unlocking the collection was canceled by the user."));
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
        m_passwordAsked = true;
    
        m_collectionPerm = collection()->applicationPermission( unlockInfo().m_peer.exePath() );
        if ( m_collectionPerm == PermissionUndefined ) {
            // ask for the ACL preference if the application is unknown by this collection
            AbstractUiManager *uiManager = BackendMaster::instance()->uiManager();
            AbstractAskAclPrefsJob* askAclPrefsJob = uiManager->createAskAclPrefsJob(unlockInfo());
            connect(askAclPrefsJob, SIGNAL(result(KJob*)), SLOT(askAclPrefsJobResult(KJob*)));
            if ( addSubjob( askAclPrefsJob ) ) {
                askAclPrefsJob->start();        
            }
            else {
                setResult(false);
                emitResult();
            }
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

void KSecretLockCollectionJob::start()
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

void KSecretDeleteCollectionJob::start()
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

void KSecretCreateItemJob::start()
{
    BackendReturn<BackendItem*> rc = m_collection->createItem(label(), attributes(),
                                     secret(), contentType(), replace(), locked());
    if(rc.isError()) {
        setError(rc.error(), rc.errorMessage());
        setItem(0);
    } else {
        setItem(rc.value());
    }
    emitResult();
}

KSecretChangeAuthenticationCollectionJob::KSecretChangeAuthenticationCollectionJob(BackendCollection *coll, const Peer& peer)
    : ChangeAuthenticationCollectionJob(coll, peer)
{
}

void KSecretChangeAuthenticationCollectionJob::start()
{
    CollectionUnlockInfo unlockInfo( peer() );
    UnlockCollectionJob *unlockJob = collection()->createUnlockJob( unlockInfo );
    connect( unlockJob, SIGNAL(finished(KJob*)), this, SLOT(slotUnlockResult(KJob*)) );
    if (addSubjob( unlockJob )) {
        unlockJob->start();
    }
    else {
        kDebug() << "Failed to add unlock subjob";
        setError(BackendErrorOther, i18n("Cannot start collection unlocking"));
    }
}

void KSecretChangeAuthenticationCollectionJob::slotUnlockResult(KJob* j)
{
    UnlockCollectionJob *unlockJob = qobject_cast< UnlockCollectionJob* >( j );
    Q_ASSERT( unlockJob != 0 );
    if (unlockJob->result()) {
        AbstractUiManager *uiManager = BackendMaster::instance()->uiManager();
        AbstractNewPasswordJob *newPasswordJob = uiManager->createNewPasswordJob( collection()->label().value() );
        if (addSubjob(newPasswordJob)) {
            connect(newPasswordJob, SIGNAL(finished(KJob*)), this, SLOT(slotNewPasswordFinished(KJob*)));
            newPasswordJob->start();
        }
        else {
            setError(BackendErrorOther, i18n("Cannot start subjob."));
            emitResult();
        }
    }
    else {
        setError(BackendErrorIsLocked, i18n("The collection cannot be unlocked."));
        emitResult();
    }
    unlockJob->deleteLater();
}

void KSecretChangeAuthenticationCollectionJob::slotNewPasswordFinished(KJob* job)
{
   AbstractNewPasswordJob *newPasswordJob = qobject_cast< AbstractNewPasswordJob* >(job);
   Q_ASSERT(newPasswordJob != 0);
   if ( !newPasswordJob->cancelled() ) {
       setError(BackendNoError);
   }
   else {
       setError(BackendErrorOther, i18n("Cannot change password"));
   }
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

void KSecretUnlockItemJob::start()
{
    connect(m_subJob, SIGNAL(result(KJob*)),
            SLOT(handleSubJobResult(KJob*)));
    if (addSubjob(m_subJob)) {
        m_subJob->start();
    }
    else {
        setError( BackendErrorOther );
        emitResult();
    }
}

void KSecretUnlockItemJob::handleSubJobResult(KJob *job)
{
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

void KSecretLockItemJob::start()
{
    if (addSubjob(m_subJob)) {
        connect(m_subJob, SIGNAL(result(KJob*)),
                SLOT(handleSubJobResult(KJob*)));
        m_subJob->start();
    }
    else {
        setError(BackendErrorOther);
        emitResult();
    }
}

void KSecretLockItemJob::handleSubJobResult(KJob *job)
{
    Q_UNUSED(job);
    Q_ASSERT(!m_subJob.isNull());
    Q_ASSERT(m_subJob == job);
//     Q_ASSERT(m_subJob->isFinished());
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

void KSecretDeleteItemJob::start()
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

void KSecretChangeAuthenticationItemJob::start()
{
    setError(BackendErrorNotSupported);
    setResult(false);
    emitResult();
}


#include "ksecretjobs.moc"
