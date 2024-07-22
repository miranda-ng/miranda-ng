/*
Copyright (C) 2006-2007 Scott Ellis
Copyright (C) 2007-2011 Jan Holub

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

#include "stdafx.h"

int iCodePage = CP_ACP;

bool a2w(const char *as, wchar_t *buff, int bufflen)
{
	if (as) MultiByteToWideChar(iCodePage, 0, as, -1, buff, bufflen);
	return true;
}

bool w2a(const wchar_t *ws, char *buff, int bufflen)
{
	if (ws) WideCharToMultiByte(iCodePage, 0, ws, -1, buff, bufflen, nullptr, nullptr);
	return true;
}

bool utf2w(const char *us, wchar_t *buff, int bufflen)
{
	if (us) MultiByteToWideChar(CP_UTF8, 0, us, -1, buff, bufflen);
	return true;
}

bool w2utf(const wchar_t *ws, char *buff, int bufflen)
{
	if (ws) WideCharToMultiByte(CP_UTF8, 0, ws, -1, buff, bufflen, nullptr, nullptr);
	return true;
}

bool a2utf(const char *as, char *buff, int bufflen)
{
	if (!as) return false;

	wchar_t *ws = mir_a2u(as);
	if (ws) WideCharToMultiByte(CP_UTF8, 0, ws, -1, buff, bufflen, nullptr, nullptr);
	mir_free(ws);
	return true;
}

bool utf2a(const char *us, char *buff, int bufflen)
{
	if (!us) return false;

	wchar_t *ws = mir_utf8decodeW(us);
	if (ws) WideCharToMultiByte(iCodePage, 0, ws, -1, buff, bufflen, nullptr, nullptr);
	mir_free(ws);
	return true;
}

wchar_t *myfgets(wchar_t *Buf, int MaxCount, FILE *File)
{
	fgetws(Buf, MaxCount, File);
	for (size_t i = mir_wstrlen(Buf) - 1; ; i--) {
		if (Buf[i] == '\n' || Buf[i] == ' ')
			Buf[i] = 0;
		else
			break;
	}

	CharLower(Buf);
	return Buf;
}
