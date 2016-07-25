/*
Miranda IM Help Plugin
Copyright (C) 2002 Richard Hughes, 2005-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (Help-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#include "stdafx.h"

#include <m_button.h>

#define CHARBUFFER_ALLOCSTEP  512  // allocated buffer increase in bytes

#ifdef EDITOR
extern const TCHAR *szControlTypeNames[] = {
	L"Unknown", L"Dialog Box", L"Button", L"Check Box", L"Radio Button",
	L"Text", L"Image", L"Edit Box", L"Group Box", L"Combo Box",
	L"List Box", L"Spin Edit Box", L"Progress Bar", L"Slider", L"List View",
	L"Tree View", L"Date/Time Picker", L"IP Address", L"Status Bar", L"Hyperlink",
	L"Contact List", L"Scroll Bar", L"Animation", L"Hotkey", L"Tabs",
	L"Colour Picker", L"Tool Bar", L"Combo Edit Box", L"Size Grip" };
#endif

int GetControlType(HWND hwndCtl)
{
	TCHAR szClassName[32];
	DWORD style;

	if (GetClassLong(hwndCtl, GCW_ATOM) == 32770)
		return CTLTYPE_DIALOG;
	if (!GetClassName(hwndCtl, szClassName, _countof(szClassName)))
		return CTLTYPE_UNKNOWN;
	if (!lstrcmpi(szClassName, L"MDIClient"))
		return CTLTYPE_DIALOG;
	else if (!lstrcmpi(szClassName, L"Static")) {
		if (GetClassName(GetParent(hwndCtl), szClassName, _countof(szClassName)) && !lstrcmpi(szClassName, L"ComboBox") || !lstrcmpi(szClassName, WC_COMBOBOXEX))
			return CTLTYPE_COMBO;
		style = GetWindowLongPtr(hwndCtl, GWL_STYLE);
		switch (style&SS_TYPEMASK) {
		case SS_BITMAP:
		case SS_BLACKFRAME:
		case SS_BLACKRECT:
		case SS_ENHMETAFILE:
		case SS_ETCHEDFRAME:
		case SS_ETCHEDHORZ:
		case SS_ETCHEDVERT:
		case SS_WHITEFRAME:
		case SS_WHITERECT:
		case SS_GRAYFRAME:
		case SS_GRAYRECT:
		case SS_ICON:
		case SS_OWNERDRAW:
			return CTLTYPE_IMAGE;
		}
		return CTLTYPE_TEXT;
	}
	else if (GetClassLong(hwndCtl, GCW_ATOM) == 32772) // WinNT/2000/XP: icon titles
		return CTLTYPE_IMAGE;                      // class="#32772"
	else if (!lstrcmpi(szClassName, L"Button")) {
		style = GetWindowLongPtr(hwndCtl, GWL_STYLE);
		switch (style & 0x24) {
		case BS_CHECKBOX:
		case BS_AUTOCHECKBOX:
		case BS_3STATE:
		case BS_AUTO3STATE:
			if (style&BS_PUSHLIKE)
				break;
			return CTLTYPE_CHECKBOX;
		case BS_RADIOBUTTON:
		case BS_AUTORADIOBUTTON:
			if (style&BS_PUSHLIKE)
				break;
			return CTLTYPE_RADIO;
		case BS_GROUPBOX:
			return CTLTYPE_GROUP;
		}
		return CTLTYPE_BUTTON;
	}
	else if (!lstrcmpi(szClassName, MIRANDABUTTONCLASS))
		return CTLTYPE_BUTTON;
	else if (!lstrcmpi(szClassName, L"Edit")) {
		if (GetClassName(GetParent(hwndCtl), szClassName, _countof(szClassName)) && !lstrcmpi(szClassName, L"ComboBox"))
			return CTLTYPE_COMBO;
		if (GetClassName(GetWindow(hwndCtl, GW_HWNDNEXT), szClassName, _countof(szClassName)) && !lstrcmpi(szClassName, UPDOWN_CLASS))
			if ((HWND)SendMessage(GetWindow(hwndCtl, GW_HWNDNEXT), UDM_GETBUDDY, 0, 0) == hwndCtl)
				return CTLTYPE_SPINEDIT;
		return CTLTYPE_EDIT;
	}
	else if (!_tcsnicmp(szClassName, L"RichEdit", 8))
		return CTLTYPE_EDIT; // RICHEDIT,RichEdit20A,RichEdit20W,RichEdit50W and future versions
	else if (!lstrcmpi(szClassName, L"ListBox")) {
		style = GetWindowLongPtr(hwndCtl, GWL_STYLE);
		if (style&LBS_COMBOBOX)
			return CTLTYPE_COMBO;
		return CTLTYPE_LIST;
	}
	else if (!lstrcmpi(szClassName, L"ComboLBox") || !lstrcmpi(szClassName, L"ComboBox") || !lstrcmpi(szClassName, WC_COMBOBOXEX))
		return CTLTYPE_COMBO;
	else if (!lstrcmpi(szClassName, L"ScrollBar")) {
		style = GetWindowLongPtr(hwndCtl, GWL_STYLE);
		if (style&SBS_SIZEBOX)
			return CTLTYPE_SIZEGRIP;
		return CTLTYPE_SCROLL;
	}
	else if (!lstrcmpi(szClassName, WC_PAGESCROLLER))
		return CTLTYPE_SCROLL;
	else if (!lstrcmpi(szClassName, UPDOWN_CLASS)) {
		if (GetClassName((HWND)SendMessage(hwndCtl, UDM_GETBUDDY, 0, 0), szClassName, _countof(szClassName)) && !lstrcmpi(szClassName, L"Edit"))
			return CTLTYPE_SPINEDIT;
		return CTLTYPE_SCROLL;
	}
	else if (!lstrcmpi(szClassName, PROGRESS_CLASS))
		return CTLTYPE_PROGRESS;
	else if (!lstrcmpi(szClassName, TRACKBAR_CLASS))
		return CTLTYPE_SLIDER;
	else if (!lstrcmpi(szClassName, WC_LISTVIEW) || !lstrcmpi(szClassName, WC_HEADER))
		return CTLTYPE_LISTVIEW;
	else if (!lstrcmpi(szClassName, WC_TREEVIEW))
		return CTLTYPE_TREEVIEW;
	else if (!lstrcmpi(szClassName, DATETIMEPICK_CLASS) || !lstrcmpi(szClassName, MONTHCAL_CLASS))
		return CTLTYPE_DATETIME;
	else if (!lstrcmpi(szClassName, WC_IPADDRESS))
		return CTLTYPE_IP;
	else if (!lstrcmpi(szClassName, STATUSCLASSNAME))
		return CTLTYPE_STATUSBAR;
	else if (!lstrcmpi(szClassName, (LPCWSTR)CLISTCONTROL_CLASS))
		return CTLTYPE_CLC; // look at !! casting
	else if (!lstrcmpi(szClassName, WNDCLASS_HYPERLINK) || !lstrcmpi(szClassName, L"SysLink"))
		return CTLTYPE_HYPERLINK;
	else if (!lstrcmpi(szClassName, ANIMATE_CLASS))
		return CTLTYPE_ANIMATION;
	else if (!lstrcmpi(szClassName, HOTKEY_CLASS))
		return CTLTYPE_HOTKEY;
	else if (!lstrcmpi(szClassName, WC_TABCONTROL))
		return CTLTYPE_TABS;
	else if (!lstrcmpi(szClassName, (LPCWSTR)WNDCLASS_COLOURPICKER))
		return CTLTYPE_COLOUR; // look at !! casting
	else if (!lstrcmpi(szClassName, TOOLBARCLASSNAME) || !lstrcmpi(szClassName, REBARCLASSNAME))
		return CTLTYPE_TOOLBAR;
	switch (SendMessage(hwndCtl, WM_GETDLGCODE, 0, (LPARAM)NULL) & 0x2070) {
	case DLGC_RADIOBUTTON:
		return CTLTYPE_RADIO;
	case DLGC_DEFPUSHBUTTON:
	case DLGC_UNDEFPUSHBUTTON:
	case DLGC_BUTTON:
		return CTLTYPE_BUTTON;
	}
	hwndCtl = GetWindow(hwndCtl, GW_CHILD); // check for owner-extended control
	if (hwndCtl != NULL)
		return GetControlType(hwndCtl);

	return CTLTYPE_UNKNOWN;
}

HWND GetControlDialog(HWND hwndCtl)
{
	TCHAR szClassName[32];
	while (hwndCtl != NULL) {
		if (GetClassLong(hwndCtl, GCW_ATOM) == 32770)
			return hwndCtl;
		if (GetClassName(hwndCtl, szClassName, _countof(szClassName)))
			if (!lstrcmpi(szClassName, L"MDIClient"))
				return hwndCtl;
		hwndCtl = GetParent(hwndCtl);
	}

	return hwndCtl;
}

// never fails
int GetControlTitle(HWND hwndCtl, TCHAR *pszTitle, int cchTitle)
{
	TCHAR *p;
	int res = 0;
	if (cchTitle)
		pszTitle[0] = _T('\0');
	switch (GetControlType(hwndCtl)) {
	case CTLTYPE_DIALOG:
	case CTLTYPE_BUTTON:
	case CTLTYPE_CHECKBOX:
	case CTLTYPE_RADIO:
	case CTLTYPE_GROUP:
	case CTLTYPE_TEXT:
	case CTLTYPE_HYPERLINK:
		res = GetWindowText(hwndCtl, pszTitle, cchTitle);
	}
	hwndCtl = GetWindow(hwndCtl, GW_HWNDPREV);
	if (hwndCtl)
		switch (GetControlType(hwndCtl)) {
		case CTLTYPE_TEXT:
		case CTLTYPE_GROUP:
			res = GetWindowText(hwndCtl, pszTitle, cchTitle);
	}
	if (res)
		for (p = pszTitle; *p != 0; p++) {
			// strip-off ampersand (&) prefix character
			if (*p == _T('&') && *(p + 1) != _T('&')) {
				MoveMemory(p, p + 1, (lstrlen(p + 1) + 1)*sizeof(TCHAR));
				res--;
				if (*(p + 1) == 0)
					break;
			}
			// strip-off last ':'
			if (*p == _T(':') && *(p + 1) == 0) {
				*p = 0;
				res--;
				break;
			}
		}

	return res;
}

// mir_free() the return value
char *GetControlModuleName(HWND hwndCtl)
{
	char szModule[512], szMainModule[512];
	char *pszFile, *buf;

	if (!GetModuleFileNameA(NULL, szMainModule, sizeof(szMainModule)))
		return NULL;
	buf = strrchr(szMainModule, '\\');
	if (buf != NULL)
		*buf = '\0';
	else
		buf = szMainModule;

	do {
		if (!GetModuleFileNameA((HINSTANCE)GetWindowLongPtr(hwndCtl, GWLP_HINSTANCE), szModule, sizeof(szModule)))
			return NULL;
		pszFile = strrchr(szModule, '\\');
		if (pszFile != NULL) {
			*pszFile = '\0';
			pszFile++;
		}
		else
			pszFile = szModule;
		if (lstrlenA(szModule)>lstrlenA(szMainModule))
			szModule[lstrlenA(szMainModule)] = '\0';
		if (!lstrcmpiA(szModule, szMainModule))
			break; // found miranda module
		hwndCtl = GetParent(hwndCtl);
	} while (hwndCtl != NULL);

	buf = strrchr(pszFile, '.');
	if (buf != NULL)
		*buf++ = '\0';

	return mir_strdup(pszFile);
}


struct CreateControlIdData {
	int id;
	HWND hwndCtl;
};

static BOOL CALLBACK CreateCtlIdEnumProc(HWND hwnd, LPARAM lParam)
{
	struct CreateControlIdData* ccid = (struct CreateControlIdData*)lParam;
	TCHAR szClassName[32];
	if (GetClassLong(hwnd, GCW_ATOM) == 32770) // class="#32770"
		return TRUE;
	if (GetClassName(hwnd, szClassName, _countof(szClassName)))
		if (!lstrcmpi(szClassName, L"MDIClient"))
			return TRUE;
	if (GetWindowLongPtr(hwnd, GWL_ID) <= 0 || GetWindowLongPtr(hwnd, GWL_ID) == 0xFFFF)
		ccid->id--;
	if (hwnd == ccid->hwndCtl)
		ccid->hwndCtl = NULL;

	return ccid->hwndCtl != NULL;
}

int GetControlID(HWND hwndCtl)
{
	struct CreateControlIdData ccid;
	TCHAR szClassName[32];

	// obey context ID when set (rarely)
	ccid.id = GetWindowContextHelpId(hwndCtl);
	if (ccid.id != 0)
		return ccid.id;

	if (GetClassName(hwndCtl, szClassName, _countof(szClassName))) {
		if (!lstrcmpi(szClassName, UPDOWN_CLASS)) { //  handle spinner controls as a whole
			DWORD style;
			HWND hwndBuddy;
			style = GetWindowLongPtr(hwndCtl, GWL_STYLE);
			if (style&UDS_ALIGNRIGHT || style&UDS_ALIGNLEFT) {
				hwndBuddy = (HWND)SendMessage(hwndCtl, UDM_GETBUDDY, 0, 0);
				if (hwndBuddy != NULL)
					hwndCtl = hwndBuddy;
			}
		}
		else if (GetClassLong(hwndCtl, GCW_ATOM) == 32770 || !lstrcmpi(szClassName, L"MDIClient"))
			return 0; // ensure this is always unset
	}
	ccid.id = GetWindowLongPtr(hwndCtl, GWL_ID);
	if (ccid.id <= 0 || ccid.id == 0xFFFF) {
		ccid.id = -1;
		ccid.hwndCtl = hwndCtl;
		EnumChildWindows(GetParent(hwndCtl), CreateCtlIdEnumProc, (LPARAM)&ccid);
		if (ccid.hwndCtl != NULL)
			return -1;
	}
	return ccid.id;
}

// mir_free() the return value
static char *Base64Encode(PBYTE pBuf, int cbBuf)
{
	/*NETLIBBASE64 nlb64;
	nlb64.pbDecoded=pBuf;
	nlb64.cbDecoded=cbBuf;
	nlb64.cchEncoded=Netlib_GetBase64EncodedBufferSize(nlb64.cbDecoded);
	nlb64.pszEncoded=(char*)mir_alloc(nlb64.cchEncoded);
	if(nlb64.pszEncoded==NULL || !CallService(MS_NETLIB_BASE64ENCODE,0,(LPARAM)&nlb64)) {
	mir_free(nlb64.pszEncoded); // does NULL check
	return NULL;
	}
	return nlb64.pszEncoded;*/

	return (char*)mir_base64_encode(pBuf, cbBuf);
}

struct CreateDialogIdBinaryData {
	int alloced, count;
	PBYTE buf;
	HWND hwndParent;
};

static BOOL CALLBACK CreateDlgIdBinEnumProc(HWND hwnd, LPARAM lParam)
{
	struct CreateDialogIdBinaryData *cdib = (struct CreateDialogIdBinaryData*)lParam;
	int type;

	if (GetParent(hwnd) != cdib->hwndParent)
		return TRUE;
	type = GetControlType(hwnd);
	if (type == CTLTYPE_DIALOG || type == CTLTYPE_TEXT || type == CTLTYPE_GROUP)
		return TRUE;
	if (cdib->count + 3>cdib->alloced) {
		PBYTE buf2;
		buf2 = (PBYTE)mir_realloc(cdib->buf, cdib->alloced + 32);
		if (buf2 == NULL)
			return FALSE;
		cdib->alloced += 32;
		cdib->buf = buf2;
	}
	cdib->buf[cdib->count] = (BYTE)type;
	*(PWORD)(cdib->buf + cdib->count + 1) = (WORD)GetWindowLongPtr(hwnd, GWL_ID);
	cdib->count += 3;

	return TRUE;
}

// mir_free() the return value
char *CreateDialogIdString(HWND hwndDlg)
{
	struct CreateDialogIdBinaryData cdib;
	char *szRet;

	ZeroMemory(&cdib, sizeof(cdib));
	if (hwndDlg == NULL)
		return NULL;
	cdib.hwndParent = hwndDlg;
	EnumChildWindows(hwndDlg, CreateDlgIdBinEnumProc, (LPARAM)&cdib);
	if (cdib.buf == NULL)
		return NULL;
	szRet = Base64Encode(cdib.buf, cdib.count);
	mir_free(cdib.buf);

	return szRet;
}


void AppendCharToCharBuffer(struct ResizableCharBuffer *rcb, char c)
{
	if (rcb->cbAlloced <= rcb->iEnd + 1) {
		char* buf = (char*)mir_realloc(rcb->sz, (rcb->cbAlloced + CHARBUFFER_ALLOCSTEP));
		if (buf == NULL)
			return;
		rcb->sz = buf;
		rcb->cbAlloced += CHARBUFFER_ALLOCSTEP;
	}
	rcb->sz[rcb->iEnd++] = c;
	rcb->sz[rcb->iEnd] = '\0';
}

void AppendToCharBuffer(struct ResizableCharBuffer *rcb, const char *fmt, ...)
{
	va_list va;
	int charsDone;
	char *buf;

	if (rcb->cbAlloced == 0) {
		buf = (char*)mir_alloc(CHARBUFFER_ALLOCSTEP);
		if (buf == NULL)
			return;
		rcb->sz = buf;
		rcb->cbAlloced = CHARBUFFER_ALLOCSTEP;
	}
	va_start(va, fmt);
	for (;;) {
		charsDone = mir_vsnprintf(rcb->sz + rcb->iEnd, rcb->cbAlloced - rcb->iEnd, fmt, va);
		if (charsDone >= 0)
			break; // returns -1 when buffer not large enough
		buf = (char*)mir_realloc(rcb->sz, rcb->cbAlloced + CHARBUFFER_ALLOCSTEP);
		if (buf == NULL) {
			charsDone = 0;
			break;
		}
		rcb->sz = buf;
		rcb->cbAlloced += CHARBUFFER_ALLOCSTEP;
	}
	va_end(va);
	rcb->iEnd += charsDone;
}
