/*

Facebook plugin for Miranda Instant Messenger
_____________________________________________

Copyright © 2009-11 Michal Zelinka, 2011-12 Robert Pösel

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#pragma once

void InitIcons(void);
HANDLE GetIconHandle(const char *name);

void InitContactMenus(void);
void UninitContactMenus(void);
void EnableMenuItem(HANDLE hMenuItem, bool enable);

/* Contact menu item indexes */
#define CMI_VISIT_PROFILE 0
#define CMI_AUTH_REVOKE 1
#define CMI_AUTH_ASK 2
#define CMI_AUTH_GRANT 3

#define CMITEMS_COUNT 4