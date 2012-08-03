/*
Copyright (C) 2005-2009 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
*/

#ifndef __MIR_MEMORY_H__
# define __MIR_MEMORY_H__

#include <windows.h>

static int strcmpnull(char *str1, char *str2)
{
	if ( str1 == NULL && str2 == NULL )
		return 0;
	if ( str1 != NULL && str2 == NULL )
		return 1;
	if ( str1 == NULL && str2 != NULL )
		return -1;

	return strcmp(str1, str2);
}

static int strcmpnullW(WCHAR *str1, WCHAR *str2)
{
	if ( str1 == NULL && str2 == NULL )
		return 0;
	if ( str1 != NULL && str2 == NULL )
		return 1;
	if ( str1 == NULL && str2 != NULL )
		return -1;

	return lstrcmpW(str1, str2);
}

# define lstrcmpnull strcmpnullW

#define INPLACE_CHAR_TO_TCHAR(_new_var_, _size_, _old_var_)									\
	TCHAR _new_var_[_size_];																\
	MultiByteToWideChar(CP_ACP, 0, _old_var_, -1, _new_var_, _size_)

#define INPLACE_TCHAR_TO_CHAR(_new_var_, _size_, _old_var_)									\
	char _new_var_[_size_];																	\
	WideCharToMultiByte(CP_ACP, 0, _old_var_, -1, _new_var_, _size_, NULL, NULL);

#endif // __MIR_MEMORY_H__
