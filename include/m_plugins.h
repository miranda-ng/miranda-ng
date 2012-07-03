/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2012 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef M_PLUGINS_H__
#define M_PLUGINS_H__

/*
	Undocumented: Do NOT use.
	Version: 0.3.4.1+ (2004/10/04)
*/
#define DBPE_DONE 1
#define DBPE_CONT 0
#define DBPE_HALT (-1)
typedef struct PLUGIN_DB_ENUM {
	int cbSize;
	// 0 = continue, 1 = found, -1 = stop now
	int (*pfnEnumCallback) (const char * pluginname, /*DATABASELINK*/ void * link, LPARAM lParam);
	LPARAM lParam;
} PLUGIN_DB_ENUM;
#define MS_PLUGINS_ENUMDBPLUGINS "Plugins/DbEnumerate"

#endif // M_PLUGINS_H__


