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

#ifndef KSECRETSTREAM_H
#define KSECRETSTREAM_H

#include "acl.h"
#include <qdatastream.h>
#include <qca_core.h>

class KSecretEncryptionFilter;
class KSecretItem;

class KSecretStream : public QDataStream {
public:
    explicit KSecretStream( QIODevice *file );

    bool isValid() const;
    
    KSecretStream &operator << ( const bool& );
    KSecretStream &operator >> ( bool& );
    
    KSecretStream &operator << ( const QHash<QString, ApplicationPermission>& );
    KSecretStream &operator >> ( QHash<QString, ApplicationPermission>& );
    
    KSecretStream &operator << ( const ApplicationPermission );
    KSecretStream &operator >> ( ApplicationPermission& );
    
    KSecretStream &operator << ( const QHash<QString, KSecretItem*> & );
    KSecretStream &operator >> ( QHash<QString, KSecretItem*> & );

//     // FIXME: why do we need to define this if the base class already provides it?
//     KSecretStream &operator << ( const quint32 & );
//     // FIXME: why do we need to define this if the base class already provides it?
//     KSecretStream &operator >> ( quint32 & );
    
    KSecretStream &operator << ( const QCA::InitializationVector& );
    KSecretStream &operator >> ( QCA::InitializationVector& );
        
    KSecretStream &operator << ( const QCA::SecureArray& );
    KSecretStream &operator >> ( QCA::SecureArray& );
};

#endif // KSECRETSTREAM_H
