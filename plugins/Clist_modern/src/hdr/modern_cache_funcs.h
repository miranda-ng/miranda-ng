/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-08 Miranda ICQ/IM project,
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

Created by Pescuma

*/
#pragma once

#ifndef __CACHE_FUNCS_H__
# define __CACHE_FUNCS_H__

#include "modern_clc.h"
#include "modern_commonprototypes.h"

void Cache_GetText(ClcData *dat, ClcContact *contact, BOOL forceRenew);
void Cache_GetFirstLineText(ClcData *dat, ClcContact *contact);
void Cache_GetSecondLineText(struct SHORTDATA *dat, ClcCacheEntry *pdnce);
void Cache_GetThirdLineText(struct SHORTDATA *dat, ClcCacheEntry *pdnce);
void Cache_GetAvatar(ClcData *dat, ClcContact *contact);
void Cache_GetTimezone(ClcData *dat, MCONTACT hContact);
int Cache_GetLineText(ClcCacheEntry *pdnce, int type, LPTSTR text, int text_size, TCHAR *variable_text, BOOL xstatus_has_priority,
	BOOL show_status_if_no_away, BOOL show_listening_if_no_away, BOOL use_name_and_message_for_xstatus,
	BOOL pdnce_time_show_only_if_different);

void amRequestAwayMsg(MCONTACT hContact);

#endif // __CACHE_FUNCS_H__
