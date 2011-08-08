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

#include <kuniqueapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <QtDBus/QDBusConnection>
#include <QtCrypto/QtCrypto>
#include <iostream>

#include "backend/backendmaster.h"
//#include "backend/temporary/temporarycollectionmanager.h"
#include "backend/ksecret/ksecretcollectionmanager.h"
#include "frontend/secret/service.h"
#include "ui/dialoguimanager.h"
#include "kwlimporter/kwlimporterjob.h"
#include "../client/version.h"

int main(int argc, char **argv)
{
    KAboutData aboutdata("ksecretsservice", 0, ki18n("KDE DaemonSecret Service"),
                         ksecretsserviceclient_version, // please note that client version always match daemon version
                         ki18n("KDE DaemonSecret Service"),
                         KAboutData::License_GPL, ki18n("(C) 2010 Michael Leupold"));
    aboutdata.addAuthor(ki18n("Michael Leupold"), ki18n("Maintainer"), "lemma@confuego.org");
    aboutdata.addAuthor(ki18n("Valentin Rusu"), ki18n("Maintainer"), "kde@rusu.info");
    aboutdata.setProgramIconName("ksecretsservice");

    KCmdLineArgs::init(argc, argv, &aboutdata);
    
    KCmdLineOptions options;
    options.add( "kwl", ki18n("Import KWallet .kwl files if any") );
    KCmdLineArgs::addCmdLineOptions( options );

    KUniqueApplication app;

    app.setQuitOnLastWindowClosed(false);

    if(!KUniqueApplication::start()) {
        kDebug() << "ksecretsservice is already running!";
        return 0;
    }

    if(!QDBusConnection::sessionBus().registerService("org.freedesktop.secrets")) {
        kDebug() << "Couldn't register org.freedesktop.Secret D-Bus service!";
        return 1;
    }

    BackendMaster *master = BackendMaster::instance();
    master->setUiManager( new DialogUiManager );
    master->addManager( new KSecretCollectionManager( "share/apps/ksecretsservice", master ) );
    Service service( BackendMaster::instance() ); // NOTE: this will also initialize QCA
    
    if ( KwlImporterJob::userHasWallets() ) {
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
        if ( args->isSet("kwl") ) {
            KwlImporterJob *importJob = new KwlImporterJob( &service );
            importJob->start();
        }
        else {
            std::cout << qPrintable( ki18n( "WARNING: found KWallet files but the -kwl option was not given, so ignoring them").toString() ) << std::endl;
        }
    }

    kDebug() << "ksecretsserviced ready";
    return app.exec();
}
