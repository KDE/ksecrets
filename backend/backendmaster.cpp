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

#include "backendmaster.h"
#include "backendcollection.h"
#include "backendcollectionmanager.h"

#include <ui/abstractuimanager.h>

#include <QtCore/QEventLoop>
#include <QtCore/QCoreApplication>

K_GLOBAL_STATIC(BackendMaster, s_backendMaster)
bool BackendMaster::s_initialized = false;

BackendMaster::BackendMaster()
    : m_uiManager(0)
{
}

BackendMaster::~BackendMaster()
{
    qRemovePostRoutine(s_backendMaster.destroy);
    delete m_uiManager;
}

BackendMaster *BackendMaster::instance()
{
    if(!s_initialized) {
        s_initialized = true;
        qAddPostRoutine(s_backendMaster.destroy);
    }
    return s_backendMaster;
}

void BackendMaster::addManager(BackendCollectionManager *manager)
{
    Q_ASSERT(manager);
    connect(manager, SIGNAL(collectionCreated(BackendCollection*)),
            SLOT(slotCollectionCreated(BackendCollection*)));
    connect(manager, SIGNAL(collectionDeleted(BackendCollection*)),
            SLOT(slotCollectionDeleted(BackendCollection*)));
    m_collectionManagers.append(manager);
}

void BackendMaster::removeManager(BackendCollectionManager *manager)
{
    manager->disconnect(SIGNAL(collectionCreated(BackendCollection*)), this,
                        SLOT(slotCollectionCreated(BackendCollection*)));
    manager->disconnect(SIGNAL(collectionDeleted(BackendCollection*)), this,
                        SLOT(slotCollectionDeleted(BackendCollection*)));
    m_collectionManagers.removeAll(manager);
}

const QList<BackendCollection*> &BackendMaster::collections() const
{
    return m_collections;
}

QList<BackendCollectionManager*> &BackendMaster::managers()
{
    return m_collectionManagers;
}

void BackendMaster::setUiManager(AbstractUiManager *uiManager)
{
    delete m_uiManager;
    m_uiManager = uiManager;
}

AbstractUiManager *BackendMaster::uiManager()
{
    Q_ASSERT(m_uiManager);
    return m_uiManager;
}

CreateCollectionMasterJob *BackendMaster::createCreateCollectionMasterJob(
    const CollectionCreateInfo& createCollectionInfo)
{
    return new CreateCollectionMasterJob(createCollectionInfo, this);
}

void BackendMaster::slotCollectionCreated(BackendCollection *collection)
{
    Q_ASSERT(collection);
    m_collections.append(collection);
    connect(collection, SIGNAL(collectionChanged(BackendCollection*)),
            SIGNAL(collectionChanged(BackendCollection*)));
    emit collectionCreated(collection);
}

void BackendMaster::slotCollectionDeleted(BackendCollection *collection)
{
    Q_ASSERT(collection);
    m_collections.removeAll(collection);
    emit collectionDeleted(collection);
}

#include "backendmaster.moc"
