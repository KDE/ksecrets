/*
 * Copyright 2010, Dario Freddi <dario.freddi@collabora.co.uk>
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


#include "kwlimporterjob.h"

#include "importsinglewalletjob.h"

#include <ksecretsservicecollection.h>
#include <ksecretsservicecollectionjobs.h>

#include <KDebug>
#include <kuiserverjobtracker.h>
#include <KLocalizedString>

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusPendingReply>

#include <KStandardDirs>
#include <QDir>

using namespace KSecretsService;

KwlImporterJob::KwlImporterJob( QObject* parent)
    : KCompositeJob(parent)
{
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
                     qMakePair(i18n("New backend"), i18n("KSecretsService storage")));

    QMetaObject::invokeMethod(this, "run", Qt::QueuedConnection);
}

void KwlImporterJob::run()
{
    if ( !userHasWallets() ) {
        kDebug() << "No wallets are found into the user directory";
        return;
    }

    // Now look for existing wallets, check if there is not a collection with the same label, and start the conversion job(s)
    QDir dir(KGlobal::dirs()->saveLocation("data", "kwallet", false), "*.kwl");

    dir.setFilter(QDir::Files | QDir::Hidden);

    uint amount = 0;
    foreach(const QFileInfo & fi, dir.entryInfoList()) {
        KJob *importJob = new ImportSingleWalletJob( fi.absoluteFilePath(), this);
        if ( !addSubjob( importJob ) ) {
            kDebug() << "Cannot add import subjob";
        }
        importJob->start();
        amount++;
    }
    setTotalAmount( Files, amount );
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
    KCompositeJob::slotResult(job);
    setProcessedAmount( Files, processedAmount(Files) + 1 );
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
