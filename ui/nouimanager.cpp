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

#include "nouimanager.h"
#include "nouiaskaclprefsjob.h"

#include <QtCore/QTimer>

NoUiAskPasswordJob::NoUiAskPasswordJob(AbstractUiManager *manager,
                                       bool cancel)
    : AbstractAskPasswordJob(manager, "", false), m_cancel(cancel)
{
}

NoUiAskPasswordJob::~NoUiAskPasswordJob()
{
}

void NoUiAskPasswordJob::start()
{
    QTimer::singleShot(0, this, SLOT(finish()));
}

void NoUiAskPasswordJob::finish()
{
    if(m_cancel) {
        setCancelled(true);
    } else {
        setPassword("default");
    }
    emitResult();
}

NoUiNewPasswordJob::NoUiNewPasswordJob(AbstractUiManager *manager,
                                       bool cancel)
    : AbstractNewPasswordJob(manager, ""), m_cancel(cancel)
{
}

NoUiNewPasswordJob::~NoUiNewPasswordJob()
{
}

void NoUiNewPasswordJob::start()
{
    QTimer::singleShot(0, this, SLOT(finish()));
}

void NoUiNewPasswordJob::finish()
{
    if(m_cancel) {
        setCancelled(true);
    } else {
        setPassword("default");
    }
    emitResult();
}

NoUiManager::NoUiManager() : m_cancelAll(false)
{
}

NoUiManager::~NoUiManager()
{
}

AbstractAskPasswordJob *NoUiManager::createAskPasswordJob(const QString &collection,
        bool secondTry)
{
    Q_UNUSED(collection);
    Q_UNUSED(secondTry);
    return new NoUiAskPasswordJob(this, m_cancelAll);
}

AbstractNewPasswordJob *NoUiManager::createNewPasswordJob(const QString &collection)
{
    Q_UNUSED(collection);
    return new NoUiNewPasswordJob(this, m_cancelAll);
}

AbstractAskAclPrefsJob *NoUiManager::createAskAclPrefsJob(const CollectionUnlockInfo &unlockCollectionInfo)
{
    return new NoUiAskAclPrefsJob(this, unlockCollectionInfo);
}

AbstractAskAclPrefsJob *NoUiManager::createAskAclPrefsJob(const CollectionCreateInfo &createCollectionInfo)
{
    return new NoUiAskAclPrefsJob(this, createCollectionInfo);
}

void NoUiManager::setCancelAll(bool cancelAll)
{
    m_cancelAll = cancelAll;
}

bool NoUiManager::cancelAll() const
{
    return m_cancelAll;
}

#include "nouimanager.moc"
