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

#include "dialogaskcreateaclprefs.h"

#include <klocalizedstring.h>
#include <QLabel>

DialogAskCreateAclPrefs::DialogAskCreateAclPrefs(QWidget* parent): 
    KDialog(parent),
    m_widget(0)
{
    setCaption( i18n("New Collection Access Policy") );
    setButtons( KDialog::Ok );
    m_widget = new AskCreateAclPrefsWidget( this );    
    setMainWidget( m_widget );
}

void DialogAskCreateAclPrefs::setCollectionLabel(const QString& label)
{
    QString locString = i18n( "You just created a KSecretService collection named '%1'.", label );
    m_widget->m_explainCollectionLabel->setText( locString );
}

void DialogAskCreateAclPrefs::setApplication(QString exePath)
{
    QString locString = i18n( "You used '%1' application to create this collection.", exePath );
    m_widget->m_explainApplicationLabel->setText( locString );
}

ApplicationPermission DialogAskCreateAclPrefs::permission() const
{
    if ( m_widget->m_permissionAllowRadio->isChecked() )
        return PermissionAllow;
    if ( m_widget->m_permissionAskRadio->isChecked() )
        return PermissionAsk;
    // we should never get here as per current dialog design
    Q_ASSERT(0);
    return PermissionDeny;
}

AskCreateAclPrefsWidget::AskCreateAclPrefsWidget( QWidget *parent ) :
    QWidget( parent )
{
    setupUi( this );
}

