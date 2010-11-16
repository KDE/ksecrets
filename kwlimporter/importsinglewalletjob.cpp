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

#include <KLocalizedString>

#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusPendingReply>

#include <frontend/secret/service.h>
#include <frontend/secret/adaptors/dbustypes.h>
#include <kwalletbackend.h>
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
    // TODO prompt for password here
    if(m_wallet->open(QByteArray()) != 0) {
        kError() << "Could not open wallet " << m_walletName;
        // Be async
        QMetaObject::invokeMethod(this, "onWalletOpened", Qt::QueuedConnection, Q_ARG(bool, false));
    } else {
        // Be async
        QMetaObject::invokeMethod(this, "onWalletOpened", Qt::QueuedConnection, Q_ARG(bool, true));
    }
}

void ImportSingleWalletJob::onWalletOpened(bool success)
{
    if(!success) {
        setErrorText(i18n("The wallet %1 could not be opened", m_walletName));
        emitResult();
        return;
    }

    // Start the conversion by creating a collection
    // TODO: How to handle prompt?
    QMap<QString, QVariant> createProperties;
    createProperties["Label"] = m_walletName;
    createProperties["Locked"] = false; // create collection unlocked
    QDBusObjectPath prompt;
    m_collectionPath = m_service->createCollection(createProperties, prompt);

    if(m_collectionPath.path() == "/") {
        setErrorText(i18n("The collection %1 could not be created", m_walletName));
        emitResult();
        return;
    }

    // Create the interface
    m_collectionInterface = new QDBusInterface("org.freedesktop.Secret", m_collectionPath.path(), QString(),
            QDBusConnection::sessionBus(), this);

    // Gather the folder list
    m_folderList = m_wallet->folderList();

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

    // Create an item
    QDBusObjectPath itemPath;
    QMap<QString, QVariant> itemProperties;

    // Read the entry
    KWallet::Entry *walletEntry = m_wallet->readEntry(entry);

    if(!walletEntry) {
        // We abort the job here - but maybe we should just spit a warning?
        setErrorText(i18n("Could not read the entry %1", entry));
        emitResult();
        return;
    }

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

    itemInput << QVariant::fromValue(itemProperties);
    itemInput << QVariant::fromValue(secret);
    itemInput << false;

    QDBusPendingReply<QDBusObjectPath, QDBusObjectPath> itemReply =
        m_collectionInterface->asyncCallWithArgumentList("CreateItem", itemInput);

    itemReply.waitForFinished();

    if(itemReply.isError()) {
        // We abort the job here - but maybe we should just spit a warning?
        setErrorText(i18n("Calling CreateItem on the secret service daemon for the entry %1 failed", entry));
        emitResult();
        return;
    }

    // TODO We ignore the second argument here
    // TODO How to check for errors?
    QDBusObjectPath item = itemReply.argumentAt(0).value<QDBusObjectPath>();

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
