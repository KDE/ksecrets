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

#ifndef DAEMON_PROMPT_H
#define DAEMON_PROMPT_H

#include <backend/backendjob.h>

#include <QtCore/QObject>
#include <QtCore/QSet>
#include <QtDBus/QDBusObjectPath>

#include "ksecretobject.h"
#include <qdbusconnection.h>

class Service;

/**
 * Implementation of prompt objects according to the org.freedesktop.Secret.Prompt
 * interface.
 */
class PromptBase : public QObject, public QDBusContext, public KSecretObject<PromptBase>
{
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * @param service Service object (used to derive the object path of the prompt)
     * @param job the job encapsulated by the prompt
     * @param parent Parent object
     */
    PromptBase(Service *service, QObject *parent);

    /**
     * Destructor.
     */
    virtual ~PromptBase();

    /**
     * Return the prompt's path on the D-Bus.
     *
     * @return the Prompt object's path
     */
    const QDBusObjectPath &objectPath() const;

    /**
     * Return the service object's path on the D-Bus.
     *
     * @return the Service object's path
     */
    const QDBusObjectPath &serviceObjectPath() const;

    /**
     * Perform the prompt.
     *
     * @param windowId Platform specific window handle to use for showing the prompt
     * @todo implement window handle handling
     */
    virtual void prompt(const QString &windowId) = 0;

    /**
     * Dismiss the prompt.
     */
    virtual void dismiss() = 0;

Q_SIGNALS:
    /**
     * Emitted when the operation performed by the prompt was completed
     *
     * @param dismissed if true the prompt was dismissed, if false it was completed
     * @param result result of the operation encapulated by the prompt object
     */
    void completed(bool dismissed, QVariant result);

protected:
    /**
     * Used to emit completed() signals in derived classes.
     *
     * @param dismissed if true the prompt was dismissed, if false it was completed
     * @param result result of the operation encapsulated by the prompt object
     */
    void emitCompleted(bool dismissed, const QVariant &result);

private:
    QDBusObjectPath m_objectPath; // the prompt object's objectpath
    QDBusObjectPath m_serviceObjectPath; // the service's objectpath
};

/**
 * Prompt object encapsulating a single BackendJob.
 */
class SingleJobPrompt : public PromptBase
{
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * @param service Service object (used to derive the object path of the prompt)
     * @param job the job encapsulated by the prompt
     * @param parent Parent object
     */
    SingleJobPrompt(Service *service, BackendJob *job, QObject *parent = 0);

    /**
     * Destructor.
     */
    virtual ~SingleJobPrompt();

    /**
     * Perform the prompt.
     *
     * @param windowId Platform specific window handle to use for showing the prompt
     * @todo implement window handle handling
     */
    virtual void prompt(const QString &windowId);

    /**
     * Dismiss the prompt.
     */
    virtual void dismiss();

private Q_SLOTS:
    /**
     * Connected to the backend job's result() signal this notifies about
     * the job's completion.
     *
     * @param job the job that finished
     */
    void jobResult(KJob *job);

private:
    bool m_prompted; // true if one of the prompt()/dismiss() methods has been called already
    BackendJob *m_job; // the encapulated job
};

/**
 * Prompt object encapsulating a Service.Unlock or a Service.Lock call with multiple targets.
 */
class ServiceMultiPrompt : public PromptBase
{
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * @param service Service object (used to derive the object path of the prompt)
     * @param jobs the unlock jobs to encapsulate
     * @param parent parent object
     */
    ServiceMultiPrompt(Service *service, const QSet<BackendJob*> jobs, QObject *parent = 0);

    /**
     * Destructor.
     */
    virtual ~ServiceMultiPrompt();

    /**
     * Perform the prompt.
     *
     * @pararm windowId Platform specific window handle to use for showing the prompt
     * @todo implement window handling
     */
    virtual void prompt(const QString &windowId);

    /**
     * Dismiss this prompt.
     */
    virtual void dismiss();

private Q_SLOTS:
    /**
     * Connected to the backend jobs' result() signals this notifies about a
     * job's completion.
     *
     * @param job the job that finished
     */
    void jobResult(KJob *job);

private:
    bool                    m_prompted; // true if one of the prompt()/dismiss() methods has been called already
    QList<QDBusObjectPath>  m_result;   // resulting unlocked/locked object paths
    QSet<BackendJob*>       m_jobs;     // encapsulated jobs
};


#endif
