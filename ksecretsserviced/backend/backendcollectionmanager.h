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

#ifndef BACKENDCOLLECTIONMANAGER_H
#define BACKENDCOLLECTIONMANAGER_H

#include "backendreturn.h"

#include <QtCore/QObject>

class CreateCollectionJob;
class BackendCollection;
class CollectionCreateInfo;

/**
 * Abstract base class for collection managers.
 *
 * A collection manager is a class that detects the availability of
 * collections and makes them available to the frontend by registering
 * them with the Master object.
 *
 * When inheriting from BackendCollectionManager it's usually sufficient to define the
 * pure virtual methods. The inheriting class is responsible for emitting the
 * \sa collectionCreated, \sa collectionDeleted and \sa collectionChanged signals.
 */
class BackendCollectionManager : public QObject
{
    Q_OBJECT

public:
    /**
     * Constructor
     *
     * @param parent parent object
     */
    BackendCollectionManager(QObject *parent = 0);

    /**
     * Destructor
     */
    virtual ~BackendCollectionManager();

    /**
     * Create a job for creating a new collection.
     *
     * @param label the label of the new collection
     * @param lock if true the collection should be locked after creation,
     *             if false it should be unlocked
     */
    virtual CreateCollectionJob *createCreateCollectionJob(const CollectionCreateInfo& createCollectionInfo) = 0;
    
    /**
     * This is called by the collection creation job to notify this manager that
     * actual collection creation will start
     */
    virtual void creatingCollection( const QString& collId );

Q_SIGNALS:
    /**
     * Issued by the manager when a new collection is created or an existing
     * collection is discovered.
     *
     * @param collection collection created or discovered
     */
    void collectionCreated(BackendCollection *collection);

    /**
     * Issued by the manager when a collection is removed from
     * the list of known collections.
     *
     * @param collection collection removed
     * @remarks the slot called is the last place a collection
     *          can be safely accessed.
     */
    void collectionDeleted(BackendCollection *collection);

    /**
     * Issued by the manager when a collection is changed.
     *
     * @param collection collection that changed
     */
    void collectionChanged(BackendCollection *collection);
    
protected:
    QString     m_creatingCollectionId; // id 
};

#endif
