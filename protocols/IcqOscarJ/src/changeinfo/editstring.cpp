// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
// 
// Copyright © 2001-2004 Richard Hughes, Martin Öberg
// Copyright © 2004-2009 Joe Kucera, Bio
// Copyright © 2012-2014 Miranda NG Team
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// -----------------------------------------------------------------------------
//  DESCRIPTION:
//
//  ChangeInfo Plugin stuff
// -----------------------------------------------------------------------------

#include "icqoscar.h"

static ChangeInfoData *dataStringEdit = NULL;
static HWND hwndEdit = NULL, hwndExpandButton = NULL, hwndUpDown = NULL;

static const char escapes[]={'a','\a',
'b','\b',
'e',27,
'f','\f',
'r','\r',
't','\t',
'v','\v',
'\\','\\'};

static void EscapesToMultiline(WCHAR *str,PDWORD selStart,PDWORD selEnd)
{ //converts "\\n" and "\\t" to "\r\n" and "\t" because a multi-line edit box can show them properly
	DWORD i;

	for(i=0; *str; str++, i++) 
	{
		if (*str != '\\') continue;
		if (str[1] == 'n') 
		{
			*str++ = '\r'; 
			i++; 
			*str = '\n';
		}
		else if (str[1] == 't') 
		{
			*str = '\t';
			memmove(str+1, str+2, sizeof(WCHAR)*(mir_wstrlen(str)-1));

			if (*selStart>i) --*selStart;
			if (*selEnd>i) --*selEnd;
		}
	}
}



static void EscapesToBinary(char *str)
{
	for (;*str;str++) 
	{
		if (*str!='\\') continue;
		if(str[1]=='n') {*str++='\r'; *str='\n'; continue;}
		if(str[1]=='0') 
		{
			char *codeend;
			*str=(char)strtol(str+1,&codeend,8);
			if (*str==0) {*str='\\'; continue;}
			memmove(str+1,codeend,mir_strlen(codeend)+1);
			continue;
		}
		for(int i=0;i<SIZEOF(escapes);i+=2)
			if(str[1]==escapes[i]) 
			{
				*str=escapes[i+1];
				memmove(str+1,str+2,mir_strlen(str)-1);
				break;
			}
	}
}



char *BinaryToEscapes(char *str)
{
	int extra = 10, len = (int)mir_strlen(str) + 11, i;
	char *out, *pout;

	out=pout=(char*)SAFE_MALLOC(len);
	for (;*str;str++) 
	{
		if ((unsigned char)*str>=' ') 
		{
			*pout++=*str; 
			continue;
		}
		if(str[0]=='\r' && str[1]=='\n') 
		{
			*pout++='\\'; 
			*pout++='n';
			str++;
			continue;
		}
		if(extra<3) 
		{
			extra+=8; len+=8;
			pout=out=(char*)SAFE_REALLOC(out,len);
		}
		*pout++='\\';
		for(i = 0; i < SIZEOF(escapes); i += 2)
			if (*str==escapes[i+1]) 
			{
				*pout++=escapes[i];
				extra--;
				break;
			}
			if(i < SIZEOF(escapes)) continue;
			*pout++='0'; extra--;
			if (*str>=8) 
			{
				*pout++=(*str>>3)+'0';
				extra--;
			}
			*pout++=(*str&7)+'0'; extra--;
	}
	*pout='\0';
	return out;
}



static LRESULT CALLBACK StringEditSubclassProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg) {
	case WM_KEYDOWN:
		if (wParam==VK_ESCAPE) 
		{
			if (dataStringEdit)
				dataStringEdit->EndStringEdit(0); 
			return 0;
		}
		if (wParam==VK_RETURN) 
		{
			if (GetWindowLongPtr(hwnd, GWL_STYLE) & ES_MULTILINE && !(GetKeyState(VK_CONTROL) & 0x8000)) break;
			if (dataStringEdit)
				dataStringEdit->EndStringEdit(1);
			return 0;
		}
		break;

	case WM_GETDLGCODE:
		return DLGC_WANTALLKEYS | mir_callNextSubclass(hwnd, StringEditSubclassProc, msg, wParam, lParam);

	case WM_KILLFOCUS:
		if ((HWND)wParam == hwndExpandButton) break;
		if (dataStringEdit)
			dataStringEdit->EndStringEdit(1);
		return 0;
	}
	return mir_callNextSubclass(hwnd, StringEditSubclassProc, msg, wParam, lParam);
}

static LRESULT CALLBACK ExpandButtonSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_LBUTTONUP:
		if (GetCapture() == hwnd) {
			//do expand
			RECT rcStart, rcEnd;
			DWORD selStart, selEnd;
			WCHAR *text;
			BOOL animEnabled = TRUE;

			GetWindowRect(hwndEdit, &rcStart);
			InflateRect(&rcStart, 2, 2);

			text = GetWindowTextUcs(hwndEdit);
			SendMessage(hwndEdit, EM_GETSEL, (WPARAM)&selStart, (LPARAM)&selEnd);
			DestroyWindow(hwndEdit);
			EscapesToMultiline(text, &selStart, &selEnd);
			hwndEdit = CreateWindowExA(WS_EX_TOOLWINDOW, "EDIT", "", WS_POPUP | WS_BORDER | WS_VISIBLE | ES_WANTRETURN | ES_AUTOVSCROLL | WS_VSCROLL | ES_MULTILINE, rcStart.left, rcStart.top, rcStart.right - rcStart.left, rcStart.bottom - rcStart.top, NULL, NULL, hInst, NULL);
			SetWindowTextUcs(hwndEdit, text);
			mir_subclassWindow(hwndEdit, StringEditSubclassProc);
			SendMessage(hwndEdit, WM_SETFONT, (WPARAM)dataStringEdit->hListFont, 0);
			SendMessage(hwndEdit, EM_SETSEL, selStart, selEnd);
			SetFocus(hwndEdit);

			rcEnd.left = rcStart.left; rcEnd.top = rcStart.top;
			rcEnd.right = rcEnd.left + 350;
			rcEnd.bottom = rcEnd.top + 150;
			if (LOBYTE(LOWORD(GetVersion())) > 4 || HIBYTE(LOWORD(GetVersion())) > 0)
				SystemParametersInfo(SPI_GETCOMBOBOXANIMATION, 0, &animEnabled, FALSE);
			if (animEnabled) {
				DWORD startTime, timeNow;
				startTime = GetTickCount();
				for (;;) {
					UpdateWindow(hwndEdit);
					timeNow = GetTickCount();
					if (timeNow > startTime + 200) break;
					SetWindowPos(hwndEdit, 0, rcEnd.left, rcEnd.top, (rcEnd.right - rcStart.right)*(timeNow - startTime) / 200 + rcStart.right - rcEnd.left, (rcEnd.bottom - rcStart.bottom)*(timeNow - startTime) / 200 + rcStart.bottom - rcEnd.top, SWP_NOZORDER);
				}
			}
			SetWindowPos(hwndEdit, 0, rcEnd.left, rcEnd.top, rcEnd.right - rcEnd.left, rcEnd.bottom - rcEnd.top, SWP_NOZORDER);

			DestroyWindow(hwnd);
			hwndExpandButton = NULL;

			SAFE_FREE((void**)&text);
		}
		break;
	}
	return mir_callNextSubclass(hwnd, ExpandButtonSubclassProc, msg, wParam, lParam);
}

void ChangeInfoData::BeginStringEdit(int iItem, RECT *rc, int i, WORD wVKey)
{
	char *szValue;
	char str[80];
	int alloced = 0;

	EndStringEdit(0);
	InflateRect(rc, -2, -2);
	rc->left -= 2;

	const SettingItem &si = setting[i];
	SettingItemData &sid = settingData[i];

	if (si.displayType & LIF_PASSWORD && !sid.changed)
		szValue = "                ";
	else if ((si.displayType & LIM_TYPE) == LI_NUMBER) {
		szValue = str;
		mir_snprintf(str, SIZEOF(str), "%d", sid.value);
	}
	else if (sid.value) {
		szValue = BinaryToEscapes((char*)sid.value);
		alloced = 1;
	}
	else szValue = "";

	iEditItem = iItem;

	if ((si.displayType & LIM_TYPE) == LI_LONGSTRING) {
		rc->right -= rc->bottom - rc->top;
		hwndExpandButton = CreateWindowA("BUTTON", "", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_ICON, rc->right, rc->top, rc->bottom - rc->top, rc->bottom - rc->top, hwndList, NULL, hInst, NULL);
		SendMessage(hwndExpandButton, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadImage(hInst, MAKEINTRESOURCE(IDI_EXPANDSTRINGEDIT), IMAGE_ICON, 0, 0, LR_SHARED));
		mir_subclassWindow(hwndExpandButton, ExpandButtonSubclassProc);
	}

	dataStringEdit = this;
	hwndEdit = CreateWindow(_T("EDIT"), _T(""), WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL | ((si.displayType&LIM_TYPE) == LI_NUMBER ? ES_NUMBER : 0) | (si.displayType&LIF_PASSWORD ? ES_PASSWORD : 0), rc->left, rc->top, rc->right - rc->left, rc->bottom - rc->top, hwndList, NULL, hInst, NULL);
	SetWindowTextUtf(hwndEdit, szValue);
	if (alloced) SAFE_FREE(&szValue);
	mir_subclassWindow(hwndEdit, StringEditSubclassProc);
	SendMessage(hwndEdit, WM_SETFONT, (WPARAM)hListFont, 0);
	if ((si.displayType & LIM_TYPE) == LI_NUMBER) {
		int *range = (int*)si.pList;
		RECT rcUpDown;
		hwndUpDown = CreateWindow(UPDOWN_CLASS, _T(""), WS_VISIBLE | WS_CHILD | UDS_AUTOBUDDY | UDS_ALIGNRIGHT | UDS_HOTTRACK | UDS_NOTHOUSANDS | UDS_SETBUDDYINT, 0, 0, 0, 0, hwndList, NULL, hInst, NULL);
		SendMessage(hwndUpDown, UDM_SETRANGE32, range[0], range[1]);
		SendMessage(hwndUpDown, UDM_SETPOS32, 0, sid.value);
		if (!(si.displayType & LIF_ZEROISVALID) && sid.value == 0)
			SetWindowTextA(hwndEdit, "");
		GetClientRect(hwndUpDown, &rcUpDown);
		rc->right -= rcUpDown.right;
		SetWindowPos(hwndEdit, 0, 0, 0, rc->right - rc->left, rc->bottom - rc->top, SWP_NOZORDER | SWP_NOMOVE);
	}
	SendMessage(hwndEdit, EM_SETSEL, 0, (LPARAM)-1);
	SetFocus(hwndEdit);
	PostMessage(hwndEdit, WM_KEYDOWN, wVKey, 0);
}

void ChangeInfoData::EndStringEdit(int save)
{
	if (hwndEdit == NULL || iEditItem == -1 || this != dataStringEdit)
		return;

	if (save) {
		char *text = (char*)SAFE_MALLOC(GetWindowTextLength(hwndEdit) + 1);
		const SettingItem &si = setting[iEditItem];
		SettingItemData &sid = settingData[iEditItem];

		GetWindowTextA(hwndEdit, (char*)text, GetWindowTextLength(hwndEdit) + 1);
		EscapesToBinary(text);
		if ((si.displayType & LIM_TYPE) == LI_NUMBER) {
			LPARAM newValue;
			int *range = (int*)si.pList;
			newValue = atoi(text);
			if (newValue) {
				if (newValue<range[0]) newValue = range[0];
				if (newValue>range[1]) newValue = range[1];
			}
			sid.changed = sid.value != newValue;
			sid.value = newValue;
			SAFE_FREE(&text);
		}
		else {
			if (!(si.displayType & LIF_PASSWORD)) {
				SAFE_FREE(&text);
				text = GetWindowTextUtf(hwndEdit);
				EscapesToBinary(text);
			}
			if ((si.displayType & LIF_PASSWORD && strcmpnull(text, "                ")) ||
				 (!(si.displayType & LIF_PASSWORD) && strcmpnull(text, (char*)sid.value) && (mir_strlen(text) + mir_strlen((char*)sid.value)))) {
				SAFE_FREE((void**)&sid.value);
				if (mir_strlen(text))
					sid.value = (LPARAM)text;
				else {
					sid.value = 0;
					SAFE_FREE(&text);
				}
				sid.changed = 1;
			}
		}

		if (sid.changed) {
			TCHAR tbuf[MAX_PATH];

			GetWindowText(hwndEdit, tbuf, SIZEOF(tbuf));
			ListView_SetItemText(hwndList, iEditItem, 1, tbuf);

			EnableDlgItem(hwndDlg, IDC_SAVE, TRUE);
		}
	}

	ListView_RedrawItems(hwndList, iEditItem, iEditItem);
	iEditItem = -1;
	dataStringEdit = NULL;
	DestroyWindow(hwndEdit);
	hwndEdit = NULL;
	if (hwndExpandButton) DestroyWindow(hwndExpandButton);
	hwndExpandButton = NULL;
	if (hwndUpDown) DestroyWindow(hwndUpDown);
	hwndUpDown = NULL;
}

int IsStringEditWindow(HWND hwnd)
{
	if (hwnd == hwndEdit) return 1;
	if (hwnd == hwndExpandButton) return 1;
	if (hwnd == hwndUpDown) return 1;
	return 0;
}
