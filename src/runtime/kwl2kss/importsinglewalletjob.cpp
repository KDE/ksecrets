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


#include "importsinglewalletjob.h"
#include "kwalletbackend/kwalletentry.h"
#include "kwalletbackend/kwalletbackend.h"
#include <QTimer>
#include <kpassworddialog.h>
#include <kwallet.h> // for KSS_ macros
#include <KLocalizedString>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusPendingReply>
#include <QtDBus/QDBusReply>
#include <QtGui/QTextDocument> // Qt::escape
#include <QDebug>
#include <QFileInfo>

#include <ksecretsservice/ksecretsservicecollection.h>
#include <ksecretsservice/ksecretsservicecollectionjobs.h>
#include <ksecretsservice/ksecretsserviceitem.h>


using namespace KSecretsService;

ImportSingleWalletJob::ImportSingleWalletJob(const QString& walletPath, QObject* parent)
    : KCompositeJob(parent), 
    m_walletPath(walletPath), 
    m_wallet(0),
    m_isSuspended(false), 
    m_hasKillRequest(false),
    m_collection(0),
    m_checkCollection(0)
{
    QFileInfo fileInfo( m_walletPath );
    m_walletName = fileInfo.baseName();
    
    m_passDlg = new KPasswordDialog();
    // don't use KStdGuiItem::open() here which has trailing ellipsis!
    m_passDlg->setButtonGuiItem(KDialog::Ok,KGuiItem( i18n( "&Open" ), "wallet-open"));
    m_passDlg->setCaption(i18n("KDE Wallet Service"));
    m_passDlg->setPixmap(KIcon("ksecretsservice").pixmap(KIconLoader::SizeHuge));

    connect( m_passDlg, SIGNAL(gotPassword(QString,bool)), this, SLOT(onGotWalletPassword(QString,bool)) );
    connect( m_passDlg, SIGNAL(cancelClicked()), this, SLOT(doKill()) );
}

ImportSingleWalletJob::~ImportSingleWalletJob()
{
    delete m_checkCollection;
    delete m_collection;
    delete m_passDlg;
}

void ImportSingleWalletJob::start()
{
    qDebug() << "Importing " << m_walletPath << ": initializing";
    // Go
    m_status = Initializing;
    QMetaObject::invokeMethod(this, "checkImportNeeded", Qt::QueuedConnection);
    
    // NOTE: this job perform the followin tasks
    // 1. tries to find a collection having the same name as the kwallet
    //      if not found, then do the import
    // 2. if found, look inside for a KwlImportStruct stored inside a secret item labelled "creator"="kwl2kss"
    //      if not found, then do the import
    // 3. if this structure shows that the kwl file is more recent, then perform the import
}

void ImportSingleWalletJob::checkImportNeeded()
{
    if(!canProceed()) {
        return;
    }
    
    qDebug() << "Importing " << m_walletPath << ": checking for existing collection";
    m_checkCollection = Collection::findCollection( m_walletName, Collection::OpenOnly );
    StringStringMap attrs;
    attrs.insert( "creator", "kwl2kss" );
    SearchCollectionItemsJob *searchItemsJob = m_checkCollection->searchItems( attrs );
    searchItemsJob->setUiDelegate(0); // we don't need notifications for this job
    connect( searchItemsJob, SIGNAL(finished(KJob*)), this, SLOT(checkImportNeededItems(KJob*)) );
    searchItemsJob->start();
}

void ImportSingleWalletJob::checkImportNeededItems(KJob *job)
{
    if(!canProceed()) {
        return;
    }
    
    bool shouldImport = false;
    SearchCollectionItemsJob *searchItemsJob = qobject_cast< SearchCollectionItemsJob* >( job );
    if ( searchItemsJob->error() == 0 ) {
        if ( searchItemsJob->items().count() >0 ) {
            SecretItem *item = searchItemsJob->items().first().data();
            GetSecretItemSecretJob *readSecretJob = item->getSecret();
            readSecretJob->setUiDelegate(0); // no notifications from this job needed
            connect( readSecretJob, SIGNAL(finished(KJob*)), this, SLOT(checkImportNeededSecret(KJob*)) );
            readSecretJob->start();
        }
        else {
            shouldImport = true;
        }
    }
    else {
        if ( searchItemsJob->error() == SearchCollectionItemsJob::CollectionNotFound ) {
            shouldImport = true;
        }
        else {
            setError( searchItemsJob->error() );
            emitResult();
        }
    }
    
    if ( shouldImport ) {
        QMetaObject::invokeMethod(this, "run", Qt::QueuedConnection);
    }
}

struct KwlImportStruct {
    time_t kwlCreateTime;
    time_t kwlUpdateTime;
    // TODO: should we use some other information pieces to detect importing need?
};

Q_DECLARE_METATYPE(KwlImportStruct)

void ImportSingleWalletJob::checkImportNeededSecret(KJob *job)
{
    if(!canProceed()) {
        return;
    }
    
    GetSecretItemSecretJob *readSecretJob = qobject_cast< GetSecretItemSecretJob* >(job);
    if ( readSecretJob->error() == 0 ) {
        if ( readSecretJob->secret().contentType() == "kwl2kss" ) {
            QFileInfo fileInfo( m_walletPath );
            KwlImportStruct secret = readSecretJob->secret().value().value<KwlImportStruct>();
            if ( secret.kwlCreateTime == fileInfo.created().toTime_t() &&
                secret.kwlUpdateTime == fileInfo.lastModified().toTime_t() ) {
                
                // no need to import this wallet as it's already in KSS
                qDebug() << "Importing " << m_walletPath << ": wallet was already imported";
                setError(0);
                emitResult();
                return;
            }
        }
    }
    
    // once here, we need to do the actual import
    delete m_checkCollection;
    m_checkCollection = 0;
    
    QMetaObject::invokeMethod(this, "run", Qt::QueuedConnection);
}

void ImportSingleWalletJob::run()
{
    if ( m_wallet == 0 ) {
         m_wallet = new KWallet::Backend(m_walletPath, true);
    }
    
    int rc = 1;
    rc = m_wallet->open(QByteArray());
    while ( rc != 0 ) {
        m_passDlg->setPrompt( i18n("<qt>The kwl2kss utility requests access to the wallet '<b>%1</b>'. Please enter the password for this wallet below.</qt>",
            Qt::escape( m_walletName ) ) );
        if ( m_passDlg->exec() == QDialog::Rejected ) {
            setError(1);
            setErrorText( i18n("Job canceled") );
            emitResult();
            return;
        }
        rc = m_wallet->open( m_passDlg->password().toUtf8() );
        if ( rc != 0 ) {
            // ask user to retry the password
            m_passDlg->showErrorMessage( i18n("Open request failed, please try again") );
        }
    }
   QMetaObject::invokeMethod( this, "onWalletOpened", Qt::QueuedConnection, Q_ARG(bool, m_wallet->isOpen()) );
}

void ImportSingleWalletJob::onWalletOpened(bool success)
{
    if(!success) {
        setError(1);
        setErrorText(i18n("The wallet %1 could not be opened", m_walletName));
        emitResult();
        return;
    }

    // NOTE: this will create a new collection the first time
    // Afterwards it will open the existing collection allowing for it to be 
    m_collection = Collection::findCollection(m_walletName);
    
    // Gather the folder list
    m_folderList = m_wallet->folderList();
    
    // count the entries in this wallet, for progress bar needs
    uint amount =0;
    foreach( const QString &folder, m_folderList ) {
        m_wallet->setFolder( folder );
        amount += m_wallet->entryList().count();
    }
    setTotalAmount( Files, amount );
    setProcessedAmount( Files, 0 );

    m_currentFolder = m_folderList.takeFirst(); // the first folder is the wallet's current folder and it'll be imported right away

    QMetaObject::invokeMethod(this, "processCurrentFolder", Qt::QueuedConnection);
}

void ImportSingleWalletJob::processCurrentFolder()
{
    if(!canProceed()) {
        return;
    }

    qDebug() << "FOLDER: " << m_currentFolder;
    m_wallet->setFolder( m_currentFolder );
    m_currentEntryList = m_wallet->entryList();

    // Start
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
        setError(0);
        emitResult();
        return;
    }

    m_currentFolder = m_folderList.takeFirst();

    // Start asynchronously
    QMetaObject::invokeMethod(this, "processCurrentFolder", Qt::QueuedConnection);
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
    qDebug() << "  ENTRY: " << entry;

    // Read the entry
    KWallet::Entry *walletEntry = m_wallet->readEntry(entry);

    if(!walletEntry) {
        // We abort the job here - but maybe we should just spit a warning?
        qDebug() << "Could not read the entry " << entry;
        setErrorText(i18n("Could not read the entry %1 from wallet %2", entry, m_walletName));
        emitResult();
        return;
    }

    // Create an item
    StringStringMap attrs;
    attrs[KSS_ATTR_ENTRYFOLDER] = m_currentFolder;
    attrs[KSS_ATTR_WALLETTYPE] = QString("%1").arg((int)walletEntry->type());

    Secret secret;
    switch (walletEntry->type()) {
        case KWallet::Entry::Password:
            secret.setValue( walletEntry->password(), "kwallet/password" );
            break;
        case KWallet::Entry::Stream:
            secret.setValue( walletEntry->value(), "kwallet/stream" );
            break;
        case KWallet::Entry::Map:
            secret.setValue( walletEntry->map(), "kwallet/map" );
            break;
        default:
            qDebug() << "Unknown entry type " << walletEntry->type();
            return;
    }
    
    CreateCollectionItemJob *createItemJob = m_collection->createItem(entry, attrs, secret, KSecretsService::ReplaceExistingItem);
    createItemJob->setUiDelegate(0); // no notification needed from this job
    connect( createItemJob, SIGNAL(finished(KJob*)), this, SLOT(createItemFinished(KJob*)) );
    createItemJob->start();
}

void ImportSingleWalletJob::createItemFinished(KJob* job)
{
    CreateCollectionItemJob *createItemJob = qobject_cast<CreateCollectionItemJob*>( job );
    if ( createItemJob->error() == 0 ) {
        // Recurse asynchronously
        QMetaObject::invokeMethod(this, "processNextEntry", Qt::QueuedConnection);
        setProcessedAmount( Files, processedAmount( Files) + 1 );
    }
    else {
        setError(1);
        setErrorText( createItemJob->errorText() );
        emitResult();
    }
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
