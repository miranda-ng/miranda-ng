/*
Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-15 Miranda NG project (http://miranda-ng.org),
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

#define mir_freeAndNil(ptr) mir_free(ptr),ptr=NULL
void mir_stradd(char* &pszDest, const char* pszSrc);
void mir_wstradd(wchar_t* &pszDest, const wchar_t* pszSrc);
#define mir_tstradd mir_wstradd

#endif /* _MIR_STRING_H_INCLUDED_ */
