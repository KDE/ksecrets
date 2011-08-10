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

#include "abstractuijobs.h"
#include "abstractuimanager.h"

#include <QtCore/QEventLoop>
#include <QtCore/QTimer>

AbstractUiJob::AbstractUiJob(AbstractUiManager *manager) : 
    m_cancelled(false)
{
}

AbstractUiJob::~AbstractUiJob()
{
}

void AbstractUiJob::exec()
{
    Q_ASSERT(false);
}

AbstractAskPasswordJob::AbstractAskPasswordJob(AbstractUiManager *manager,
        const QString &collection,
        bool secondTry)
    : AbstractUiJob(manager), m_collection(collection), m_secondTry(secondTry)
{
}

AbstractAskPasswordJob::~AbstractAskPasswordJob()
{
}

const QString &AbstractAskPasswordJob::collection() const
{
    return m_collection;
}

bool AbstractAskPasswordJob::isSecondTry() const
{
    return m_secondTry;
}

const QCA::SecureArray &AbstractAskPasswordJob::password() const
{
    return m_password;
}

void AbstractAskPasswordJob::setPassword(const QCA::SecureArray &password)
{
    m_password = password;
}

AbstractNewPasswordJob::AbstractNewPasswordJob(AbstractUiManager* manager,
        const QString& collection)
    : AbstractUiJob(manager), m_collection(collection)
{
}

AbstractNewPasswordJob::~AbstractNewPasswordJob()
{
}

const QString &AbstractNewPasswordJob::collection() const
{
    return m_collection;
}

const QCA::SecureArray &AbstractNewPasswordJob::password() const
{
    return m_password;
}

void AbstractNewPasswordJob::setPassword(const QCA::SecureArray &password)
{
    m_password = password;
}

#include "abstractuijobs.moc"
