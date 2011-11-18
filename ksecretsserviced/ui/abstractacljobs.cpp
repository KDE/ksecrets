/*
 * Copyright 2010, Valentin Rusu <kde@rusu.info>
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

#include "abstractacljobs.h"
#include <backend/backendcollection.h>
#include <QtGui/QDialog>

AbstractAskAclPrefsJob::AbstractAskAclPrefsJob(AbstractUiManager *manager,
        const CollectionCreateInfo &jobInfo) :
    AbstractUiJob(manager),
    m_denied(false),
    m_permission(PermissionUndefined)
{
    m_jobHelper = new AskAclPrefsJobHelper< CollectionCreateInfo >(jobInfo);
}

AbstractAskAclPrefsJob::AbstractAskAclPrefsJob(AbstractUiManager *manager,
        const CollectionUnlockInfo &jobInfo) :
    AbstractUiJob(manager),
    m_denied(false),
    m_permission(PermissionUndefined)
{
    m_jobHelper = new AskAclPrefsJobHelper< CollectionUnlockInfo >(jobInfo);
}


template <> 
QString AskAclPrefsJobHelper< CollectionCreateInfo >::collectionLabel() const {
    return m_jobInfo.m_label;
}

template <> 
QString AskAclPrefsJobHelper< CollectionCreateInfo >::collectionCreatorApplication() const {
    return m_jobInfo.m_peer.exePath();
}

template <>
QString AskAclPrefsJobHelper< CollectionUnlockInfo >::collectionLabel() const {
    return m_jobInfo.m_collection->label().value();
}

template <> 
QString AskAclPrefsJobHelper< CollectionUnlockInfo >::collectionCreatorApplication() const {
    return m_jobInfo.m_collection->creatorApplication();
}

#include "abstractacljobs.moc"
