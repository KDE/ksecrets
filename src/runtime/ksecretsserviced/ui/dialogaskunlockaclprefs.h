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

#ifndef DIALOGASKUNLOCKACLPREFS_H
#define DIALOGASKUNLOCKACLPREFS_H

#include <ui_dialogaskunlockaclprefs.h>
#include <acl.h>

#include <kdialog.h>

class AskUnlockAclPrefsWidget;

class DialogAskUnlockAclPrefs : public KDialog
{
    Q_OBJECT
public:
    explicit DialogAskUnlockAclPrefs(QWidget* parent = 0);

    void setCollectionLabel( const QString& label );
    void setApplication(QString exePath);
    void setOriginalApplication( const QString& exePath );
    
    ApplicationPermission permission() const;

private:
    AskUnlockAclPrefsWidget* m_widget;
    QString m_collectionLabel;
};

class AskUnlockAclPrefsWidget : public QWidget, public Ui_AskUnlockAclPrefsWidget
{
    Q_OBJECT
public:
    explicit AskUnlockAclPrefsWidget( QWidget* parent );
};

#endif // DIALOGASKUNLOCKACLPREFS_H
