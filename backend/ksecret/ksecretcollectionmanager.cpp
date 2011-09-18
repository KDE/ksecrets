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

#include "ksecretcollectionmanager.h"
#include "ksecretcollection.h"
#include "ksecretjobs.h"

#include "../lib/secrettool.h"
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#include <QtCore/QTimer>
#include <QtCore/QDir>

KSecretCollectionManager::KSecretCollectionManager(const QString &path, QObject *parent)
    : BackendCollectionManager(parent), m_watcher(this)
{
    KGlobal::dirs()->addResourceType("ksecret", 0, path);
    // FIXME: what is this watcher's utility indeed? who's intended to created collections under our back?
//     m_watcher.addDir(KGlobal::dirs()->saveLocation("ksecret"));
//     connect(&m_watcher, SIGNAL(dirty(QString)), SLOT(slotDirectoryChanged(QString)));
//     m_watcher.startScan();
    // list directory contents to discover existing collections on startup
    QTimer::singleShot(0, this, SLOT(slotStartupDiscovery()));
}

KSecretCollectionManager::~KSecretCollectionManager()
{
    // TODO: cleanup?
}

CreateCollectionJob *KSecretCollectionManager::createCreateCollectionJob(const CollectionCreateInfo &createCollectionInfo)
{
    KSecretCreateCollectionJob *job = new KSecretCreateCollectionJob(createCollectionInfo, this);
    connect(job, SIGNAL(result(KJob*)),
            SLOT(createCollectionJobResult(KJob*)));
    return job;
}

void KSecretCollectionManager::addCollection(KSecretCollection *collection)
{
    m_collections.insert(collection->path(), collection);
    emit collectionCreated( collection );
    m_creatingCollectionId = "";
}

void KSecretCollectionManager::slotDirectoryChanged(const QString &path)
{
    // list all collections in the directory and check if there's a collection
    // which we don't know yet.
    QDir dir(path);
    QStringList entries = dir.entryList(QStringList("*.ksecret"), QDir::Files);
    Q_FOREACH(const QString & file, entries) {
        QFileInfo fi( dir.filePath( file ) );
        // avoid double inclusion and also avoid trying to read collections that are in the process of serialization
        if(!m_collections.contains(file) && !(m_creatingCollectionId == fi.baseName() ) ) {
            kDebug() << "FILE: " << file << " deserializing...";
            QString errorMessage;
            KSecretCollection *coll = KSecretCollection::createFromFile( dir.filePath(file), this, errorMessage);
            if(coll) {
                addCollection(coll);
                kDebug() << "   ...DONE";
            }
            else {
                kDebug() << " ERROR: " << errorMessage;
            }
        }
    }
}

void KSecretCollectionManager::slotStartupDiscovery()
{
    kDebug() << "Discovering collections...";
    slotDirectoryChanged(KGlobal::dirs()->saveLocation("ksecret"));
}

void KSecretCollectionManager::createCollectionJobResult(KJob *job)
{
    KSecretCreateCollectionJob *ccj = qobject_cast<KSecretCreateCollectionJob*>(job);
    Q_ASSERT(ccj);
    if ( !ccj->isDismissed() ) {

        connect(ccj->collection(), SIGNAL(collectionDeleted(BackendCollection*)),
                SIGNAL(collectionDeleted(BackendCollection*)));
        connect(ccj->collection(), SIGNAL(collectionChanged(BackendCollection*)),
                SIGNAL(collectionChanged(BackendCollection*)));
    }
    else {
        // FIXME: what should we do here, when job is dismissed by the user via the cancel button ?
    }
}

#include "ksecretcollectionmanager.moc"
