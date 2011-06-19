/*
 * Copyright 2010, Dario Freddi <dario.freddi@collabora.co.uk>
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


#include "kwlimporterjob.h"

#include "importsinglewalletjob.h"

#include <KDebug>
#include <kuiserverjobtracker.h>
#include <KLocalizedString>

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusPendingReply>

#include <backend/backendmaster.h>
#include <backend/backendcollectionmanager.h>
#include <backend/ksecret/ksecretcollectionmanager.h>

#include <frontend/secret/collection.h>
#include <frontend/secret/service.h>

#include <ui/nouimanager.h>
#include <KStandardDirs>
#include <QDir>


KwlImporterJob::KwlImporterJob(Service *service, QObject* parent)
    : KCompositeJob(parent),
    _service( service )
{
    Q_ASSERT( _service != 0 );
    kDebug();

    setCapabilities(KJob::Killable | KJob::Suspendable);

    _jobTracker = new KUiServerJobTracker(this);
    _jobTracker->registerJob(this);
}

KwlImporterJob::~KwlImporterJob()
{

}

void KwlImporterJob::start()
{
    kDebug();

    emit description(this,
                     i18nc("@title job", "Converting Password storage"),
                     qMakePair(i18n("Old backend"), i18n("KWallet Files")),
                     qMakePair(i18n("New backend"), i18n("KSecretService storage")));

    QMetaObject::invokeMethod(this, "run", Qt::QueuedConnection);
}

void KwlImporterJob::run()
{
    if ( !userHasWallets() ) {
        kDebug() << "No wallets are found into the user directory";
        return;
    }

    // Obtain the collections labels
    QStringList collectionLabels;
    foreach(const QDBusObjectPath & path, _service->collections()) {
        QDBusInterface ifaceCollection("org.freedesktop.\bDaemonSecret\b", path.path(),
                                       "org.freedesktop.\bDaemonSecret\b.Collection");
        if(ifaceCollection.isValid()) {
            QDBusPendingReply< QString > reply = ifaceCollection.asyncCall("Label");
            reply.waitForFinished();

            if(!reply.isError()) {
                collectionLabels << reply;
            }
        }
    }

    // Now look for existing wallets, check if there is not a collection with the same label, and start the conversion job(s)
    QDir dir(KGlobal::dirs()->saveLocation("data", "kwallet", false), "*.kwl");
    QStringList wallets;

    dir.setFilter(QDir::Files | QDir::Hidden);

    foreach(const QFileInfo & fi, dir.entryInfoList()) {
        QString fn = fi.fileName();
        if(fn.endsWith(QLatin1String(".kwl"))) {
            fn.truncate(fn.length() - 4);
        }
        wallets += fn;
    }

    foreach(const QString & walletName, wallets) {
        if(!collectionLabels.contains(walletName)) {
            KJob *job = new ImportSingleWalletJob(_service, walletName, this);
            addSubjob(job);
            job->start();
        }
    }
}

bool KwlImporterJob::userHasWallets() 
{
    QDir dir(KGlobal::dirs()->saveLocation("data", "kwallet", false), "*.kwl");
    QStringList wallets;

    dir.setFilter(QDir::Files | QDir::Hidden);

    return dir.entryInfoList().count() >0;
}

void KwlImporterJob::slotResult(KJob* job)
{
    // Did job have an error ?
    if(job->error() && !error()) {
        // Store it in the parent only if first error
        setError(job->error());
        setErrorText(job->errorText());

        removeSubjob(job);
        // Kill all the other subjobs
        foreach(KJob * job, subjobs()) {
            job->kill();
        }

        emitResult();
        return;
    }

    removeSubjob(job);

    // Any jobs left?
    if(!hasSubjobs()) {
        // Finished!
        emitResult();
    }
}

bool KwlImporterJob::doKill()
{
    // Kill all the subjobs
    foreach(KJob * job, subjobs()) {
        if(!job->kill()) {
            return false;
        }
    }

    return true;
}

bool KwlImporterJob::doResume()
{
    // Resume all the subjobs
    foreach(KJob * job, subjobs()) {
        if(!job->resume()) {
            return false;
        }
    }

    return true;
}

bool KwlImporterJob::doSuspend()
{
    // Suspend all the subjobs
    foreach(KJob * job, subjobs()) {
        if(!job->suspend()) {
            return false;
        }
    }

    return true;
}

#include "kwlimporterjob.moc"
