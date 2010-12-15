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

#include "kcsecretsyncmodule.h"
#include "configwidget.h"

#include <kgenericfactory.h>
#include <kaboutdata.h>

K_PLUGIN_FACTORY(KSecretSyncFactory, registerPlugin<KCSecretSyncModule>();)
K_EXPORT_PLUGIN(KSecretSyncFactory("kcm_ksecretsync"))

/**
 * This module loads during KDE session startup in order to get the
 */
extern "C"
{
  /**
   * @see kcm_ksecretsync.desktop entry X-KDE-Init
   */
  KCModule *init_startup(QWidget *, const char *)
  {
      // TODO: implement session startup code
      // this will start the sync daemon that will stop by itself
      // if the network sync is not enabled. Maybe check the enable
      // flag right away, before starting the daemon in order to preserve 
      // KDE session startup time.
      return 0;
  };
}

KCSecretSyncModule::KCSecretSyncModule(QWidget* parent, const QVariantList& args) :
    KCModule( KSecretSyncFactory::componentData(), parent, args)
{
    KAboutData  *aboutdata = new KAboutData(
                        "kcm_ksecretsync", 0, ki18n("KDE Secret Service Synchronization Daemon configuration module"),
                        KDE_VERSION_STRING, ki18n("KDE Secret Service"),
                        KAboutData::License_GPL, ki18n("(C) 2010 Valentin Rusu"));
    aboutdata->addAuthor(ki18n("Valentin Rusu"), ki18n("Maintainer"), "kde@rusu.info");
    setAboutData( aboutdata );
    setButtons( KCModule::Apply | KCModule::Help);
    
    _configWidget = new ConfigWidget( this );
    connect( _configWidget->_enableSync, SIGNAL(toggled(bool)), this, SLOT(configChanged()) );
    connect( _configWidget->_intervalSpinBox, SIGNAL(valueChanged(int)), this, SLOT(configChanged()) );
    connect( _configWidget, SIGNAL(computerListChanged()), this, SLOT(configChanged()) );
    
    // TODO: use D-Bus to check if the daemon is running and to hide the 'run' button here
}

void KCSecretSyncModule::configChanged()
{
    emit changed(true);
}

void KCSecretSyncModule::load()
{
    KCModule::load();
}

void KCSecretSyncModule::save()
{
    KCModule::save();
}

void KCSecretSyncModule::defaults()
{
    KCModule::defaults();
}

#include "kcsecretsyncmodule.moc"
