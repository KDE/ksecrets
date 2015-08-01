/*
 * Copyright 2010, Valentin Rusu <valir@kde.org>
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

#ifndef CONFIGWIDGET_H
#define CONFIGWIDGET_H

#include <ui_configwidget.h>

#include <QWidget>

class KSecretSyncCfg;

class ConfigWidget : public QWidget, public Ui_ConfigWidget
{
    Q_OBJECT
public:
    explicit ConfigWidget(QWidget* parent, Qt::WindowFlags f = 0);
    virtual ~ConfigWidget();
    void load(KSecretSyncCfg*);
    void save(KSecretSyncCfg*);
    void defaults();

protected Q_SLOTS:
    void onAddComputer();
    void onDeleteComputer();
    void enableSyncToggled( bool );
    void onSynchIntervalChanged( int );
    
Q_SIGNALS:
    void computerListChanged();
    
private:
    void createActions();
    
private:
    QWidget *_mainWindow;
};

#endif // CONFIGWIDGET_H
