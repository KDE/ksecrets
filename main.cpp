/* This file is part of the KDE project
 *
 * Copyright (C) 2011 Valentin Rusu <kde@rusu.info>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <kuniqueapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <iostream>
#include <QString>

#include "../client/version.h"
#include "kwlimporterjob.h"


int main(int argc, char **argv)
{
    KAboutData aboutdata("kwl2kss", 0, ki18n("KWallet to KSecretsService import utility"),
                         ksecretsserviceclient_version, // please note that client version always match our version
                         ki18n("KWallet to KSecretsService import utility"),
                         KAboutData::License_GPL, ki18n("(C) 2011 Valentin Rusu"));
    aboutdata.addAuthor(ki18n("Valentin Rusu"), ki18n("Maintainer"), "kde@rusu.info");
    aboutdata.setProgramIconName("ksecretsservice");

    KCmdLineArgs::init(argc, argv, &aboutdata);
    
    KCmdLineOptions options;
    KCmdLineArgs::addCmdLineOptions( options );

    KUniqueApplication app;

    app.setQuitOnLastWindowClosed(false);

    if(!KUniqueApplication::start()) {
        kDebug() << "kwl2kss is already running!";
        return 0;
    }

    if ( KwlImporterJob::userHasWallets() ) {
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
        if ( args->isSet("kwl") ) {
            KwlImporterJob *importJob = new KwlImporterJob(&app);
            importJob->start();
        }
        else {
            std::cout << qPrintable( ki18n( "WARNING: found KWallet files but the -kwl option was not given, so ignoring them").toString() ) << std::endl;
        }
    }

    kDebug() << "ksecretsserviced ready";
    return app.exec();
}
