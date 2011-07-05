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

#include "backenditem.h"
#include "backendcollection.h"
#include <kdebug.h>

#define ATTR_CONTENT_TYPE "content-type"

BackendItem::BackendItem(BackendCollection *collection) : QObject(collection),
    m_collection(collection)
{
}

BackendItem::~BackendItem()
{
}

BackendReturn<QString> BackendItem::contentType() const
{
    BackendReturn<QString> result("");
    BackendReturn< QMap< QString, QString > > attrsRet = attributes();
    if ( attrsRet.error() == NoError ) {
        result = attrsRet.value()[ATTR_CONTENT_TYPE];
    }
    else {
        result.setError( ErrorOther, "Cannot get contentType attribute for current item");
    }
    return result;
}

BackendReturn<void> BackendItem::setContentType(const QString& contentType)
{
    BackendReturn<void> result;
    if (contentType.length()>0) {
        BackendReturn< QMap< QString, QString > > attrsRet = attributes();
        if ( attrsRet.error() == NoError ) {
            QMap< QString, QString > attrs = attrsRet.value();
            attrs[ATTR_CONTENT_TYPE] = contentType;
            BackendReturn< void > setRet = setAttributes( attrs );
            if ( setRet.isError() ) {
                result.setError( setRet.error(), "Cannot set contentType attribute for current item" );
            }
        }
        else {
            result.setError( ErrorOther, "Cannot get contentType attribute for current item" );
        }
    }
    return result;
}

#include "backenditem.moc"
