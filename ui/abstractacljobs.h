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

#ifndef ABSTRACTACLJOBS_H
#define ABSTRACTACLJOBS_H

#include "abstractuijobs.h"
#include <jobinfostructs.h>
#include <acl.h>

class AskAclPrefsJobHelperBase
{
public:
    AskAclPrefsJobHelperBase() {}
    virtual ~AskAclPrefsJobHelperBase() {}

    virtual const Peer &peer() const = 0;
    virtual QString collectionLabel() const = 0;
    virtual QString collectionCreatorApplication() const =0;
};

template <class INFO>
class AskAclPrefsJobHelper : public AskAclPrefsJobHelperBase
{
public:
    AskAclPrefsJobHelper(const INFO &jobInfo) : m_jobInfo(jobInfo) {}

    virtual const Peer &peer() const {
        return m_jobInfo.m_peer;
    }
    
    virtual QString collectionLabel() const {
        throw 1; // this must be implemented via template specialization
    }

    virtual QString collectionCreatorApplication() const {
        throw 1; // this must be implemented via template specialization
    }
    
private:
    INFO m_jobInfo;
};

template <> QString AskAclPrefsJobHelper< CollectionCreateInfo >::collectionLabel() const;

template <> QString AskAclPrefsJobHelper< CollectionUnlockInfo >::collectionLabel() const;
template <> QString AskAclPrefsJobHelper< CollectionUnlockInfo >::collectionCreatorApplication() const;

/**
 * Job that asks ACL handling user preferences
 */
class AbstractAskAclPrefsJob : public AbstractUiJob
{
    Q_OBJECT

public:
    AbstractAskAclPrefsJob(AbstractUiManager *manager, const CollectionCreateInfo &jobInfo);
    AbstractAskAclPrefsJob(AbstractUiManager *manager, const CollectionUnlockInfo &jobInfo);

    bool denied() const {
        return m_denied;
    }
    void setDenied(bool denied) {
        m_denied = denied;
    }
    const AskAclPrefsJobHelperBase *jobHelper() const { 
        return m_jobHelper; 
    }
    void setPermission( ApplicationPermission permission ) {
        m_permission = permission;
    }
    ApplicationPermission permission() const {
        return m_permission;
    }

private:
    AskAclPrefsJobHelperBase*  m_jobHelper;
    bool m_denied;
    ApplicationPermission m_permission;
};

#endif // ABSTRACTACLJOBS_H
