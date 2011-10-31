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

#include "ksecretsapp.h"
#include "ksecretsappjob.h"

#include <QTimer>
#include <kcmdlineargs.h>
#include <ksecretsservicecollection.h>
#include <ksecretsservicedbustypes.h>
#include <iostream>

using namespace KSecretsService;
using namespace std;

Q_DECLARE_METATYPE( KSecretsService::ReadCollectionItemsJob::Item )

ostream& operator << (ostream &out, const QString& str)
{
    out << qPrintable( str );
    return out;
}

ostream& operator << (ostream &out, const KLocalizedString &str )
{
    out << str.toString();
    return out;
}

KSecretsApp::KSecretsApp()
{
    QTimer::singleShot( 0, this, SLOT(slotParseArgs()) );
}

void KSecretsApp::slotParseArgs()
{
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    if ( args->isSet( "lc") ) {
        listCollections();
    }
    else { 
        if ( args->isSet( "l" ) ) {
            QString collName = args->getOption( "l" );
            listCollection( collName );
        }
        else {
            KCmdLineArgs::usage();
            quit();
        }
    }
}

void KSecretsApp::listCollections()
{
    cout << ki18n("Listing collections...") << endl;
    ListCollectionsJob *listJob = Collection::listCollections();
    connect( listJob, SIGNAL(finished(KJob*)), this, SLOT(slotListCollectionsDone(KJob*)) );
    listJob->start();
}

void KSecretsApp::slotListCollectionsDone(KJob* job)
{
    ListCollectionsJob *listJob = qobject_cast< ListCollectionsJob* >(job);
    if ( listJob->error() == 0 ) {
        foreach( const QString &collName, listJob->collections() ) {
            cout << collName << endl;
        }
    }
    else {
        cerr << listJob->errorString() << endl;
    }
    job->deleteLater();
    quit();
}

void KSecretsApp::listCollection(QString collName)
{
    cout << ki18n("Listing collection ") << collName << "..." << endl;
    Collection *coll = Collection::findCollection( collName, Collection::OpenOnly );
    ReadCollectionItemsJob *listJob = coll->items();
    connect( listJob, SIGNAL(finished(KJob*)), this, SLOT(slotListItemsDone(KJob*)) );
    listJob->start();
}


void KSecretsApp::slotListItemsDone(KJob* job)
{
    ReadCollectionItemsJob *listJob = qobject_cast< ReadCollectionItemsJob* >(job);
    if ( listJob->error() == 0 ){
        KSecretsAppJob *listItemsJob = new KSecretsAppJob(this);
        connect( listItemsJob, SIGNAL(finished(KJob*)), this, SLOT(slotReadAllItemsDone(KJob*)) );
        
        foreach( const ReadCollectionItemsJob::Item &item, listJob->items() ) {
            ReadItemPropertyJob *readJob = item->attributes();
            QVariant varItem;
            varItem.setValue< ReadCollectionItemsJob::Item >( item );
            listItemsJob->setCustomData( varItem );
            if ( listItemsJob->addSubjob( readJob ) ) {
                connect( readJob, SIGNAL(finished(KJob*)), this, SLOT(slotReadItemAttributesDone(KJob*)) );
                readJob->start();
            }
        }
    }
    else {
        cerr << listJob->errorString() << endl;
        quit();
    }
    job->deleteLater();
}

void KSecretsApp::slotReadItemAttributesDone(KJob* job)
{
    ReadItemPropertyJob *readJob = qobject_cast< ReadItemPropertyJob* >(job);
    if ( readJob->error() == 0 ) {
        ReadItemPropertyJob *readLabelJob = readJob->secretItem()->label();
        QVariant varAttrs;
        varAttrs.setValue< StringStringMap >( readJob->propertyValue().value< StringStringMap >() );
        readLabelJob->setCustomData( varAttrs );
        connect( readLabelJob, SIGNAL(finished(KJob*)), this, SLOT(slotReadItemLabelDone(KJob*)) );
        readLabelJob->start();
    }
    else {
        cerr << readJob->errorString() << endl;
    }
    job->deleteLater();
}

void KSecretsApp::slotReadItemLabelDone(KJob* job)
{
    ReadItemPropertyJob *readJob = qobject_cast< ReadItemPropertyJob* >(job);
    if ( readJob->error() == 0 ) {
        cout << readJob->propertyValue().toString() << "" << endl;
        cout << "    " << ki18n("attributes:") << endl;
        StringStringMap attrs = readJob->customData().value< StringStringMap >();
        StringStringMap::ConstIterator it = attrs.constBegin();
        for ( ; it != attrs.constEnd(); ++it ) {
            cout << "        " << it.key() << " = " << it.value() << endl;
        }
    }
    else {
        cerr << readJob->errorString() << endl;
    }
}

void KSecretsApp::slotReadAllItemsDone(KJob* job)
{
    if ( job->error() ) {
        cerr << job->errorString() << endl;
    }
    quit();
}


#include "ksecretsapp.moc"
