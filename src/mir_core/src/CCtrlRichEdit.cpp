/*

Object UI extensions
Copyright (c) 2008  Victor Pavlychko, George Hazan
Copyright (c) 2012-17 Miranda NG project

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

#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlRichEdit class

CCtrlRichEdit::CCtrlRichEdit(CDlgBase *dlg, int ctrlId)
   : CCtrlEdit(dlg, ctrlId)
{}

int CCtrlRichEdit::GetRichTextLength(int iCodePage) const
{
	GETTEXTLENGTHEX gtl;
	gtl.codepage = iCodePage;
	gtl.flags = GTL_PRECISE | GTL_USECRLF;
	if (iCodePage == CP_ACP)
		gtl.flags |= GTL_NUMBYTES;
	else
		gtl.flags |= GTL_NUMCHARS;

	return (int)SendMessage(m_hwnd, EM_GETTEXTLENGTHEX, (WPARAM)&gtl, 0);
}

char* CCtrlRichEdit::GetRichTextUtf() const
{
	int textBufferSize = GetRichTextLength(CP_UTF8);
	if (textBufferSize == 0)
		return nullptr;

	textBufferSize++;
	char *textBuffer = (char*)mir_alloc(textBufferSize+1);

	GETTEXTEX  gt = {};
	gt.cb = textBufferSize;
	gt.flags = GT_USECRLF;
	gt.codepage = CP_UTF8;
	SendMessage(m_hwnd, EM_GETTEXTEX, (WPARAM)&gt, (LPARAM)textBuffer);
	return textBuffer;
}

int CCtrlRichEdit::SetRichText(const wchar_t *text)
{
	SETTEXTEX st;
	st.flags = ST_DEFAULT;
	st.codepage = 1200;
	SendMessage(m_hwnd, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)text);

	return GetRichTextLength(1200);
}

int CCtrlRichEdit::SetRichTextRtf(const char *text)
{
	SETTEXTEX st;
	st.flags = ST_DEFAULT;
	st.codepage = CP_UTF8;
	SendMessage(m_hwnd, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)text);

	return GetRichTextLength(1200);
}

/////////////////////////////////////////////////////////////////////////////////////////

static DWORD CALLBACK MessageStreamCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	static DWORD dwRead;
	char **ppText = (char **)dwCookie;

	if (*ppText == nullptr) {
		*ppText = (char *)mir_alloc(cb + 2);
		memcpy(*ppText, pbBuff, cb);
		*pcb = cb;
		dwRead = cb;
		*(*ppText + cb) = '\0';
	}
	else {
		char *p = (char *)mir_realloc(*ppText, dwRead + cb + 2);
		memcpy(p + dwRead, pbBuff, cb);
		*ppText = p;
		*pcb = cb;
		dwRead += cb;
		*(*ppText + dwRead) = '\0';
	}
	return 0;
}

char* CCtrlRichEdit::GetRichTextRtf(bool bText, bool bSelection) const
{
	char *pszText = nullptr;
	DWORD dwFlags = SF_USECODEPAGE | (CP_UTF8 << 16);
	if (bText)
		dwFlags |= SF_TEXT;
	else
		dwFlags |= SF_RTFNOOBJS | SFF_PLAINRTF;
	if (bSelection)
		dwFlags |= SFF_SELECTION;

	EDITSTREAM stream = { 0 };
	stream.pfnCallback = MessageStreamCallback;
	stream.dwCookie = (DWORD_PTR)&pszText; // pass pointer to pointer
	SendMessage(m_hwnd, EM_STREAMOUT, dwFlags, (LPARAM)&stream);
	return pszText; // pszText contains the text
}
