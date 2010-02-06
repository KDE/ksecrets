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

#include "attributemap.h"

namespace orgFreedesktopSecret
{

AttributeMap::AttributeMap() : QMap<QString, QString>()
{
}

AttributeMap::AttributeMap(const AttributeMap &other) : QMap<QString, QString>(other)
{
}

AttributeMap::AttributeMap(const QMap<QString, QString> &other) : QMap<QString, QString>(other)
{
}

AttributeMap::~AttributeMap()
{
}

}

QDBusArgument &operator<<(QDBusArgument &argument, const QMap<QString, QString> &map)
{
   argument.beginMap();
   QMap<QString, QString>::const_iterator it = map.constBegin();
   QMap<QString, QString>::const_iterator end = map.constEnd();
   for ( ; it != end; ++it) {
      argument.beginMapEntry();
      argument << it.key() << it.value();
      argument.endMapEntry();
   }
   argument.endMap();
   return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, QMap<QString, QString> &map)
{
   QString key;
   QString value;
   map.clear();
   argument.beginMap();
   while (!argument.atEnd()) {
      argument.beginMapEntry();
      argument >> key >> value;
      map.insert(key, value);
      argument.endMapEntry();
   }
   argument.endMap();
   return argument;
}
