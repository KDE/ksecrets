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

#ifndef CONFIGCONSTANTS_H
#define CONFIGCONSTANTS_H

/**
 * This is the save timer interval default value
 * The save timer is started whenever a setting is changed and allow for it to be
 * persisted into the configuration file
 */
#define SAVE_TIMER_INTERVAL 2000
/**
 * Avoid too short sync interval 
 */
#define MIN_SYNC_INTERVAL   1

#define MAIN_ENABLE_SYNC_ENTRY "enableSync"
#define MAIN_SYNC_INTERVAL "syncInterval"
#define MAIN_CURRENT_TAB "currentTab"
#define MAIN_COMPUTER_COUNT "computerCount"

#endif // CONFIGCONSTANTS_H
