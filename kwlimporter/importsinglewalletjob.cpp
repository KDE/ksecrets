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


#include "importsinglewalletjob.h"
#include "../frontend/secret/adaptors/dbustypes.h"
#include "kwalletbackend/kwalletbackend.h"
#include "../frontend/secret/service.h"
#include "../backend/backendmaster.h"
#include "../peer.h"
#include "../jobinfostructs.h"
#include "../../client/ksecretsservicecollection.h"
#include "service_interface.h"
#include "collection_interface.h"

#include <kpassworddialog.h>
#include <KLocalizedString>

#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusPendingReply>
#include <QtDBus/QDBusReply>
#include <QtGui/QTextDocument> // Qt::escape
#include <KDebug>

ImportSingleWalletJob::ImportSingleWalletJob(Service *service, const QString& walletName, QObject* parent)
    : KJob(parent)
    , m_walletName(walletName)
    , m_service(service)
    , m_isSuspended(false)
    , m_hasKillRequest(false)
{

}

ImportSingleWalletJob::~ImportSingleWalletJob()
{

}

void ImportSingleWalletJob::start()
{
    // Go
    m_status = Initializing;
    QMetaObject::invokeMethod(this, "run", Qt::QueuedConnection);
}

void ImportSingleWalletJob::run()
{
    m_wallet = new KWallet::Backend(m_walletName);

    int rc = 1;
    KPasswordDialog *passDdlg = 0;
    do {
        rc = m_wallet->open(QByteArray());
        if ( rc != 0 ) {
            passDdlg = new KPasswordDialog();
            passDdlg->setPrompt( i18n("<qt>The KSecretService daemon requests access to the wallet '<b>%1</b>'. Please enter the password for this wallet below.</qt>",
                Qt::escape( m_walletName ) ) );
            // don't use KStdGuiItem::open() here which has trailing ellipsis!
            passDdlg->setButtonGuiItem(KDialog::Ok,KGuiItem( i18n( "&Open" ), "wallet-open"));
            passDdlg->setCaption(i18n("KDE Wallet Service"));
            passDdlg->setPixmap(KIcon("kwalletmanager").pixmap(KIconLoader::SizeHuge));

            // FIXME: activate notification if no window is visible like the commented code below
/*            if (w != KWindowSystem::activeWindow() && w != 0L) {
                // If the dialog is modal to a minimized window it might not be visible
                // (but still blocking the calling application). Notify the user about
                // the request to open the wallet.
                KNotification *notification = new KNotification("needsPassword", kpd,
                                                                KNotification::Persistent |
                                                                KNotification::CloseWhenWidgetActivated);
                QStringList actions(i18nc("Text of a button to ignore the open-wallet notification", "Ignore"));
                if (appid.isEmpty()) {
                    notification->setText(i18n("<b>KDE</b> has requested to open a wallet (%1).",
                                                Qt::escape(wallet)));
                    actions.append(i18nc("Text of a button for switching to the (unnamed) application "
                                            "requesting a password", "Switch there"));
                } else {
                    notification->setText(i18n("<b>%1</b> has requested to open a wallet (%2).",
                                                Qt::escape(appid), Qt::escape(wallet)));
                    actions.append(i18nc("Text of a button for switching to the application requesting "
                                            "a password", "Switch to %1", Qt::escape(appid)));
                }
                notification->setActions(actions);
                connect(notification, SIGNAL(action1Activated()),
                        notification, SLOT(close()));
                connect(notification, SIGNAL(action2Activated()),
                        this, SLOT(activatePasswordDialog()));
                notification->sendEvent();
            }*/
            
            if ( passDdlg->exec() == KDialog::Accepted ) {
                rc = m_wallet->open( passDdlg->password().toUtf8() );
            }
            else {
                break;
            }
        }
    } while ( ! m_wallet->isOpen() );

    delete passDdlg;
    
    kDebug() << "KWallet::open returned " << rc;
    // Be async
    QMetaObject::invokeMethod(this, "onWalletOpened", Qt::QueuedConnection, Q_ARG(bool, m_wallet->isOpen()) );
}

void ImportSingleWalletJob::onWalletOpened(bool success)
{
    if(!success) {
        setErrorText(i18n("The wallet %1 could not be opened", m_walletName));
        emitResult();
        return;
    }

// FIXME: prototype changed
//    m_collection = KSecretService::instance()->createCollection( m_walletName );
    
    // Gather the folder list
    m_folderList = m_wallet->folderList();
    m_folderList.takeFirst(); // the first folder is the wallet's current folder and it'll be imported right away

    // Check in the root folder first, one never knows
    QMetaObject::invokeMethod(this, "processCurrentFolder", Qt::QueuedConnection);
}

void ImportSingleWalletJob::processCurrentFolder()
{
    if(!canProceed()) {
        return;
    }

    m_currentEntryList = m_wallet->entryList();

    // Start
    QMetaObject::invokeMethod(this, "processNextEntry", Qt::QueuedConnection);
}

void ImportSingleWalletJob::processNextEntry()
{
    if(!canProceed()) {
        return;
    }

    if(m_currentEntryList.isEmpty()) {
        // Ok, we're done with this folder. Let's skip to the next one
        QMetaObject::invokeMethod(this, "processNextFolder", Qt::QueuedConnection);
        return;
    }

    QString entry = m_currentEntryList.takeFirst();

    // FIXME: I assume the secret is empty, and the attributes constitute the map. Wonder if that makes sense, though.
    //        Another possible way I am thinking of is serialization.

    // Read the entry
    KWallet::Entry *walletEntry = m_wallet->readEntry(entry);

    if(!walletEntry) {
        // We abort the job here - but maybe we should just spit a warning?
        kDebug() << "Could not read the entry " << entry;
        setErrorText(i18n("Could not read the entry %1", entry));
        emitResult();
        return;
    }

    // Create an item
//     KSecretServiceCollection::Entry newItem;
    QMap< QString, QVariant > itemProperties;
    
    // If it's a map, populate attributes
    if(walletEntry->type() == KWallet::Entry::Map) {
        StringStringMap itemAttributes;

        // Read the entry
        QByteArray v = walletEntry->map();
        QMap< QString, QString > value;

        QDataStream ds(&v, QIODevice::ReadOnly);
        ds >> value;
        itemAttributes = value;

        itemProperties["Attributes"] = QVariant::fromValue(itemAttributes);
    }

    itemProperties["Label"] = entry;
    itemProperties["Locked"] = false;
    QList<QVariant> itemInput;

    Secret secret;
    
    if(walletEntry->type() != KWallet::Entry::Map) {
        // Read the entry
        secret.setValue(walletEntry->value());
    } else {
        secret.setValue(QByteArray());
    }

// FIXME
//     KSecretServiceCollection::Entry item;
//     itemInput << QVariant::fromValue(itemProperties);
//     itemInput << QVariant::fromValue(secret);
//     itemInput << false;
//
//    KJob* itemJob = m_collection->writeEntryAsync( secret.value(), itemProperties );
/*
    if( itemJob->error() ) {
        // We abort the job here - but maybe we should just spit a warning?
        kDebug() << i18n("Calling CreateItem on the secret service daemon for the entry %1 failed", entry);
        setErrorText(i18n("Calling CreateItem on the secret service daemon for the entry %1 failed", entry));
        emitResult();
        return;
    }*/
    
    // TODO: launch the itemJob and if everything OK, then process next entry


    // Recurse asynchronously
    QMetaObject::invokeMethod(this, "processNextEntry", Qt::QueuedConnection);
}

void ImportSingleWalletJob::processNextFolder()
{
    if(!canProceed()) {
        return;
    }

    m_status = ProcessingNextFolder;

    if(m_folderList.isEmpty()) {
        // We're done!
        emitResult();
        return;
    }

    m_wallet->setFolder(m_folderList.takeFirst());

    // Start asynchronously
    QMetaObject::invokeMethod(this, "processCurrentFolder", Qt::QueuedConnection);
}

bool ImportSingleWalletJob::doKill()
{
    if(m_status == Initializing) {
        return false;
    }

    m_hasKillRequest = true;

    return true;
}

bool ImportSingleWalletJob::doResume()
{
    m_isSuspended = false;

    switch(m_status) {
    case ProcessingCurrentFolder:
    case ProcessingEntry:
        processNextEntry();
        break;
    case ProcessingNextFolder:
        processCurrentFolder();
        break;
    default:
        return false;
        break;
    }

    return true;
}

bool ImportSingleWalletJob::doSuspend()
{
    if(m_status == Initializing) {
        return false;
    }

    m_isSuspended = true;

    return true;
}

bool ImportSingleWalletJob::canProceed()
{
    return !m_hasKillRequest && !m_isSuspended;
}

#include "importsinglewalletjob.moc"
