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

#ifndef TEMPBLOCKINGCOLLECTIONMANAGER_H
#define TEMPBLOCKINGCOLLECTIONMANAGER_H

#include "backend/backendcollectionmanager.h"

class TempBlockingCollection;
class KJob;

// implement a temporary collection manager that blocks every call.
class TempBlockingCollectionManager : public BackendCollectionManager
{
    Q_OBJECT

public:
    friend class TempBlockingCollection;

public:
    TempBlockingCollectionManager(QObject *parent = 0);
    virtual ~TempBlockingCollectionManager();
    virtual CreateCollectionJob *createCreateCollectionJob(const CollectionCreateInfo &createCollectionInfod);

private Q_SLOTS:
    void createCollectionJobResult(KJob *job);
};

#endif
