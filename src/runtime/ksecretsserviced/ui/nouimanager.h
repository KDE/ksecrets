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

#ifndef NOUIMANAGER_H
#define NOUIMANAGER_H

#include "abstractuijobs.h"
#include "abstractuimanager.h"

class NoUiAskPasswordJob : public AbstractAskPasswordJob
{
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * @param manager the manager creating this job
     * @param cancel if true, this job will be cancelled
     */
    NoUiAskPasswordJob(AbstractUiManager *manager, bool cancel);

    /**
     * Destructor.
     */
    virtual ~NoUiAskPasswordJob();

protected:
    /**
     * Does basically nothing - no UI, no password to enter.
     */
    virtual void start();

private Q_SLOTS:
    /**
     * Called from start to emit the result signal.
     */
    void finish();

private:
    bool m_cancel;
};

class NoUiNewPasswordJob : public AbstractNewPasswordJob
{
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * @param manager the manager creating this job
     * @param cancel if true, this job will be cancelled
     */
    NoUiNewPasswordJob(AbstractUiManager *manager, bool cancel);

    /**
     * Destructor.
     */
    virtual ~NoUiNewPasswordJob();

protected:
    /**
     * Does basically nothing - no UI, no new password to enter.
     */
    virtual void start();

private Q_SLOTS:
    /**
     * Called from start to emit the result signal.
     */
    void finish();

private:
    bool m_cancel;
};

/**
 * Implement AbstractUiManager to provide ui jobs which don't show a
 * user interface. While this might seem paradox at first look, it's highly
 * useful when implementing automatic unit-tests.
 *
 * @remarks a password reported by this ui manager's jobs will always have
 *          the value "default".
 */
class NoUiManager : public AbstractUiManager
{
public:
    /**
     * Constructor.
     */
    NoUiManager();

    /**
     * Destructor.
     */
    virtual ~NoUiManager();

    /**
     * Create a job to ask for a user's password to unlock a collection.
     */
    virtual AbstractAskPasswordJob *createAskPasswordJob(const QString &collection,
            bool secondTry);

    /**
     * Create a job to as a user for a new password for a collection.
     */
    virtual AbstractNewPasswordJob *createNewPasswordJob(const QString &collection);

    /**
     * @see AbstractUiManager::createAskPasswordJob
     */
    virtual AbstractAskAclPrefsJob *createAskAclPrefsJob(const CollectionUnlockInfo &unlockCollectionInfo);
    virtual AbstractAskAclPrefsJob *createAskAclPrefsJob(const CollectionCreateInfo &createCollectionInfo);


    /**
     * Set to true to cancel all created jobs.
     */
    void setCancelAll(bool cancelAll);

    /**
     * True if all created jobs are automatically cancelled, false else.
     */
    bool cancelAll() const;

private:
    bool m_cancelAll;
};

#endif
