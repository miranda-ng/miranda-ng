/*
	New Away System - plugin for Miranda IM
	Copyright (c) 2005-2007 Chervov Dmitry

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#pragma once

#include "m_utils.h"
#include ".\CommonLibs\CString.h"


__inline TCString Path_ToRelative(TCString &Path)
{
	CString Str;
	Str.GetBuffer(MAX_PATH);
	CallService(MS_UTILS_PATHTORELATIVE, (WPARAM)(const char*)TCHAR2ANSI(Path), (LPARAM)(char*)Str);
	Str.ReleaseBuffer();
	return ANSI2TCHAR(Str);
}


__inline TCString Path_ToAbsolute(TCString &Path)
{
	CString Str;
	Str.GetBuffer(MAX_PATH);
	CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)(const char*)TCHAR2ANSI(Path), (LPARAM)(char*)Str);
	Str.ReleaseBuffer();
	return ANSI2TCHAR(Str);
}
