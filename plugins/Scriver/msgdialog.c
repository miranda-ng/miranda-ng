/*
Scriver

Copyright 2000-2012 Miranda ICQ/IM project,

all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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
#include "commonheaders.h"
#include "statusicon.h"
#include "infobar.h"

#define TIMERID_MSGSEND      0
#define TIMERID_FLASHWND     1
#define TIMERID_TYPE         2
#define TIMERID_UNREAD       3
#define TIMEOUT_TYPEOFF      10000      //send type off after 10 seconds of inactivity
#define TIMEOUT_UNREAD		 800       //multiple-send bombproofing: send max 3 messages every 4 seconds
#define VALID_AVATAR(x)      (x==PA_FORMAT_PNG||x==PA_FORMAT_JPEG||x==PA_FORMAT_ICON||x==PA_FORMAT_BMP||x==PA_FORMAT_GIF)

#define ENTERCLICKTIME   1000   //max time in ms during which a double-tap on enter will cause a send

extern HCURSOR hCurSplitNS, hCurSplitWE, hCurHyperlinkHand, hDragCursor;
extern HANDLE hHookWinEvt;
extern HANDLE hHookWinPopup;
extern struct CREOleCallback reOleCallback, reOleCallback2;
extern HINSTANCE g_hInst;

static void UpdateReadChars(HWND hwndDlg, struct MessageWindowData * dat);

static WNDPROC OldMessageEditProc, OldLogEditProc;
static ToolbarButton toolbarButtons[] = {
	{_T("Quote"), IDC_QUOTE, 0, 4, 24},
	{_T("Smiley"), IDC_SMILEYS, 0, 10, 24},
	{_T("Add Contact"), IDC_ADD, 0, 10, 24},
	{_T("User Menu"), IDC_USERMENU, 1, 0, 24},
	{_T("User Details"), IDC_DETAILS, 1, 0, 24},
	{_T("History"), IDC_HISTORY, 1, 0, 24},
	{_T("Send"), IDOK, 1, 0, 38}
};

static DWORD CALLBACK StreamOutCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG * pcb)
{
	MessageSendQueueItem * msi = (MessageSendQueueItem *) dwCookie;
	msi->sendBuffer = (char *)mir_realloc(msi->sendBuffer, msi->sendBufferSize + cb + 2);
	memcpy (msi->sendBuffer + msi->sendBufferSize, pbBuff, cb);
	msi->sendBufferSize += cb;
	*((TCHAR *)(msi->sendBuffer+msi->sendBufferSize)) = '\0';
	*pcb = cb;
    return 0;
}

static TCHAR *GetIEViewSelection(struct MessageWindowData *dat) {
	IEVIEWEVENT event;
	ZeroMemory(&event, sizeof(event));
	event.cbSize = sizeof(event);
#ifdef _UNICODE
	event.dwFlags = 0;
#else
	event.dwFlags = IEEF_NO_UNICODE;
#endif
	event.codepage = dat->windowData.codePage;
	event.hwnd = dat->windowData.hwndLog;
	event.hContact = dat->windowData.hContact;
	event.iType = IEE_GET_SELECTION;
	return mir_tstrdup((TCHAR *)CallService(MS_IEVIEW_EVENT, 0, (LPARAM)&event));
}

static TCHAR *GetQuotedTextW(TCHAR * text) {
	int i, j, l, newLine, wasCR;
	TCHAR *out;
#ifdef _UNICODE
	l = (int)wcslen(text);
#else
	l = strlen(text);
#endif
	newLine = 1;
	wasCR = 0;
	for (i=j=0; i<l; i++) {
		if (text[i]=='\r') {
			wasCR = 1;
			newLine = 1;
			j += text[i+1]!='\n' ? 2 : 1;
		} else if (text[i]=='\n') {
			newLine = 1;
			j += wasCR ? 1 : 2;
			wasCR = 0;
		} else {
			j++;
			if (newLine) {
				//for (;i<l && text[i]=='>';i++) j--;
				j+=2;
			}
			newLine = 0;
			wasCR = 0;
		}
	}
	j+=3;
	out = (TCHAR *)mir_alloc(sizeof(TCHAR) * j);
	newLine = 1;
	wasCR = 0;
	for (i=j=0; i<l; i++) {
		if (text[i]=='\r') {
			wasCR = 1;
			newLine = 1;
			out[j++] = '\r';
			if (text[i+1]!='\n') {
				out[j++]='\n';
			}
		} else if (text[i]=='\n') {
			newLine = 1;
			if (!wasCR) {
				out[j++]='\r';
			}
			out[j++]='\n';
			wasCR = 0;
		} else {
			if (newLine) {
				out[j++]='>';
				out[j++]=' ';
				//for (;i<l && text[i]=='>';i++) j--;
			}
			newLine = 0;
			wasCR = 0;
			out[j++]=text[i];
		}
	}
	out[j++]='\r';
	out[j++]='\n';
	out[j++]='\0';
	return out;
}

static void saveDraftMessage(HWND hwnd, HANDLE hContact, int codepage) {
	char *textBuffer = GetRichTextEncoded(hwnd, codepage);
	if (textBuffer != NULL) {
		g_dat->draftList = tcmdlist_append2(g_dat->draftList, hContact, textBuffer);
		mir_free(textBuffer);
	} else {
		g_dat->draftList = tcmdlist_remove2(g_dat->draftList, hContact);
	}
}

void NotifyLocalWinEvent(HANDLE hContact, HWND hwnd, unsigned int type) {
	MessageWindowEventData mwe = { 0 };
	BOOL bChat = FALSE;
	if (hContact==NULL || hwnd==NULL) return;
	mwe.cbSize = sizeof(mwe);
	mwe.hContact = hContact;
	mwe.hwndWindow = hwnd;
	mwe.szModule = SRMMMOD;
	mwe.uType = type;
	mwe.uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;
	bChat = (WindowList_Find(g_dat->hMessageWindowList, hContact) == NULL);
	mwe.hwndInput = GetDlgItem(hwnd, bChat ? IDC_CHAT_MESSAGE : IDC_MESSAGE);
	mwe.hwndLog = GetDlgItem(hwnd, bChat ? IDC_CHAT_LOG : IDC_LOG);
	NotifyEventHooks(hHookWinEvt, 0, (LPARAM)&mwe);
}

static BOOL IsUtfSendAvailable(HANDLE hContact)
{
	char* szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
	if ( szProto == NULL )
		return FALSE;

	return ( CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_4, 0) & PF4_IMSENDUTF ) ? TRUE : FALSE;
}

#if defined(_UNICODE)
int RTL_Detect(WCHAR *pszwText)
{
    WORD *infoTypeC2;
    int i;
    int iLen = lstrlenW(pszwText);

    infoTypeC2 = (WORD *)mir_alloc(sizeof(WORD) * (iLen + 2));

    if(infoTypeC2) {
        ZeroMemory(infoTypeC2, sizeof(WORD) * (iLen + 2));

        GetStringTypeW(CT_CTYPE2, pszwText, iLen, infoTypeC2);

        for(i = 0; i < iLen; i++) {
            if(infoTypeC2[i] == C2_RIGHTTOLEFT) {
                mir_free(infoTypeC2);
                return 1;
            }
        }
        mir_free(infoTypeC2);
    }
    return 0;
}
#endif

static void AddToFileList(TCHAR ***pppFiles,int *totalCount,const TCHAR* szFilename)
{
	*pppFiles=(TCHAR**)mir_realloc(*pppFiles,(++*totalCount+1)*sizeof(TCHAR*));
	(*pppFiles)[*totalCount] = NULL;
	(*pppFiles)[*totalCount-1] = mir_tstrdup( szFilename );

	if ( GetFileAttributes(szFilename) & FILE_ATTRIBUTE_DIRECTORY ) {
		WIN32_FIND_DATA fd;
		HANDLE hFind;
		TCHAR szPath[MAX_PATH];
		lstrcpy(szPath,szFilename);
		lstrcat(szPath,_T("\\*"));
		if (( hFind = FindFirstFile( szPath, &fd )) != INVALID_HANDLE_VALUE ) {
			do {
				if ( !lstrcmp(fd.cFileName,_T(".")) || !lstrcmp(fd.cFileName,_T(".."))) continue;
				lstrcpy(szPath,szFilename);
				lstrcat(szPath,_T("\\"));
				lstrcat(szPath,fd.cFileName);
				AddToFileList(pppFiles,totalCount,szPath);
			}
				while( FindNextFile( hFind,&fd ));
			FindClose( hFind );
}	}	}

static void SetDialogToType(HWND hwndDlg)
{
	BOOL showToolbar = SendMessage(GetParent(hwndDlg), CM_GETTOOLBARSTATUS, 0, 0);
	struct MessageWindowData *dat = (struct MessageWindowData *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	ParentWindowData *pdat = dat->parent;

	if (pdat->flags2 & SMF2_SHOWINFOBAR) {
		ShowWindow(dat->infobarData->hWnd, SW_SHOW);
	} else {
		ShowWindow(dat->infobarData->hWnd, SW_HIDE);
	}
	if (dat->windowData.hContact) {
		ShowToolbarControls(hwndDlg, SIZEOF(toolbarButtons), toolbarButtons, g_dat->buttonVisibility, showToolbar ? SW_SHOW : SW_HIDE);
		if (!DBGetContactSettingByte(dat->windowData.hContact, "CList", "NotOnList", 0)) {
			ShowWindow(GetDlgItem(hwndDlg, IDC_ADD), SW_HIDE);
		}
		if (!g_dat->smileyAddInstalled) {
			ShowWindow(GetDlgItem(hwndDlg, IDC_SMILEYS), SW_HIDE);
		}
	} else {
		ShowToolbarControls(hwndDlg, SIZEOF(toolbarButtons), toolbarButtons, g_dat->buttonVisibility, SW_HIDE);
	}
	ShowWindow(GetDlgItem(hwndDlg, IDC_MESSAGE), SW_SHOW);
	if (dat->windowData.hwndLog != NULL) {
		ShowWindow (GetDlgItem(hwndDlg, IDC_LOG), SW_HIDE);
	} else {
		ShowWindow (GetDlgItem(hwndDlg, IDC_LOG), SW_SHOW);
	}
	ShowWindow(GetDlgItem(hwndDlg, IDC_SPLITTER), SW_SHOW);
	UpdateReadChars(hwndDlg, dat);
	EnableWindow(GetDlgItem(hwndDlg, IDOK), GetRichTextLength(GetDlgItem(hwndDlg, IDC_MESSAGE), dat->windowData.codePage, FALSE)?TRUE:FALSE);
	SendMessage(hwndDlg, DM_CLISTSETTINGSCHANGED, 0, 0);
	SendMessage(hwndDlg, WM_SIZE, 0, 0);
}


void SetStatusIcon(struct MessageWindowData *dat) {
	if (dat->szProto != NULL) {
		char *szProto = dat->szProto;
		HANDLE hContact = dat->windowData.hContact;

		char* szMetaProto = (char*)CallService(MS_MC_GETPROTOCOLNAME, 0, 0);
		if ((INT_PTR)szMetaProto != CALLSERVICE_NOTFOUND && strcmp(dat->szProto, szMetaProto) == 0 &&
			DBGetContactSettingByte(NULL,"CLC","Meta",0) == 0) {
			hContact = (HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT,(WPARAM)dat->windowData.hContact, 0);
			if (hContact != NULL) {
				szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hContact,0);
			} else {
				hContact = dat->windowData.hContact;
			}
		}

		CallService(MS_SKIN2_RELEASEICON, (WPARAM)dat->statusIcon, 0);
		dat->statusIcon = LoadSkinnedProtoIcon(szProto, dat->wStatus);

		CallService(MS_SKIN2_RELEASEICON, (WPARAM)dat->statusIconBig, 0);
		dat->statusIconBig = LoadSkinnedProtoIconBig(szProto, dat->wStatus);
        if ((int)dat->statusIconBig == CALLSERVICE_NOTFOUND) {
            dat->statusIconBig = NULL;
        }
		if (dat->statusIconOverlay != NULL) DestroyIcon(dat->statusIconOverlay);
		{
			int index = ImageList_ReplaceIcon(g_dat->hHelperIconList, 0, dat->statusIcon);
			dat->statusIconOverlay = ImageList_GetIcon(g_dat->hHelperIconList, index, ILD_TRANSPARENT|INDEXTOOVERLAYMASK(1));
		}
	}
}

void GetTitlebarIcon(struct MessageWindowData *dat, TitleBarData *tbd) {
	if (dat->showTyping && (g_dat->flags2 & SMF2_SHOWTYPINGWIN)) {
		tbd->hIconNot = tbd->hIcon = GetCachedIcon("scriver_TYPING");
	} else if (dat->showUnread && (GetActiveWindow() != dat->hwndParent || GetForegroundWindow() != dat->hwndParent)) {
		tbd->hIcon = (g_dat->flags & SMF_STATUSICON) ? dat->statusIcon : g_dat->hMsgIcon;
		tbd->hIconNot = (g_dat->flags & SMF_STATUSICON) ? g_dat->hMsgIcon : GetCachedIcon("scriver_OVERLAY");
	} else {
		tbd->hIcon = (g_dat->flags & SMF_STATUSICON) ? dat->statusIcon : g_dat->hMsgIcon;
		tbd->hIconNot = NULL;
	}
	tbd->hIconBig = (g_dat->flags & SMF_STATUSICON) ? dat->statusIconBig : g_dat->hMsgIconBig;
}

HICON GetTabIcon(struct MessageWindowData *dat) {
	if (dat->showTyping) {
		return GetCachedIcon("scriver_TYPING");
	} else if (dat->showUnread != 0) {
		return dat->statusIconOverlay;
	}
	return dat->statusIcon;
}


struct MsgEditSubclassData
{
	DWORD lastEnterTime;
};

static LRESULT CALLBACK LogEditSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static BOOL inMenu = FALSE;
	switch (msg) {
		case WM_MEASUREITEM:
			MeasureMenuItem(wParam, lParam);
			return TRUE;
		case WM_DRAWITEM:
			return DrawMenuItem(wParam, lParam);
		case WM_SETCURSOR:
			if (inMenu) {
				SetCursor(LoadCursor(NULL, IDC_ARROW));
				return TRUE;
			}
		break;
		case WM_CONTEXTMENU:
		{
			HMENU hMenu, hSubMenu;
			TCHAR *pszWord;
			POINT pt;
			POINTL ptl;
			int uID;
			CHARRANGE sel, all = { 0, -1 };
			hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_CONTEXT));
			hSubMenu = GetSubMenu(hMenu, 0);
			CallService(MS_LANGPACK_TRANSLATEMENU, (WPARAM) hSubMenu, 0);
			SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM) & sel);
			if (sel.cpMin == sel.cpMax)
				EnableMenuItem(hSubMenu, IDM_COPY, MF_BYCOMMAND | MF_GRAYED);

			if (lParam == 0xFFFFFFFF) {
				SendMessage(hwnd, EM_POSFROMCHAR, (WPARAM) & pt, (LPARAM) sel.cpMax);
				ClientToScreen(hwnd, &pt);
			} else {
				pt.x = (short) LOWORD(lParam);
				pt.y = (short) HIWORD(lParam);
			}
			ptl.x = (LONG)pt.x;
			ptl.y = (LONG)pt.y;
			ScreenToClient(hwnd, (LPPOINT)&ptl);
			pszWord = GetRichTextWord(hwnd, &ptl);
			if ( pszWord && pszWord[0] ) {
				TCHAR szMenuText[4096];
				mir_sntprintf( szMenuText, 4096, TranslateT("Look up \'%s\':"), pszWord );
				ModifyMenu( hSubMenu, 5, MF_STRING|MF_BYPOSITION, 5, szMenuText );
				SetSearchEngineIcons(hMenu, g_dat->hSearchEngineIconList);
			}
			else ModifyMenu( hSubMenu, 5, MF_STRING|MF_GRAYED|MF_BYPOSITION, 5, TranslateT( "No word to look up" ));
			inMenu = TRUE;
			uID = TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, NULL);
			inMenu = FALSE;
			switch (uID) {
			case IDM_COPY:
				SendMessage(hwnd, WM_COPY, 0, 0);
				break;
			case IDM_COPYALL:
				SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM) & all);
				SendMessage(hwnd, WM_COPY, 0, 0);
				SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM) & sel);
				break;
			case IDM_SELECTALL:
				SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM) & all);
				break;
			case IDM_CLEAR:
				SendMessage(GetParent(hwnd), DM_CLEARLOG, 0, 0);
				break;
			case IDM_SEARCH_GOOGLE:
			case IDM_SEARCH_BING:
			case IDM_SEARCH_YAHOO:
			case IDM_SEARCH_WIKIPEDIA:
			case IDM_SEARCH_FOODNETWORK:
			case IDM_SEARCH_GOOGLE_MAPS:
			case IDM_SEARCH_GOOGLE_TRANSLATE:
				SearchWord(pszWord, uID - IDM_SEARCH_GOOGLE + SEARCHENGINE_GOOGLE);
				PostMessage(GetParent(hwnd), WM_MOUSEACTIVATE, 0, 0 );
				break;
			}
			DestroyMenu(hMenu);
			mir_free(pszWord);
			return TRUE;
		}
	}
	return CallWindowProc(OldLogEditProc, hwnd, msg, wParam, lParam);
}

static LRESULT CALLBACK MessageEditSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int result = -1;
	struct MsgEditSubclassData *dat;
	struct MessageWindowData *pdat;
	CommonWindowData *windowData;
	BOOL isCtrl = GetKeyState(VK_CONTROL) & 0x8000;
	BOOL isAlt = GetKeyState(VK_MENU) & 0x8000;
	dat = (struct MsgEditSubclassData *) GetWindowLongPtr(hwnd, GWLP_USERDATA);
	pdat=(struct MessageWindowData *)GetWindowLongPtr(GetParent(hwnd),GWLP_USERDATA);
	windowData = &pdat->windowData;

	result = InputAreaShortcuts(hwnd, msg, wParam, lParam, windowData);
	if (result != -1) {
		return result;
	}

	switch (msg) {
	case EM_SUBCLASSED:
		dat = (struct MsgEditSubclassData *) mir_alloc(sizeof(struct MsgEditSubclassData));
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) dat);
		dat->lastEnterTime = 0;
		return 0;

	case WM_KEYDOWN:
		{
			if (wParam == VK_RETURN) {
				if ((isCtrl != 0) ^ (0 != (g_dat->flags & SMF_SENDONENTER))) {
					PostMessage(GetParent(hwnd), WM_COMMAND, IDOK, 0);
					return 0;
				}
				if (g_dat->flags & SMF_SENDONDBLENTER) {
					if (dat->lastEnterTime + ENTERCLICKTIME < GetTickCount())
						dat->lastEnterTime = GetTickCount();
					else {
						SendMessage(hwnd, WM_KEYDOWN, VK_BACK, 0);
						SendMessage(hwnd, WM_KEYUP, VK_BACK, 0);
						PostMessage(GetParent(hwnd), WM_COMMAND, IDOK, 0);
						return 0;
					}
				}
			}
			else
				dat->lastEnterTime = 0;
			if (((wParam == VK_INSERT && (GetKeyState(VK_SHIFT) & 0x8000)) || (wParam == 'V' && (GetKeyState(VK_CONTROL) & 0x8000))) &&
				!(GetKeyState(VK_MENU) & 0x8000))
			{
				SendMessage(hwnd, WM_PASTE, 0, 0);
				return 0;
			}

		}
		break;
		//fall through
	case WM_MOUSEWHEEL:
		if ((GetWindowLong(hwnd, GWL_STYLE) & WS_VSCROLL) == 0) {
			SendMessage(GetDlgItem(GetParent(hwnd), IDC_LOG), WM_MOUSEWHEEL, wParam, lParam);
		}
		break;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_KILLFOCUS:
		dat->lastEnterTime = 0;
		break;
	case WM_SYSCHAR:
		dat->lastEnterTime = 0;
		if ((wParam == 's' || wParam == 'S') && isAlt) {
			PostMessage(GetParent(hwnd), WM_COMMAND, IDOK, 0);
			return 0;
		}
		break;
	case WM_PASTE:
		if (IsClipboardFormatAvailable(CF_HDROP))
		{
			if (OpenClipboard(hwnd))
			{
				HANDLE hDrop = GetClipboardData(CF_HDROP);
				if (hDrop)
					SendMessage(hwnd, WM_DROPFILES, (WPARAM)hDrop, 0);
				CloseClipboard();
			}
		}
		else
			SendMessage(hwnd, EM_PASTESPECIAL, CF_TEXT, 0);
		return 0;
	case WM_DROPFILES:
		SendMessage(GetParent(hwnd), WM_DROPFILES, wParam, lParam);
		return 0;
	case WM_CONTEXTMENU:
		InputAreaContextMenu(hwnd, wParam, lParam, pdat->windowData.hContact);
		return TRUE;
	case EM_UNSUBCLASSED:
		mir_free(dat);
		return 0;
	}
	return CallWindowProc(OldMessageEditProc, hwnd, msg, wParam, lParam);
}

static void SubclassMessageEdit(HWND hwnd) {
	OldMessageEditProc = (WNDPROC) SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR) MessageEditSubclassProc);
	SendMessage(hwnd, EM_SUBCLASSED, 0, 0);
}

static void UnsubclassMessageEdit(HWND hwnd) {
	SendMessage(hwnd, EM_UNSUBCLASSED, 0, 0);
	SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR) OldMessageEditProc);
}

static void SubclassLogEdit(HWND hwnd) {
	OldLogEditProc = (WNDPROC) SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR) LogEditSubclassProc);
	SendMessage(hwnd, EM_SUBCLASSED, 0, 0);
}

static void UnsubclassLogEdit(HWND hwnd) {
	SendMessage(hwnd, EM_UNSUBCLASSED, 0, 0);
	SetWindowLong(hwnd, GWLP_WNDPROC, (LONG_PTR) OldLogEditProc);
}

static void MessageDialogResize(HWND hwndDlg, struct MessageWindowData *dat, int w, int h) {
	HDWP hdwp;
	ParentWindowData *pdat = dat->parent;
	int hSplitterPos = dat->splitterPos, toolbarHeight = pdat->flags2&SMF2_SHOWTOOLBAR ? IsToolbarVisible(SIZEOF(toolbarButtons), g_dat->buttonVisibility) ? dat->toolbarSize.cy : dat->toolbarSize.cy / 3 : 0;
	int hSplitterMinTop = toolbarHeight + dat->windowData.minLogBoxHeight, hSplitterMinBottom = dat->windowData.minEditBoxHeight;
	int infobarHeight = INFO_BAR_INNER_HEIGHT;
	int avatarWidth = 0, avatarHeight = 0;
	int toolbarWidth = w;
	int messageEditWidth = w - 2;
	int logY, logH;

	if (!(pdat->flags2 & SMF2_SHOWINFOBAR)) {
		infobarHeight = 0;
	}
	hSplitterPos = dat->desiredInputAreaHeight + SPLITTER_HEIGHT + 3;
	if (h - hSplitterPos - INFO_BAR_HEIGHT< hSplitterMinTop) {
		hSplitterPos = h - hSplitterMinTop - INFO_BAR_HEIGHT;
	}
	if (hSplitterPos < avatarHeight) {
		hSplitterPos = avatarHeight;
	}
	if (hSplitterPos < hSplitterMinBottom) {
		hSplitterPos = hSplitterMinBottom;
	}
	if (!(pdat->flags2 & SMF2_SHOWINFOBAR)) {
		if (dat->avatarPic && (g_dat->flags&SMF_AVATAR)) {
			avatarWidth = BOTTOM_RIGHT_AVATAR_HEIGHT;
			avatarHeight = toolbarHeight + hSplitterPos - 2;
			if (avatarHeight < BOTTOM_RIGHT_AVATAR_HEIGHT) {
				avatarHeight = BOTTOM_RIGHT_AVATAR_HEIGHT;
				hSplitterPos = avatarHeight - toolbarHeight + 2;
			} else {
				avatarHeight = BOTTOM_RIGHT_AVATAR_HEIGHT;
			}
			avatarWidth = avatarHeight;
			if (avatarWidth > BOTTOM_RIGHT_AVATAR_HEIGHT && avatarWidth > w/4) {
				avatarWidth = w /4;
			}
			if ((toolbarWidth - avatarWidth - 2) < dat->toolbarSize.cx) {
				avatarWidth = toolbarWidth - dat->toolbarSize.cx - 2;
			}
			toolbarWidth -= avatarWidth + 2;
			messageEditWidth -= avatarWidth + 1;
		}
	}

	dat->splitterPos = hSplitterPos;

	logY = infobarHeight;
	logH = h-hSplitterPos-toolbarHeight - infobarHeight;
	hdwp = BeginDeferWindowPos(15);
	hdwp = DeferWindowPos(hdwp, dat->infobarData->hWnd, 0, 1, 0, w - 2, infobarHeight - 2, SWP_NOZORDER);
	hdwp = DeferWindowPos(hdwp, GetDlgItem(hwndDlg, IDC_LOG), 0, 1, logY, w-2, logH, SWP_NOZORDER);
	hdwp = DeferWindowPos(hdwp, GetDlgItem(hwndDlg, IDC_MESSAGE), 0, 1, h - hSplitterPos + SPLITTER_HEIGHT, messageEditWidth, hSplitterPos - SPLITTER_HEIGHT - 1, SWP_NOZORDER);
	hdwp = DeferWindowPos(hdwp, GetDlgItem(hwndDlg, IDC_AVATAR), 0, w-avatarWidth - 1, h - (avatarHeight + avatarWidth) / 2 - 1, avatarWidth, avatarWidth, SWP_NOZORDER);

	hdwp = DeferWindowPos(hdwp, GetDlgItem(hwndDlg, IDC_SPLITTER), 0, 0, h - hSplitterPos-1, toolbarWidth, SPLITTER_HEIGHT, SWP_NOZORDER);
	hdwp = ResizeToolbar(hwndDlg, hdwp, toolbarWidth, h - hSplitterPos - toolbarHeight + 1, toolbarHeight, SIZEOF(toolbarButtons), toolbarButtons, g_dat->buttonVisibility);

	/*
	if (hSplitterPos - SPLITTER_HEIGHT - toolbarHeight - 2< dat->avatarHeight) {
		hSplitterPos = dat->avatarHeight + SPLITTER_HEIGHT + toolbarHeight + 2;
	}
	dat->splitterPos = hSplitterPos;
	hdwp = BeginDeferWindowPos(12);
	hdwp = DeferWindowPos(hdwp, GetDlgItem(hwndDlg, IDC_LOG), 0, 0, 0, w-vSplitterPos, h-hSplitterPos, SWP_NOZORDER);
	hdwp = DeferWindowPos(hdwp, GetDlgItem(hwndDlg, IDC_MESSAGE), 0, 0, h - hSplitterPos + SPLITTER_HEIGHT, w, hSplitterPos - SPLITTER_HEIGHT - toolbarHeight -2, SWP_NOZORDER);
	hdwp = DeferWindowPos(hdwp, GetDlgItem(hwndDlg, IDC_SPLITTER), 0, 0, h - hSplitterPos-1, w, SPLITTER_HEIGHT + 1, SWP_NOZORDER);
	if (dat->avatarHeight + 1 < hSplitterPos - toolbarHeight) {
		aPos = h - (hSplitterPos + toolbarHeight + dat->avatarHeight - 1) / 2;
	} else {
		aPos = h - (hSplitterPos + toolbarHeight + dat->avatarHeight + 1) / 2;
	}
	vPos = h - toolbarHeight;
	hdwp = ResizeToolbar(hwndDlg, hdwp, w, vPos, toolbarHeight, SIZEOF(buttonControls),
					buttonControls, buttonWidth, buttonSpacing, buttonAlignment, g_dat->buttonVisibility);

*/
	EndDeferWindowPos(hdwp);
	if (dat->windowData.hwndLog != NULL) {
		IEVIEWWINDOW ieWindow;
		ieWindow.cbSize = sizeof(IEVIEWWINDOW);
		ieWindow.iType = IEW_SETPOS;
		ieWindow.parent = hwndDlg;
		ieWindow.hwnd = dat->windowData.hwndLog;
        ieWindow.x = 0;
        ieWindow.y = logY;
        ieWindow.cx = w;
        ieWindow.cy = logH;
		CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWindow);
	} else {
		RedrawWindow(GetDlgItem(hwndDlg, IDC_LOG), NULL, NULL, RDW_INVALIDATE);
	}
	RedrawWindow(GetDlgItem(hwndDlg, IDC_MESSAGE), NULL, NULL, RDW_INVALIDATE);

    RefreshInfobar(dat->infobarData);

	RedrawWindow(GetDlgItem(hwndDlg, IDC_AVATAR), NULL, NULL, RDW_INVALIDATE);
}

static void UpdateReadChars(HWND hwndDlg, struct MessageWindowData * dat)
{
	if (dat->parent->hwndActive == hwndDlg) {
		TCHAR szText[256];
		StatusBarData sbd;
		int len = GetRichTextLength(GetDlgItem(hwndDlg, IDC_MESSAGE), dat->windowData.codePage, FALSE);
		sbd.iItem = 1;
		sbd.iFlags = SBDF_TEXT | SBDF_ICON;
		sbd.hIcon = NULL;
		sbd.pszText = szText;
		mir_sntprintf(szText, SIZEOF(szText), _T("%d"), len);
		SendMessage(dat->hwndParent, CM_UPDATESTATUSBAR, (WPARAM)&sbd, (LPARAM)hwndDlg);
	}
}

void ShowAvatar(HWND hwndDlg, struct MessageWindowData *dat)
{
	INT_PTR res = CallService(MS_AV_GETAVATARBITMAP, (WPARAM)dat->windowData.hContact, 0);
	dat->ace = res != CALLSERVICE_NOTFOUND ? (AVATARCACHEENTRY*)res : NULL;
	dat->avatarPic = (dat->ace != NULL && (dat->ace->dwFlags & AVS_HIDEONCLIST) == 0) ? dat->ace->hbmPic : NULL;
	SendMessage(hwndDlg, WM_SIZE, 0, 0);

    RefreshInfobar(dat->infobarData);

	RedrawWindow(GetDlgItem(hwndDlg, IDC_AVATAR), NULL, NULL, RDW_INVALIDATE);
}

static BOOL IsTypingNotificationSupported(struct MessageWindowData *dat) {
	DWORD typeCaps;
	if (!dat->windowData.hContact)
		return FALSE;
	if (!dat->szProto)
		return FALSE;
	typeCaps = CallProtoService(dat->szProto, PS_GETCAPS, PFLAGNUM_4, 0);
	if (!(typeCaps & PF4_SUPPORTTYPING))
		return FALSE;
	return TRUE;
}

static BOOL IsTypingNotificationEnabled(struct MessageWindowData *dat) {
	DWORD protoStatus;
	DWORD protoCaps;
	if (!DBGetContactSettingByte(dat->windowData.hContact, SRMMMOD, SRMSGSET_TYPING, DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_TYPINGNEW, SRMSGDEFSET_TYPINGNEW)))
		return FALSE;
	protoStatus = CallProtoService(dat->szProto, PS_GETSTATUS, 0, 0);
	if (protoStatus < ID_STATUS_ONLINE)
		return FALSE;
	protoCaps = CallProtoService(dat->szProto, PS_GETCAPS, PFLAGNUM_1, 0);
	if (protoCaps & PF1_VISLIST && DBGetContactSettingWord(dat->windowData.hContact, dat->szProto, "ApparentMode", 0) == ID_STATUS_OFFLINE)
		return FALSE;
	if (protoCaps & PF1_INVISLIST && protoStatus == ID_STATUS_INVISIBLE && DBGetContactSettingWord(dat->windowData.hContact, dat->szProto, "ApparentMode", 0) != ID_STATUS_ONLINE)
		return FALSE;
	if (DBGetContactSettingByte(dat->windowData.hContact, "CList", "NotOnList", 0)
		&& !DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_TYPINGUNKNOWN, SRMSGDEFSET_TYPINGUNKNOWN))
		return FALSE;
	return TRUE;
}

// Don't send to protocols who don't support typing
// Don't send to users who are unchecked in the typing notification options
// Don't send to protocols that are offline
// Don't send to users who are not visible and
// Don't send to users who are not on the visible list when you are in invisible mode.
static void NotifyTyping(struct MessageWindowData *dat, int mode) {
	if (!IsTypingNotificationSupported(dat)) {
		return;
	}
	if (!IsTypingNotificationEnabled(dat)) {
		return;
	}
	// End user check
	dat->nTypeMode = mode;
	CallService(MS_PROTO_SELFISTYPING, (WPARAM) dat->windowData.hContact, dat->nTypeMode);
}

static INT_PTR CALLBACK ConfirmSendAllDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		{
			RECT rcParent, rcChild;
			TranslateDialogDefault( hwndDlg );
			GetWindowRect(GetParent(hwndDlg), &rcParent);
			GetWindowRect(hwndDlg, &rcChild);
			rcChild.bottom -= rcChild.top;
			rcChild.right -= rcChild.left;
			rcParent.bottom -= rcParent.top;
			rcParent.right -= rcParent.left;
			rcChild.left = rcParent.left + (rcParent.right - rcChild.right) / 2;
			rcChild.top = rcParent.top + (rcParent.bottom - rcChild.bottom) / 2;
			MoveWindow(hwndDlg, rcChild.left, rcChild.top, rcChild.right, rcChild.bottom, FALSE);
		}
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDYES:
		case IDNO:
		case IDCANCEL:
			{
				int result = LOWORD(wParam);
				if (IsDlgButtonChecked(hwndDlg, IDC_REMEMBER)) {
					result |= 0x10000;
				}
				EndDialog(hwndDlg, result);
			}
			return TRUE;
		}
		break;
	}

	return FALSE;
}


INT_PTR CALLBACK DlgProcMessage(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HMENU hToolbarMenu;
	struct MessageWindowData *dat;
	dat = (struct MessageWindowData *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	if (!dat && msg!=WM_INITDIALOG) return FALSE;
	switch (msg) {
	case WM_INITDIALOG:
		{
			int len = 0;
			int notifyUnread = 0;
			RECT minEditInit;
			NewMessageWindowLParam *newData = (NewMessageWindowLParam *) lParam;
			//TranslateDialogDefault(hwndDlg);
			dat = (struct MessageWindowData *) mir_alloc(sizeof(struct MessageWindowData));
			ZeroMemory(dat, sizeof(struct MessageWindowData));
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) dat);
			dat->windowData.hContact = newData->hContact;
			NotifyLocalWinEvent(dat->windowData.hContact, hwndDlg, MSG_WINDOW_EVT_OPENING);
//			SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_SETTEXTMODE, TM_PLAINTEXT, 0);

			dat->hwnd = hwndDlg;
			dat->hwndParent = GetParent(hwndDlg);
			dat->parent = (ParentWindowData *) GetWindowLongPtr(dat->hwndParent, GWLP_USERDATA);
			dat->windowData.hwndLog = NULL;
			dat->szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) dat->windowData.hContact, 0);
			dat->avatarPic = 0;
			if (dat->windowData.hContact && dat->szProto != NULL)
				dat->wStatus = DBGetContactSettingWord(dat->windowData.hContact, dat->szProto, "Status", ID_STATUS_OFFLINE);
			else
				dat->wStatus = ID_STATUS_OFFLINE;
			dat->hDbEventFirst = NULL;
			dat->hDbEventLast = NULL;
			dat->hDbUnreadEventFirst = NULL;
			dat->messagesInProgress = 0;
			dat->nTypeSecs = 0;
			dat->nLastTyping = 0;
			dat->showTyping = 0;
			dat->showUnread = 0;
			dat->sendAllConfirm = 0;
			dat->nTypeMode = PROTOTYPE_SELFTYPING_OFF;
			SetTimer(hwndDlg, TIMERID_TYPE, 1000, NULL);
			dat->lastMessage = 0;
			dat->lastEventType = -1;
			dat->lastEventTime = time(NULL);
			dat->startTime = time(NULL);
			dat->flags = 0;
			if (DBGetContactSettingByte(dat->windowData.hContact, SRMMMOD, "UseRTL", (BYTE) 0)) {
				dat->flags |= SMF_RTL;
			}
			dat->flags |= ServiceExists(MS_IEVIEW_WINDOW) ? g_dat->flags & SMF_USEIEVIEW : 0;
			{
				PARAFORMAT2 pf2;
				ZeroMemory((void *)&pf2, sizeof(pf2));
				pf2.cbSize = sizeof(pf2);
				pf2.dwMask = PFM_RTLPARA;
				if (!(dat->flags & SMF_RTL)) {
					pf2.wEffects = 0;
					SetWindowLong(GetDlgItem(hwndDlg, IDC_MESSAGE),GWL_EXSTYLE,GetWindowLong(GetDlgItem(hwndDlg, IDC_MESSAGE),GWL_EXSTYLE) & ~(WS_EX_RIGHT | WS_EX_RTLREADING | WS_EX_LEFTSCROLLBAR));
				} else {
					pf2.wEffects = PFE_RTLPARA;
					SetWindowLong(GetDlgItem(hwndDlg, IDC_MESSAGE),GWL_EXSTYLE,GetWindowLong(GetDlgItem(hwndDlg, IDC_MESSAGE),GWL_EXSTYLE) | WS_EX_RIGHT | WS_EX_RTLREADING | WS_EX_LEFTSCROLLBAR);
				}
				SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_SETPARAFORMAT, 0, (LPARAM)&pf2);
				/* Workaround to make Richedit display RTL messages correctly */
				ZeroMemory((void *)&pf2, sizeof(pf2));
				pf2.cbSize = sizeof(pf2);
				pf2.dwMask = PFM_RTLPARA | PFM_OFFSETINDENT | PFM_RIGHTINDENT;
				pf2.wEffects = PFE_RTLPARA;
				pf2.dxStartIndent = 30;
				pf2.dxRightIndent = 30;
				SendDlgItemMessage(hwndDlg, IDC_LOG, EM_SETPARAFORMAT, 0, (LPARAM)&pf2);
				pf2.dwMask = PFM_RTLPARA;
				pf2.wEffects = 0;
				SendDlgItemMessage(hwndDlg, IDC_LOG, EM_SETPARAFORMAT, 0, (LPARAM)&pf2);
				if (dat->flags & SMF_RTL) {
					SetWindowLong(GetDlgItem(hwndDlg, IDC_LOG),GWL_EXSTYLE,GetWindowLong(GetDlgItem(hwndDlg, IDC_LOG),GWL_EXSTYLE) | WS_EX_LEFTSCROLLBAR);
				} else {
					SetWindowLong(GetDlgItem(hwndDlg, IDC_LOG),GWL_EXSTYLE,GetWindowLong(GetDlgItem(hwndDlg, IDC_LOG),GWL_EXSTYLE) & ~WS_EX_LEFTSCROLLBAR);
				}
			}
			dat->windowData.codePage = DBGetContactSettingWord(dat->windowData.hContact, SRMMMOD, "CodePage", (WORD) CP_ACP);
			dat->ace = NULL;
			GetWindowRect(GetDlgItem(hwndDlg, IDC_MESSAGE), &minEditInit);
			dat->windowData.minEditBoxHeight = minEditInit.bottom - minEditInit.top;
			dat->windowData.minLogBoxHeight = dat->windowData.minEditBoxHeight;
			dat->toolbarSize.cy = TOOLBAR_HEIGHT;
			dat->toolbarSize.cx = GetToolbarWidth(SIZEOF(toolbarButtons), toolbarButtons);
			if (dat->splitterPos == -1) {
				dat->splitterPos = dat->windowData.minEditBoxHeight;
			}
			WindowList_Add(g_dat->hMessageWindowList, hwndDlg, dat->windowData.hContact);

			if (newData->szInitialText) {
	#if defined(_UNICODE)
				if(newData->isWchar)
					SetDlgItemText(hwndDlg, IDC_MESSAGE, (TCHAR *)newData->szInitialText);
				else
					SetDlgItemTextA(hwndDlg, IDC_MESSAGE, newData->szInitialText);
	#else
				SetDlgItemTextA(hwndDlg, IDC_MESSAGE, newData->szInitialText);
	#endif
			} else if (g_dat->flags & SMF_SAVEDRAFTS) {
				TCmdList *draft = tcmdlist_get2(g_dat->draftList, dat->windowData.hContact);
				if (draft != NULL) {
					len = SetRichTextEncoded(GetDlgItem(hwndDlg, IDC_MESSAGE), draft->szCmd, dat->windowData.codePage);
				}
				PostMessage(GetDlgItem(hwndDlg, IDC_MESSAGE), EM_SETSEL, len, len);
			}

			SendMessage(hwndDlg, DM_CHANGEICONS, 0, 0);
			// Make them flat buttons
			{
				int i;
				for (i = 0; i < SIZEOF(toolbarButtons) ; i++)
					SendMessage(GetDlgItem(hwndDlg, toolbarButtons[i].controlId), BUTTONSETASFLATBTN, 0, 0);
			}
			SendMessage(GetDlgItem(hwndDlg, IDC_ADD), BUTTONADDTOOLTIP, (WPARAM) Translate("Add Contact Permanently to List"), 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_USERMENU), BUTTONADDTOOLTIP, (WPARAM) Translate("User Menu"), 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_DETAILS), BUTTONADDTOOLTIP, (WPARAM) Translate("View User's Details"), 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_HISTORY), BUTTONADDTOOLTIP, (WPARAM) Translate("View User's History"), 0);

			SendMessage(GetDlgItem(hwndDlg, IDC_QUOTE), BUTTONADDTOOLTIP, (WPARAM) Translate("Quote Text"), 0);
			SendMessage(GetDlgItem(hwndDlg, IDC_SMILEYS), BUTTONADDTOOLTIP, (WPARAM) Translate("Insert Emoticon"), 0);
			SendMessage(GetDlgItem(hwndDlg, IDOK), BUTTONADDTOOLTIP, (WPARAM) Translate("Send Message"), 0);

			SendDlgItemMessage(hwndDlg, IDC_LOG, EM_SETOLECALLBACK, 0, (LPARAM) & reOleCallback);
			SendDlgItemMessage(hwndDlg, IDC_LOG, EM_SETEVENTMASK, 0, ENM_MOUSEEVENTS | ENM_LINK | ENM_KEYEVENTS);
			SendDlgItemMessage(hwndDlg, IDC_LOG, EM_SETEDITSTYLE, SES_EXTENDBACKCOLOR, SES_EXTENDBACKCOLOR);
			SendDlgItemMessage(hwndDlg, IDC_LOG, EM_SETLANGOPTIONS, 0, (LPARAM) SendDlgItemMessage(hwndDlg, IDC_LOG, EM_GETLANGOPTIONS, 0, 0) & ~(IMF_AUTOKEYBOARD | IMF_AUTOFONTSIZEADJUST));
			SendDlgItemMessage(hwndDlg, IDC_LOG, EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(0,0));
			/* duh, how come we didnt use this from the start? */
			SendDlgItemMessage(hwndDlg, IDC_LOG, EM_AUTOURLDETECT, (WPARAM) TRUE, 0);

			SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_SETLANGOPTIONS, 0, (LPARAM) SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_GETLANGOPTIONS, 0, 0) & ~IMF_AUTOKEYBOARD);
			SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_SETOLECALLBACK, 0, (LPARAM) & reOleCallback2);
			SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_SETEVENTMASK, 0, ENM_MOUSEEVENTS | ENM_KEYEVENTS | ENM_CHANGE | ENM_REQUESTRESIZE);
			if (dat->windowData.hContact) {
				if (dat->szProto) {
					int nMax;
					nMax = CallProtoService(dat->szProto, PS_GETCAPS, PFLAG_MAXLENOFMESSAGE, (LPARAM) dat->windowData.hContact);
					if (nMax)
						SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_LIMITTEXT, (WPARAM) nMax, 0);
				}
			}
			/* get around a lame bug in the Windows template resource code where richedits are limited to 0x7FFF */
			SendDlgItemMessage(hwndDlg, IDC_LOG, EM_LIMITTEXT, (WPARAM) sizeof(TCHAR) * 0x7FFFFFFF, 0);
			RichUtil_SubClass(GetDlgItem(hwndDlg, IDC_MESSAGE));
			RichUtil_SubClass(GetDlgItem(hwndDlg, IDC_LOG));
			SubclassLogEdit(GetDlgItem(hwndDlg, IDC_LOG));
			SubclassMessageEdit(GetDlgItem(hwndDlg, IDC_MESSAGE));
			dat->infobarData = CreateInfobar(hwndDlg, dat);
			if (dat->flags & SMF_USEIEVIEW) {
				IEVIEWWINDOW ieWindow;
				ieWindow.cbSize = sizeof(IEVIEWWINDOW);
				ieWindow.iType = IEW_CREATE;
				ieWindow.dwFlags = 0;
				ieWindow.dwMode = IEWM_SCRIVER;
				ieWindow.parent = hwndDlg;
				ieWindow.x = 0;
				ieWindow.y = 0;
				ieWindow.cx = 200;
				ieWindow.cy = 300;
				CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWindow);
				dat->windowData.hwndLog = ieWindow.hwnd;
				if (dat->windowData.hwndLog == NULL) {
					dat->flags ^= SMF_USEIEVIEW;
				}
			}
			if (dat->windowData.hContact) {
				int historyMode = DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_LOADHISTORY, SRMSGDEFSET_LOADHISTORY);
				// This finds the first message to display, it works like shit
				dat->hDbEventFirst = (HANDLE) CallService(MS_DB_EVENT_FINDFIRSTUNREAD, (WPARAM) dat->windowData.hContact, 0);
				if (dat->hDbEventFirst != NULL) {
					DBEVENTINFO dbei = { 0 };
					dbei.cbSize = sizeof(dbei);
					CallService(MS_DB_EVENT_GET, (WPARAM) dat->hDbEventFirst, (LPARAM) & dbei);
					if (DbEventIsMessageOrCustom(&dbei) && !(dbei.flags & DBEF_READ) && !(dbei.flags & DBEF_SENT)) {
						notifyUnread = 1;
					}
				}
				switch (historyMode) {
				case LOADHISTORY_COUNT:
					{
						int i;
						HANDLE hPrevEvent;
						DBEVENTINFO dbei = { 0 };
						dbei.cbSize = sizeof(dbei);
						for (i = DBGetContactSettingWord(NULL, SRMMMOD, SRMSGSET_LOADCOUNT, SRMSGDEFSET_LOADCOUNT); i > 0; i--) {
							if (dat->hDbEventFirst == NULL)
								hPrevEvent = (HANDLE) CallService(MS_DB_EVENT_FINDLAST, (WPARAM) dat->windowData.hContact, 0);
							else
								hPrevEvent = (HANDLE) CallService(MS_DB_EVENT_FINDPREV, (WPARAM) dat->hDbEventFirst, 0);
							if (hPrevEvent == NULL)
								break;
							dbei.cbBlob = 0;
							dat->hDbEventFirst = hPrevEvent;
							CallService(MS_DB_EVENT_GET, (WPARAM) dat->hDbEventFirst, (LPARAM) & dbei);
							if (!DbEventIsShown(&dbei, dat))
								i++;
						}
						break;
					}
				case LOADHISTORY_TIME:
					{
						HANDLE hPrevEvent;
						DBEVENTINFO dbei = { 0 };
						DWORD firstTime;

						dbei.cbSize = sizeof(dbei);
						if (dat->hDbEventFirst == NULL) {
							dbei.timestamp = time(NULL);
							hPrevEvent = (HANDLE) CallService(MS_DB_EVENT_FINDLAST, (WPARAM) dat->windowData.hContact, 0);
						} else {
							CallService(MS_DB_EVENT_GET, (WPARAM) dat->hDbEventFirst, (LPARAM) & dbei);
							hPrevEvent = (HANDLE) CallService(MS_DB_EVENT_FINDPREV, (WPARAM) dat->hDbEventFirst, 0);
						}
						firstTime = dbei.timestamp - 60 * DBGetContactSettingWord(NULL, SRMMMOD, SRMSGSET_LOADTIME, SRMSGDEFSET_LOADTIME);
						for (;;) {
							if (hPrevEvent == NULL)
								break;
							dbei.cbBlob = 0;
							CallService(MS_DB_EVENT_GET, (WPARAM) hPrevEvent, (LPARAM) & dbei);
							if (dbei.timestamp < firstTime)
								break;
							if (DbEventIsShown(&dbei, dat))
								dat->hDbEventFirst = hPrevEvent;
							hPrevEvent = (HANDLE) CallService(MS_DB_EVENT_FINDPREV, (WPARAM) hPrevEvent, 0);
						}
						break;
					}
				}
			}
			SendMessage(dat->hwndParent, CM_ADDCHILD, (WPARAM) hwndDlg, (LPARAM) dat->windowData.hContact);
			{
				DBEVENTINFO dbei = { 0 };
				HANDLE hdbEvent;

				dbei.cbSize = sizeof(dbei);
				hdbEvent = (HANDLE) CallService(MS_DB_EVENT_FINDLAST, (WPARAM) dat->windowData.hContact, 0);
				if (hdbEvent) {
					do {
						ZeroMemory(&dbei, sizeof(dbei));
						dbei.cbSize = sizeof(dbei);
						CallService(MS_DB_EVENT_GET, (WPARAM) hdbEvent, (LPARAM) & dbei);
						if (dbei.eventType == EVENTTYPE_MESSAGE && !(dbei.flags & DBEF_SENT)) {
							dat->lastMessage = dbei.timestamp;
							break;
						}
					}
					while ((hdbEvent = (HANDLE) CallService(MS_DB_EVENT_FINDPREV, (WPARAM) hdbEvent, 0)));
				}
			}
			SendMessage(hwndDlg, DM_OPTIONSAPPLIED, 0, 0);
			SendMessage(GetParent(hwndDlg), CM_POPUPWINDOW, (WPARAM) (newData->flags & NMWLP_INCOMING), (LPARAM) hwndDlg);
			if (notifyUnread) {
				if (GetForegroundWindow() != dat->hwndParent || dat->parent->hwndActive != hwndDlg) {
					dat->showUnread = 1;
					SendMessage(hwndDlg, DM_UPDATEICON, 0, 0);
					SetTimer(hwndDlg, TIMERID_UNREAD, TIMEOUT_UNREAD, NULL);
				}
				SendMessage(dat->hwndParent, CM_STARTFLASHING, 0, 0);
			}
			dat->messagesInProgress = ReattachSendQueueItems(hwndDlg, dat->windowData.hContact);
			if (dat->messagesInProgress > 0) {
				SendMessage(hwndDlg, DM_SHOWMESSAGESENDING, 0, 0);
			}
			NotifyLocalWinEvent(dat->windowData.hContact, hwndDlg, MSG_WINDOW_EVT_OPEN);
			return TRUE;
		}
	case DM_GETCONTEXTMENU:
		{
			HMENU hMenu = (HMENU) CallService(MS_CLIST_MENUBUILDCONTACT, (WPARAM) dat->windowData.hContact, 0);
			SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, (LONG_PTR)hMenu);
			return TRUE;
		}
	case WM_CONTEXTMENU:
		if (dat->hwndParent == (HWND) wParam) {
			POINT pt;
			HMENU hMenu = (HMENU) CallService(MS_CLIST_MENUBUILDCONTACT, (WPARAM) dat->windowData.hContact, 0);
			GetCursorPos(&pt);
			TrackPopupMenu(hMenu, 0, pt.x, pt.y, 0, hwndDlg, NULL);
			DestroyMenu(hMenu);
		}
		break;
	case WM_LBUTTONDBLCLK:
		SendMessage(dat->hwndParent, WM_SYSCOMMAND, SC_MINIMIZE, 0);
		break;
	case WM_RBUTTONUP:
		{
			int i;
			POINT pt;
			MENUITEMINFO mii;
			hToolbarMenu = CreatePopupMenu();
			for (i = 0; i < SIZEOF(toolbarButtons); i++) {
				ZeroMemory(&mii, sizeof(mii));
				mii.cbSize = sizeof(mii);
				mii.fMask = MIIM_ID | MIIM_STRING | MIIM_STATE | MIIM_DATA | MIIM_BITMAP;
				mii.fType = MFT_STRING;
				mii.fState = (g_dat->buttonVisibility & (1<< i)) ? MFS_CHECKED : MFS_UNCHECKED;
				mii.wID = i + 1;
				mii.dwItemData = (ULONG_PTR)g_dat->hButtonIconList;
				mii.hbmpItem = HBMMENU_CALLBACK;
				mii.dwTypeData = TranslateTS((toolbarButtons[i].name));
				InsertMenuItem(hToolbarMenu, i, TRUE, &mii);
			}
//			CallService(MS_LANGPACK_TRANSLATEMENU, (WPARAM) hToolbarMenu, 0);
			pt.x = (short) LOWORD(GetMessagePos());
			pt.y = (short) HIWORD(GetMessagePos());
			i = TrackPopupMenu(hToolbarMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwndDlg, NULL);
			if (i > 0) {
				g_dat->buttonVisibility ^= (1 << (i - 1));
				DBWriteContactSettingDword(NULL, SRMMMOD, SRMSGSET_BUTTONVISIBILITY, g_dat->buttonVisibility);
				WindowList_Broadcast(g_dat->hMessageWindowList, DM_OPTIONSAPPLIED, 0, 0);
			}
			DestroyMenu(hToolbarMenu);
			return TRUE;
		}
	case WM_DROPFILES:
		if (dat->szProto==NULL) break;
		if (!(CallProtoService(dat->szProto, PS_GETCAPS, PFLAGNUM_1,0)&PF1_FILESEND)) break;
		if (dat->wStatus==ID_STATUS_OFFLINE) break;
		if (dat->windowData.hContact!=NULL) {
			TCHAR szFilename[MAX_PATH];
			HDROP hDrop = (HDROP)wParam;
			int fileCount = DragQueryFile(hDrop,-1,NULL,0), totalCount = 0, i;
			TCHAR** ppFiles = NULL;
			for ( i=0; i < fileCount; i++ ) {
				DragQueryFile(hDrop, i, szFilename, SIZEOF(szFilename));
				AddToFileList(&ppFiles, &totalCount, szFilename);
			}
			CallServiceSync(MS_FILE_SENDSPECIFICFILEST, (WPARAM)dat->windowData.hContact, (LPARAM)ppFiles);
			for(i=0;ppFiles[i];i++) mir_free(ppFiles[i]);
			mir_free(ppFiles);
		}
		break;

	case DM_AVATARCHANGED:
		ShowAvatar(hwndDlg, dat);
		break;

	case DM_GETAVATAR:
	{
		PROTO_AVATAR_INFORMATION ai = { sizeof(ai), dat->windowData.hContact };
		CallProtoService(dat->szProto, PS_GETAVATARINFO, GAIF_FORCE, (LPARAM)&ai);
		ShowAvatar(hwndDlg, dat);
		break;
	}
	case DM_TYPING:
		dat->nTypeSecs = (int) lParam > 0 ? (int) lParam : 0;
		break;
	case DM_CHANGEICONS:
		SendDlgItemMessage(hwndDlg, IDC_ADD, BM_SETIMAGE, IMAGE_ICON, (LPARAM) GetCachedIcon("scriver_ADD"));
		SendDlgItemMessage(hwndDlg, IDC_DETAILS, BM_SETIMAGE, IMAGE_ICON, (LPARAM) GetCachedIcon("scriver_USERDETAILS"));
		SendDlgItemMessage(hwndDlg, IDC_HISTORY, BM_SETIMAGE, IMAGE_ICON, (LPARAM) GetCachedIcon("scriver_HISTORY"));
		SendDlgItemMessage(hwndDlg, IDC_QUOTE, BM_SETIMAGE, IMAGE_ICON, (LPARAM) GetCachedIcon("scriver_QUOTE"));
		SendDlgItemMessage(hwndDlg, IDC_SMILEYS, BM_SETIMAGE, IMAGE_ICON, (LPARAM) GetCachedIcon("scriver_SMILEY"));
		SendDlgItemMessage(hwndDlg, IDOK, BM_SETIMAGE, IMAGE_ICON, (LPARAM) GetCachedIcon("scriver_SEND"));
		SendMessage(hwndDlg, DM_UPDATESTATUSBAR, 0, 0);
		SetStatusIcon(dat);

	case DM_UPDATEICON:
		{
			TitleBarData tbd = {0};
			TabControlData tcd;
			tbd.iFlags = TBDF_ICON;
			GetTitlebarIcon(dat, &tbd);
			SendMessage(dat->hwndParent, CM_UPDATETITLEBAR, (WPARAM)&tbd, (LPARAM)hwndDlg);
			tcd.iFlags = TCDF_ICON;
			tcd.hIcon = GetTabIcon(dat);
			SendMessage(dat->hwndParent, CM_UPDATETABCONTROL, (WPARAM)&tcd, (LPARAM)hwndDlg);
			SendDlgItemMessage(hwndDlg, IDC_USERMENU, BM_SETIMAGE, IMAGE_ICON, (LPARAM)dat->statusIcon);
		}
		break;
	case DM_UPDATETABCONTROL:
		{
			TabControlData tcd;
			tcd.iFlags = TCDF_TEXT | TCDF_ICON;
			tcd.hIcon = GetTabIcon(dat);
			tcd.pszText = GetTabName(dat->windowData.hContact);
			SendMessage(dat->hwndParent, CM_UPDATETABCONTROL, (WPARAM)&tcd, (LPARAM)hwndDlg);
			mir_free(tcd.pszText);
		}
		break;
	case DM_UPDATETITLEBAR:
		{
			TitleBarData tbd = {0};
			tbd.iFlags = TBDF_TEXT | TBDF_ICON;
			GetTitlebarIcon(dat, &tbd);
			tbd.pszText = GetWindowTitle(dat->windowData.hContact, dat->szProto);
			SendMessage(dat->hwndParent, CM_UPDATETITLEBAR, (WPARAM)&tbd, (LPARAM)hwndDlg);
			mir_free(tbd.pszText);
		}
		break;

	case DM_CLISTSETTINGSCHANGED:
		{
            if ((HANDLE)wParam == dat->windowData.hContact) {
                if (dat->windowData.hContact && dat->szProto) {
        			DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *) lParam;
                    char idbuf[128];
                    char buf[128];
                    GetContactUniqueId(dat, idbuf, sizeof(idbuf));
                    mir_snprintf(buf, sizeof(buf), Translate("User Menu - %s"), idbuf);
                    SendMessage(GetDlgItem(hwndDlg, IDC_USERMENU), BUTTONADDTOOLTIP, (WPARAM) buf, 0);

                    if (!cws || (!strcmp(cws->szModule, dat->szProto) && !strcmp(cws->szSetting, "Status"))) {
                        DWORD wStatus;
                        wStatus = DBGetContactSettingWord( dat->windowData.hContact, dat->szProto, "Status", ID_STATUS_OFFLINE);
                        // log status change - should be moved to a separate place
                        if (dat->wStatus != wStatus && DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_SHOWSTATUSCH, SRMSGDEFSET_SHOWSTATUSCH)) {
                            DBEVENTINFO dbei;
                            TCHAR buffer[512];
                            char blob[2048];
                            HANDLE hNewEvent;
                            int iLen;
                            TCHAR *szOldStatus = mir_tstrdup((TCHAR *) CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM) dat->wStatus, GSMDF_TCHAR));
                            TCHAR *szNewStatus = mir_tstrdup((TCHAR *) CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, (WPARAM) wStatus, GSMDF_TCHAR));
                            if (wStatus == ID_STATUS_OFFLINE) {
                                iLen = mir_sntprintf(buffer, SIZEOF(buffer), TranslateT("signed off (was %s)"), szOldStatus);
                                SendMessage(hwndDlg, DM_TYPING, 0, 0);
                            }
                            else if (dat->wStatus == ID_STATUS_OFFLINE) {
                                iLen = mir_sntprintf(buffer, SIZEOF(buffer), TranslateT("signed on (%s)"), szNewStatus);
                            }
                            else {
                                iLen = mir_sntprintf(buffer, SIZEOF(buffer), TranslateT("is now %s (was %s)"), szNewStatus, szOldStatus);
                            }
                            mir_free(szOldStatus);
                            mir_free(szNewStatus);
                        #if defined( _UNICODE )
                            {
                                int ansiLen = WideCharToMultiByte(CP_ACP, 0, buffer, -1, blob, sizeof(blob), 0, 0);
                                memcpy( blob+ansiLen, buffer, sizeof(TCHAR)*(iLen+1));
                                dbei.cbBlob = ansiLen + sizeof(TCHAR)*(iLen+1);
                            }
                        #else
                            {
                                int wLen = MultiByteToWideChar(CP_ACP, 0, buffer, -1, NULL, 0 );
                                memcpy( blob, buffer, iLen+1 );
                                MultiByteToWideChar(CP_ACP, 0, buffer, -1, (WCHAR*)&blob[iLen+1], wLen+1 );
                                dbei.cbBlob = iLen+1 + sizeof(WCHAR)*wLen;
                            }
                        #endif
                            //iLen = strlen(buffer) + 1;
                            //MultiByteToWideChar(CP_ACP, 0, buffer, iLen, (LPWSTR) & buffer[iLen], iLen);
                            dbei.cbSize = sizeof(dbei);
                            dbei.pBlob = (PBYTE) blob;
                        //	dbei.cbBlob = (strlen(buffer) + 1) * (sizeof(TCHAR) + 1);
                            dbei.eventType = EVENTTYPE_STATUSCHANGE;
                            dbei.flags = 0;
                            dbei.timestamp = time(NULL);
                            dbei.szModule = dat->szProto;
                            hNewEvent = (HANDLE) CallService(MS_DB_EVENT_ADD, (WPARAM) dat->windowData.hContact, (LPARAM) & dbei);
                            if (dat->hDbEventFirst == NULL) {
                                dat->hDbEventFirst = hNewEvent;
                                SendMessage(hwndDlg, DM_REMAKELOG, 0, 0);
                            }
                        }
                        dat->wStatus = wStatus;
                    }
                    SetStatusIcon(dat);
                    SendMessage(hwndDlg, DM_UPDATEICON, 0, 0);
                    SendMessage(hwndDlg, DM_UPDATETITLEBAR, 0, 0);
                    SendMessage(hwndDlg, DM_UPDATETABCONTROL, 0, 0);
                    ShowAvatar(hwndDlg, dat);
                }
			}
			break;
		}
	case DM_OPTIONSAPPLIED:
		{
			PARAFORMAT2 pf2 = {0};
			CHARFORMAT2 cf2 = {0};
			LOGFONT lf;
			COLORREF colour;
			dat->flags &= ~SMF_USEIEVIEW;
			dat->flags |= ServiceExists(MS_IEVIEW_WINDOW) ? g_dat->flags & SMF_USEIEVIEW : 0;
			if (dat->flags & SMF_USEIEVIEW && dat->windowData.hwndLog == NULL) {
				IEVIEWWINDOW ieWindow;
				ieWindow.cbSize = sizeof(IEVIEWWINDOW);
				ieWindow.iType = IEW_CREATE;
				ieWindow.dwFlags = 0;
				ieWindow.dwMode = IEWM_SCRIVER;
				ieWindow.parent = hwndDlg;
				ieWindow.x = 0;
				ieWindow.y = 0;
				ieWindow.cx = 200;
				ieWindow.cy = 300;
				CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWindow);
				dat->windowData.hwndLog = ieWindow.hwnd;
				if (dat->windowData.hwndLog == NULL) {
					dat->flags ^= SMF_USEIEVIEW;
				}
			} else if (!(dat->flags & SMF_USEIEVIEW) && dat->windowData.hwndLog != NULL) {
				if (dat->windowData.hwndLog != NULL) {
					IEVIEWWINDOW ieWindow;
					ieWindow.cbSize = sizeof(IEVIEWWINDOW);
					ieWindow.iType = IEW_DESTROY;
					ieWindow.hwnd = dat->windowData.hwndLog;
					CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWindow);
				}
				dat->windowData.hwndLog = NULL;
			}

			SendMessage(hwndDlg, DM_GETAVATAR, 0, 0);
			SetDialogToType(hwndDlg);

			colour = DBGetContactSettingDword(NULL, SRMMMOD, SRMSGSET_BKGCOLOUR, SRMSGDEFSET_BKGCOLOUR);
			SendDlgItemMessage(hwndDlg, IDC_LOG, EM_SETBKGNDCOLOR, 0, colour);
			colour = DBGetContactSettingDword(NULL, SRMMMOD, SRMSGSET_INPUTBKGCOLOUR, SRMSGDEFSET_INPUTBKGCOLOUR);
			SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_SETBKGNDCOLOR, 0, colour);
			InvalidateRect(GetDlgItem(hwndDlg, IDC_MESSAGE), NULL, FALSE);
			LoadMsgDlgFont(MSGFONTID_MESSAGEAREA, &lf, &colour, FALSE);
			cf2.cbSize = sizeof(cf2);
			cf2.dwMask = CFM_COLOR | CFM_FACE | CFM_CHARSET | CFM_SIZE | CFM_WEIGHT | CFM_BOLD | CFM_ITALIC;
			cf2.crTextColor = colour;
			cf2.bCharSet = lf.lfCharSet;
			_tcsncpy(cf2.szFaceName, lf.lfFaceName, LF_FACESIZE);
			cf2.dwEffects = ((lf.lfWeight >= FW_BOLD) ? CFE_BOLD : 0) | (lf.lfItalic ? CFE_ITALIC : 0);
			cf2.wWeight = (WORD)lf.lfWeight;
			cf2.bPitchAndFamily = lf.lfPitchAndFamily;
			cf2.yHeight = abs(lf.lfHeight) * 1440 / g_dat->logPixelSY;
			SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_SETCHARFORMAT, (WPARAM)SCF_ALL, (LPARAM)&cf2);
			SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_SETLANGOPTIONS, 0, (LPARAM) SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_GETLANGOPTIONS, 0, 0) & ~IMF_AUTOKEYBOARD);

			pf2.cbSize = sizeof(pf2);
			pf2.dwMask = PFM_OFFSET;
			pf2.dxOffset = (g_dat->flags & SMF_INDENTTEXT) ? g_dat->indentSize * 1440 / g_dat->logPixelSX : 0;
 			SetDlgItemText(hwndDlg, IDC_LOG, _T(""));
			SendDlgItemMessage(hwndDlg, IDC_LOG, EM_SETPARAFORMAT, 0, (LPARAM)&pf2);
			SendDlgItemMessage(hwndDlg, IDC_LOG, EM_SETLANGOPTIONS, 0, (LPARAM) SendDlgItemMessage(hwndDlg, IDC_LOG, EM_GETLANGOPTIONS, 0, 0) & ~(IMF_AUTOKEYBOARD | IMF_AUTOFONTSIZEADJUST));

			SendMessage(hwndDlg, DM_REMAKELOG, 0, 0);
			SendMessage(hwndDlg, DM_UPDATETITLEBAR, 0, 0);
			SendMessage(hwndDlg, DM_UPDATETABCONTROL, 0, 0);
			SendMessage(hwndDlg, DM_UPDATESTATUSBAR, 0, 0);
			SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_REQUESTRESIZE, 0, 0);
			SetupInfobar(dat->infobarData);
			break;
		}
    case DM_USERNAMETOCLIP:
		{
			char buf[128];
			HGLOBAL hData;

			if(dat->windowData.hContact) {
                GetContactUniqueId(dat, buf, sizeof(buf));
				if (!OpenClipboard(hwndDlg) || !lstrlenA(buf)) break;
				EmptyClipboard();
				hData = GlobalAlloc(GMEM_MOVEABLE, lstrlenA(buf) + 1);
				lstrcpyA(GlobalLock(hData), buf);
				GlobalUnlock(hData);
				SetClipboardData(CF_TEXT, hData);
				CloseClipboard();
			}
			break;
		}
	case DM_SWITCHINFOBAR:
	case DM_SWITCHTOOLBAR:
		SetDialogToType(hwndDlg);
//		SendMessage(dat->hwndParent, DM_SWITCHTOOLBAR, 0, 0);
		break;
	case DM_GETCODEPAGE:
		SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, dat->windowData.codePage);
		return TRUE;
	case DM_SETCODEPAGE:
		dat->windowData.codePage = (int) lParam;
		SendMessage(hwndDlg, DM_REMAKELOG, 0, 0);
		break;
	case DM_SWITCHTYPING:
		if (IsTypingNotificationSupported(dat)) {
			StatusIconData sid = {0};
			sid.cbSize = sizeof(sid);
			sid.szModule = SRMMMOD;
			sid.dwId = 1;
			{
				BYTE typingNotify = (DBGetContactSettingByte(dat->windowData.hContact, SRMMMOD, SRMSGSET_TYPING,
					DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_TYPINGNEW, SRMSGDEFSET_TYPINGNEW)));
				DBWriteContactSettingByte(dat->windowData.hContact, SRMMMOD, SRMSGSET_TYPING, (BYTE)!typingNotify);
				sid.flags = typingNotify ? MBF_DISABLED : 0;
			}
			ModifyStatusIcon((WPARAM)dat->windowData.hContact, (LPARAM) &sid);
		}
		break;
	case DM_SWITCHRTL:
		{
			PARAFORMAT2 pf2;
			ZeroMemory((void *)&pf2, sizeof(pf2));
			pf2.cbSize = sizeof(pf2);
			pf2.dwMask = PFM_RTLPARA;
			dat->flags ^= SMF_RTL;
			if (dat->flags&SMF_RTL) {
				pf2.wEffects = PFE_RTLPARA;
				SetWindowLong(GetDlgItem(hwndDlg, IDC_MESSAGE),GWL_EXSTYLE,GetWindowLong(GetDlgItem(hwndDlg, IDC_MESSAGE),GWL_EXSTYLE) | WS_EX_RIGHT | WS_EX_RTLREADING | WS_EX_LEFTSCROLLBAR);
				SetWindowLong(GetDlgItem(hwndDlg, IDC_LOG),GWL_EXSTYLE,GetWindowLong(GetDlgItem(hwndDlg, IDC_LOG),GWL_EXSTYLE) | WS_EX_LEFTSCROLLBAR);
			} else {
				pf2.wEffects = 0;
				SetWindowLong(GetDlgItem(hwndDlg, IDC_MESSAGE),GWL_EXSTYLE,GetWindowLong(GetDlgItem(hwndDlg, IDC_MESSAGE),GWL_EXSTYLE) &~ (WS_EX_RIGHT | WS_EX_RTLREADING | WS_EX_LEFTSCROLLBAR));
				SetWindowLong(GetDlgItem(hwndDlg, IDC_LOG),GWL_EXSTYLE,GetWindowLong(GetDlgItem(hwndDlg, IDC_LOG),GWL_EXSTYLE) &~ (WS_EX_LEFTSCROLLBAR));
			}
			SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_SETPARAFORMAT, 0, (LPARAM)&pf2);
		}
		SendMessage(hwndDlg, DM_REMAKELOG, 0, 0);
		break;
	case DM_GETWINDOWSTATE:
		{
			UINT state = 0;

			state |= MSG_WINDOW_STATE_EXISTS;
			if (IsWindowVisible(hwndDlg))
				state |= MSG_WINDOW_STATE_VISIBLE;
			if (GetForegroundWindow()==dat->hwndParent)
				state |= MSG_WINDOW_STATE_FOCUS;
			if (IsIconic(dat->hwndParent))
				state |= MSG_WINDOW_STATE_ICONIC;
			SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, state);
			return TRUE;

		}
	case DM_ACTIVATE:
	case WM_ACTIVATE:
		if (LOWORD(wParam) != WA_ACTIVE)
			break;
		//fall through
	case WM_MOUSEACTIVATE:
		if (dat->hDbUnreadEventFirst != NULL) {
			HANDLE hDbEvent = dat->hDbUnreadEventFirst;
			dat->hDbUnreadEventFirst = NULL;
			while (hDbEvent != NULL) {
				DBEVENTINFO dbei;
				ZeroMemory(&dbei, sizeof(dbei));
				dbei.cbSize = sizeof(dbei);
				dbei.cbBlob = 0;
				CallService(MS_DB_EVENT_GET, (WPARAM) hDbEvent, (LPARAM) & dbei);
				if (!(dbei.flags & DBEF_SENT) && (DbEventIsMessageOrCustom(&dbei) || dbei.eventType == EVENTTYPE_URL)) {
					CallService(MS_CLIST_REMOVEEVENT, (WPARAM) dat->windowData.hContact, (LPARAM) hDbEvent);
				}
				hDbEvent = (HANDLE) CallService(MS_DB_EVENT_FINDNEXT, (WPARAM) hDbEvent, 0);
			}
		}
		if (dat->showUnread) {
			dat->showUnread = 0;
			KillTimer(hwndDlg, TIMERID_UNREAD);
			SendMessage(hwndDlg, DM_UPDATEICON, 0, 0);
		}
		break;
	case WM_LBUTTONDOWN:
		SendMessage(dat->hwndParent, WM_LBUTTONDOWN, wParam, lParam);
		return TRUE;
	case DM_SETFOCUS:
		if (lParam == WM_MOUSEACTIVATE) {
			HWND hLog;
			RECT rc;
			POINT pt;
			GetCursorPos(&pt);
			if (dat->windowData.hwndLog != NULL) {
				hLog = dat->windowData.hwndLog;
			} else {
				hLog = GetDlgItem(hwndDlg, IDC_LOG);
			}
			GetWindowRect(hLog, &rc);
			if (pt.x >= rc.left && pt.x <= rc.right && pt.y >= rc.top && pt.y <=rc.bottom) {
		//		SetFocus(hLog);
				return TRUE;
			}
		}
		if (g_dat->hFocusWnd == hwndDlg) {
			SetFocus(GetDlgItem(hwndDlg, IDC_MESSAGE));
		}
		return TRUE;
	case WM_SETFOCUS:
		SendMessage(dat->hwndParent, CM_ACTIVATECHILD, 0, (LPARAM)hwndDlg);
		g_dat->hFocusWnd = hwndDlg;
		PostMessage(hwndDlg, DM_SETFOCUS, 0, 0);
		return TRUE;
	case DM_SETPARENT:
		dat->hwndParent = (HWND) lParam;
		dat->parent = (ParentWindowData *) GetWindowLongPtr(dat->hwndParent, GWLP_USERDATA);
		SetParent(hwndDlg, dat->hwndParent);
		return TRUE;
	case WM_GETMINMAXINFO:
		{
			MINMAXINFO *mmi = (MINMAXINFO *) lParam;
			mmi->ptMinTrackSize.x = dat->toolbarSize.cx + BOTTOM_RIGHT_AVATAR_HEIGHT;
			mmi->ptMinTrackSize.y = dat->windowData.minLogBoxHeight + dat->toolbarSize.cy + dat->windowData.minEditBoxHeight + max(INFO_BAR_HEIGHT, BOTTOM_RIGHT_AVATAR_HEIGHT - dat->toolbarSize.cy) + 5;
			return 0;
		}
	case WM_SIZE:
		{
			if (wParam==SIZE_RESTORED || wParam==SIZE_MAXIMIZED) {
				RECT rc;
				int dlgWidth, dlgHeight;
				dlgWidth = LOWORD(lParam);
				dlgHeight = HIWORD(lParam);
				/*if (dlgWidth == 0 && dlgHeight ==0) */{
					GetClientRect(hwndDlg, &rc);
					dlgWidth = rc.right - rc.left;
					dlgHeight = rc.bottom - rc.top;
				}
				MessageDialogResize(hwndDlg, dat, dlgWidth, dlgHeight);
			}
			return TRUE;
		}
	case DM_SPLITTERMOVED:
		{
			POINT pt;
			RECT rc;
			RECT rcLog;
			if ((HWND) lParam == GetDlgItem(hwndDlg, IDC_SPLITTER)) {
				int oldSplitterY;
				GetWindowRect(GetDlgItem(hwndDlg, IDC_LOG), &rcLog);
				GetClientRect(hwndDlg, &rc);
				pt.x = 0;
				pt.y = wParam;
				ScreenToClient(hwndDlg, &pt);
				oldSplitterY = dat->splitterPos;
				dat->splitterPos = rc.bottom - pt.y;
				SendMessage(hwndDlg, WM_SIZE, 0, 0);
			}
			break;
		}
	case DM_REMAKELOG:
		dat->lastEventType = -1;
		if (wParam == 0 || (HANDLE) wParam == dat->windowData.hContact) {
			//StreamInEvents(hwndDlg, dat->hDbEventFirst, 0, 0);
			StreamInEvents(hwndDlg, dat->hDbEventFirst, -1, 0);
		}
		InvalidateRect(GetDlgItem(hwndDlg, IDC_LOG), NULL, FALSE);
		break;
	case DM_APPENDTOLOG:   //takes wParam=hDbEvent
		StreamInEvents(hwndDlg, (HANDLE) wParam, 1, 1);
		break;
	case DM_SCROLLLOGTOBOTTOM:
		if (dat->windowData.hwndLog == NULL) {
			SCROLLINFO si = { 0 };
			if ((GetWindowLong(GetDlgItem(hwndDlg, IDC_LOG), GWL_STYLE) & WS_VSCROLL) == 0)
				break;
			si.cbSize = sizeof(si);
			si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
			if (GetScrollInfo(GetDlgItem(hwndDlg, IDC_LOG), SB_VERT, &si)) {
				if (GetDlgItem(hwndDlg, IDC_LOG) != GetFocus()) {
//				if (si.nPos + si.nPage >= si.nMax) {
					si.fMask = SIF_POS;
					si.nPos = si.nMax - si.nPage + 1;
					SetScrollInfo(GetDlgItem(hwndDlg, IDC_LOG), SB_VERT, &si, TRUE);
					PostMessage(GetDlgItem(hwndDlg, IDC_LOG), WM_VSCROLL, MAKEWPARAM(SB_BOTTOM, 0), 0);
				}
			}
			RedrawWindow(GetDlgItem(hwndDlg, IDC_LOG), NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		} else {
			IEVIEWWINDOW ieWindow;
			ieWindow.cbSize = sizeof(IEVIEWWINDOW);
			ieWindow.iType = IEW_SCROLLBOTTOM;
			ieWindow.hwnd = dat->windowData.hwndLog;
			CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWindow);
		}
		break;
	case HM_DBEVENTADDED:
		if ((HANDLE) wParam == dat->windowData.hContact)
		{
			DBEVENTINFO dbei = { 0 };

			dbei.cbSize = sizeof(dbei);
			dbei.cbBlob = 0;
			CallService(MS_DB_EVENT_GET, lParam, (LPARAM) & dbei);
			if (dat->hDbEventFirst == NULL)
				dat->hDbEventFirst = (HANDLE) lParam;
			if (DbEventIsShown(&dbei, dat)) {
				int heFlags = HistoryEvents_GetFlags(dbei.eventType);
				if (heFlags != -1 && (heFlags & HISTORYEVENTS_FLAG_DEFAULT))
					heFlags = -1;

				if (DbEventIsMessageOrCustom(&dbei) && !(dbei.flags & (DBEF_SENT))) {
					/* store the event when the container is hidden so that clist notifications can be removed */
					if (!IsWindowVisible(GetParent(hwndDlg)) && dat->hDbUnreadEventFirst == NULL)
						dat->hDbUnreadEventFirst = (HANDLE) lParam;
					dat->lastMessage = dbei.timestamp;
					SendMessage(hwndDlg, DM_UPDATESTATUSBAR, 0, 0);
					if (GetForegroundWindow()==dat->hwndParent && dat->parent->hwndActive == hwndDlg)
						SkinPlaySound("RecvMsgActive");
					else SkinPlaySound("RecvMsgInactive");
					if ((g_dat->flags2 & SMF2_SWITCHTOACTIVE) && (IsIconic(dat->hwndParent) || GetActiveWindow() != dat->hwndParent) && IsWindowVisible(dat->hwndParent)) {
						SendMessage(dat->hwndParent, CM_ACTIVATECHILD, 0, (LPARAM) hwndDlg);
					}
					if (IsAutoPopup(dat->windowData.hContact)) {
						SendMessage(GetParent(hwndDlg), CM_POPUPWINDOW, (WPARAM) 1, (LPARAM) hwndDlg);
					}
				}
				if ((HANDLE) lParam != dat->hDbEventFirst && (HANDLE) CallService(MS_DB_EVENT_FINDNEXT, lParam, 0) == NULL)
					SendMessage(hwndDlg, DM_APPENDTOLOG, lParam, 0);
				else
					SendMessage(hwndDlg, DM_REMAKELOG, 0, 0);
				if (!(dbei.flags & DBEF_SENT) && dbei.eventType != EVENTTYPE_STATUSCHANGE && dbei.eventType != EVENTTYPE_JABBER_CHATSTATES && dbei.eventType != EVENTTYPE_JABBER_PRESENCE && (heFlags == -1 || (heFlags & HISTORYEVENTS_FLAG_FLASH_MSG_WINDOW))) {
					if (GetActiveWindow() != dat->hwndParent || GetForegroundWindow() != dat->hwndParent || dat->parent->hwndActive != hwndDlg) {
						dat->showUnread = 1;
						SendMessage(hwndDlg, DM_UPDATEICON, 0, 0);
						SetTimer(hwndDlg, TIMERID_UNREAD, TIMEOUT_UNREAD, NULL);
					}
					SendMessage(dat->hwndParent, CM_STARTFLASHING, 0, 0);
				}
			}
		}
		break;
	case DM_UPDATESTATUSBAR:
		if (dat->parent->hwndActive == hwndDlg) {
			TCHAR szText[256];
			StatusBarData sbd= {0};
			StatusIconData sid = {0};
			sbd.iFlags = SBDF_TEXT | SBDF_ICON;
			if (dat->messagesInProgress && (g_dat->flags & SMF_SHOWPROGRESS)) {
				sbd.hIcon = GetCachedIcon("scriver_DELIVERING");
				sbd.pszText = szText;
				mir_sntprintf(szText, SIZEOF(szText), TranslateT("Sending in progress: %d message(s) left..."), dat->messagesInProgress);
			} else if (dat->nTypeSecs) {
				TCHAR *szContactName = GetNickname(dat->windowData.hContact, dat->szProto);
				sbd.hIcon = GetCachedIcon("scriver_TYPING");
				sbd.pszText = szText;
				mir_sntprintf(szText, SIZEOF(szText), TranslateT("%s is typing a message..."), szContactName);
				mir_free(szContactName);
				dat->nTypeSecs--;
			} else if (dat->lastMessage) {
				TCHAR date[64], time[64];
				tmi.printTimeStamp(NULL, dat->lastMessage, _T("d"), date, SIZEOF(date), 0);
	            tmi.printTimeStamp(NULL, dat->lastMessage, _T("t"), time, SIZEOF(time), 0);
				mir_sntprintf(szText, SIZEOF(szText), TranslateT("Last message received on %s at %s."), date, time);
				sbd.pszText = szText;
			} else {
				sbd.pszText =  _T("");
			}
			SendMessage(dat->hwndParent, CM_UPDATESTATUSBAR, (WPARAM)&sbd, (LPARAM)hwndDlg);
			UpdateReadChars(hwndDlg, dat);
			sid.cbSize = sizeof(sid);
			sid.szModule = SRMMMOD;
			sid.flags = MBF_DISABLED;
			ModifyStatusIcon((WPARAM)dat->windowData.hContact, (LPARAM) &sid);
			sid.dwId = 1;
			if (IsTypingNotificationSupported(dat) && g_dat->flags2 & SMF2_SHOWTYPINGSWITCH) {
				sid.flags = (DBGetContactSettingByte(dat->windowData.hContact, SRMMMOD, SRMSGSET_TYPING,
					DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_TYPINGNEW, SRMSGDEFSET_TYPINGNEW))) ? 0 : MBF_DISABLED;
			} else {
				sid.flags = MBF_HIDDEN;
			}
			ModifyStatusIcon((WPARAM)dat->windowData.hContact, (LPARAM) &sid);
		}
		break;
	case DM_CLEARLOG:
	// IEVIew MOD Begin
		if (dat->windowData.hwndLog != NULL) {
			IEVIEWEVENT event;
			ZeroMemory(&event, sizeof(event));
			event.cbSize = sizeof(event);
			event.iType = IEE_CLEAR_LOG;
			event.dwFlags = ((dat->flags & SMF_RTL) ? IEEF_RTL : 0);
			event.hwnd = dat->windowData.hwndLog;
			event.hContact = dat->windowData.hContact;
			event.codepage = dat->windowData.codePage;
			event.pszProto = dat->szProto;
			CallService(MS_IEVIEW_EVENT, 0, (LPARAM)&event);
		}
	// IEVIew MOD End
		SetDlgItemText(hwndDlg, IDC_LOG, _T(""));
		dat->hDbEventFirst = NULL;
		dat->lastEventType = -1;
		break;
	case WM_TIMER:
		if (wParam == TIMERID_MSGSEND) {
			ReportSendQueueTimeouts(hwndDlg);
		} else if (wParam == TIMERID_TYPE) {
			if (dat->nTypeMode == PROTOTYPE_SELFTYPING_ON && GetTickCount() - dat->nLastTyping > TIMEOUT_TYPEOFF) {
				NotifyTyping(dat, PROTOTYPE_SELFTYPING_OFF);
			}
			if (dat->showTyping) {
				if (dat->nTypeSecs) {
					dat->nTypeSecs--;
				}
				else {
					dat->showTyping = 0;
					SendMessage(hwndDlg, DM_UPDATESTATUSBAR, 0, 0);
					SendMessage(hwndDlg, DM_UPDATEICON, 0, 0);
				}
			}
			else {
				if (dat->nTypeSecs) {
					dat->showTyping = 1;
					SendMessage(hwndDlg, DM_UPDATESTATUSBAR, 0, 0);
					SendMessage(hwndDlg, DM_UPDATEICON, 0, 0);
				}
			}
		} else if (wParam == TIMERID_UNREAD) {
			TabControlData tcd;
			tcd.iFlags = TCDF_ICON;
			if (!dat->showTyping) {
				dat->showUnread++;
				if (dat->showUnread & 1) {
					tcd.hIcon = dat->statusIconOverlay;
				} else {
					tcd.hIcon = dat->statusIcon;
				}
				SendMessage(dat->hwndParent, CM_UPDATETABCONTROL, (WPARAM)&tcd, (LPARAM)hwndDlg);
			}
		}
		break;
	case DM_SENDMESSAGE:
		if (lParam) {
			MessageSendQueueItem *msi = (MessageSendQueueItem *)lParam;
			MessageSendQueueItem *item = CreateSendQueueItem(hwndDlg);
			item->hContact = dat->windowData.hContact;
			item->proto = mir_strdup(dat->szProto);
			item->flags = msi->flags;
			item->codepage = dat->windowData.codePage;
			if ( IsUtfSendAvailable( dat->windowData.hContact )) {
				char* szMsgUtf;
				#if defined( _UNICODE )
					szMsgUtf = mir_utf8encodeW( (TCHAR *)&msi->sendBuffer[strlen(msi->sendBuffer) + 1] );
					item->flags &= ~PREF_UNICODE;
				#else
					szMsgUtf = mir_utf8encodecp(msi->sendBuffer, dat->windowData.codePage);
				#endif
				if (!szMsgUtf) {
					break;
				}
				if  (*szMsgUtf == 0) {
					mir_free(szMsgUtf);
					break;
				}
				item->sendBufferSize = (int)strlen(szMsgUtf) + 1;
				item->sendBuffer = szMsgUtf;
				item->flags |= PREF_UTF;
			} else {
				item->sendBufferSize = msi->sendBufferSize;
				item->sendBuffer = (char *) mir_alloc(msi->sendBufferSize);
				memcpy(item->sendBuffer, msi->sendBuffer, msi->sendBufferSize);
			}
			SendMessage(hwndDlg, DM_STARTMESSAGESENDING, 0, 0);
			SendSendQueueItem(item);
		}
		break;
	case DM_STARTMESSAGESENDING:
		dat->messagesInProgress++;
	case DM_SHOWMESSAGESENDING:
		SetTimer(hwndDlg, TIMERID_MSGSEND, 1000, NULL);
		if (g_dat->flags & SMF_SHOWPROGRESS) {
			SendMessage(dat->hwnd, DM_UPDATESTATUSBAR, 0, 0);
		}
		break;
	case DM_STOPMESSAGESENDING:
		if (dat->messagesInProgress>0) {
			dat->messagesInProgress--;
			if (g_dat->flags & SMF_SHOWPROGRESS) {
				SendMessage(dat->hwnd, DM_UPDATESTATUSBAR, 0, 0);
			}
		}
		if (dat->messagesInProgress == 0) {
			KillTimer(hwndDlg, TIMERID_MSGSEND);
		}
		break;
	case DM_SHOWERRORMESSAGE:
		if (lParam) {
			ErrorWindowData *ewd = (ErrorWindowData *) lParam;
			SendMessage(hwndDlg, DM_STOPMESSAGESENDING, 0, 0);
			ewd->queueItem->hwndErrorDlg = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_MSGSENDERROR), hwndDlg, ErrorDlgProc, (LPARAM) ewd);//hwndDlg
		}
		break;
	case DM_ERRORDECIDED:
		{
			MessageSendQueueItem *item = (MessageSendQueueItem *) lParam;
			item->hwndErrorDlg = NULL;
			switch (wParam) {
			case MSGERROR_CANCEL:
				RemoveSendQueueItem(item);
				SetFocus(GetDlgItem(hwndDlg, IDC_MESSAGE));
				break;
			case MSGERROR_RETRY:
				SendMessage(hwndDlg, DM_STARTMESSAGESENDING, 0, 0);
				SendSendQueueItem(item);
				break;
			}
		}
		break;
	case WM_MEASUREITEM:
		if (!MeasureMenuItem(wParam, lParam)) {
			return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);
		}
		return TRUE;

	case WM_DRAWITEM:
			if (!DrawMenuItem(wParam, lParam)) {
				LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT) lParam;
				if (dis->hwndItem == GetDlgItem(hwndDlg, IDC_AVATAR)) {
					RECT rect;
					HDC hdcMem = CreateCompatibleDC(dis->hDC);
					int avatarWidth = 0;
					int avatarHeight = 0;
					int itemWidth = dis->rcItem.right - dis->rcItem.left + 1;
					int itemHeight = dis->rcItem.bottom - dis->rcItem.top + 1;
					HBITMAP hbmMem = CreateCompatibleBitmap(dis->hDC, itemWidth, itemHeight);
					hbmMem = (HBITMAP) SelectObject(hdcMem, hbmMem);
					rect.top = 0;
					rect.left = 0;
					rect.right = itemWidth - 1;
					rect.bottom = itemHeight - 1;
					FillRect(hdcMem, &rect, GetSysColorBrush(COLOR_BTNFACE));

					if (dat->avatarPic && (g_dat->flags&SMF_AVATAR)) {
						BITMAP bminfo;
						GetObject(dat->avatarPic, sizeof(bminfo), &bminfo);
						if ( bminfo.bmWidth != 0 && bminfo.bmHeight != 0 ) {
							AVATARDRAWREQUEST adr;
							avatarHeight = itemHeight;
							avatarWidth = bminfo.bmWidth * avatarHeight / bminfo.bmHeight;
							if (avatarWidth > itemWidth) {
								avatarWidth = itemWidth;
								avatarHeight = bminfo.bmHeight * avatarWidth / bminfo.bmWidth;
							}
							ZeroMemory(&adr, sizeof(adr));
							adr.cbSize = sizeof (AVATARDRAWREQUEST);
							adr.hContact = dat->windowData.hContact;
							adr.hTargetDC = hdcMem;
							adr.rcDraw.left = (itemWidth - avatarWidth) / 2;
							adr.rcDraw.top = (itemHeight - avatarHeight) / 2;
							adr.rcDraw.right = avatarWidth - 1;
							adr.rcDraw.bottom = avatarHeight - 1;
							adr.dwFlags = AVDRQ_DRAWBORDER | AVDRQ_HIDEBORDERONTRANSPARENCY;

							CallService(MS_AV_DRAWAVATAR, (WPARAM)0, (LPARAM)&adr);
						}
					}
					BitBlt(dis->hDC, 0, 0, itemWidth, itemHeight, hdcMem, 0, 0, SRCCOPY);
					hbmMem = (HBITMAP) SelectObject(hdcMem, hbmMem);
					DeleteObject(hbmMem);
					DeleteDC(hdcMem);
					return TRUE;
				}
				return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);
			}
	case WM_COMMAND:
		if (!lParam && CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(wParam), MPCF_CONTACTMENU), (LPARAM) dat->windowData.hContact))
			break;
		switch (LOWORD(wParam)) {
		case IDC_SENDALL:
			{
				int result;
				if (dat->sendAllConfirm == 0) {
					result = DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_CONFIRM_SENDALL), hwndDlg, ConfirmSendAllDlgProc, (LPARAM)hwndDlg);
					if (result & 0x10000) {
						dat->sendAllConfirm = result;
					}
				} else {
					result = dat->sendAllConfirm;
				}
				if (LOWORD(result) != IDYES) {
					break;
				}

			}
		case IDOK:
			//this is a 'send' button
			if (!IsWindowEnabled(GetDlgItem(hwndDlg, IDOK)))
				break;
			//if(GetKeyState(VK_CTRL) & 0x8000) {    // copy user name
					//SendMessage(hwndDlg, DM_USERNAMETOCLIP, 0, 0);
			//}
			if (dat->windowData.hContact != NULL) {
				GETTEXTEX  gt = {0};
				PARAFORMAT2 pf2;
				MessageSendQueueItem msi = { 0 };
				int bufSize;
				int ansiBufSize = GetRichTextLength(GetDlgItem(hwndDlg, IDC_MESSAGE), dat->windowData.codePage, TRUE) + 1;
				bufSize = ansiBufSize;
				ZeroMemory((void *)&pf2, sizeof(pf2));
				pf2.cbSize = sizeof(pf2);
				pf2.dwMask = PFM_RTLPARA;
				SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_GETPARAFORMAT, 0, (LPARAM)&pf2);
				if (pf2.wEffects & PFE_RTLPARA)
					msi.flags |= PREF_RTL;
				#if defined( _UNICODE )
					bufSize += GetRichTextLength(GetDlgItem(hwndDlg, IDC_MESSAGE), 1200, TRUE) + 2;
				#endif
				msi.sendBufferSize = bufSize;
				msi.sendBuffer = (char *) mir_alloc(msi.sendBufferSize);
				msi.flags |= PREF_TCHAR;

				gt.flags = GT_USECRLF;
				gt.cb = ansiBufSize;
				gt.codepage = dat->windowData.codePage;
				SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_GETTEXTEX, (WPARAM) &gt, (LPARAM) msi.sendBuffer);
				#if defined( _UNICODE )
					gt.cb = bufSize - ansiBufSize;
					gt.codepage = 1200;
					SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_GETTEXTEX, (WPARAM) &gt, (LPARAM) &msi.sendBuffer[ansiBufSize]);
					if ( RTL_Detect((wchar_t *)&msi.sendBuffer[ansiBufSize] ))
						msi.flags |= PREF_RTL;
				#endif
				if (msi.sendBuffer[0] == 0) {
					mir_free (msi.sendBuffer);
					break;
				}
				{
					/* Store messaging history */
					char *msgText = GetRichTextEncoded(GetDlgItem(hwndDlg, IDC_MESSAGE), dat->windowData.codePage);
					TCmdList *cmdListNew = tcmdlist_last(dat->windowData.cmdList);
					while (cmdListNew != NULL && cmdListNew->temporary) {
						dat->windowData.cmdList = tcmdlist_remove(dat->windowData.cmdList, cmdListNew);
						cmdListNew = tcmdlist_last(dat->windowData.cmdList);
					}
					if (msgText != NULL) {
						dat->windowData.cmdList = tcmdlist_append(dat->windowData.cmdList, msgText, 20, FALSE);
						mir_free(msgText);
					}
					dat->windowData.cmdListCurrent = NULL;
				}
				if (dat->nTypeMode == PROTOTYPE_SELFTYPING_ON)
					NotifyTyping(dat, PROTOTYPE_SELFTYPING_OFF);

				SetDlgItemText(hwndDlg, IDC_MESSAGE, _T(""));
				EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);
				if (DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_AUTOMIN, SRMSGDEFSET_AUTOMIN))
					ShowWindow(dat->hwndParent, SW_MINIMIZE);
				if (LOWORD(wParam) == IDC_SENDALL) {
					SendMessage(dat->hwndParent, DM_SENDMESSAGE, 0, (LPARAM) &msi);
				} else {
					SendMessage(hwndDlg, DM_SENDMESSAGE, 0, (LPARAM) &msi);
				}
				mir_free (msi.sendBuffer);
			}
			return TRUE;
		case IDCANCEL:
			DestroyWindow(hwndDlg);
			return TRUE;
		case IDC_USERMENU:
			{
				if(GetKeyState(VK_SHIFT) & 0x8000) {    // copy user name
					SendMessage(hwndDlg, DM_USERNAMETOCLIP, 0, 0);
				} else {
					RECT rc;
					HMENU hMenu = (HMENU) CallService(MS_CLIST_MENUBUILDCONTACT, (WPARAM) dat->windowData.hContact, 0);
					GetWindowRect(GetDlgItem(hwndDlg, LOWORD(wParam)), &rc);
					TrackPopupMenu(hMenu, 0, rc.left, rc.bottom, 0, hwndDlg, NULL);
					DestroyMenu(hMenu);
				}
			}
			break;
		case IDC_HISTORY:
			CallService(MS_HISTORY_SHOWCONTACTHISTORY, (WPARAM) dat->windowData.hContact, 0);
			break;
		case IDC_DETAILS:
			CallService(MS_USERINFO_SHOWDIALOG, (WPARAM) dat->windowData.hContact, 0);
			break;
		case IDC_SMILEYS:
			if (g_dat->smileyAddInstalled) {
				SMADD_SHOWSEL3 smaddInfo;
				RECT rc;
				smaddInfo.cbSize = sizeof(SMADD_SHOWSEL3);
				smaddInfo.hwndParent = dat->hwndParent;
				smaddInfo.hwndTarget = GetDlgItem(hwndDlg, IDC_MESSAGE);
				smaddInfo.targetMessage = EM_REPLACESEL;
				smaddInfo.targetWParam = TRUE;
				smaddInfo.Protocolname = dat->szProto;
				if (dat->szProto!=NULL && strcmp(dat->szProto,"MetaContacts")==0) {
					HANDLE hContact = (HANDLE) CallService(MS_MC_GETMOSTONLINECONTACT, (WPARAM) dat->windowData.hContact, 0);
					if (hContact!=NULL) {
						smaddInfo.Protocolname = (char*) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
					}
				}
				GetWindowRect(GetDlgItem(hwndDlg, IDC_SMILEYS), &rc);
				smaddInfo.Direction = 0;
				smaddInfo.xPosition = rc.left;
				smaddInfo.yPosition = rc.bottom;
				smaddInfo.hContact = dat->windowData.hContact;
				CallService(MS_SMILEYADD_SHOWSELECTION, 0, (LPARAM) &smaddInfo);
			}
			break;
		case IDC_QUOTE:
			{
				DBEVENTINFO dbei = { 0 };
				SETTEXTEX  st;
				TCHAR *buffer = NULL;
				st.flags = ST_SELECTION;
#ifdef _UNICODE
				st.codepage = 1200;
#else
				st.codepage = CP_ACP;
#endif
				if (dat->hDbEventLast==NULL) break;
				if (dat->windowData.hwndLog != NULL) {
					buffer = GetIEViewSelection(dat);
				} else {
					buffer = GetRichEditSelection(GetDlgItem(hwndDlg, IDC_LOG));
				}
				if (buffer!=NULL) {
					TCHAR *quotedBuffer = GetQuotedTextW(buffer);
					SendMessage(GetDlgItem(hwndDlg, IDC_MESSAGE), EM_SETTEXTEX, (WPARAM) &st, (LPARAM)quotedBuffer);
					mir_free(quotedBuffer);
					mir_free(buffer);
				} else {
                    dbei.cbSize = sizeof(dbei);
                    dbei.cbBlob = CallService(MS_DB_EVENT_GETBLOBSIZE, (WPARAM) dat->hDbEventLast, 0);
                    if (dbei.cbBlob == 0xFFFFFFFF) break;
                    dbei.pBlob = (PBYTE) mir_alloc(dbei.cbBlob);
                    CallService(MS_DB_EVENT_GET, (WPARAM)  dat->hDbEventLast, (LPARAM) & dbei);
                    if (DbEventIsMessageOrCustom(&dbei) || dbei.eventType == EVENTTYPE_STATUSCHANGE) {
                        TCHAR *buffer = DbGetEventTextT( &dbei, CP_ACP );
                        if (buffer!=NULL) {
                            TCHAR *quotedBuffer = GetQuotedTextW(buffer);
                            SendMessage(GetDlgItem(hwndDlg, IDC_MESSAGE), EM_SETTEXTEX, (WPARAM) &st, (LPARAM)quotedBuffer);
                            mir_free(quotedBuffer);
                            mir_free(buffer);
                        }
                    }
                    mir_free(dbei.pBlob);
                }
                SetFocus(GetDlgItem(hwndDlg, IDC_MESSAGE));
				break;
			}
		case IDC_ADD:
			{
				ADDCONTACTSTRUCT acs = { 0 };

				acs.handle = dat->windowData.hContact;
				acs.handleType = HANDLE_CONTACT;
				acs.szProto = 0;
				CallService(MS_ADDCONTACT_SHOW, (WPARAM) hwndDlg, (LPARAM) & acs);
			}
			if (!DBGetContactSettingByte(dat->windowData.hContact, "CList", "NotOnList", 0)) {
				ShowWindow(GetDlgItem(hwndDlg, IDC_ADD), SW_HIDE);
			}
		case IDC_MESSAGE:
			if (HIWORD(wParam) == EN_CHANGE) {
				int len = GetRichTextLength(GetDlgItem(hwndDlg, IDC_MESSAGE), dat->windowData.codePage, FALSE);
				dat->windowData.cmdListCurrent = NULL;
				UpdateReadChars(hwndDlg, dat);
				EnableWindow(GetDlgItem(hwndDlg, IDOK), len != 0);
				if (!(GetKeyState(VK_CONTROL) & 0x8000) && !(GetKeyState(VK_SHIFT) & 0x8000)) {
					dat->nLastTyping = GetTickCount();
					if (len != 0) {
						if (dat->nTypeMode == PROTOTYPE_SELFTYPING_OFF) {
							NotifyTyping(dat, PROTOTYPE_SELFTYPING_ON);
						}
					} else {
						if (dat->nTypeMode == PROTOTYPE_SELFTYPING_ON) {
							NotifyTyping(dat, PROTOTYPE_SELFTYPING_OFF);
						}
					}
				}
			}
			break;
		}
		break;
	case WM_NOTIFY:
		{
			LPNMHDR pNmhdr;
			pNmhdr = (LPNMHDR)lParam;
			switch (pNmhdr->idFrom) {
			case IDC_LOG:
				switch (pNmhdr->code) {
				case EN_MSGFILTER:
					{
						int result = InputAreaShortcuts(GetDlgItem(hwndDlg, IDC_MESSAGE), ((MSGFILTER *) lParam)->msg, ((MSGFILTER *) lParam)->wParam, ((MSGFILTER *) lParam)->lParam, &dat->windowData);
						if (result != -1) {
							SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
							return TRUE;
						}
					}
					switch (((MSGFILTER *) lParam)->msg) {
					case WM_CHAR:
						if (!(GetKeyState(VK_CONTROL) & 0x8000)) {
							SetFocus(GetDlgItem(hwndDlg, IDC_MESSAGE));
							SendMessage(GetDlgItem(hwndDlg, IDC_MESSAGE), ((MSGFILTER *) lParam)->msg, ((MSGFILTER *) lParam)->wParam, ((MSGFILTER *) lParam)->lParam);
							SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
						}
						return TRUE;
					case WM_LBUTTONDOWN:
						{
							HCURSOR hCur = GetCursor();
							if (hCur == LoadCursor(NULL, IDC_SIZENS) || hCur == LoadCursor(NULL, IDC_SIZEWE)
								|| hCur == LoadCursor(NULL, IDC_SIZENESW) || hCur == LoadCursor(NULL, IDC_SIZENWSE)) {
									SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
									return TRUE;
								}
								break;
						}
					case WM_MOUSEMOVE:
						{
							HCURSOR hCur = GetCursor();
							if (hCur == LoadCursor(NULL, IDC_SIZENS) || hCur == LoadCursor(NULL, IDC_SIZEWE)
								|| hCur == LoadCursor(NULL, IDC_SIZENESW) || hCur == LoadCursor(NULL, IDC_SIZENWSE))
								SetCursor(LoadCursor(NULL, IDC_ARROW));
							break;
						}
					case WM_RBUTTONUP:
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
						return TRUE;
					}
					break;
				case EN_LINK:
					switch (((ENLINK *) lParam)->msg) {
					case WM_SETCURSOR:
						SetCursor(hCurHyperlinkHand);
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
						return TRUE;
					case WM_RBUTTONDOWN:
					case WM_LBUTTONUP:
						if (HandleLinkClick(g_hInst, hwndDlg, GetDlgItem(hwndDlg, IDC_MESSAGE),(ENLINK*)lParam)) {
							SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
							return TRUE;
						}
						break;
					}
				}
				break;
			case IDC_MESSAGE:
				switch (((NMHDR *) lParam)->code) {
				case EN_MSGFILTER:
					switch (((MSGFILTER *) lParam)->msg) {
					case WM_RBUTTONUP:
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
						return TRUE;
					}
					break;
				case EN_REQUESTRESIZE:
					{
						REQRESIZE *rr = (REQRESIZE *)lParam;
						int height = rr->rc.bottom - rr->rc.top + 1;
						if (height < g_dat->minInputAreaHeight) {
							height = g_dat->minInputAreaHeight;
						}
						if (dat->desiredInputAreaHeight != height) {
							dat->desiredInputAreaHeight = height;
							SendMessage(hwndDlg, WM_SIZE, 0, 0);
							PostMessage(hwndDlg, DM_SCROLLLOGTOBOTTOM, 0, 0);
						}
					}
					break;
				}
			}
		}
		break;
	case WM_CHAR:
		SetFocus(GetDlgItem(hwndDlg, IDC_MESSAGE));
		SendMessage(GetDlgItem(hwndDlg, IDC_MESSAGE), msg, wParam, lParam);
		break;
	case WM_DESTROY:
		NotifyLocalWinEvent(dat->windowData.hContact, hwndDlg, MSG_WINDOW_EVT_CLOSING);
		if (dat->nTypeMode == PROTOTYPE_SELFTYPING_ON) {
			NotifyTyping(dat, PROTOTYPE_SELFTYPING_OFF);
		}
		CallService(MS_SKIN2_RELEASEICON, (WPARAM)dat->statusIcon, 0);
		CallService(MS_SKIN2_RELEASEICON, (WPARAM)dat->statusIconBig , 0);
		if (dat->statusIconOverlay != NULL) DestroyIcon(dat->statusIconOverlay);
		dat->statusIcon = NULL;
		dat->statusIconOverlay = NULL;
		ReleaseSendQueueItems(hwndDlg);
		if (g_dat->flags & SMF_SAVEDRAFTS) {
			saveDraftMessage(GetDlgItem(hwndDlg, IDC_MESSAGE), dat->windowData.hContact, dat->windowData.codePage);
		} else {
			g_dat->draftList = tcmdlist_remove2(g_dat->draftList, dat->windowData.hContact);
		}
		tcmdlist_free(dat->windowData.cmdList);
		WindowList_Remove(g_dat->hMessageWindowList, hwndDlg);
		UnsubclassMessageEdit(GetDlgItem(hwndDlg, IDC_MESSAGE));
		UnsubclassLogEdit(GetDlgItem(hwndDlg, IDC_LOG));
		{
			HFONT hFont;
			hFont = (HFONT) SendDlgItemMessage(hwndDlg, IDC_MESSAGE, WM_GETFONT, 0, 0);
			if (hFont != NULL && hFont != (HFONT) SendDlgItemMessage(hwndDlg, IDOK, WM_GETFONT, 0, 0))
				DeleteObject(hFont);
		}
		DBWriteContactSettingByte(dat->windowData.hContact, SRMMMOD, "UseRTL", (BYTE) ((dat->flags & SMF_RTL) ? 1 : 0));
		DBWriteContactSettingWord(dat->windowData.hContact, SRMMMOD, "CodePage", (WORD) dat->windowData.codePage);
		if (dat->windowData.hContact && (g_dat->flags & SMF_DELTEMP)) {
			if (DBGetContactSettingByte(dat->windowData.hContact, "CList", "NotOnList", 0)) {
				CallService(MS_DB_CONTACT_DELETE, (WPARAM)dat->windowData.hContact, 0);
			}
		}
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
		SendMessage(dat->hwndParent, CM_REMOVECHILD, 0, (LPARAM) hwndDlg);
		if (dat->windowData.hwndLog != NULL) {
			IEVIEWWINDOW ieWindow;
			ieWindow.cbSize = sizeof(IEVIEWWINDOW);
			ieWindow.iType = IEW_DESTROY;
			ieWindow.hwnd = dat->windowData.hwndLog;
			CallService(MS_IEVIEW_WINDOW, 0, (LPARAM)&ieWindow);
		}
		NotifyLocalWinEvent(dat->windowData.hContact, hwndDlg, MSG_WINDOW_EVT_CLOSE);
		mir_free(dat);
		break;
	}
	return FALSE;
}
