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

#include "dialoguimanager.h"
#include "dialogaskaclprefsjob.h"

#include <QtCore/QTimer>
#include <kpassworddialog.h>
#include <knewpassworddialog.h>

DialogAskPasswordJob::DialogAskPasswordJob(AbstractUiManager *manager,
        const QString &collection,
        bool secondTry)
    : AbstractAskPasswordJob(manager, collection, secondTry), m_dialog(0)
{
}

DialogAskPasswordJob::~DialogAskPasswordJob()
{
}

void DialogAskPasswordJob::start()
{
    Q_ASSERT(!m_dialog);
    // TODO: provide parent widget!
    m_dialog = new KPasswordDialog;
    m_dialog->setAttribute(Qt::WA_DeleteOnClose, true);
    // TODO: needs proper string
    m_dialog->setPrompt("Collection " + collection() + " wants password.");
    connect(m_dialog, SIGNAL(finished(int)), this, SLOT(dialogFinished(int)));
    m_dialog->show();
}

void DialogAskPasswordJob::dialogFinished(int result)
{
    Q_ASSERT(m_dialog);
    if(result == QDialog::Accepted) {
        setPassword(QCA::SecureArray(m_dialog->password().toUtf8()));
    } else {
        setCancelled(true);
    }
    emitResult();
}

DialogNewPasswordJob::DialogNewPasswordJob(AbstractUiManager *manager,
        const QString &collection)
    : AbstractNewPasswordJob(manager, collection), m_dialog(0)
{
}

DialogNewPasswordJob::~DialogNewPasswordJob()
{
}

void DialogNewPasswordJob::start()
{
    Q_ASSERT(!m_dialog);
    // TODO: provide parent widget!
    m_dialog = new KNewPasswordDialog;
    m_dialog->setAttribute(Qt::WA_DeleteOnClose, true);
    // TODO: needs proper string
    m_dialog->setPrompt("Collection " + collection() + " wants new password.");
    connect(m_dialog, SIGNAL(finished(int)), this, SLOT(dialogFinished(int)));
    m_dialog->show();
}

void DialogNewPasswordJob::dialogFinished(int result)
{
    Q_ASSERT(m_dialog);
    if(result == QDialog::Accepted) {
        setPassword(QCA::SecureArray(m_dialog->password().toUtf8()));
    } else {
        setCancelled(true);
    }
    emitResult();
}

AbstractAskPasswordJob *DialogUiManager::createAskPasswordJob(const QString &collection,
        bool secondTry)
{
    return new DialogAskPasswordJob(this, collection, secondTry);
}

AbstractNewPasswordJob *DialogUiManager::createNewPasswordJob(const QString &collection)
{
    return new DialogNewPasswordJob(this, collection);
}

AbstractAskAclPrefsJob *DialogUiManager::createAskAclPrefsJob(const CollectionUnlockInfo &unlockCollectionInfo)
{
    return new DialogAskUnlockAclPrefsJob(this, unlockCollectionInfo);
}

AbstractAskAclPrefsJob *DialogUiManager::createAskAclPrefsJob(const CollectionCreateInfo &createCollectionInfo)
{
    return new DialogAskCreateAclPrefsJob(this, createCollectionInfo);
}

#include "dialoguimanager.moc"
