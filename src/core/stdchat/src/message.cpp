/*
Chat module plugin for Miranda IM

Copyright (C) 2003 Jörgen Persson
Copyright 2003-2009 Miranda IM project,

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

#include "chat.h"
#include <math.h>

static DWORD CALLBACK Message_StreamCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG * pcb)
{
	static DWORD dwRead;
	char ** ppText = (char **)dwCookie;

	if (*ppText == NULL) {
		*ppText = (char *)mir_alloc(cb + 1);
		memcpy(*ppText, pbBuff, cb);
		(*ppText)[cb] = 0;
		*pcb = cb;
		dwRead = cb;
	}
	else {
		char  *p = (char *)mir_alloc(dwRead + cb + 1);
		memcpy(p, *ppText, dwRead);
		memcpy(p + dwRead, pbBuff, cb);
		p[dwRead + cb] = 0;
		mir_free(*ppText);
		*ppText = p;
		*pcb = cb;
		dwRead += cb;
	}

	return 0;
}

char* Message_GetFromStream(HWND hwndDlg, SESSION_INFO *si)
{
	EDITSTREAM stream;
	char* pszText = NULL;
	DWORD dwFlags;

	if (hwndDlg == 0 || si == 0)
		return NULL;

	memset(&stream, 0, sizeof(stream));
	stream.pfnCallback = Message_StreamCallback;
	stream.dwCookie = (DWORD_PTR)&pszText; // pass pointer to pointer

	dwFlags = SF_RTFNOOBJS | SFF_PLAINRTF | SF_USECODEPAGE | (CP_UTF8 << 16);
	SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_STREAMOUT, dwFlags, (LPARAM)& stream);
	return pszText; // pszText contains the text
}
