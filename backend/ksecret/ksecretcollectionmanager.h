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

#ifndef KSECRETCOLLECTIONMANAGER_H
#define KSECRETCOLLECTIONMANAGER_H

#include "../backendcollectionmanager.h"

#include <kdirwatch.h>

#include <QtCore/QMap>

class KJob;
class KSecretCollection;
class KSecretCreateCollectionJob;

/**
 * Manager for collections stored in ksecret files.
 */
class KSecretCollectionManager : public BackendCollectionManager
{
    Q_OBJECT

public:
    /**
     * Constructor
     *
     * @param parent parent object
     */
    explicit KSecretCollectionManager(const QString &path, QObject *parent = 0);

    /**
     * Destructor
     */
    virtual ~KSecretCollectionManager();

    /**
     * Create a call for creating a new collection.
     *
     * @param label the label of the new collection
     * @param lock if true, the collection should be locked after creation,
     *             if false it should stay unlocked
     */
    virtual CreateCollectionJob *createCreateCollectionJob(const CollectionCreateInfo &createCollectionInfod);

protected:
    /**
     * This methods adds a newly created collection to the manager.
     *
     * @param collection collection to add
     */
    void addCollection(KSecretCollection *coll);

private Q_SLOTS:
    /**
     * Connected to a filesystem watcher this slot is called whenever
     * a ksecret file is added or removed.
     *
     * @param path directory that changed
     */
    void slotDirectoryChanged(const QString &path);

    /**
     * Collection discovery method which can be called using a single-shot timer.
     */
    void slotStartupDiscovery();

    /**
     * Called when a CreateCollectionJob run by this manager finishes.
     *
     * @param job the job that finished
     */
    void createCollectionJobResult(KJob *job);

private:
    friend class KSecretCreateCollectionJob;

    // filesystem watcher to detect new/removed ksecret files
    KDirWatch m_watcher;

    // map of paths pointing to the respective collection objects
    QMap<QString, KSecretCollection*> m_collections;
};

#endif
