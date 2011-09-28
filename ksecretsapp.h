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

#ifndef KSECRETSAPP_H
#define KSECRETSAPP_H

#include <kapplication.h>

class KJob;

class KSecretsApp : public KApplication {
    Q_OBJECT
     void listCollection(QString collName);
      void listCollections();
public:
    KSecretsApp();
    
    
protected Q_SLOTS:
    void slotParseArgs();
    void slotListCollectionsDone(KJob*);
    void slotListItemsDone(KJob*);
    void slotReadItemAttributesDone(KJob*);
    void slotReadAllItemsDone(KJob*);    
};


#endif // KSECRETSAPP_H
