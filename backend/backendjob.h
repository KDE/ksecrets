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

#ifndef BACKENDJOB_H
#define BACKENDJOB_H

#include <QtCore/QMap>
#include <qca.h>
#include <qwindowdefs.h>
#include "../jobinfostructs.h"
#include "backendreturn.h"
#include <kcompositejob.h>
#include "../lib/daemonjob.h"

/**
 * Queued job for implementing various backend actions which need queueing.
 */
class BackendJob : public DaemonJob
{
    Q_OBJECT

public:
    /**
     * Types of jobs.
     */
    enum JobType {
        TypeCreateCollectionMaster,
        TypeCreateCollection,
        TypeUnlockCollection,
        TypeLockCollection,
        TypeDeleteCollection,
        TypeChangeAuthenticationCollection,
        TypeCreateItem,
        TypeDeleteItem,
        TypeLockItem,
        TypeUnlockItem,
        TypeChangeAuthenticationItem,
        TypeMultiPrompt
    };

    /**
     * Constructor.
     *
     * @param type the type of job
     * @param queue the queue object this call will be enqueued to
     * @todo add WId parameter in a way that makes sense (!)
     */
    BackendJob(JobType type);

    /**
     * Default implementation for isImmediate() which unconditionally returns
     * true. This is for convenience reasons are many of the inheriting classes
     * implement jobs immediately.
     *
     * @warning this MUST be reimplemented if the job being implemented is not
     *          always immediate or needs special handling.
     */
    virtual bool isImmediate() const;

    /**
     * Get the job's type.
     *
     * @return the job's type
     */
    JobType type() const;

    /**
     * Check if this call has been dismissed.
     *
     * @return true if this call has been dismissed, false else
     */
    bool isDismissed() const;
    
    /**
     * Dismiss this job.
     */
    void dismiss();

    /**
     * Check if the job had an error.
     *
     * @return the error or NoError if there was no error
     */
    ErrorType error() const;

    /**
     * Get the error message if there is any.
     *
     * @return the error message or an empty string if there is none
     */
    const QString &errorMessage() const;

    /**
     * Default implementation for start() 
     *
     * @warning this MUST be reimplemented if the job being implemented is not
     *          always immediate or needs special handling.
     */
    virtual void start();
    
protected:
    /**
     * Set the job result to be an error.
     *
     * @param error error code
     * @param errorMessage error message
     */
    void setError(ErrorType error, const QString &errorMessage = QString());

private:
    JobType m_type;
    bool m_dismissed;
    ErrorType m_error;
    QString m_errorMessage;
};

class BackendMaster;
class BackendCollectionManager;
class BackendCollection;
class BackendItem;

/**
 * Queued job class for creating a collection.
 *
 * This is the job that operates on the collection manager chosen before.
 */
class CreateCollectionJob : public BackendJob
{
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * @param label human-readable label for the new collection
     * @param locked true if the collection should be locked after creation,
     *               false else
     * @param manager the collection manager that should create the collection
     */
    CreateCollectionJob(const CollectionCreateInfo &createCollectionInfo,
                        BackendCollectionManager *manager);

    /**
     * The collection that was created.
     *
     * @return the collection created or 0 in case of an error
     */
    BackendCollection *collection() const { 
        return m_collection; 
    }

protected:
    /**
     * Get the label for the new collection.
     */
    const QString &label() const;

    /**
     * Check whether the new collection should be created locked.
     */
    bool locked() const;

    /**
     * The manager to call for creating the collection.
     */
    BackendCollectionManager *manager() const;

    /**
     * Set the collection created of the job.
     */
    void setCollection(BackendCollection *collection);

    /**
     * Get the create collection information structure
     */
    const CollectionCreateInfo &createCollectionInfo() const {
        return m_createCollectionInfo;
    }

private:
    CollectionCreateInfo m_createCollectionInfo; // let child classes access this member

    BackendCollectionManager *m_manager;

    BackendCollection *m_collection;
};

/**
 * Queued job class for creating a collection.
 *
 * This is the job that operates on the BackendMaster. When a collection
 * manager for creating the collection is chosen, this job creates a
 * CreateCollectionJob and calls it on the manager that was chosen.
 */
class CreateCollectionMasterJob : public BackendJob
{
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * @param label human-readable label for the new collection
     * @param locked true if the collection should be locked after creation,
     *               false else
     * @param master the master used to create the collection
     */
    CreateCollectionMasterJob(const CollectionCreateInfo& createCollectionInfo,
                              BackendMaster *master);

    /**
     * Destructor.
     */
    virtual ~CreateCollectionMasterJob();

    /**
     * Checks if this call can be made immediately/synchronously.
     *
     * @return true if the call can be made immediately, false if it needs to
     *         be queued
     */
    virtual bool isImmediate() const;

    /**
     * Get the collection created by the job.
     *
     * @return the BackendCollection which was created or 0 on failure.
     */
    BackendCollection *collection() const;

protected:
    /**
     * Start the job asynchronously.
     */
    virtual void start();

private Q_SLOTS:
    /**
     * Called when the CreateCollectionJob sent to the collection manager
     * is finished.
     *
     * @param subJob the sub job that finished
     */
    void createCollectionJobFinished(KJob*);

private:
    CollectionCreateInfo m_createCollectionInfo;
    BackendMaster       *m_master;
    KJob                *m_subJob;
    BackendCollection   *m_collection;
};

/**
 * Base class for jobs implementing parameterless functionality and returning
 * a boolean value.
 */
class BooleanResultJob : public BackendJob
{
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * @param type concrete type of the job
     */
    BooleanResultJob(BackendJob::JobType type);

    /**
     * Get the boolean result of the job.
     *
     * @return the result of the job
     */
    bool result() const;

protected:
    /**
     * Set the result of the job.
     *
     * @param result the job's result
     * @remarks this is called by classes implementing the jobs
     */
    void setResult(bool result);

private:
    bool m_result;
};

/**
 * Base class for jobs locking a collection.
 *
 * @remarks inherited classes should use setResult() to set the result
 */
class LockCollectionJob : public BooleanResultJob
{
    Q_OBJECT

public:
    /**
     * Constructor for a job that locks a collection.
     */
    explicit LockCollectionJob(BackendCollection *collection);

    /**
     * Get the collection to lock.
     *
     * @remarks public as needed by ServiceMultiPrompt
     */
    BackendCollection *collection();

private:
    BackendCollection *m_collection;
};

/**
 * Base class for jobs locking an item.
 *
 * @remarks inherited classes should use setResult() to set the result
 */
class LockItemJob : public BooleanResultJob
{
    Q_OBJECT

public:
    /**
     * Constructor for a job that locks an item.
     */
    explicit LockItemJob(BackendItem *item);

    /**
     * Get the item to lock.
     *
     * @remarks public as needed by ServiceMultiPrompt
     */
    BackendItem *item();

private:
    BackendItem *m_item;
};

/**
 * Base class for jobs unlocking a collection.
 *
 * @remarks inherited classes should use setResult() to set the result
 */
class UnlockCollectionJob : public BooleanResultJob
{
    Q_OBJECT

public:
    /**
     * Constructor.
     */
    UnlockCollectionJob(const CollectionUnlockInfo &unlockInfo, BackendCollection *collection);

    /**
     * Get the collection to unlock.
     *
     * @remarks public as needed by ServiceMultiPrompt
     */
    BackendCollection *collection() const {
        return m_collection;
    }

    /**
     * Get the collection unlock info
     */
    const CollectionUnlockInfo& unlockInfo() const {
        return m_unlockInfo;
    }

private:
    BackendCollection *m_collection;
    CollectionUnlockInfo m_unlockInfo;
};

/**
 * Base class for jobs unlocking an item.
 *
 * @remarks inherited classes should use setResult() to set the result
 */
class UnlockItemJob : public BooleanResultJob
{
    Q_OBJECT

public:
    /**
     * Constructor.
     */
    explicit UnlockItemJob(const ItemUnlockInfo& unlockInfo);

    /**
     * Get the item to unlock.
     *
     * @remarks public as needed by ServiceMultiPrompt
     */
    BackendItem *item();

private:
    ItemUnlockInfo m_unlockInfo;
};

/**
 * Base class for a job for deleting a collection.
 */
class DeleteCollectionJob : public BooleanResultJob
{
    Q_OBJECT

public:
    /**
     * Constructor.
     */
    explicit DeleteCollectionJob(const CollectionDeleteInfo& deleteInfo);

protected:
    /**
     * Get the collection to delete.
     */
    BackendCollection *collection();

    /**
     * Get the collection delete information
     */
    const CollectionDeleteInfo& collectionDeleteInfo() const {
        return m_deleteInfo;
    }

private:
    CollectionDeleteInfo m_deleteInfo;
};

/**
 * Base class for a job deleting an item.
 */
class DeleteItemJob : public BooleanResultJob
{
    Q_OBJECT

public:
    /**
     * Constructor.
     */
    explicit DeleteItemJob(const ItemDeleteInfo& deleteInfo);

protected:
    /**
     * Get the item to delete.
     */
    BackendItem *item();

private:
    ItemDeleteInfo m_deleteInfo;
};

/**
 * Base class for a job changing a collection's authentication.
 */
class ChangeAuthenticationCollectionJob : public BooleanResultJob
{
    Q_OBJECT

public:
    /**
     * Constructor.
     */
    explicit ChangeAuthenticationCollectionJob(BackendCollection *collection, const Peer& peer);

protected:
    /**
     * Get the collection to change the authentication for.
     */
    BackendCollection *collection();
    const Peer& peer() const;

private:
    BackendCollection *m_collection;
    Peer               m_peer;
};

/**
 * Base class for a job changing an item's authentication.
 */
class ChangeAuthenticationItemJob : public BooleanResultJob
{
    Q_OBJECT

public:
    /**
     * Constructor.
     */
    explicit ChangeAuthenticationItemJob(BackendItem *item);

protected:
    /**
     * Get the collection to change the authentication for.
     */
    BackendItem *item();

private:
    BackendItem *m_item;
};

/**
 * Base class for jobs creating a new item inside a collection.
 */
class CreateItemJob : public BackendJob
{
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * @param createInfo Item creation contextual information
     * @param collection Collection to create the item in
     */
    CreateItemJob(const ItemCreateInfo& createInfo,
                  BackendCollection *collection);

    /**
     * The item that was created or 0 if creating the item failed.
     */
    BackendItem *item() const;

    /**
     * The collection inside which the item should be created.
     *
     * @remarks this is public because retrieving the collection is needed
     *          by PromptSingleJob objects.
     */
    BackendCollection *collection();

protected:
    /**
     * The label to assign to the new item.
     */
    const QString &label() const;

    /**
     * The attributes to assign to the new item.
     */
    const QMap<QString, QString> &attributes() const;

    /**
     * The secret that should be stored.
     *
     * @todo make secure
     */
    const QCA::SecureArray &secret() const;

    /**
     * If true, the new item should be created locked, if false it should be unlocked
     * after creation.
     */
    bool locked() const;

    /**
     * If true, an existing item with the same attributes should be replaced by the new item,
     * if false creating an item with the same attributes as an existing item will fail.
     *
     * @todo or will it create a second item?
     */
    bool replace() const;

    /**
     * Set the item created.
     *
     * @param item the item which was created.
     * @remarks this is called by inheriting classes
     */
    void setItem(BackendItem *item);
    
    const QString& contentType() const;

private:
    BackendCollection *m_collection;
    ItemCreateInfo m_createInfo;

    BackendItem *m_item;
};

#endif // BACKENDJOB_H
