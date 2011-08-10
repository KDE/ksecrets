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

#ifndef ABSTRACTUIMANAGER_H
#define ABSTRACTUIMANAGER_H

#include "abstractuijobs.h"
#include "abstractacljobs.h"

/**
 * Abstract factory that provides means to create a user interface for the various
 * backend classes.
 *
 * This class can be reimplemented to provide a standard UI, a stripped-down UI
 * (probably for mobile devices) or even a bogus UI which can be used inside
 * unit-tests.
 */
class AbstractUiManager /* : public JobQueue */
{
public:
    virtual ~AbstractUiManager() {
        // TODO: clean-up
    };

    /**
     * Create a job for asking for user's password for unlocking a collection.
     *
     * @param collection label of the collection to be unlocked
     * @param secondTry set to true if the user already entered a wrong password before
     * @return a job which can be enqueued to ask the user for the unlock password
     */
    virtual AbstractAskPasswordJob *createAskPasswordJob(const QString &collection,
            bool secondTry) = 0;

    /**
     * Create a job for asking a user for a new password for a collection.
     *
     * @param collection label of the collection to change the password for
     * @return a job which can be enqueued to ask the user for a new password
     */
    virtual AbstractNewPasswordJob *createNewPasswordJob(const QString &collection) = 0;

    /**
     * Create a job for asking a user for his or her ACL handling preferences
     *
     * @param createCollectionInfo contextual information about collection creation job
     * @return a job which can be enqueued to ask for user ACL preferences
     */
    virtual AbstractAskAclPrefsJob *createAskAclPrefsJob(const CollectionUnlockInfo &unlockCollectionInfo) = 0;
    virtual AbstractAskAclPrefsJob *createAskAclPrefsJob(const CollectionCreateInfo &createCollectionInfo) = 0;
};

#endif
