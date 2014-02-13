/* This file is part of the KDE project
 *
 * Copyright (C) 2011 Valentin Rusu <kde@rusu.info>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "ksecretsservicecollectionjobs.h"
#include "ksecretsservicecollectionjobs_p.h"
#include "ksecretsservicecollection_p.h"
#include "ksecretsserviceitem_p.h"
#include "service_interface.h"
#include "collection_interface.h"
#include "item_interface.h"
#include "ksecretsservicedbustypes.h"
#include "ksecretsservicesecret_p.h"
#include "promptjob.h"

#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QSharedDataPointer>
#include <QDebug>
#include <prompt_interface.h>
#include <QWidget>
#include <klocalizedstring.h>

using namespace KSecretsService;

CollectionJobPrivate::CollectionJobPrivate() :
    collection( 0 )
{
}

CollectionJob::CollectionJob(Collection *collection, QObject* parent) : 
            KCompositeJob( parent ), 
            d( new CollectionJobPrivate() )
{
    d->collection = collection;
}

CollectionJob::~CollectionJob()
{
}

Collection *CollectionJob::collection() const 
{ 
    return d->collection; 
}

void CollectionJob::finishedWithError( CollectionError err, const QString &errTxt )
{
    // FIXME: check that this will also abort the parent job
    setError( err );
    setErrorText( errTxt );
    emitResult();
}

void CollectionJob::finishedOk()
{
    setError( NoError );
    emitResult();
}

bool CollectionJob::addSubjob( KJob* theJob )
{
    return KCompositeJob::addSubjob( theJob );
}

void CollectionJob::startFindCollection()
{
    if ( !d->collection->d->isValid() ) {
        FindCollectionJob *findJob = new FindCollectionJob( d->collection, this );
        if ( addSubjob( findJob ) ) {
            findJob->start();
        }
        else {
            qDebug() << "FindCollectionJob failed to start";
            setError( InternalError );
            emitResult();
        }
    }
    else {
        // collection was already found or created, just trigger this 
        unlockCollection();
    }
}

void CollectionJob::slotResult(KJob* job)
{
    KCompositeJob::slotResult(job);
    if ( job->error() == 0 ) {
        FindCollectionJob *findJob = qobject_cast< FindCollectionJob* >( job );
        if ( findJob != 0 ) {
            unlockCollection();
        }
        else {
            UnlockCollectionJob *unlockJob = qobject_cast< UnlockCollectionJob* >( job );
            if ( unlockJob != 0 ) {
                onFindCollectionFinished();
            }
        }
    }
}

void CollectionJob::unlockCollection()
{
    UnlockCollectionJob *unlockJob = new UnlockCollectionJob( collection(), 0 ); // FIXME: put a real window id here
    if ( addSubjob( unlockJob ) ) {
        unlockJob->start();
        // virtual method slotResult will be called upon job finish
    }
    else {
        qDebug() << "Cannot add unlock subjob";
        finishedWithError(InternalError, i18n("Cannot start secret collection unlocking") );
    }
}

void CollectionJob::onFindCollectionFinished()
{
    // nothing to do in this base implementation
}


FindCollectionJob::FindCollectionJob(   Collection *collection, 
                                        QObject *parent ) : 
            CollectionJob( collection, parent ),
            d( new FindCollectionJobPrivate( this, collection->d.data() ) )
{
    d->collectionName = collection->d->collectionName;
    d->findCollectionOptions = collection->d->findOptions;
}

FindCollectionJob::~FindCollectionJob()
{
}

void FindCollectionJob::start() 
{
    // meanwhile another findJob instance would have already connected our collection object
    if ( ! collection()->d->isValid() ) {
        d->startCreateOrOpenCollection();
    }
    else {
        setError( 0 );
        emitResult();
    }
}

void FindCollectionJob::foundCollection()
{
    d->collectionPrivate->setStatus( Collection::FoundExisting );
    finishedOk();
}

void FindCollectionJob::createdCollection()
{
    d->collectionPrivate->setStatus( Collection::NewlyCreated );
    finishedOk();
}

FindCollectionJobPrivate::FindCollectionJobPrivate(FindCollectionJob *fcj, CollectionPrivate *cp ) :
        findJob( fcj), collectionPrivate( cp )
{
}

void FindCollectionJobPrivate::createFinished(QDBusPendingCallWatcher* watcher)
{
    QDBusPendingReply< QDBusObjectPath, QDBusObjectPath > createReply = *watcher;
    Q_ASSERT( createReply.isFinished() ); // "on ne sait jamais"
    if ( watcher->isError() ) {
        qDebug() << "creating collection '" << collectionName << "' failed";
        qDebug() << "DBusError.type = " << createReply.error().type();
        qDebug() << "DBusError.name = " << createReply.error().name();
        qDebug() << "DBusError.message = " << createReply.error().message();
        findJob->finishedWithError( CollectionJob::CreateError, createReply.error().message() );
    }
    else {
        QDBusObjectPath collPath = createReply.argumentAt<0>();
        QDBusObjectPath promptPath = createReply.argumentAt<1>();
        
        if ( collPath.path().compare( QStringLiteral( "/" ) ) == 0 ) {
            // we need prompting
            Q_ASSERT( promptPath.path().compare( QStringLiteral( "/" ) ) ); // we should have a prompt path here other than "/"
            PromptJob *promptJob = new PromptJob( promptPath, collectionPrivate->promptParentId(), this );
            if ( findJob->addSubjob( promptJob ) ) {
                connect( promptJob, SIGNAL(finished(KJob*)), this, SLOT(createPromptFinished(KJob*)) );
                promptJob->start();
            }
            else {
                promptJob->deleteLater();
                qDebug() << "cannot add prompt subjob!";
                findJob->finishedWithError( CollectionJob::InternalError, i18n("Cannot add prompt job") );
            }
        }
        else {
            findJob->d->collectionPrivate->setDBusPath( collPath );
            findJob->createdCollection();
        }
    }
    watcher->deleteLater();
}

void FindCollectionJobPrivate::createPromptFinished( KJob* job )
{
    PromptJob *promptJob = dynamic_cast< PromptJob* >( job );
    if ( promptJob->error() == 0 ) {
        if ( !promptJob->isDismissed() ) {
            QDBusVariant promptResult = promptJob->result();
            QDBusObjectPath collPath = promptResult.variant().value< QDBusObjectPath >();
            findJob->d->collectionPrivate->setDBusPath( collPath );
            findJob->createdCollection();
        }
        else {
            findJob->finishedWithError( CollectionJob::OperationCancelledByTheUser, i18n("The operation was cancelled by the user") );
        }
    }
    else {
        findJob->finishedWithError( CollectionJob::InternalError, i18n("Error encountered when trying to prompt the user") );
    }
    job->deleteLater();
}


void FindCollectionJobPrivate::startCreateOrOpenCollection()
{
    OpenSessionJob *openSessionJob = DBusSession::openSession();
    if ( findJob->addSubjob( openSessionJob ) ) {
        connect( openSessionJob, SIGNAL(finished(KJob*)), this, SLOT(openSessionFinished(KJob*)) );
        openSessionJob->start();
    }
    else {
        qDebug() << "Cannot OpenSessionJob subjob";
        findJob->finishedWithError( CollectionJob::InternalError, i18n("Cannot open session") );
    }
}

void FindCollectionJobPrivate::openSessionFinished(KJob* theJob)
{
    if ( !theJob->error() ) {
        
        QList< QDBusObjectPath > collPaths = DBusSession::serviceIf()->collections();
        foreach ( const QDBusObjectPath &collPath, collPaths ) {
            OrgFreedesktopSecretCollectionInterface *coll = DBusSession::createCollectionIf( collPath );
            coll->deleteLater();
            if ( coll->label() == collectionName ) {
                findJob->d->collectionPrivate->setDBusPath( collPath );
                findJob->foundCollection();
                return; // sometimes middle method returns are awfully handy :-)
            }
        }

        // we get here because collection doesn't exist
        
        if ( collectionPrivate->findOptions == Collection::CreateCollection ) {
            OpenSessionJob *openSessionJob = dynamic_cast< OpenSessionJob * >( theJob );
            QVariantMap creationProperties = collectionPrivate->collectionProperties;
            creationProperties.insert( QStringLiteral( "org.freedesktop.Secret.Collection.Label" ), collectionName);
            QDBusPendingReply< QDBusObjectPath, QDBusObjectPath > createReply = openSessionJob->serviceInterface()->CreateCollection(
                creationProperties, collectionName );
            QDBusPendingCallWatcher *createReplyWatch = new QDBusPendingCallWatcher( createReply, this );
            connect( createReplyWatch, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(createFinished(QDBusPendingCallWatcher*)) );
        }
        else {
            findJob->finishedWithError( CollectionJob::CollectionNotFound, i18n("Cannot find a secret collection named '%1'", collectionName) );
        }
    }
}


ListCollectionsJob::ListCollectionsJob() :
    d( new ListCollectionsJobPrivate( this ) )
{
    connect( d.data(), SIGNAL(listingDone()), this, SLOT(slotListCollectionsDone()) );
    connect( d.data(), SIGNAL(listingError()), this, SLOT(slotListCollectionsError()) );
}

ListCollectionsJob::~ListCollectionsJob()
{
}

bool ListCollectionsJob::addSubjob(KJob* job)
{
    return KCompositeJob::addSubjob( job );
}

void ListCollectionsJob::start()
{
    d->startListingCollections();
}

void ListCollectionsJob::slotListCollectionsDone()
{
    setError(0);
    emitResult();
}

void ListCollectionsJob::slotListCollectionsError()
{
    setError(1);
    setErrorText( i18n("Cannot list collections because a backend communication error") );
    emitResult();
}

const QStringList &ListCollectionsJob::collections() const 
{
    return d->collections;
}

ListCollectionsJobPrivate::ListCollectionsJobPrivate( ListCollectionsJob *job ) :
    listCollectionsJob(job)
{
}

void ListCollectionsJobPrivate::startListingCollections()
{
    OpenSessionJob *openSessionJob = DBusSession::openSession();
    if (listCollectionsJob->addSubjob( openSessionJob )) {
        connect( openSessionJob, SIGNAL(finished(KJob*)), this, SLOT(slotOpenSessionFinished(KJob*)) );
        openSessionJob->start();
    }
    else {
        qDebug() << "Cannot add OpenSessionJob!";
        emit listingError();
    }
}

void ListCollectionsJobPrivate::slotOpenSessionFinished(KJob* job) {
    OpenSessionJob *openSessionJob = qobject_cast< OpenSessionJob* >(job);
    Q_ASSERT(openSessionJob != 0);
    if (openSessionJob->error() == 0) {
        QList<QDBusObjectPath> collPaths = DBusSession::serviceIf()->collections();
        foreach( const QDBusObjectPath &path, collPaths ) {
            OrgFreedesktopSecretCollectionInterface *coll = DBusSession::createCollectionIf( path );
            if (coll->isValid()) {
                collections.append( coll->label() );
            }
            else {
                qDebug() << "Cannot bind to collection " << path.path();
                emit listingError();
            }
            coll->deleteLater();
        }
        emit listingDone();
    }
    else {
        qDebug() << "OpenSessionJob returned error " << openSessionJob->errorString();
        emit listingError();
    }
}

DeleteCollectionJob::DeleteCollectionJob( Collection* collection, QObject* parent ) :
        CollectionJob( collection, parent),
        d( new DeleteCollectionJobPrivate( collection->d.data(), this ) )
{
}
    
DeleteCollectionJob::~DeleteCollectionJob()
{
}

void DeleteCollectionJob::start() 
{
    // ensure we have the connection to the daemon and we have a valid collection
    // this will trigger onFindCollectionFinished
    startFindCollection();
}

void DeleteCollectionJob::onFindCollectionFinished() 
{
    connect( d.data(), SIGNAL(deleteIsDone(CollectionJob::CollectionError,QString)), this, SLOT(deleteIsDone(CollectionJob::CollectionError,QString)) );
    // now performe the real delete operation on the backend
    d->startDelete();
}

void KSecretsService::DeleteCollectionJob::deleteIsDone(CollectionError err, const QString& errMsg )
{
    finishedWithError( err, errMsg );
    d->cp->setStatus( Collection::Deleted );
}

DeleteCollectionJobPrivate::DeleteCollectionJobPrivate( CollectionPrivate* collp, QObject* parent ) : 
        QObject( parent ),
        cp( collp )
{
}

void DeleteCollectionJobPrivate::startDelete() 
{
    QDBusPendingReply<QDBusObjectPath> deleteReply = cp->collectionInterface()->Delete();
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher( deleteReply, this );
    connect( watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(callFinished(QDBusPendingCallWatcher*)) );
}

void DeleteCollectionJobPrivate::callFinished( QDBusPendingCallWatcher*  watcher ) 
{
    Q_ASSERT( watcher->isFinished() );

    QDBusPendingReply< QDBusObjectPath > deleteReply = *watcher;
    CollectionJob::CollectionError err = CollectionJob::NoError;
    QString msg;
    
    if ( deleteReply.isError() ) {
        err = CollectionJob::DeleteError;
        const QDBusError &dbusErr = deleteReply.error();
        msg = QStringLiteral("d-bus error %1 (%2)").arg( QDBusError::errorString( dbusErr.type() ) ).arg( dbusErr.message() );
    }

    qDebug() << "callFinished with err=" << (int)err << " and msg='" << msg << "'";
    emit deleteIsDone( err, msg );
    watcher->deleteLater();
}


RenameCollectionJob::RenameCollectionJob( Collection *coll, const QString &newName, QObject *parent ) : 
            CollectionJob( coll, parent ),
            d( new RenameCollectionJobPrivate( coll->d.data(), this ) )
{
    d->newName = newName;
}

RenameCollectionJob::~RenameCollectionJob()
{
}

void RenameCollectionJob::start()
{
    startFindCollection(); // this will trigger onFindCollectionFinished if collection exists
}

void RenameCollectionJob::onFindCollectionFinished()
{
    connect( d.data(), SIGNAL(renameIsDone(CollectionJob::CollectionError,QString)), this, SLOT(renameIsDone(CollectionJob::CollectionError,QString)) );
    d->startRename();
}

void RenameCollectionJob::renameIsDone( CollectionJob::CollectionError err, const QString& msg)
{
    finishedWithError( err, msg );
    // FIXME: should we change the status of newly created collections here? consider the opened status, for example.
}

RenameCollectionJobPrivate::RenameCollectionJobPrivate( CollectionPrivate *collPrivate, QObject *parent ) :
            QObject( parent ),
            collectionPrivate( collPrivate )
{
}

void RenameCollectionJobPrivate::startRename()
{
    if ( collectionPrivate->collectionInterface()->setProperty( "Label", QVariant( newName ) ) ) {
        emit renameIsDone( CollectionJob::NoError, QStringLiteral( "" ) );
    }
    else {
        emit renameIsDone( CollectionJob::RenameError, QStringLiteral( "Cannot rename secret collection to %1" ).arg( newName ) );
    }
}

SearchCollectionItemsJob::SearchCollectionItemsJob( Collection *collection, 
                                const QStringStringMap &attributes,
                                QObject *parent ) :
    CollectionJob( collection, parent ),
    d( new SearchCollectionItemsJobPrivate( collection->d.data(), this ) )
{
    d->attributes = attributes;
}

SearchCollectionItemsJob::~SearchCollectionItemsJob()
{
}

void SearchCollectionItemsJob::start()
{
    startFindCollection(); // this will trigger onFindCollectionFinished
}

SearchCollectionItemsJob::ItemList SearchCollectionItemsJob::items() const
{
    QList< QExplicitlySharedDataPointer< SecretItem > > items;
    foreach( QSharedDataPointer< SecretItemPrivate > ip, d->items ) {
        items.append( QExplicitlySharedDataPointer< SecretItem>( new SecretItem(  ip ) ) );
    }
    return items;
}

void SearchCollectionItemsJob::onFindCollectionFinished()
{
    d->startSearchItems();
}

SearchCollectionItemsJobPrivate::SearchCollectionItemsJobPrivate( CollectionPrivate* cp, SearchCollectionItemsJob *job ) :
    QObject( job ),
    collectionPrivate( cp ),
    searchItemJob( job )
{
}

void SearchCollectionItemsJobPrivate::startSearchItems()
{
    QDBusPendingReply< QList< QDBusObjectPath > > reply = collectionPrivate->collectionInterface()->SearchItems( attributes );
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher( reply );
    connect( watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(searchFinished(QDBusPendingCallWatcher*)) );
}

void SearchCollectionItemsJobPrivate::searchFinished(QDBusPendingCallWatcher* watcher)
{
    Q_ASSERT(watcher != 0);
    QDBusPendingReply< QList< QDBusObjectPath > > reply = *watcher;
    if ( !reply.isError() ) {
        QList< QDBusObjectPath > itemList = reply.argumentAt<0>();
        foreach( QDBusObjectPath itemPath, itemList ) {
            items.append( QSharedDataPointer<SecretItemPrivate>( new SecretItemPrivate( itemPath ) ) );
        }
        searchItemJob->finishedOk();
    }
    else {
        qDebug() << "ERROR searching items";
        searchItemJob->finishedWithError( CollectionJob::InternalError, i18n("ERROR searching items") );
    }
    watcher->deleteLater();
}


SearchCollectionSecretsJob::SearchCollectionSecretsJob( Collection* collection, const QStringStringMap &attributes, QObject* parent ) : 
    CollectionJob( collection, parent ),
    d( new SearchCollectionSecretsJobPrivate( collection->d.data(), attributes ) )
{
}

SearchCollectionSecretsJob::~SearchCollectionSecretsJob()
{
}

QList< Secret > SearchCollectionSecretsJob::secrets() const
{
    QList< Secret > result;
    foreach( QSharedDataPointer< SecretPrivate > sp, d->secretsList ) {
        result.append( Secret( sp ) );
    }
    return result;
}

void SearchCollectionSecretsJob::start()
{
    startFindCollection(); // this will trigger onFindCollectionFinished
}

void SearchCollectionSecretsJob::onFindCollectionFinished()
{
    connect( d.data(), SIGNAL(searchIsDone(CollectionJob::CollectionError,QString)), this, SLOT(searchIsDone(CollectionJob::CollectionError,QString)) );
    d->startSearchSecrets();
}

void SearchCollectionSecretsJob::searchIsDone( CollectionJob::CollectionError err, const QString& msg)
{
    finishedWithError( err, msg );
}

SearchCollectionSecretsJobPrivate::SearchCollectionSecretsJobPrivate( CollectionPrivate *cp, const QStringStringMap &attrs, QObject *parent ) :
    QObject( parent ),
    collectionPrivate( cp ),
    attributes( attrs )
{
}

void SearchCollectionSecretsJobPrivate::startSearchSecrets()
{
    QDBusPendingReply<QList<QDBusObjectPath> > searchReply = collectionPrivate->collectionInterface()->SearchItems( attributes );
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher( searchReply, this );
    connect( watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(searchSecretsReply(QDBusPendingCallWatcher*)));
}

void SearchCollectionSecretsJobPrivate::searchSecretsReply( QDBusPendingCallWatcher *watcher )
{
    Q_ASSERT( watcher );
    QDBusPendingReply<QList<QDBusObjectPath> > searchReply = *watcher;
    if ( !searchReply.isError() ) {
        QList< QDBusObjectPath > pathList = searchReply.value();
        qDebug() << "FOUND " << pathList.count() << " secrets";
        if ( pathList.count() >0 ) {
            QDBusPendingReply<DBusObjectPathSecretMap> getReply = DBusSession::serviceIf()->GetSecrets( pathList, DBusSession::sessionPath() );
            QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher( getReply );
            connect( watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(getSecretsReply(QDBusPendingCallWatcher*)) );
        }
        else {
           emit searchIsDone( CollectionJob::NoError, QStringLiteral( "" ) );
        }
    }
    else {
        qDebug() << "ERROR searching items";
        emit searchIsDone( CollectionJob::InternalError, QStringLiteral( "ERROR searching items" ) );
    }
    watcher->deleteLater();
}

void SearchCollectionSecretsJobPrivate::getSecretsReply(QDBusPendingCallWatcher* watcher)
{
    Q_ASSERT(watcher != 0);
    QDBusPendingReply<DBusObjectPathSecretMap> getReply = *watcher;
    if ( !getReply.isError() ) {
        foreach (DBusSecretStruct secret, getReply.value()) {
            SecretPrivate *sp =0;
            if ( SecretPrivate::fromSecretStruct( secret, sp ) ) {
                secretsList.append( QSharedDataPointer<SecretPrivate>( sp ) );
            }
            else {
                qDebug() << "ERROR decrypting the secret";
                emit searchIsDone( CollectionJob::InternalError, QStringLiteral( "ERROR decrypting the secret" ) );
            }
        }
        emit searchIsDone( CollectionJob::NoError, QStringLiteral( "" ) );
    }
    else {
        qDebug() << "ERROR trying to retrieve the secrets";
        emit searchIsDone( CollectionJob::InternalError, QStringLiteral( "ERROR trying to retrieve the secrets" ) );
    }
    watcher->deleteLater();
}


CreateCollectionItemJob::CreateCollectionItemJob( Collection *collection,
                              const QString& label,
                              const QMap< QString, QString >& attributes, 
                              const Secret& secret,
                              CreateItemOptions options
                            ) :
            CollectionJob( collection, collection ),
            d( new CreateCollectionItemJobPrivate( collection->d.data(), collection ) )
{
    d->createItemJob = this;
    d->label = label;
    d->attributes = attributes;
    d->secretPrivate = secret.d;
    d->options = options;
}

CreateCollectionItemJob::~CreateCollectionItemJob()
{
}

SecretItem * CreateCollectionItemJob::item() const 
{
    return d->item;
}

void CreateCollectionItemJob::start()
{
    if ( d->label.length() == 0) {
        finishedWithError( CollectionJob::MissingParameterError, i18n("Please specify an item properly") );
    }
    else
        startFindCollection();
}

void CreateCollectionItemJob::onFindCollectionFinished()
{
    d->startCreateItem();
}

CreateCollectionItemJobPrivate::CreateCollectionItemJobPrivate( CollectionPrivate *cp, QObject *parent ) :
        QObject( parent ),
        collectionPrivate( cp )
{
}

void CreateCollectionItemJobPrivate::startCreateItem()
{
    QVariantMap varMap;
    varMap[ QStringLiteral( "Label" ) ] = label;
    attributes[ QStringLiteral( "Label" ) ] = label;
    QVariant varAttrs;
    varAttrs.setValue<StringStringMap>(attributes);
    varMap[ QStringLiteral( "Attributes" ) ] = varAttrs;
    DBusSecretStruct secretStruct;
    if ( secretPrivate->toSecretStruct( secretStruct ) ) {
        QDBusPendingReply<QDBusObjectPath, QDBusObjectPath> createReply = collectionPrivate->collectionInterface()->CreateItem( varMap, secretStruct, options == ReplaceExistingItem );
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher( createReply );
        connect( watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(createItemReply(QDBusPendingCallWatcher*)) );
    }
    else {
        qDebug() << "ERROR preparing DBusSecretStruct";
        createItemJob->finishedWithError( CollectionJob::CreateError, i18n("Cannot prepare secret structure") );
    }
}

void CreateCollectionItemJobPrivate::createItemReply(QDBusPendingCallWatcher* watcher)
{
    QDBusPendingReply<QDBusObjectPath, QDBusObjectPath> createReply = *watcher;
    if ( !createReply.isError() ) {
        QDBusObjectPath itemPath = createReply.argumentAt<0>();
        QDBusObjectPath promptPath = createReply.argumentAt<1>();
        if ( itemPath.path().compare( QStringLiteral( "/" ) ) == 0 ) {
            PromptJob *promptJob = new PromptJob( promptPath, collectionPrivate->promptParentWindowId, this );
            if ( createItemJob->addSubjob( promptJob ) ) {
                connect( promptJob, SIGNAL(finished(KJob*)), this, SLOT(createPromptFinished(KJob*)) );
                promptJob->start();
            }
            else {
                qDebug() << "ERROR creating prompt job for " << promptPath.path();
                createItemJob->finishedWithError( CollectionJob::CreateError, i18n("Cannot create prompt job!") );
            }
        }
        else {
            QSharedDataPointer< SecretItemPrivate > itemPrivate( new SecretItemPrivate( itemPath ) );
            if ( itemPrivate->isValid() ) {
                item = new SecretItem( itemPrivate );
                createItemJob->finishedOk();
            }
            else {
                item = NULL;
                qDebug() << "ERROR creating item, as it's invalid. path = " << itemPath.path();
                createItemJob->finishedWithError( CollectionJob::CreateError, i18n("The backend returned an invalid item path or it's no longer present") );
            }
        }
    }
    else {
        qDebug() << "ERROR trying to create item : " << createReply.error().message();
        createItemJob->finishedWithError( CollectionJob::CreateError, i18n("Backend communication error") );
    }
    watcher->deleteLater();
}

void CreateCollectionItemJobPrivate::createPromptFinished(KJob*)
{
    // TODO: implement this
}


ReadCollectionItemsJob::ReadCollectionItemsJob( Collection *collection,
                                        QObject *parent ) :
    CollectionJob( collection, parent ),
    d( new ReadCollectionItemsJobPrivate( collection->d.data() ) )
{
}

ReadCollectionItemsJob::~ReadCollectionItemsJob()
{
}

void ReadCollectionItemsJob::start()
{
    startFindCollection();
}

void ReadCollectionItemsJob::onFindCollectionFinished()
{
    // this is a property read - Qt seems to read properties synchrounously
    setError( 0 );
    setErrorText( QStringLiteral( "" ) );
    emitResult();
}

QList< QExplicitlySharedDataPointer< SecretItem > > ReadCollectionItemsJob::items() const 
{
    QList< QExplicitlySharedDataPointer< SecretItem > > result;
    foreach( QSharedDataPointer< SecretItemPrivate > ip, d->readItems() ) {
        result.append( QExplicitlySharedDataPointer< SecretItem>( new SecretItem( ip ) ) );
    }
    return result;
}

ReadCollectionItemsJobPrivate::ReadCollectionItemsJobPrivate( CollectionPrivate *cp ) :
    collectionPrivate( cp )
{
}

QList< QSharedDataPointer< SecretItemPrivate > > ReadCollectionItemsJobPrivate::readItems() const 
{
    QList< QSharedDataPointer< SecretItemPrivate > > result;
    if ( collectionPrivate->collectionInterface() ) {
        foreach( QDBusObjectPath path, collectionPrivate->collectionInterface()->items() ) {
            result.append( QSharedDataPointer<SecretItemPrivate>( new SecretItemPrivate( path ) ) );
        }
    }
    return result;
}

ReadCollectionPropertyJob::ReadCollectionPropertyJob( Collection *coll, const char *propName, QObject *parent ) :
    CollectionJob( coll, parent ),
    d( new ReadCollectionPropertyJobPrivate( coll->d.data(), this ) ),
    propertyReadMember(0)
{
    d->propertyName = propName;
}

ReadCollectionPropertyJob::~ReadCollectionPropertyJob()
{
}

ReadCollectionPropertyJob::ReadCollectionPropertyJob( Collection *coll, void (Collection::*propReadMember)( ReadCollectionPropertyJob* ), QObject *parent ) :
    CollectionJob( coll, parent ),
    d( new ReadCollectionPropertyJobPrivate( coll->d.data(), this ) ),
    propertyReadMember( propReadMember )
{
}

void ReadCollectionPropertyJob::start()
{
    startFindCollection(); // this will trigger onFindCollectionFinished
}

void ReadCollectionPropertyJob::onFindCollectionFinished()
{
    if ( propertyReadMember ) {
        (collection()->*propertyReadMember)( this );
        finishedOk();
    }
    else {
        d->startReadingProperty();
    }
}

const QVariant& ReadCollectionPropertyJob::propertyValue() const
{
    return d->value;
}

ReadCollectionPropertyJobPrivate::ReadCollectionPropertyJobPrivate( CollectionPrivate *cp, ReadCollectionPropertyJob *job ) :
    collectionPrivate( cp ),
    readPropertyJob( job )
{
}
    
void ReadCollectionPropertyJobPrivate::startReadingProperty()
{
    value = collectionPrivate->collectionInterface()->property( propertyName );
    readPropertyJob->finishedOk();
}


WriteCollectionPropertyJob::WriteCollectionPropertyJob( Collection *coll, const char *propName, const QVariant& value, QObject *parent ) :
    CollectionJob( coll, parent ),
    d( new WriteCollectionPropertyJobPrivate( coll->d.data(), this ) )
{
    d->propertyName = propName;
    d->value = value;
}

WriteCollectionPropertyJob::~WriteCollectionPropertyJob()
{
}

void WriteCollectionPropertyJob::start()
{
    startFindCollection(); // this will trigger onFindCollectionFinished
}

void WriteCollectionPropertyJob::onFindCollectionFinished()
{
    d->startWritingProperty();
}

WriteCollectionPropertyJobPrivate::WriteCollectionPropertyJobPrivate( CollectionPrivate *cp, WriteCollectionPropertyJob *job ) :
    collectionPrivate( cp ),
    writePropertyJob( job )
{
}
    
void WriteCollectionPropertyJobPrivate::startWritingProperty()
{
    value = collectionPrivate->collectionInterface()->setProperty( propertyName, value );
    writePropertyJob->finishedOk();
}

ChangeCollectionPasswordJob::ChangeCollectionPasswordJob(Collection* collection): 
    CollectionJob( collection ),
    d( new ChangeCollectionPasswordJobPrivate( collection->d.data(), this ) )
{
}

void ChangeCollectionPasswordJob::start()
{
    startFindCollection();
}

void ChangeCollectionPasswordJob::onFindCollectionFinished()
{
    d->startChangingPassword();
}

ChangeCollectionPasswordJobPrivate::ChangeCollectionPasswordJobPrivate( CollectionPrivate *cp, ChangeCollectionPasswordJob *job ) :
    collectionPrivate( cp ),
    theJob( job )
{
}

void ChangeCollectionPasswordJobPrivate::startChangingPassword()
{
    QDBusPendingReply< QDBusObjectPath > reply = collectionPrivate->collectionInterface()->ChangePassword();
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher( reply );
    connect( watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(changePasswordStarted(QDBusPendingCallWatcher*)) );
}

void ChangeCollectionPasswordJobPrivate::changePasswordStarted( QDBusPendingCallWatcher *watcher )
{
    Q_ASSERT(watcher != 0);
    QDBusPendingReply< QDBusObjectPath > reply = *watcher;
    if ( !reply.isError() ) {
        QDBusObjectPath promptPath = reply.argumentAt<0>();
        PromptJob *promptJob = new PromptJob( promptPath, collectionPrivate->promptParentId(), this );
        if ( theJob->addSubjob( promptJob ) ) {
            connect( promptJob, SIGNAL(finished(KJob*)), this, SLOT(promptFinished(KJob*)) );
            promptJob->start();
        }
        else {
            promptJob->deleteLater();
            qDebug() << "cannot add prompt subjob!";
            theJob->finishedWithError( CollectionJob::InternalError, i18n("Cannot add prompt job") );
        }
    }
    else {
        qDebug() << "ERROR when starting password change " << reply.error().message();
        theJob->finishedWithError( CollectionJob::InternalError, reply.error().message() );
    }
    watcher->deleteLater();
}

void ChangeCollectionPasswordJobPrivate::promptFinished( KJob* pj )
{
    PromptJob *promptJob = dynamic_cast< PromptJob* >( pj );
    if ( promptJob->error() == 0 ) {
        if ( !promptJob->isDismissed() ) {
            theJob->finishedOk();
        }
        else {
            theJob->finishedWithError( CollectionJob::OperationCancelledByTheUser, i18n("The operation was cancelled by the user") );
        }
    }
    else {
        theJob->finishedWithError( CollectionJob::InternalError, i18n("Error encountered when trying to prompt the user") );
    }
    pj->deleteLater();
}


LockCollectionJob::LockCollectionJob( Collection *coll, const WId winId ) :
    CollectionJob( coll ),
    d( new LockCollectionJobPrivate( coll->d.data(), this ) )
{
    d->windowId = winId;
}

void LockCollectionJob::start()
{
    startFindCollection();
}

void LockCollectionJob::onFindCollectionFinished()
{
    d->startLockingCollection();
}

LockCollectionJobPrivate::LockCollectionJobPrivate( CollectionPrivate *cp, LockCollectionJob *j ) :
    collectionPrivate( cp ),
    theJob( j )
{
}

void LockCollectionJobPrivate::startLockingCollection()
{
    QList< QDBusObjectPath > lockList;
    lockList.append( QDBusObjectPath( collectionPrivate->collectionInterface()->path() ) );
    QDBusPendingReply<QList<QDBusObjectPath> , QDBusObjectPath> reply = DBusSession::serviceIf()->Lock( lockList );
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher( reply );
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(slotLockFinished(QDBusPendingCallWatcher*)) );
}

void LockCollectionJobPrivate::slotLockFinished( QDBusPendingCallWatcher *watcher )
{
    Q_ASSERT(watcher);
    QDBusPendingReply<QList<QDBusObjectPath> , QDBusObjectPath> reply = *watcher;
    if ( !reply.isError() ) {
        QDBusObjectPath promptPath = reply.argumentAt<1>();
        if ( promptPath.path().compare( QStringLiteral( "/" ) ) ) {
            PromptJob *promptJob = new PromptJob( promptPath, windowId, this ); 
            connect( promptJob, SIGNAL(finished(KJob*)), this, SLOT(slotPromptFinished(KJob*)) );
            if ( theJob->addSubjob( promptJob ) ) {
                promptJob->start();
            }
            else {
                promptJob->deleteLater();
                qDebug() << "cannot add prompt subjob";
                theJob->finishedWithError( CollectionJob::InternalError, i18n("Cannot add prompt job") );
            }
        }
        else {
            QList< QDBusObjectPath > objList = reply.argumentAt<0>();
            checkResult( objList );
        }
    }
    else {
        qDebug() << "ERROR when trying to lock collection " << reply.error().message();
        theJob->finishedWithError( CollectionJob::InternalError, reply.error().message() );
    }
    watcher->deleteLater();
}

void LockCollectionJobPrivate::slotPromptFinished( KJob* j )
{
    PromptJob *promptJob = qobject_cast< PromptJob* >(j);
    if ( promptJob->error() == 0 ) {
        if ( !promptJob->isDismissed() ) {
            QDBusVariant res = promptJob->result();
            if (res.variant().canConvert< QList< QDBusObjectPath> >()) {
                QList< QDBusObjectPath > objList = res.variant().value< QList< QDBusObjectPath > >();
                checkResult( objList );
            }
            else {
                theJob->finishedWithError( CollectionJob::InternalError, i18n("Unlock operation returned unexpected result") );
            }
        }
        else {
            theJob->finishedWithError( CollectionJob::OperationCancelledByTheUser, i18n("The operation was cancelled by the user") );
        }
    }
    else {
        theJob->finishedWithError( CollectionJob::InternalError, i18n("Error encountered when trying to prompt the user") );
    }
}

void LockCollectionJobPrivate::checkResult( const QList< QDBusObjectPath > & objList ) const
{
    if ( objList.count() == 1 && objList.first().path() == collectionPrivate->collectionInterface()->path() ) {
        theJob->finishedOk();
    }
    else {
        qDebug() << "objList.count() = " << objList.count();
        theJob->finishedWithError( CollectionJob::InternalError, i18n("Unlock operation returned unexpected result") );
    }
}

UnlockCollectionJob::UnlockCollectionJob( Collection* collection, const WId winId  ) : 
    CollectionJob( collection ),
    d( new UnlockCollectionJobPrivate( collection->d.data(), this ) )
{
    d->windowId = winId;
}

void UnlockCollectionJob::start()
{
    startFindCollection();
}

void UnlockCollectionJob::unlockCollection()
{
    // do not call parent implementation to avoid weird situations as we're already
    // un unlocking job. Call onFindCollectionFinished instead.
    onFindCollectionFinished();
}

void UnlockCollectionJob::onFindCollectionFinished()
{
    d->startUnlockingCollection();
}

UnlockCollectionJobPrivate::UnlockCollectionJobPrivate( CollectionPrivate *cp, UnlockCollectionJob *job ) :
    collectionPrivate( cp ),
    theJob( job )
{
}

void UnlockCollectionJobPrivate::startUnlockingCollection()
{
    QList< QDBusObjectPath > unlockList;
    unlockList.append( QDBusObjectPath( collectionPrivate->collectionInterface()->path() ) );
    QDBusPendingReply<QList<QDBusObjectPath> , QDBusObjectPath> reply = DBusSession::serviceIf()->Unlock(unlockList);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher( reply );
    connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(slotUnlockFinished(QDBusPendingCallWatcher*)) );
}

void UnlockCollectionJobPrivate::slotUnlockFinished( QDBusPendingCallWatcher *watcher ) 
{
    Q_ASSERT(watcher);
    QDBusPendingReply<QList<QDBusObjectPath> , QDBusObjectPath> reply = *watcher;
    if ( !reply.isError() ) {
        QDBusObjectPath promptPath = reply.argumentAt<1>();
        if ( promptPath.path().compare( QStringLiteral( "/" ) ) ) {
            PromptJob *promptJob = new PromptJob( promptPath, windowId, this ); 
            connect( promptJob, SIGNAL(finished(KJob*)), this, SLOT(slotPromptFinished(KJob*)) );
            if ( theJob->addSubjob( promptJob ) ) {
                promptJob->start();
            }
            else {
                promptJob->deleteLater();
                qDebug() << "cannot add prompt subjob";
                theJob->finishedWithError( CollectionJob::InternalError, i18n("Cannot add prompt job") );
            }
        }
        else {
            QList< QDBusObjectPath > objList = reply.argumentAt<0>();
            checkResult( objList );
        }
    }
    else {
        qDebug() << "ERROR when trying to lock collection " << reply.error().message();
        theJob->finishedWithError( CollectionJob::InternalError, reply.error().message() );
    }
    watcher->deleteLater();
}

void UnlockCollectionJobPrivate::slotPromptFinished( KJob* j )
{
    PromptJob *promptJob = qobject_cast< PromptJob* >(j);
    if ( promptJob->error() == 0 ) {
        if ( !promptJob->isDismissed() ) {
            QDBusVariant res = promptJob->result();
            /**
             * NOTE: thanks to randomguy3 for helping me figuring out that QtDbus
             * puts here a QDBusArgument because it won't know how to demarshall
             * directly the QList<QDBusObjectPath>.
             * http://randomguy3.wordpress.com/2010/09/07/the-magic-of-qtdbus-and-the-propertychanged-signal/
             */
            if ( res.variant().canConvert< QDBusArgument >() ) {
                QDBusArgument arg = res.variant().value< QDBusArgument >();
                QList< QDBusObjectPath > objList;
                arg >> objList;
                checkResult( objList );
            }
            else {
                theJob->finishedWithError( CollectionJob::InternalError, i18n("Unlock operation returned unexpected result") );
            }
        }
        else {
            theJob->finishedWithError( CollectionJob::OperationCancelledByTheUser, i18n("The operation was cancelled by the user") );
        }
    }
    else {
        theJob->finishedWithError( CollectionJob::InternalError, i18n("Error encountered when trying to prompt the user") );
    }
}

void UnlockCollectionJobPrivate::checkResult( const QList< QDBusObjectPath > & objList ) const
{
    if ( objList.count() == 1 && objList.first().path() == collectionPrivate->collectionInterface()->path() ) {
        theJob->finishedOk();
    }
    else {
        qDebug() << "objList.count() = " << objList.count();
        theJob->finishedWithError( CollectionJob::InternalError, i18n("Unlock operation returned unexpected result") );
    }
}


#include "ksecretsservicecollectionjobs.moc"
//#include "ksecretsservicecollectionjobs_p.moc"
