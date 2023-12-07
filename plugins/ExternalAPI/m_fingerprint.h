/*
Fingerprint Mod+ (client version) icons module for Miranda IM

Copyright © 2006-2007 Artem Shpynov aka FYR, Bio, Faith Healer. 2009-2010 HierOS

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

#pragma once

__forceinline bool Finger_IsPresent()
{	return ServiceExists("Fingerprint/SameClients");
}

 /*
 *  Service SameClientsW MS_FP_SAMECLIENTSW
 *	 wParam - LPCWSTR first MirVer value 
 *  lParam - LPCWSTR second MirVer value 
 *	 returns LPCWSTR: client desription (DO NOT DESTROY) if clients are same otherwise NULL
 */

__forceinline LPCWSTR Finger_IsSameClents(LPCWSTR pwszName1, LPCWSTR pwszName2)
{	return (LPCWSTR)CallService("Fingerprint/SameClients", (WPARAM)pwszName1, (WPARAM)pwszName2);
}

/*
 *  Service GetClientDescrW MS_FP_GETCLIENTDESCRW
 *	 wParam - LPCWSTR MirVer value to get client for.
 *  lParam - NULL (unused)
 *  returns LPCWSTR: client description (DO NOT DESTROY) otherwise NULL
 */

__forceinline LPCWSTR Finger_GetClientDescr(LPCWSTR pwszName)
{	return (LPCWSTR)CallService("Fingerprint/GetClientDescr", (WPARAM)pwszName, 0);
}

/*
 *  Service GetClientIconW MS_FP_GETCLIENTICONW
 *	 wParam - LPWSTR MirVer value to get client for.
 *  lParam - int noCopy - if wParam is equal to "1"  will return icon handler without copiing icon.
 *  the returned in this case handler is static valid  only till next service call.
 */

__forceinline HICON Finger_GetClientIcon(LPCWSTR pwszName, BOOL bNoCopy)
{	return (HICON)CallService("Fingerprint/GetClientIcon", (WPARAM)pwszName, bNoCopy);
}
