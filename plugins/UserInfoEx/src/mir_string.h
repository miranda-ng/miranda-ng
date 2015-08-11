/*
UserinfoEx plugin for Miranda IM

Copyright:
� 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

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

#define mir_tcschr    mir_wcschr
#define mir_tcsrchr   mir_wcsrchr
#define mir_tcsncat_c mir_wcsncat_c
#define mir_IsEmpty   mir_IsEmptyW

#define mir_strchr(s,c)			(((s)!=0)?strchr((s),(c)):0)
#define mir_strrchr(s,c)		(((s)!=0)?strrchr((s),(c)):0)

#define mir_wcschr(s,c)			(((s)!=0)?wcschr((s),(c)):0)
#define mir_wcsrchr(s,c)		(((s)!=0)?wcsrchr((s),(c)):0)

char* mir_strncat_c(char *pszDest, const char cSrc);
char* mir_wcsncat_c(char *pszDest, const char cSrc);

char* mir_strnerase(char *pszDest, size_t sizeFrom, size_t sizeTo);

int   mir_IsEmptyA(char *str);
int   mir_IsEmptyW(wchar_t *str);

#endif /* _MIR_STRING_H_INCLUDED_ */