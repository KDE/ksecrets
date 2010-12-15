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

#ifndef KCSECRETSYNCMODULE_H
#define KCSECRETSYNCMODULE_H

#include <kcmodule.h>

class ConfigWidget;

class KCSecretSyncModule : public KCModule
{
    Q_OBJECT
public:
    KCSecretSyncModule( QWidget* parent =0L, const QVariantList &args = QVariantList());
    virtual void load();
    virtual void save();
    virtual void defaults();
    
private Q_SLOTS:
    void configChanged();
    
private:
    ConfigWidget* _configWidget;

};

#endif // KCSECRETSYNCMODULE_H
