/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
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

void Cache_GetText(ClcData *dat, ClcContact *contact);
void Cache_GetFirstLineText(ClcData *dat, ClcContact *contact);
void Cache_GetNthLineText(ClcData *dat, ClcCacheEntry *pdnce, int n);
void Cache_GetAvatar(ClcData *dat, ClcContact *contact);
void Cache_GetTimezone(ClcData *dat, MCONTACT hContact);
int  Cache_GetLineText(ClcCacheEntry *pdnce, int type, LPTSTR text, int text_size, ClcLineInfo &line);

#endif // __CACHE_FUNCS_H__
