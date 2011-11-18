/*
 * Copyright 2010, Valentin Rusu <kde@rusu.info>
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

#include "dialogaskunlockaclprefs.h"

#include <klocalizedstring.h>
#include <QLabel>

DialogAskUnlockAclPrefs::DialogAskUnlockAclPrefs(QWidget* parent): 
    KDialog(parent),
    m_widget(0)
{
    setCaption( i18n("Unlock Collection Access Policy") );
    setButtons( KDialog::Ok );
    m_widget = new AskUnlockAclPrefsWidget( this );    
    setMainWidget( m_widget );
}

void DialogAskUnlockAclPrefs::setApplication(QString exePath)
{
    // setCollectionLabel must have been called first
    Q_ASSERT( ! m_collectionLabel.isEmpty() );
    QString locString = i18n( "The application '%1' asked to open the KSecretService collection named '%2'.",
                              exePath, m_collectionLabel );
    m_widget->m_explainCollectionLabel->setText( locString );
}

void DialogAskUnlockAclPrefs::setCollectionLabel(const QString& label)
{
    m_collectionLabel = label;
}

void DialogAskUnlockAclPrefs::setOriginalApplication(const QString& exePath)
{
    QString locString = i18n( "This collection was created by the application '%1'. What should KSecretService do now?",
                              exePath );
    m_widget->m_explainApplicationLabel->setText( locString );
}

ApplicationPermission DialogAskUnlockAclPrefs::permission() const
{
    if ( m_widget->m_permissionAllowRadio->isChecked() )
        return PermissionAllow;
    if ( m_widget->m_permissionAskRadio->isChecked() )
        return PermissionAsk;
    if ( m_widget->m_permissionDenyRadio->isChecked() )
        return PermissionDeny;
    Q_ASSERT(0); // we should never reach here !
    return PermissionDeny;
}

AskUnlockAclPrefsWidget::AskUnlockAclPrefsWidget( QWidget *parent ) :
    QWidget( parent )
{
    setupUi( this );
}

#include "dialogaskunlockaclprefs.moc"
