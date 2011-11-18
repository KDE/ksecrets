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

#include "prompt.h"
#include "service.h"
#include "adaptors/promptadaptor.h"

#include <backend/backendcollection.h>
#include <backend/backenditem.h>
#include "../lib/secrettool.h"

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusObjectPath>
#include <kmessagebox.h>
#include <kdebug.h>

PromptBase::PromptBase(Service *service, QObject *parent)
    : QObject(parent), m_serviceObjectPath(service->objectPath())
{
    Q_ASSERT(service);
    m_objectPath.setPath(service->objectPath().path() + "/prompts/" + createId());

    new orgFreedesktopSecret::PromptAdaptor(this);
    QDBusConnection::sessionBus().registerObject(m_objectPath.path(), this);
}

PromptBase::~PromptBase()
{
}

const QDBusObjectPath &PromptBase::objectPath() const
{
    return m_objectPath;
}

const QDBusObjectPath &PromptBase::serviceObjectPath() const
{
    return m_serviceObjectPath;
}

void PromptBase::emitCompleted(bool dismissed, const QVariant &result)
{
    emit completed(dismissed, result);
}

SingleJobPrompt::SingleJobPrompt(Service* service, BackendJob* job, QObject* parent)
    : PromptBase(service, parent), m_prompted(false), m_job(job)
{
    connect(job, SIGNAL(result(KJob*)), SLOT(jobResult(KJob*)));
}

SingleJobPrompt::~SingleJobPrompt()
{
    // TODO: delete the job?
}

void SingleJobPrompt::prompt(const QString &windowId)
{
    Q_UNUSED(windowId);
    if(m_prompted) {
        return;
    }
    m_prompted = true;

    // TODO: convert windowId to a WId and pass it to the job
    m_job->start();
}

void SingleJobPrompt::dismiss()
{
    if(m_prompted) {
        return;
    }
    m_prompted = true;

    m_job->dismiss();
}

void SingleJobPrompt::jobResult(KJob *job)
{
    Q_ASSERT(job == m_job);
    // check for errors first
    if(m_job->isDismissed()) {
        emit completed(true, QVariant(""));
    } else if(m_job->error() != BackendNoError) {
        // TODO: figure out how to handle errors gracefully.
        // FIXME; should we use KMessage here instead of KMessageBox ?
        KMessageBox::error( 0, m_job->errorMessage() );
        emit completed(false, QVariant(""));
    } else {
        switch(m_job->type()) {

        case BackendJob::TypeUnlockCollection:
        case BackendJob::TypeLockCollection:
        case BackendJob::TypeDeleteCollection:
        case BackendJob::TypeChangeAuthenticationCollection:
        case BackendJob::TypeDeleteItem:
        case BackendJob::TypeLockItem:
        case BackendJob::TypeUnlockItem:
        case BackendJob::TypeChangeAuthenticationItem: {
            BooleanResultJob *brj = qobject_cast<BooleanResultJob*>(m_job);
            Q_ASSERT(brj);
            emitCompleted(false, QVariant(brj->result()));
        }
        break;

        case BackendJob::TypeCreateCollectionMaster: {
            CreateCollectionMasterJob *ccmj = qobject_cast<CreateCollectionMasterJob*>(m_job);
            Q_ASSERT(ccmj);
            QDBusObjectPath result("/");
            if(ccmj->collection()) {
                BackendCollection *coll = ccmj->collection();
                result.setPath(serviceObjectPath().path() + "/collection/" + coll->id());
            }

            emitCompleted(false, qVariantFromValue(result));
        }
        break;

        case BackendJob::TypeCreateCollection: {
            CreateCollectionJob *ccj = qobject_cast<CreateCollectionJob*>(m_job);
            Q_ASSERT(ccj);
            QDBusObjectPath result("/");
            if(ccj->collection()) {
                BackendCollection *coll = ccj->collection();
                result.setPath(serviceObjectPath().path() + "/collection/" + coll->id());
            }

            emitCompleted(false, qVariantFromValue(result));
        }
        break;

        case BackendJob::TypeCreateItem: {
            CreateItemJob *cij = qobject_cast<CreateItemJob*>(m_job);
            Q_ASSERT(cij);
            QDBusObjectPath result("/");
            if(cij->item()) {
                BackendItem *item = cij->item();
                result.setPath(serviceObjectPath().path() + "/collection/" +
                               cij->collection()->id() + '/' + item->id());
            }

            emitCompleted(false, qVariantFromValue(result));
        }
        break;

        default:
            // should not happen!
            Q_ASSERT(false);
        }
    }

    deleteLater();
}

ServiceMultiPrompt::ServiceMultiPrompt(Service *service, const QSet<BackendJob*> jobs,
                                       QObject *parent)
    : PromptBase(service, parent), m_prompted(false), m_jobs(jobs)
{
    bool jobTypeDetermined = false;
    bool jobTypeUnlock = false;

    Q_FOREACH(BackendJob * job, m_jobs) {
        // make sure the subjobs are either all unlock or all lock calls
        bool currentJobTypeUnlock = (job->type() == BackendJob::TypeUnlockCollection ||
                                     job->type() == BackendJob::TypeUnlockItem);
        if(jobTypeDetermined && jobTypeUnlock != currentJobTypeUnlock) {
            Q_ASSERT(false);
        } else if(!jobTypeDetermined) {
            jobTypeUnlock = currentJobTypeUnlock;
            jobTypeDetermined = true;
        }
        connect(job, SIGNAL(result(KJob*)), SLOT(jobResult(KJob*)));
    }
}

ServiceMultiPrompt::~ServiceMultiPrompt()
{
    // TODO: delete jobs if not started?
}

void ServiceMultiPrompt::prompt(const QString &windowId)
{
    Q_UNUSED(windowId);
    if(m_prompted) {
        return;
    }
    m_prompted = true;

    // NOTE: this job will autodelete itself when done or dismissed
    BackendJob *mainJob = new BackendJob( BackendJob::TypeMultiPrompt );
    
    // TODO: convert windowId to a WId and pass it to the job
    Q_FOREACH(BackendJob * job, m_jobs) {
        if (!mainJob->addSubjob(job)) {
            emitCompleted(true, qVariantFromValue(m_result));
            break;
        }
        else {
            job->start();
        }
    }
}

void ServiceMultiPrompt::dismiss()
{
    if(m_prompted) {
        return;
    }
    m_prompted = true;

    Q_FOREACH(BackendJob * job, m_jobs) {
        disconnect(job, SIGNAL(result(KJob*)), this, SLOT(jobResult(KJob*)));
        job->dismiss();
    }

    // emit result right away so we don't have to catch all result() signals
    // from individual jobs.
    emitCompleted(true, qVariantFromValue(QList<QDBusObjectPath>()));
    deleteLater();
}

void ServiceMultiPrompt::jobResult(KJob *job)
{
    BackendJob *bj = qobject_cast<BackendJob*>(job);
    Q_ASSERT(bj);
    Q_ASSERT(m_jobs.contains(bj));
    // remove job from the set of jobs we're waiting for
    m_jobs.remove(bj);
    if(bj->error() != BackendNoError) {
        // TODO: figure out what to do with erroneous jobs
    } else {
        switch(bj->type()) {

        case BackendJob::TypeUnlockCollection: {
            UnlockCollectionJob *ucj = qobject_cast<UnlockCollectionJob*>(bj);
            Q_ASSERT(ucj);
            if(ucj->result()) {
                BackendCollection *coll = ucj->collection();
                QDBusObjectPath path;
                path.setPath(serviceObjectPath().path() + "/collection/" + coll->id());
                m_result.append(path);
                // FIXME: find out why on my 64 bit system the m_result contains one empty string despite path is non-empty
            }
        }
        break;

        case BackendJob::TypeLockCollection: {
            LockCollectionJob *lcj = qobject_cast<LockCollectionJob*>(bj);
            Q_ASSERT(lcj);
            if(lcj->result()) {
                BackendCollection *coll = lcj->collection();
                QDBusObjectPath path;
                path.setPath(serviceObjectPath().path() + "/collection/" + coll->id());
                m_result.append(path);
            }
        }
        break;

        case BackendJob::TypeUnlockItem: {
            UnlockItemJob *uij = qobject_cast<UnlockItemJob*>(bj);
            Q_ASSERT(uij);
            if(uij->result()) {
                BackendItem *item = uij->item();
                BackendCollection *coll = 0;
                // TODO: I NEED THE COLLECTION DAMMIT
                QDBusObjectPath path;
                path.setPath(serviceObjectPath().path() + "/collection/" + coll->id() +
                             '/' + item->id());
                m_result.append(path);
            }
        }
        break;

        case BackendJob::TypeLockItem: {
            LockItemJob *lij = qobject_cast<LockItemJob*>(bj);
            Q_ASSERT(lij);
            if(lij->result()) {
                BackendItem *item = lij->item();
                BackendCollection *coll = 0;
                // TODO: I NEED THE COLLECTION DAMMIT
                QDBusObjectPath path;
                path.setPath(serviceObjectPath().path() + "/collection/" + coll->id() +
                             '/' + item->id());
                m_result.append(path);
            }
        }
        break;

        default:
            Q_ASSERT(false);
        }
    }

    if(m_jobs.isEmpty()) {
        // all jobs finished
        QVariant varResult = qVariantFromValue( m_result );
        emitCompleted( false, varResult );
        deleteLater();
    }
}

#include "prompt.moc"
