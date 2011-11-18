/*
 *  Copyright (C) 2010  Valentin Rusu <kde@rusu.info>
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

#include "dialogaskaclprefsjob.h"
#include "dialogaskcreateaclprefs.h"
#include "dialogaskunlockaclprefs.h"
#include <peer.h>

#include <QtGui/QDialog>

DialogAskCreateAclPrefsJob::DialogAskCreateAclPrefsJob(AbstractUiManager* manager,
        const CollectionCreateInfo& jobInfo):
    AbstractAskAclPrefsJob(manager, jobInfo),
    m_dialog(0)
{

}

void DialogAskCreateAclPrefsJob::start()
{
    Q_ASSERT( m_dialog == 0 );
    m_dialog = new DialogAskCreateAclPrefs;
    m_dialog->setCollectionLabel( jobHelper()->collectionLabel() );
    m_dialog->setApplication( jobHelper()->peer().exePath() );
    m_dialog->setAttribute(Qt::WA_DeleteOnClose, true);
    connect(m_dialog, SIGNAL(finished(int)), this, SLOT(dialogFinished(int)));
    m_dialog->show();
}

void DialogAskCreateAclPrefsJob::dialogFinished(int result)
{
    Q_ASSERT( m_dialog );
    if(result == QDialog::Accepted) {
        setPermission( m_dialog->permission() );
    } else {
        // TODO Is it necessary for this dialog to be canceled ? 
        // In my opinion that would not be necessary, as the collection has just
        // been created and we now really need to know what ACL handling would 
        // apply to it.
        Q_ASSERT(0);
    }
    emitResult();
}



DialogAskUnlockAclPrefsJob::DialogAskUnlockAclPrefsJob(AbstractUiManager* manager,
        const CollectionUnlockInfo& jobInfo):
    AbstractAskAclPrefsJob(manager, jobInfo),
    m_dialog(0)
{

}

void DialogAskUnlockAclPrefsJob::start()
{
    Q_ASSERT( m_dialog == 0 );
    m_dialog = new DialogAskUnlockAclPrefs;
    m_dialog->setCollectionLabel( jobHelper()->collectionLabel() );
    m_dialog->setApplication( jobHelper()->peer().exePath() );
    m_dialog->setOriginalApplication( jobHelper()->collectionCreatorApplication() );
    m_dialog->setAttribute(Qt::WA_DeleteOnClose, true);
    connect(m_dialog, SIGNAL(finished(int)), this, SLOT(dialogFinished(int)));
    m_dialog->show();
}

void DialogAskUnlockAclPrefsJob::dialogFinished(int result)
{
    Q_ASSERT( m_dialog );
    if(result == QDialog::Accepted) {
        setPermission( m_dialog->permission() );
    } else {
        setCancelled(true);
    }
    emitResult();
}

#include "dialogaskaclprefsjob.moc"
