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

#ifndef BACKENDMASTER_H
#define BACKENDMASTER_H

#include "backendreturn.h"
#include "backendjob.h"

#include <QtCore/QObject>
#include <QtCore/QList>

class BackendCollection;
class BackendCollectionManager;
class AbstractUiManager;

/**
 * @todo find a better name for this class
 *
 * Objects of this class contain all instantiated CollectionManagers and
 * Collections and serve as an entry point to implementing services based on the
 * backend.
 *
 * @remarks this class exists to allow several different frontends to operate on
 *          the same backends.
 */
class BackendMaster : public QObject
{
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * @remarks only called by K_GLOBAL_STATIC
     */
    BackendMaster();

    /**
     * Destructor
     */
    virtual ~BackendMaster();

    /**
     * Get the singleton BackendMaster instance.
     */
    static BackendMaster *instance();

    /**
     * Adds a new collection manager to the list of known managers.
     *
     * @param manager New collection manager to add
     */
    void addManager(BackendCollectionManager *manager);

    /**
     * Remove a collection manager from the list of known managers.
     *
     * @param manager collection manager to remove
     */
    void removeManager(BackendCollectionManager *manager);

    /**
     * Get the list of registered collection managers
     *
     * @return the list of registered collection managers
     */
    QList<BackendCollectionManager*> &managers();

    /**
     * Get the list of known collections.
     *
     * @return The list of known collections
     */
    const QList<BackendCollection*> &collections() const;

    /**
     * Create a job for creating a new collection. The job decides which manager
     * to use for creation (usually based on user input).
     *
     * @param label the label of the new collection
     * @param lock if true, the collection should be locked after creation,
     *             if false it should be unlocked
     * @return the collection creation job
     */
    CreateCollectionMasterJob *createCreateCollectionMasterJob(const CollectionCreateInfo& createCollectionInfo);

    /**
     * Set the user interface manager to use.
     *
     * @remarks Any previously set user interface manager will be deleted.
     *          This MUST be set before using any of the backend methods.
     */
    void setUiManager(AbstractUiManager *manager);

    /**
     * Get the user interface manager to use.
     */
    AbstractUiManager *uiManager();

private Q_SLOTS:
    /**
     * Notify the Master about a new collection being available.
     *
     * @param collection New collection to add to the list of known collections
     */
    void slotCollectionCreated(BackendCollection *collection);

    /**
     * Notify the Master about a collection getting unavailable.
     *
     * @param collection Collection to remove from the list of known collections
     */
    void slotCollectionDeleted(BackendCollection *collection);

Q_SIGNALS:
    /**
     * Notify a listening service about creation of a collection.
     *
     * @param collection New collection that is now available
     */
    void collectionCreated(BackendCollection *collection);

    /**
     * Notify a listening service about deletion of a collection.
     *
     * @param collection Existing collection that will be removed
     */
    void collectionDeleted(BackendCollection *collection);

    /**
     * Notify a listening service about a collection that changed
     *
     * @param collection Collection that changed
     */
    void collectionChanged(BackendCollection *collection);

private:
    QList<BackendCollection*> m_collections;
    QList<BackendCollectionManager*> m_collectionManagers;

    /**
     * @note QtCrypto doc states that QCA::Initializer should not go out
     * of scope all the time QCA objects are used. As the backend master
     * is destroyed by a qAddPostRoutine, then it'll get destroyed after
     * a main() local variable and that will make the daemon crash on exit.
     * So keep this initializer here, to get it destroyed along with the manager
     */
    QCA::Initializer qcaInit;
    AbstractUiManager *m_uiManager;
    static bool s_initialized;
};

#endif
