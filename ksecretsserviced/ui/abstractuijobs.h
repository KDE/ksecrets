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

#ifndef ABSTRACTUIJOBS_H
#define ABSTRACTUIJOBS_H

#include <QtCore/QQueue>
#include <QtCrypto>
#include <kglobal.h>
#include <kjob.h>
#include <kcompositejob.h>
#include "../lib/daemonjob.h"

class AbstractUiManager;

/*
 * Abstract master-class of user interface jobs.
 */
class AbstractUiJob : public DaemonJob
{
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * @param manager user interface job manager and parent object
     */
    explicit AbstractUiJob(AbstractUiManager *manager);

    /**
     * Destructor.
     */
    virtual ~AbstractUiJob();

    /**
     * Default implementation for user interface jobs. As a ui job can rarely be
     * run synchronously, exec() crashes.
     */
    virtual void exec();
    /**
     * Check if the dialog was cancelled by the user.
     *
     * @return true if the user cancelled false else
     */
    bool cancelled() const { 
        return m_cancelled; 
    }

    // TODO: get rid of isImmediate usage in the code, all jobs should be async
    bool isImmediate() const {
        return false;
    }
    
protected:
    /**
     * Set if the user cancels the UI operation.
     *
     * @param cancelled true if the user cancelled the dialog, false if not
     */
    void setCancelled(bool cancelled =true) {
        m_cancelled = cancelled;
    }
    
private:
    friend class UiJobManager;
    bool m_cancelled;              // true if the operation was cancelled
};

/**
 * Job that asks a user for a password to unlock a collection.
 */
class AbstractAskPasswordJob : public AbstractUiJob
{
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * @param manager ui job manager and parent object
     * @param collection label of the collection that should be opened
     * @param secondTry set to true if this is not the user's first try to enter
     *                  the password correctly
     */
    AbstractAskPasswordJob(AbstractUiManager *manager, const QString &collection,
                           bool secondTry);

    /**
     * Destructor.
     */
    virtual ~AbstractAskPasswordJob();

    /**
     * Get the label of the collection that's about to be unlocked.
     */
    const QString &collection() const;

    /**
     * Check whether this is not the user's first try to enter the password.
     */
    bool isSecondTry() const;

    /**
     * Get the password the user entered.
     *
     * @return the password entered by the user or an empty array if the user
     *         didn't enter a password
     */
    const QCA::SecureArray &password() const;

protected:

    /**
     * Set the password entered by the user.
     *
     * @remarks This is used by derived classes
     * @param password password entered by the user
     */
    void setPassword(const QCA::SecureArray &password);

private:
    QString m_collection;          // name of the collection to be unlocked
    bool m_secondTry;              // true if this is not the first try
    QCA::SecureArray m_password;   // the password entered by the user
};

/**
 * Job that asks a user for a new password for a collection.
 */
class AbstractNewPasswordJob : public AbstractUiJob
{
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * @param manager ui job manager and parent object
     * @param collection label of the collection the password is for
     */
    AbstractNewPasswordJob(AbstractUiManager *manager, const QString &collection);

    /**
     * Destructor.
     */
    virtual ~AbstractNewPasswordJob();

    /**
     * Get the label of the collection the password is for.
     */
    const QString &collection() const;

    /**
     * Get the password the user entered.
     *
     * @return the password entered by the user or an empty array if the user
     *         didn't enter a password
     */
    const QCA::SecureArray &password() const;

protected:

    /**
     * Set the password entered by the user.
     */
    void setPassword(const QCA::SecureArray &password);

private:
    QString m_collection;          // name of the collection the password is for
    QCA::SecureArray m_password;   // the new password entered by the user
};

#endif
