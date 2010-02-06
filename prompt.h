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

#include <backend/asynccall.h>

#include <QtCore/QObject>
#include <QtDBus/QDBusObjectPath>

// TODO: when doing getResult() calls, be sure to check for errors!!!

class Service;

/**
 * Implemention of prompt objects according to the org.freedesktop.Secret.Prompt
 * interface.
 *
 * @remarks a Prompt can contain several PendingCall objects which are all executed
 *          when prompt() is called.
 */
class PromptBase : public QObject
{
   Q_OBJECT

public:
   /**
    * Constructor.
    *
    * @param service Service object (used to derive the object path of the prompt)
    * @param parent Parent object (the object that created the call)
    */
   PromptBase(Service *service, QObject *parent = 0);

   /**
    * Return the prompt's path on the D-Bus.
    *
    * @return the Prompt object's path
    */
   const QDBusObjectPath &objectPath() const;

   /**
    * Perform the prompt.
    *
    * @param windowId Platform specific window handle to use for showing the prompt
    * @todo implement window handle handling
    */
   void prompt(const QString &windowId);

   /**
    * Dismiss the prompt.
    */
   void dismiss();

Q_SIGNALS:
   /**
    * Emitted when the operation performed by the prompt was completed
    *
    * @param dismissed if true the prompt was dismissed, if false it was completed
    * @param result result of the operation encapsulated by the prompt object
    */
   void completed(bool dismissed, QVariant result);

protected:
   /**
    * This method is called after all pending calls this prompt is attached to
    * have completed. It extracts the results from the pending calls and marshalls
    * them into a result variant to transmit to the client.
    *
    * @return the result of the call marshalled as a variant
    * @remarks implemented in inherited classes only
    */
   virtual QVariant getResult() const = 0;

   /**
    * Get the service's object path.
    *
    * @return the objectpath of the service that created the call
    * @remarks called by inherited classes
    */
   const QDBusObjectPath &serviceObjectPath() const;

   /**
    * Add a call to the list of pending calls to wait for.
    *
    * @param call the call to add
    * @remarks each inherited class must call this for every pending call it creates
    */
   void addCall(AsyncCall *call);

private Q_SLOTS:
   /**
    * Connected to the pending call's completed() signal this notifies about
    * the call's completion.
    *
    * @param call the pending call
    * @param dismissed true if the call was dismissed, false else
    */
   void slotCompleted(AsyncCall *call, bool dismissed);
   
private:
   bool m_prompted;
   QDBusObjectPath m_objectPath;
   QDBusObjectPath m_serviceObjectPath;
   QList<AsyncCall*> m_pendingCalls; // calls we are still waiting for
};

/**
 * Prompt for the Service::createCollection() call.
 */
class PromptServiceCreateCollection : public PromptBase
{
   Q_OBJECT

public:
   /**
    * Constructor.
    *
    * @param properties Properties to set for the new collection
    * @param service Service object (used to derive the object path of the prompt)
    * @param parent Parent object (the object that created the call)
    */
   PromptServiceCreateCollection(const QString &label, bool locked, Service *service,
                                 QObject *parent = 0);

protected:
   /**
    * Returns the results of the pending createCollection call as a variant.
    *
    * @return the result of the call marshalled as a variant
    */
   virtual QVariant getResult() const;

private:
   AsyncCreateCollection *m_call;
};

/**
 * Prompt for the Service::unlock() call.
 */
class PromptServiceUnlock : public PromptBase
{
   Q_OBJECT

public:
   /**
    * Constructor.
    *
    * @param objects map of backend objects to corresponding dbus paths to unlock
    * @param service Service object (used to derive the object path of the prompt)
    * @param parent Parent object (the object that created the call)
    * @remarks collections and items in the objectMap so when the pending calls finish the
    *          backend objects can be mapped back to the corresponding D-Bus paths
    */
   PromptServiceUnlock(const QMap<BackendBase*, QDBusObjectPath> &objects,
                       Service *service, QObject *parent = 0);

protected:
   /**
    * Returns the results of the pending unlock call as a variant.
    *
    * @return the result of the call marshalled as a variant
    */
   virtual QVariant getResult() const;

private:
   QMap<BackendBase*, QDBusObjectPath> m_objects;
   QList<AsyncSimpleBooleanCall*> m_calls;
};

/**
 * Prompt for the Service::lock() call.
 */
class PromptServiceLock : public PromptBase
{
   Q_OBJECT

public:
   /**
    * Constructor.
    *
    * @param objects map of backend objects to corresponding dbus paths to lock
    * @param service Service object (used to derive the object path of the prompt)
    * @param parent Parent object (the object that created the call)
    */
   PromptServiceLock(const QMap<BackendBase*, QDBusObjectPath> &objects,
                     Service *service, QObject *parent = 0);

protected:
   /**
    * Returns the results of the pending lock call as a variant.
    *
    * @return the result of the call marshalled as a variant
    */
   virtual QVariant getResult() const;

private:
   QMap<BackendBase*, QDBusObjectPath> m_objects;
   QList<AsyncSimpleBooleanCall*> m_calls;
};

/**
 * Prompt for the Collection::delete() call.
 */
class PromptCollectionDelete : public PromptBase
{
   Q_OBJECT

public:
   /**
    * Constructor.
    *
    * @param collection the backend collection to delete
    * @param service Service object (used to derive the object path of the prompt)
    * @param parent Parent object
    */
   PromptCollectionDelete(BackendCollection *collection, Service *service, QObject *parent = 0);

protected:
   /**
    * Returns the results of the pending delete call as a variant.
    *
    * @return the result of the call marshalled as a variant
    */
   virtual QVariant getResult() const;

private:
   AsyncSimpleBooleanCall *m_call;
};

/**
 * Prompt for the Collection::createItem() call.
 */
class PromptCollectionCreateItem : public PromptBase
{
   Q_OBJECT

public:
   /**
    * Constructor.
    *
    * @param collection the backend collection to create the item in
    * @param label label for the new item
    * @param attributes attributes for the new item
    * @param locked if true, try to lock the item after creation, if false try to unlock the
    *               item after creation
    * @param secret secret to store
    * @param replace if true an existing item with the same attributes will be replaced
    * @param service Service object (used to derive the object path of the prompt)
    * @param parent Parent object
    */
   PromptCollectionCreateItem(BackendCollection *collection,
                              const QString &label, const QMap<QString, QString> &attributes,
                              bool locked, const QCA::SecureArray &secret, bool replace,
                              Service *service, QObject *parent = 0);

protected:
   /**
    * Returns the results of the pending createItem call as a variant.
    *
    * @return the result of the call marshalled as a variant
    */
   virtual QVariant getResult() const;

private:
   AsyncCreateItem *m_call;
};

/**
 * Prompt for the Item::delete() call.
 */
class PromptItemDelete : public PromptBase
{
   Q_OBJECT

public:
   /**
    * Constructor.
    *
    * @param item the backend item to delete
    * @param service Service object (used to derive the object path of the prompt)
    * @param parent Parent object
    */
   PromptItemDelete(BackendItem *item, Service *service, QObject *parent = 0);

protected:
   /**
    * Returns the results of the pending deleteItem call as a variant.
    *
    * @return the result of the call marshalled as a variant
    */
   virtual QVariant getResult() const;

private:
   AsyncSimpleBooleanCall *m_call;
};

#endif
