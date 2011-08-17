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

#include "kio_ksecretsservice.h"

#include <kcomponentdata.h>
#include <kdebug.h>
#include "client/ksecretsservicecollection.h"
#include "client/ksecretsservicecollectionjobs.h"
#include <QCoreApplication>

using namespace KSecretsService;

extern "C" int KDE_EXPORT kdemain( int argc, char **argv ) 
{
    kDebug() << "Entering kio_ksecretsservice";
    
    KComponentData instance( "kio_ksecretsservice" );
    QCoreApplication app( argc, argv ); // need this for jobs event loop
    
    if ( argc != 4 ) {
        fprintf( stderr, "Usage: kio_ksecretsservice protocol domain-socket1 domain-socket2\n" );
        exit( -1 );
    }
    
    Secrets slave( argv[2], argv[3] );
    slave.dispatchLoop();
    return 0;
}


// code borrowed from kio_settings.cpp
static void createDirEntry(KIO::UDSEntry& entry, const QString& name, const QString& iconName)
{
    entry.clear();
    entry.insert( KIO::UDSEntry::UDS_NAME, name );
    entry.insert( KIO::UDSEntry::UDS_FILE_TYPE, S_IFDIR );
    entry.insert( KIO::UDSEntry::UDS_ACCESS, 0500 );
    entry.insert( KIO::UDSEntry::UDS_MIME_TYPE, "inode/directory" );
    entry.insert( KIO::UDSEntry::UDS_ICON_NAME, iconName );
}

Secrets::Secrets(const QByteArray& pool, const QByteArray& app): 
    SlaveBase("secrets", pool, app)
{

}

void Secrets::listDir(const KUrl& url)
{
    kDebug() << "Entering listDir " << url.url();
    initClientLib();
    const QString fileName = url.fileName();
    kDebug() << fileName;

    int itemCount =0;
    // root dir?
    if ( fileName.isEmpty() ) {
        ListCollectionsJob *listJob = Collection::listCollections();
        if (listJob->exec()) {
            KIO::UDSEntry entry;
            foreach( const QString &collName, listJob->collections() ) {
                kDebug() << "collection : " << collName;
                entry.clear();
                createDirEntry( entry, collName, "wallet-open" );
                // TODO: add access and modification times
                entry.insert( KIO::UDSEntry::UDS_COMMENT, i18n("Secrets collection stored in KSecretsService") );
                listEntry( entry, false );
            }
            totalSize( listJob->collections().count() );
            entry.clear();
            listEntry( entry, true ); // finished enumerating, we're ready
            finished();
        }
        else {
            kDebug() << "Cannot list collections : " << listJob->errorString();
            error( KIO::ERR_COULD_NOT_CONNECT, i18n("Could not connect to KSecretsService daemon") );
        }
    }
    else {
        error( KIO::ERR_DOES_NOT_EXIST, url.prettyUrl() );
    }
}

void Secrets::get(const KUrl& url)
{
    const QString fileName = url.fileName();
    kDebug() << "Entering get URL=" << url.url() << " FILE=" << fileName;
    if (fileName.isEmpty()) {
        error( KIO::ERR_IS_DIRECTORY, url.prettyUrl() );
    }
    else {
        data(QByteArray());
        finished();
    }
}

void Secrets::stat(const KUrl& url)
{
    kDebug() << "Entering stat " << url.url();
    initClientLib();
    const QString fileName = url.fileName();
    kDebug() << fileName;
    
    KIO::UDSEntry entry;
    
    // root dir?
    if (fileName.isEmpty()) {
        createDirEntry( entry, ".", "kwalletmanager");
        statEntry( entry );
        finished();
        return;
    }
    
    error( KIO::ERR_DOES_NOT_EXIST, url.url() );
}

void Secrets::initClientLib()
{
}

#include "kio_ksecretsservice.moc"
