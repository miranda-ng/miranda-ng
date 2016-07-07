/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-16 Miranda NG project (http://miranda-ng.org)
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

#include "stdafx.h"

#define CMP_UINT(x, y) { if ((x) != (y)) return (x) < (y) ? -1 : 1; }

int DBEventSortingKey::Compare(const MDB_val *ax, const MDB_val *bx)
{
	const DBEventSortingKey *a = (DBEventSortingKey *)ax->mv_data;
	const DBEventSortingKey *b = (DBEventSortingKey *)bx->mv_data;

	CMP_UINT(a->hContact, b->hContact);
	CMP_UINT(a->ts, b->ts);
	CMP_UINT(a->hEvent, b->hEvent);
	return 0;
}

int DBSettingKey::Compare(const MDB_val *ax, const MDB_val *bx)
{
	const DBSettingKey *a = (DBSettingKey *)ax->mv_data;
	const DBSettingKey *b = (DBSettingKey *)bx->mv_data;

	CMP_UINT(a->hContact, b->hContact);
	CMP_UINT(a->dwModuleId, b->dwModuleId);
	return (min(ax->mv_size, bx->mv_size) > sizeof(DBSettingKey)) ? strcmp(a->szSettingName, b->szSettingName) : 0;
}
