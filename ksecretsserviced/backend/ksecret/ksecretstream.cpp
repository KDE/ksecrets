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


#include "ksecretstream.h"
#include "ksecretencryptionfilter.h"
#include "ksecretitem.h"

KSecretStream::KSecretStream( QIODevice *device ) :
    QDataStream( device )
{
}

bool KSecretStream::isValid() const
{
    return true;
}

KSecretStream &KSecretStream::operator << ( const bool& b )
{
    quint32 value = b ? 1 : 0;
    (QDataStream&)*this << value;
    return *this;
}

KSecretStream &KSecretStream::operator >> ( bool& b )
{
    quint32 value;
    (QDataStream&)*this >> value;
    b = value == 1;
    return *this;
}

KSecretStream &KSecretStream::operator << ( const QHash<QString, ApplicationPermission>& perms )
{
    quint64 size = perms.size();
    (QDataStream&)*this << size;
    QHashIterator< QString, ApplicationPermission > it( perms );
    while ( it.hasNext() ) {
        it.next();
        *this << it.key();
        *this << it.value();
    }
    return *this;
}

KSecretStream &KSecretStream::operator >> ( QHash<QString, ApplicationPermission>& perms)
{
    quint64 size;
    (QDataStream&)*this >> size;
    while ( size-- >0 ) {
        QString key;
        *this >> key;
        
        ApplicationPermission perm;
        *this >> perm;
        
        perms.insert( key, perm );
    }
    return *this;
}

KSecretStream& KSecretStream::operator<<(const ApplicationPermission perm)
{
    qint32 value = (qint32)perm;
    *this << value;
    return *this;
}

KSecretStream& KSecretStream::operator>>(ApplicationPermission& perm)
{
    qint32 value;
    (QDataStream&)*this >> value;
    perm = (ApplicationPermission)value;
    return *this;
}


KSecretStream &KSecretStream::operator << ( const QHash<QString, KSecretItem*> & items )
{
    qint64 size = items.size();
    (QDataStream&)*this << size;
    QHashIterator< QString, KSecretItem* > it( items );
    while ( it.hasNext() ) {
        it.next();
        *this << it.key();
        *this << it.value();
    }
    return *this;
}

KSecretStream &KSecretStream::operator >> ( QHash<QString, KSecretItem*> & items )
{
    qint64 size;
    (QDataStream&)*this >> size;
    while ( size-- >0 ) {
        QString key;
        *this >> key;
        
        KSecretItem *item =0;
        *this >> item;
        Q_ASSERT( item != 0 );
        
        items.insert( key, item );
    }
    return *this;
}

KSecretStream& KSecretStream::operator<<(const QCA::InitializationVector& vector)
{
    (QDataStream&)*this << vector.toByteArray();
    return *this;
}

KSecretStream& KSecretStream::operator>>(QCA::InitializationVector& vector)
{
    QByteArray arr;
    (QDataStream&)*this >> arr;
    if ( status() == QDataStream::Ok ) {
        vector = QCA::InitializationVector( arr );
    }
    return *this;
}

KSecretStream& KSecretStream::operator<<(const QCA::SecureArray& arr)
{
    (QDataStream&)*this << arr.toByteArray();
    return *this;
}

KSecretStream& KSecretStream::operator>>(QCA::SecureArray& arr)
{
    QByteArray ba;
    (QDataStream&)*this >> ba;
    if ( status() == QDataStream::Ok ) {
        arr = QCA::SecureArray( ba );
    }
    return *this;
}
