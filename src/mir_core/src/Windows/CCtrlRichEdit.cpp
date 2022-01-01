/*

Object UI extensions
Copyright (c) 2008  Victor Pavlychko, George Hazan
Copyright (C) 2012-22 Miranda NG team

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

#include "../stdafx.h"

#include <RichOle.h>

/////////////////////////////////////////////////////////////////////////////////////////
// CCtrlRichEdit class

CCtrlRichEdit::CCtrlRichEdit(CDlgBase *dlg, int ctrlId)
   : CCtrlEdit(dlg, ctrlId)
{}

int CCtrlRichEdit::GetRichTextLength(int iCodePage) const
{
	GETTEXTLENGTHEX gtl;
	gtl.codepage = iCodePage;
	gtl.flags = GTL_PRECISE;
	if (iCodePage == CP_ACP)
		gtl.flags |= GTL_NUMBYTES;
	else
		gtl.flags |= GTL_NUMCHARS | GT_USECRLF;

	return (int)SendMessage(m_hwnd, EM_GETTEXTLENGTHEX, (WPARAM)&gtl, 0);
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
	static uint32_t dwRead;
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
	uint32_t dwFlags = SF_USECODEPAGE | (CP_UTF8 << 16);
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

/////////////////////////////////////////////////////////////////////////////////////////////////////// 

struct CREOleCallback : public IRichEditOleCallback
{
	CREOleCallback() : refCount(0), nextStgId(0), pictStg(nullptr) {}
	unsigned refCount;
	IStorage *pictStg;
	int nextStgId;

	STDMETHOD(QueryInterface)(REFIID riid, LPVOID FAR *ppvObj)
	{
		if (IsEqualIID(riid, IID_IRichEditOleCallback)) {
			*ppvObj = this;
			AddRef();
			return S_OK;
		}
		*ppvObj = nullptr;
		return E_NOINTERFACE;
	}

	STDMETHOD_(ULONG, AddRef)(THIS)
	{
		if (refCount == 0)
			StgCreateDocfile(nullptr, STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE | STGM_DELETEONRELEASE, 0, &pictStg);

		return ++refCount;
	}

	STDMETHOD_(ULONG, Release)(THIS)
	{
		if (--refCount == 0) {
			if (pictStg) {
				pictStg->Release();
				pictStg = nullptr;
			}
		}
		return refCount;
	}

	STDMETHOD(GetNewStorage)(LPSTORAGE *lplpstg)
	{
		wchar_t sztName[64];
		mir_snwprintf(sztName, L"s%u", nextStgId++);
		if (pictStg == nullptr)
			return STG_E_MEDIUMFULL;
		return pictStg->CreateStorage(sztName, STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, lplpstg);
	}

	STDMETHOD(ContextSensitiveHelp)(BOOL) 
	{ return S_OK; }
	STDMETHOD(GetInPlaceContext)(LPOLEINPLACEFRAME*, LPOLEINPLACEUIWINDOW*, LPOLEINPLACEFRAMEINFO)
	{ return E_INVALIDARG; }
	STDMETHOD(ShowContainerUI)(BOOL)
	{ return S_OK; }
	STDMETHOD(QueryInsertObject)(LPCLSID, LPSTORAGE, LONG)
	{ return S_OK; }
	STDMETHOD(DeleteObject)(LPOLEOBJECT)
	{ return S_OK; }
	STDMETHOD(QueryAcceptData)(LPDATAOBJECT, CLIPFORMAT*, DWORD, BOOL, HGLOBAL)
	{ return S_OK; }
	STDMETHOD(GetClipboardData)(CHARRANGE*, DWORD, LPDATAOBJECT*)
	{ return E_NOTIMPL; }
	STDMETHOD(GetDragDropEffect)(BOOL, DWORD, LPDWORD)
	{ return S_OK; }
	STDMETHOD(GetContextMenu)(uint16_t, LPOLEOBJECT, CHARRANGE*, HMENU*)
	{ return E_INVALIDARG; }
};

struct CREOleCallback2 : public CREOleCallback
{
	STDMETHOD(QueryAcceptData)(LPDATAOBJECT, CLIPFORMAT *lpcfFormat, DWORD, BOOL, HGLOBAL)
	{	*lpcfFormat = CF_UNICODETEXT;
		return S_OK;
	}
};

CREOleCallback reOleCallback;
CREOleCallback2 reOleCallback2;

void CCtrlRichEdit::SetReadOnly(bool bReadOnly)
{
	SendMsg(EM_SETOLECALLBACK, 0, (LPARAM)(bReadOnly ? &reOleCallback : &reOleCallback2));
}
