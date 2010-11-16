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

#ifndef DIALOGUIMANAGER_H
#define DIALOGUIMANAGER_H

#include "abstractuijobs.h"
#include "abstractuimanager.h"

class KPasswordDialog;
class KNewPasswordDialog;

class DialogAskPasswordJob : public AbstractAskPasswordJob
{
    Q_OBJECT

public:
    /**
     * Constructor.
     */
    DialogAskPasswordJob(AbstractUiManager *manager, const QString &collection, bool secondTry);

    /**
     * Destructor.
     */
    ~DialogAskPasswordJob();

protected:
    /**
     * Contains the actual workload of showing the dialog.
     */
    virtual void start();

private Q_SLOTS:
    /**
     * Called when the dialog shown is either accepted or rejected.
     */
    void dialogFinished(int result);

private:
    KPasswordDialog *m_dialog;
};

class DialogNewPasswordJob : public AbstractNewPasswordJob
{
    Q_OBJECT

public:
    /**
     * Constructor.
     */
    DialogNewPasswordJob(AbstractUiManager *manager, const QString &collection);

    /**
     * Destructor.
     */
    ~DialogNewPasswordJob();

protected:
    /**
     * Contains the actual workload of showing the dialog.
     */
    virtual void start();

private Q_SLOTS:
    /**
     * Called when the dialog shown is either accepted or rejected.
     */
    void dialogFinished(int result);

private:
    KNewPasswordDialog *m_dialog;
};

/**
 * Implement AbstractUiManager to provide a user interface using dialogs.
 */
class DialogUiManager : public AbstractUiManager
{
public:
    /**
     * Destructor.
     */
    virtual ~DialogUiManager() {};

    /**
     * Create a job to ask for a user's password to unlock a collection.
     */
    virtual AbstractAskPasswordJob *createAskPasswordJob(const QString &collection,
            bool secondTry);

    /**
     * Create a job to ask a user for a new password for a collection.
     */
    virtual AbstractNewPasswordJob *createNewPasswordJob(const QString &collection);

    /**
     * Create a job to ask a user his or her preferences for ACL handling
     * @see AbstractUiManager::createAskAclPrefsJob
     */
    virtual AbstractAskAclPrefsJob *createAskAclPrefsJob(const CollectionUnlockInfo &unlockCollectionInfo);
    virtual AbstractAskAclPrefsJob *createAskAclPrefsJob(const CollectionCreateInfo &createCollectionInfo);

};

#endif
