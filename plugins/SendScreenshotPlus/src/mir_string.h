/*
Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-09 Miranda ICQ/IM project,

This file is part of Send Screenshot Plus, a Miranda IM plugin.
Copyright (c) 2010 Ing.U.Horn

Parts of this file based on original sorce code
from UserInfoEx Plugin

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef _MIR_STRING_H_INCLUDED_
#define _MIR_STRING_H_INCLUDED_

#define mir_tcslen		mir_wcslen
#define mir_tcsadd		mir_wcsadd
#define mir_tcsncpy		mir_wcsncpy
#define mir_tcsncat		mir_wcsncat
#define mir_tstrdup		mir_wstrdup

#define mir_strlen(s)			(((s)!=0)?strlen(s):0)
#define mir_strcpy(d,s)			(((s)!=0&&(d)!=0)?strcpy(d,s):0)

#define mir_wcslen(s)			(((s)!=0)?wcslen(s):0)

#define mir_freeAndNil(ptr) mir_free(ptr),ptr=NULL

wchar_t*	mir_wcsncpy(wchar_t* pszDest, const wchar_t* pszSrc, const size_t cchDest);
wchar_t*	mir_wcsncat(wchar_t* pszDest, const wchar_t* pszSrc, const size_t cchDest);

void		mir_stradd(char* &pszDest, const char* pszSrc);
void		mir_wcsadd(wchar_t* &pszDest, const wchar_t* pszSrc);

#endif /* _MIR_STRING_H_INCLUDED_ */
