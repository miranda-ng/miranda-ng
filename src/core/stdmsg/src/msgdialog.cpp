/*

Copyright 2000-12 Miranda IM, 2012-15 Miranda NG project,
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

#define TIMERID_FLASHWND     1
#define TIMERID_TYPE         2
#define TIMEOUT_FLASHWND     900
#define TIMEOUT_TYPEOFF      10000      //send type off after 10 seconds of inactivity
#define SB_CHAR_WIDTH        45
#define SB_TIME_WIDTH        60
#define SB_GRIP_WIDTH        20         // pixels - buffer used to prevent sizegrip from overwriting statusbar icons
#define VALID_AVATAR(x)      (x == PA_FORMAT_PNG || x == PA_FORMAT_JPEG || x == PA_FORMAT_ICON || x == PA_FORMAT_BMP || x == PA_FORMAT_GIF)

static void UpdateReadChars(HWND hwndDlg, HWND hwndStatus);

static const UINT infoLineControls[] = { IDC_PROTOCOL, IDC_NAME };
static const UINT buttonLineControls[] = { IDC_ADD, IDC_USERMENU, IDC_DETAILS, IDC_HISTORY };
static const UINT sendControls[] = { IDC_MESSAGE };

static void NotifyLocalWinEvent(MCONTACT hContact, HWND hwnd, unsigned int type)
{
	if (hContact != NULL && hwnd != NULL) {
		MessageWindowEventData mwe = { 0 };
		mwe.cbSize = sizeof(mwe);
		mwe.hContact = hContact;
		mwe.hwndWindow = hwnd;
		mwe.szModule = SRMMMOD;
		mwe.uType = type;
		mwe.uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;
		mwe.hwndInput = GetDlgItem(hwnd, IDC_MESSAGE);
		mwe.hwndLog = GetDlgItem(hwnd, IDC_LOG);
		NotifyEventHooks(hHookWinEvt, 0, (LPARAM)&mwe);
	}
}

static BOOL IsUtfSendAvailable(MCONTACT hContact)
{
	char *szProto = GetContactProto(hContact);
	if (szProto == NULL)
		return FALSE;

	return (CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_4, 0) & PF4_IMSENDUTF) ? TRUE : FALSE;
}

static int RTL_Detect(const TCHAR *ptszText)
{
	int iLen = (int)_tcslen(ptszText);
	WORD *infoTypeC2 = (WORD*)alloca(sizeof(WORD)* (iLen + 2));
	GetStringTypeEx(LOCALE_USER_DEFAULT, CT_CTYPE2, ptszText, iLen, infoTypeC2);

	for (int i = 0; i < iLen; i++)
		if (infoTypeC2[i] == C2_RIGHTTOLEFT)
			return 1;

	return 0;
}

int SendMessageDirect(const TCHAR *szMsg, MCONTACT hContact, char *szProto)
{
	if (hContact == NULL)
		return NULL;

	int flags = 0;
	int bufSize = 0;
	char *sendBuffer = NULL;

	if (RTL_Detect(szMsg))
		flags |= PREF_RTL;

	if (IsUtfSendAvailable(hContact)) {
		flags |= PREF_UTF;
		sendBuffer = mir_utf8encodeT(szMsg);
		if (!sendBuffer || !sendBuffer[0]) {
			mir_free(sendBuffer);
			return NULL;
		}
		bufSize = (int)strlen(sendBuffer) + 1;
	}
	else {
		flags |= PREF_TCHAR;
		sendBuffer = mir_t2a(szMsg);
		if (!sendBuffer || !sendBuffer[0]) {
			mir_free(sendBuffer);
			return NULL;
		}
		bufSize = (int)strlen(sendBuffer) + 1;

		size_t bufSizeT = (_tcslen(szMsg) + 1) * sizeof(TCHAR);
		sendBuffer = (char*)mir_realloc(sendBuffer, bufSizeT + bufSize);
		memcpy((TCHAR*)&sendBuffer[bufSize], szMsg, bufSizeT);
		bufSize += (int)bufSizeT;
	}

	if (sendBuffer == NULL)
		return NULL;

	if (db_mc_isMeta(hContact))
		hContact = db_mc_getSrmmSub(hContact);

	int sendId = CallContactService(hContact, PSS_MESSAGE, flags, (LPARAM)sendBuffer);
	msgQueue_add(hContact, sendId, sendBuffer, flags);
	return sendId;
}

static void AddToFileList(TCHAR ***pppFiles, int *totalCount, const TCHAR* szFilename)
{
	*pppFiles = (TCHAR**)mir_realloc(*pppFiles, (++*totalCount + 1)*sizeof(TCHAR*));
	(*pppFiles)[*totalCount] = NULL;
	(*pppFiles)[*totalCount - 1] = mir_tstrdup(szFilename);

	if (GetFileAttributes(szFilename) & FILE_ATTRIBUTE_DIRECTORY) {
		WIN32_FIND_DATA fd;
		TCHAR szPath[MAX_PATH];
		mir_sntprintf(szPath, SIZEOF(szPath), _T("%s\\*"), szFilename);
		HANDLE hFind = FindFirstFile(szPath, &fd);
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				if (!_tcscmp(fd.cFileName, _T(".")) || !_tcscmp(fd.cFileName, _T(".."))) continue;
				mir_sntprintf(szPath, SIZEOF(szPath), _T("%s\\%s"), szFilename, fd.cFileName);
				AddToFileList(pppFiles, totalCount, szPath);
			}
			while (FindNextFile(hFind, &fd));
			FindClose(hFind);
		}
	}
}

static void ShowMultipleControls(HWND hwndDlg, const UINT * controls, int cControls, int state)
{
	for (int i = 0; i < cControls; i++)
		ShowWindow(GetDlgItem(hwndDlg, controls[i]), state);
}

static void UpdateReadChars(HWND hwndDlg, HWND hwndStatus)
{
	if (hwndStatus && (g_dat.flags & SMF_SHOWREADCHAR)) {
		TCHAR buf[32];
		int len = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_MESSAGE));

		mir_sntprintf(buf, SIZEOF(buf), _T("%d"), len);
		SendMessage(hwndStatus, SB_SETTEXT, 1, (LPARAM)buf);
	}
}

static void ShowTime(SrmmWindowData *dat)
{
	if (dat->hwndStatus && dat->hTimeZone) {
		SYSTEMTIME st;
		GetSystemTime(&st);
		if (dat->wMinute != st.wMinute) {
			TCHAR buf[32];
			unsigned i = (g_dat.flags & SMF_SHOWREADCHAR) ? 2 : 1;

			tmi.printDateTime(dat->hTimeZone, _T("t"), buf, SIZEOF(buf), 0);
			SendMessage(dat->hwndStatus, SB_SETTEXT, i, (LPARAM)buf);
			dat->wMinute = st.wMinute;
		}
	}
}

static void SetupStatusBar(HWND hwndDlg, SrmmWindowData *dat)
{
	int i = 0, statwidths[4];
	int icons_width = GetStatusIconsCount(dat->hContact) * (GetSystemMetrics(SM_CXSMICON) + 2) + SB_GRIP_WIDTH;

	RECT rc;
	GetWindowRect(dat->hwndStatus, &rc);
	int cx = rc.right - rc.left;

	if (dat->hTimeZone) {
		if (g_dat.flags & SMF_SHOWREADCHAR)
			statwidths[i++] = cx - SB_TIME_WIDTH - SB_CHAR_WIDTH - icons_width;
		statwidths[i++] = cx - SB_TIME_WIDTH - icons_width;
	}
	else if (g_dat.flags & SMF_SHOWREADCHAR)
		statwidths[i++] = cx - SB_CHAR_WIDTH - icons_width;

	statwidths[i++] = cx - icons_width;
	statwidths[i++] = -1;
	SendMessage(dat->hwndStatus, SB_SETPARTS, i, (LPARAM)statwidths);

	UpdateReadChars(hwndDlg, dat->hwndStatus);
	ShowTime(dat);
	SendMessage(hwndDlg, DM_STATUSICONCHANGE, 0, 0);
}

static void SetDialogToType(HWND hwndDlg)
{
	SrmmWindowData *dat = (SrmmWindowData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	if (dat->hContact)
		ShowMultipleControls(hwndDlg, infoLineControls, SIZEOF(infoLineControls), (g_dat.flags&SMF_SHOWINFO) ? SW_SHOW : SW_HIDE);
	else
		ShowMultipleControls(hwndDlg, infoLineControls, SIZEOF(infoLineControls), SW_HIDE);

	if (dat->hContact) {
		ShowMultipleControls(hwndDlg, buttonLineControls, SIZEOF(buttonLineControls), (g_dat.flags&SMF_SHOWBTNS) ? SW_SHOW : SW_HIDE);
		if (!db_get_b(dat->hContact, "CList", "NotOnList", 0))
			ShowWindow(GetDlgItem(hwndDlg, IDC_ADD), SW_HIDE);
	}
	else ShowMultipleControls(hwndDlg, buttonLineControls, SIZEOF(buttonLineControls), SW_HIDE);

	ShowMultipleControls(hwndDlg, sendControls, SIZEOF(sendControls), SW_SHOW);
	if (!dat->hwndStatus) {
		int grip = (GetWindowLongPtr(hwndDlg, GWL_STYLE) & WS_THICKFRAME) ? SBARS_SIZEGRIP : 0;
		dat->hwndStatus = CreateWindowEx(0, STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | grip, 0, 0, 0, 0, hwndDlg, NULL, g_hInst, NULL);
		SendMessage(dat->hwndStatus, SB_SETMINHEIGHT, GetSystemMetrics(SM_CYSMICON), 0);
	}

	ShowWindow(GetDlgItem(hwndDlg, IDCANCEL), SW_HIDE);
	ShowWindow(GetDlgItem(hwndDlg, IDC_SPLITTER), SW_SHOW);
	ShowWindow(GetDlgItem(hwndDlg, IDOK), (g_dat.flags & SMF_SENDBTN) ? SW_SHOW : SW_HIDE);
	EnableWindow(GetDlgItem(hwndDlg, IDOK), GetWindowTextLength(GetDlgItem(hwndDlg, IDC_MESSAGE)) != 0);
	if (dat->avatarPic == NULL || !(g_dat.flags & SMF_AVATAR))
		ShowWindow(GetDlgItem(hwndDlg, IDC_AVATAR), SW_HIDE);
	SendMessage(hwndDlg, DM_UPDATETITLE, 0, 0);
	SendMessage(hwndDlg, WM_SIZE, 0, 0);
}

struct MsgEditSubclassData
{
	DWORD lastEnterTime;
};

static void SetEditorText(HWND hwnd, const TCHAR* txt)
{
	SetWindowText(hwnd, txt);
	SendMessage(hwnd, EM_SETSEL, -1, -1);
}

#define EM_SUBCLASSED             (WM_USER+0x101)
#define ENTERCLICKTIME   1000   //max time in ms during which a double-tap on enter will cause a send

static LRESULT CALLBACK MessageEditSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	SrmmWindowData *pdat = (SrmmWindowData *)GetWindowLongPtr(GetParent(hwnd), GWLP_USERDATA);
	struct MsgEditSubclassData *dat = (struct MsgEditSubclassData *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (msg) {
	case WM_DROPFILES:
		SendMessage(GetParent(hwnd), WM_DROPFILES, (WPARAM)wParam, (LPARAM)lParam);
		break;

	case EM_SUBCLASSED:
		dat = (struct MsgEditSubclassData *) mir_alloc(sizeof(struct MsgEditSubclassData));
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)dat);
		dat->lastEnterTime = 0;
		return 0;

	case WM_CHAR:
		if (GetWindowLongPtr(hwnd, GWL_STYLE) & ES_READONLY)
			break;

		if (wParam == 1 && GetKeyState(VK_CONTROL) & 0x8000) {      //ctrl-a
			SendMessage(hwnd, EM_SETSEL, 0, -1);
			return 0;
		}

		if (wParam == 23 && GetKeyState(VK_CONTROL) & 0x8000) {     // ctrl-w
			SendMessage(GetParent(hwnd), WM_CLOSE, 0, 0);
			return 0;
		}
		break;

	case WM_KEYDOWN:
		if (wParam == VK_RETURN) {
			if (!(GetKeyState(VK_SHIFT) & 0x8000) &&
				 ((GetKeyState(VK_CONTROL) & 0x8000) != 0) != ((g_dat.flags & SMF_SENDONENTER) != 0)) {
				PostMessage(GetParent(hwnd), WM_COMMAND, IDOK, 0);
				return 0;
			}
			if (g_dat.flags & SMF_SENDONDBLENTER) {
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
		else dat->lastEnterTime = 0;

		if (((wParam == VK_INSERT && (GetKeyState(VK_SHIFT) & 0x8000)) || (wParam == 'V' && (GetKeyState(VK_CONTROL) & 0x8000))) &&
			 !(GetKeyState(VK_MENU) & 0x8000)) { // ctrl-v (paste clean text)
			SendMessage(hwnd, WM_PASTE, 0, 0);
			return 0;
		}

		if (wParam == VK_UP && (GetKeyState(VK_CONTROL) & 0x8000) &&
			 ((g_dat.flags & (SMF_AUTOCLOSE | SMF_CTRLSUPPORT)) == SMF_CTRLSUPPORT)) {
			if (pdat->cmdList.getCount()) {
				if (pdat->cmdListInd < 0) {
					pdat->cmdListInd = pdat->cmdList.getCount() - 1;
					SetEditorText(hwnd, pdat->cmdList[pdat->cmdListInd]);
				}
				else if (pdat->cmdListInd > 0) {
					SetEditorText(hwnd, pdat->cmdList[--pdat->cmdListInd]);
				}
			}
			EnableWindow(GetDlgItem(GetParent(hwnd), IDOK), GetWindowTextLength(hwnd) != 0);
			UpdateReadChars(GetParent(hwnd), pdat->hwndStatus);
			return 0;
		}

		if (wParam == VK_DOWN && (GetKeyState(VK_CONTROL) & 0x8000) &&
			 ((g_dat.flags & (SMF_AUTOCLOSE | SMF_CTRLSUPPORT)) == SMF_CTRLSUPPORT)) {
			if (pdat->cmdList.getCount() && pdat->cmdListInd >= 0) {
				if (pdat->cmdListInd < (pdat->cmdList.getCount() - 1))
					SetEditorText(hwnd, pdat->cmdList[++pdat->cmdListInd]);
				else {
					pdat->cmdListInd = -1;
					SetEditorText(hwnd, pdat->cmdList[pdat->cmdList.getCount() - 1]);
				}
			}

			EnableWindow(GetDlgItem(GetParent(hwnd), IDOK), GetWindowTextLength(hwnd) != 0);
			UpdateReadChars(GetParent(hwnd), pdat->hwndStatus);
		}
		break;

	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_MOUSEWHEEL:
	case WM_KILLFOCUS:
		dat->lastEnterTime = 0;
		break;

	case WM_SYSCHAR:
		dat->lastEnterTime = 0;
		if ((wParam == 's' || wParam == 'S') && GetKeyState(VK_MENU) & 0x8000) {
			PostMessage(GetParent(hwnd), WM_COMMAND, IDOK, 0);
			return 0;
		}
		break;

	case WM_CONTEXTMENU:
	{
		static const CHARRANGE all = { 0, -1 };

		MessageWindowPopupData mwpd = { 0 };
		mwpd.cbSize = sizeof(mwpd);
		mwpd.uType = MSG_WINDOWPOPUP_SHOWING;
		mwpd.uFlags = MSG_WINDOWPOPUP_INPUT;
		mwpd.hContact = pdat->hContact;
		mwpd.hwnd = hwnd;

		HMENU hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_CONTEXT));

		mwpd.hMenu = GetSubMenu(hMenu, 2);
		TranslateMenu(mwpd.hMenu);

		CHARRANGE sel;
		SendMessage(hwnd, EM_EXGETSEL, 0, (LPARAM)&sel);
		if (sel.cpMin == sel.cpMax) {
			EnableMenuItem(mwpd.hMenu, IDM_CUT, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(mwpd.hMenu, IDM_COPY, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(mwpd.hMenu, IDM_DELETE, MF_BYCOMMAND | MF_GRAYED);
		}
		if (!SendMessage(hwnd, EM_CANUNDO, 0, 0))
			EnableMenuItem(mwpd.hMenu, IDM_UNDO, MF_BYCOMMAND | MF_GRAYED);

		if (!SendMessage(hwnd, EM_CANREDO, 0, 0))
			EnableMenuItem(mwpd.hMenu, IDM_REDO, MF_BYCOMMAND | MF_GRAYED);

		if (!SendMessage(hwnd, EM_CANPASTE, 0, 0)) {
			if (!IsClipboardFormatAvailable(CF_HDROP))
				EnableMenuItem(mwpd.hMenu, IDM_PASTE, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(mwpd.hMenu, IDM_PASTESEND, MF_BYCOMMAND | MF_GRAYED);
		}

		if (lParam == 0xFFFFFFFF) {
			SendMessage(hwnd, EM_POSFROMCHAR, (WPARAM)&mwpd.pt, (LPARAM)sel.cpMax);
			ClientToScreen(hwnd, &mwpd.pt);
		}
		else {
			mwpd.pt.x = GET_X_LPARAM(lParam);
			mwpd.pt.y = GET_Y_LPARAM(lParam);
		}


		// First notification
		NotifyEventHooks(hHookWinPopup, 0, (LPARAM)&mwpd);

		// Someone added items?
		if (GetMenuItemCount(mwpd.hMenu) > 0) {
			SetCursor(LoadCursor(NULL, IDC_ARROW));
			mwpd.selection = TrackPopupMenu(mwpd.hMenu, TPM_RETURNCMD, mwpd.pt.x, mwpd.pt.y, 0, hwnd, NULL);
		}

		// Second notification
		mwpd.uType = MSG_WINDOWPOPUP_SELECTED;
		NotifyEventHooks(hHookWinPopup, 0, (LPARAM)&mwpd);

		switch (mwpd.selection) {
		case IDM_UNDO:
			SendMessage(hwnd, WM_UNDO, 0, 0);
			break;

		case IDM_REDO:
			SendMessage(hwnd, EM_REDO, 0, 0);
			break;

		case IDM_CUT:
			SendMessage(hwnd, WM_CUT, 0, 0);
			break;

		case IDM_COPY:
			SendMessage(hwnd, WM_COPY, 0, 0);
			break;

		case IDM_PASTE:
			SendMessage(hwnd, WM_PASTE, 0, 0);
			break;

		case IDM_PASTESEND:
			SendMessage(hwnd, EM_PASTESPECIAL, CF_TEXT, 0);
			PostMessage(GetParent(hwnd), WM_COMMAND, IDOK, 0);
			break;

		case IDM_DELETE:
			SendMessage(hwnd, EM_REPLACESEL, TRUE, 0);
			break;

		case IDM_SELECTALL:
			SendMessage(hwnd, EM_EXSETSEL, 0, (LPARAM)&all);
			break;

		case IDM_CLEAR:
			SetWindowText(hwnd, _T(""));
			break;
		}
		DestroyMenu(hMenu);
		return 0;
	}

	case WM_PASTE:
		if (IsClipboardFormatAvailable(CF_HDROP)) {
			if (OpenClipboard(hwnd)) {
				HANDLE hDrop = GetClipboardData(CF_HDROP);
				if (hDrop)
					SendMessage(hwnd, WM_DROPFILES, (WPARAM)hDrop, 0);
				CloseClipboard();
			}
		}
		else SendMessage(hwnd, EM_PASTESPECIAL, CF_TEXT, 0);
		return 0;

	case WM_DESTROY:
		mir_free(dat);
		break;
	}
	return mir_callNextSubclass(hwnd, MessageEditSubclassProc, msg, wParam, lParam);
}

static LRESULT CALLBACK SplitterSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	SrmmWindowData *pdat = (SrmmWindowData *)GetWindowLongPtr(GetParent(hwnd), GWLP_USERDATA);

	switch (msg) {
	case WM_NCHITTEST:
		return HTCLIENT;

	case WM_SETCURSOR:
	{
		RECT rc;
		GetClientRect(hwnd, &rc);
		SetCursor(rc.right > rc.bottom ? hCurSplitNS : hCurSplitWE);
	}
		return TRUE;

	case WM_LBUTTONDOWN:
		SetCapture(hwnd);
		return 0;

	case WM_MOUSEMOVE:
		if (GetCapture() == hwnd) {
			RECT rc;
			GetClientRect(hwnd, &rc);
			SendMessage(GetParent(hwnd), DM_SPLITTERMOVED, rc.right > rc.bottom ? (short)HIWORD(GetMessagePos()) + rc.bottom / 2 : (short)LOWORD(GetMessagePos()) + rc.right / 2, (LPARAM)hwnd);
		}
		return 0;

	case WM_LBUTTONUP:
		ReleaseCapture();
		return 0;
	}
	return mir_callNextSubclass(hwnd, SplitterSubclassProc, msg, wParam, lParam);
}

static int MessageDialogResize(HWND hwndDlg, LPARAM lParam, UTILRESIZECONTROL * urc)
{
	SrmmWindowData *dat = (SrmmWindowData *)lParam;

	if (!(g_dat.flags & SMF_SHOWINFO) && !(g_dat.flags & SMF_SHOWBTNS)) {
		for (int i = 0; i < SIZEOF(buttonLineControls); i++)
			if (buttonLineControls[i] == urc->wId)
				OffsetRect(&urc->rcItem, 0, -dat->lineHeight);
	}

	switch (urc->wId) {
	case IDC_NAME:
	{
		HWND h = GetDlgItem(hwndDlg, IDC_NAME);
		int len = GetWindowTextLength(h);
		if (len > 0) {
			TCHAR buf[256];
			GetWindowText(h, buf, SIZEOF(buf));

			HDC hdc = GetDC(h);
			HFONT hFont = (HFONT)SelectObject(hdc, (HFONT)SendDlgItemMessage(hwndDlg, IDOK, WM_GETFONT, 0, 0));

			SIZE textSize;
			GetTextExtentPoint32(hdc, buf, (int)mir_tstrlen(buf), &textSize);
			urc->rcItem.right = urc->rcItem.left + textSize.cx + 10;
			if ((g_dat.flags&SMF_SHOWBTNS) && urc->rcItem.right > urc->dlgNewSize.cx - dat->nLabelRight)
				urc->rcItem.right = urc->dlgNewSize.cx - dat->nLabelRight;
			SelectObject(hdc, hFont);
			ReleaseDC(h, hdc);
		}
	}
	case IDC_PROTOCOL:
		return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;

	case IDC_ADD:
	case IDC_USERMENU:
	case IDC_DETAILS:
	case IDC_HISTORY:
		return RD_ANCHORX_RIGHT | RD_ANCHORY_TOP;

	case IDC_LOG:
		if (!(g_dat.flags&SMF_SHOWINFO) && !(g_dat.flags&SMF_SHOWBTNS))
			urc->rcItem.top -= dat->lineHeight;
		urc->rcItem.bottom -= dat->splitterPos - dat->originalSplitterPos;
		return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;

	case IDC_SPLITTER:
		urc->rcItem.top -= dat->splitterPos - dat->originalSplitterPos;
		urc->rcItem.bottom -= dat->splitterPos - dat->originalSplitterPos;
		return RD_ANCHORX_WIDTH | RD_ANCHORY_BOTTOM;

	case IDC_MESSAGE:
		if (!(g_dat.flags & SMF_SENDBTN))
			urc->rcItem.right = urc->dlgNewSize.cx - urc->rcItem.left;
		if ((g_dat.flags & SMF_AVATAR) && dat->avatarPic)
			urc->rcItem.left = dat->avatarWidth + 4;

		urc->rcItem.top -= dat->splitterPos - dat->originalSplitterPos;
		if (!(g_dat.flags & SMF_SENDBTN))
			return RD_ANCHORX_CUSTOM | RD_ANCHORY_BOTTOM;
		return RD_ANCHORX_WIDTH | RD_ANCHORY_BOTTOM;

	case IDCANCEL:
	case IDOK:
		urc->rcItem.top -= dat->splitterPos - dat->originalSplitterPos;
		return RD_ANCHORX_RIGHT | RD_ANCHORY_BOTTOM;

	case IDC_AVATAR:
		urc->rcItem.top = urc->rcItem.bottom - (dat->avatarHeight + 2);
		urc->rcItem.right = urc->rcItem.left + (dat->avatarWidth + 2);
		return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM;
	}
	return RD_ANCHORX_LEFT | RD_ANCHORY_TOP;
}

void ShowAvatar(HWND hwndDlg, SrmmWindowData *dat)
{
	if (g_dat.flags & SMF_AVATAR) {
		AVATARCACHEENTRY *ace = (AVATARCACHEENTRY *)CallService(MS_AV_GETAVATARBITMAP, (WPARAM)dat->getActiveContact(), 0);
		if (ace && (INT_PTR)ace != CALLSERVICE_NOTFOUND && (ace->dwFlags & AVS_BITMAP_VALID) && !(ace->dwFlags & AVS_HIDEONCLIST))
			dat->avatarPic = ace->hbmPic;
		else
			dat->avatarPic = NULL;
	}
	else dat->avatarPic = NULL;

	SendMessage(hwndDlg, DM_UPDATESIZEBAR, 0, 0);
	SendMessage(hwndDlg, DM_AVATARSIZECHANGE, 0, 0);
}

static void NotifyTyping(SrmmWindowData *dat, int mode)
{
	if (!dat->hContact)
		return;
	// Don't send to protocols who don't support typing
	// Don't send to users who are unchecked in the typing notification options
	// Don't send to protocols that are offline
	// Don't send to users who are not visible and
	// Don't send to users who are not on the visible list when you are in invisible mode.

	if (!db_get_b(dat->hContact, SRMMMOD, SRMSGSET_TYPING, db_get_b(NULL, SRMMMOD, SRMSGSET_TYPINGNEW, SRMSGDEFSET_TYPINGNEW)))
		return;

	if (!dat->szProto)
		return;

	DWORD protoStatus = CallProtoService(dat->szProto, PS_GETSTATUS, 0, 0);
	DWORD protoCaps = CallProtoService(dat->szProto, PS_GETCAPS, PFLAGNUM_1, 0);
	DWORD typeCaps = CallProtoService(dat->szProto, PS_GETCAPS, PFLAGNUM_4, 0);

	if (!(typeCaps & PF4_SUPPORTTYPING))
		return;

	if (protoStatus < ID_STATUS_ONLINE)
		return;

	if (protoCaps & PF1_VISLIST && db_get_w(dat->hContact, dat->szProto, "ApparentMode", 0) == ID_STATUS_OFFLINE)
		return;

	if (protoCaps & PF1_INVISLIST && protoStatus == ID_STATUS_INVISIBLE && db_get_w(dat->hContact, dat->szProto, "ApparentMode", 0) != ID_STATUS_ONLINE)
		return;

	if (!(g_dat.flags & SMF_TYPINGUNKNOWN) && db_get_b(dat->hContact, "CList", "NotOnList", 0))
		return;

	// End user check
	dat->nTypeMode = mode;
	CallService(MS_PROTO_SELFISTYPING, (WPARAM)dat->hContact, dat->nTypeMode);
}

void Button_SetIcon_IcoLib(HWND hwndDlg, int itemId, int iconId, const char* tooltip)
{
	HWND hWnd = GetDlgItem(hwndDlg, itemId);
	SendMessage(hWnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadSkinnedIcon(iconId));
	SendMessage(hWnd, BUTTONSETASFLATBTN, TRUE, 0);
	SendMessage(hWnd, BUTTONADDTOOLTIP, (WPARAM)tooltip, 0);
}

void Button_FreeIcon_IcoLib(HWND hwndDlg, int itemId)
{
	Skin_ReleaseIcon((HICON)SendDlgItemMessage(hwndDlg, itemId, BM_SETIMAGE, IMAGE_ICON, 0));
}

void Window_FreeIcon_IcoLib(HWND hwndDlg)
{
	Skin_ReleaseIcon((HICON)SendMessage(hwndDlg, WM_SETICON, ICON_BIG, 0));
	Skin_ReleaseIcon((HICON)SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, 0));
}

INT_PTR CALLBACK DlgProcMessage(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	SrmmWindowData *dat = (SrmmWindowData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			NewMessageWindowLParam *newData = (NewMessageWindowLParam *)lParam;
			dat = new SrmmWindowData();
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);

			dat->hContact = newData->hContact;
			dat->bIsMeta = db_mc_isMeta(dat->hContact) != 0;
			dat->hTimeZone = tmi.createByContact(dat->hContact, 0, TZF_KNOWNONLY);
			dat->wMinute = 61;

			NotifyLocalWinEvent(dat->hContact, hwndDlg, MSG_WINDOW_EVT_OPENING);
			if (newData->szInitialText) {
				int len;

				if (newData->isWchar)
					SetDlgItemText(hwndDlg, IDC_MESSAGE, (TCHAR*)newData->szInitialText);
				else
					SetDlgItemTextA(hwndDlg, IDC_MESSAGE, newData->szInitialText);
				len = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_MESSAGE));
				PostMessage(GetDlgItem(hwndDlg, IDC_MESSAGE), EM_SETSEL, len, len);
			}

			dat->szProto = GetContactProto(dat->hContact);
			RichUtil_SubClass(GetDlgItem(hwndDlg, IDC_LOG));
			RichUtil_SubClass(GetDlgItem(hwndDlg, IDC_MESSAGE));

			// avatar stuff
			dat->limitAvatarH = db_get_b(NULL, SRMMMOD, SRMSGSET_LIMITAVHEIGHT, SRMSGDEFSET_LIMITAVHEIGHT) ? db_get_dw(NULL, SRMMMOD, SRMSGSET_AVHEIGHT, SRMSGDEFSET_AVHEIGHT) : 0;

			if (dat->hContact && dat->szProto != NULL)
				dat->wStatus = db_get_w(dat->hContact, dat->szProto, "Status", ID_STATUS_OFFLINE);
			else
				dat->wStatus = ID_STATUS_OFFLINE;
			dat->wOldStatus = dat->wStatus;
			dat->splitterPos = (int)db_get_dw(db_get_b(NULL, SRMMMOD, SRMSGSET_SAVEPERCONTACT, SRMSGDEFSET_SAVEPERCONTACT) ? dat->hContact : NULL, SRMMMOD, "splitterPos", (DWORD)-1);
			dat->cmdListInd = -1;
			dat->nTypeMode = PROTOTYPE_SELFTYPING_OFF;
			SetTimer(hwndDlg, TIMERID_TYPE, 1000, NULL);
			{
				RECT rc, rc2;
				GetWindowRect(GetDlgItem(hwndDlg, IDC_USERMENU), &rc);
				GetWindowRect(hwndDlg, &rc2);
				dat->nLabelRight = rc2.right - rc.left;

				POINT pt;
				GetWindowRect(GetDlgItem(hwndDlg, IDC_SPLITTER), &rc);
				pt.y = (rc.top + rc.bottom) / 2;
				pt.x = 0;
				ScreenToClient(hwndDlg, &pt);
				dat->originalSplitterPos = pt.y;
				if (dat->splitterPos == -1)
					dat->splitterPos = dat->originalSplitterPos;// + 60;
				GetWindowRect(GetDlgItem(hwndDlg, IDC_ADD), &rc);
				dat->lineHeight = rc.bottom - rc.top + 3;
			}
			WindowList_Add(g_dat.hMessageWindowList, hwndDlg, dat->hContact);
			GetWindowRect(GetDlgItem(hwndDlg, IDC_MESSAGE), &dat->minEditInit);
			SendMessage(hwndDlg, DM_UPDATESIZEBAR, 0, 0);
			dat->hwndStatus = NULL;
			Button_SetIcon_IcoLib(hwndDlg, IDC_ADD, SKINICON_OTHER_ADDCONTACT, LPGEN("Add contact permanently to list"));
			Button_SetIcon_IcoLib(hwndDlg, IDC_DETAILS, SKINICON_OTHER_USERDETAILS, LPGEN("View user's details"));
			Button_SetIcon_IcoLib(hwndDlg, IDC_HISTORY, SKINICON_OTHER_HISTORY, LPGEN("View user's history"));
			Button_SetIcon_IcoLib(hwndDlg, IDC_USERMENU, SKINICON_OTHER_DOWNARROW, LPGEN("User menu"));
			SendDlgItemMessage(hwndDlg, IDC_NAME, BUTTONSETASFLATBTN, TRUE, 0);

			EnableWindow(GetDlgItem(hwndDlg, IDC_PROTOCOL), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_AVATAR), FALSE);

			SendDlgItemMessage(hwndDlg, IDC_LOG, EM_SETOLECALLBACK, 0, (LPARAM)& reOleCallback);
			SendDlgItemMessage(hwndDlg, IDC_LOG, EM_SETEVENTMASK, 0, ENM_MOUSEEVENTS | ENM_LINK | ENM_SCROLL);
			SendDlgItemMessage(hwndDlg, IDC_LOG, EM_AUTOURLDETECT, TRUE, 0);

			if (dat->hContact && dat->szProto) {
				int nMax = CallProtoService(dat->szProto, PS_GETCAPS, PFLAG_MAXLENOFMESSAGE, (LPARAM)dat->hContact);
				if (nMax)
					SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_LIMITTEXT, (WPARAM)nMax, 0);

				// get around a lame bug in the Windows template resource code where richedits are limited to 0x7FFF
				SendDlgItemMessage(hwndDlg, IDC_LOG, EM_LIMITTEXT, (WPARAM) sizeof(TCHAR) * 0x7FFFFFFF, 0);
			}

			mir_subclassWindow(GetDlgItem(hwndDlg, IDC_MESSAGE), MessageEditSubclassProc);
			SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_SUBCLASSED, 0, 0);
			mir_subclassWindow(GetDlgItem(hwndDlg, IDC_SPLITTER), SplitterSubclassProc);

			if (dat->hContact) {
				int historyMode = db_get_b(NULL, SRMMMOD, SRMSGSET_LOADHISTORY, SRMSGDEFSET_LOADHISTORY);
				// This finds the first message to display, it works like shit
				dat->hDbEventFirst = db_event_firstUnread(dat->hContact);
				switch (historyMode) {
				case LOADHISTORY_COUNT:
					for (int i = db_get_w(NULL, SRMMMOD, SRMSGSET_LOADCOUNT, SRMSGDEFSET_LOADCOUNT); i--;) {
						MEVENT hPrevEvent;
						if (dat->hDbEventFirst == NULL)
							hPrevEvent = db_event_last(dat->hContact);
						else
							hPrevEvent = db_event_prev(dat->hContact, dat->hDbEventFirst);
						if (hPrevEvent == NULL)
							break;

						DBEVENTINFO dbei = { sizeof(dbei) };
						dat->hDbEventFirst = hPrevEvent;
						db_event_get(hPrevEvent, &dbei);
						if (!DbEventIsShown(&dbei))
							i++;
					}
					break;

				case LOADHISTORY_TIME:
					DBEVENTINFO dbei = { sizeof(dbei) };
					if (dat->hDbEventFirst == NULL)
						dbei.timestamp = (DWORD)time(NULL);
					else
						db_event_get(dat->hDbEventFirst, &dbei);

					DWORD firstTime = dbei.timestamp - 60 * db_get_w(NULL, SRMMMOD, SRMSGSET_LOADTIME, SRMSGDEFSET_LOADTIME);
					for (;;) {
						MEVENT hPrevEvent;
						if (dat->hDbEventFirst == NULL)
							hPrevEvent = db_event_last(dat->hContact);
						else
							hPrevEvent = db_event_prev(dat->hContact, dat->hDbEventFirst);
						if (hPrevEvent == NULL)
							break;

						dbei.cbBlob = 0;
						db_event_get(hPrevEvent, &dbei);
						if (dbei.timestamp < firstTime)
							break;
						dat->hDbEventFirst = hPrevEvent;
					}
					break;
				}
			}

			MEVENT hdbEvent = db_event_last(dat->hContact);
			if (hdbEvent) {
				do {
					DBEVENTINFO dbei = { sizeof(dbei) };
					db_event_get(hdbEvent, &dbei);
					if ((dbei.eventType == EVENTTYPE_MESSAGE) && !(dbei.flags & DBEF_SENT)) {
						dat->lastMessage = dbei.timestamp;
						PostMessage(hwndDlg, DM_UPDATELASTMESSAGE, 0, 0);
						break;
					}
				}
				while (hdbEvent = db_event_prev(dat->hContact, hdbEvent));
			}

			SendMessage(hwndDlg, DM_OPTIONSAPPLIED, 1, 0);

			//restore saved msg if any...
			if (dat->hContact) {
				DBVARIANT dbv;
				if (!db_get_ts(dat->hContact, SRMSGMOD, DBSAVEDMSG, &dbv)) {
					if (dbv.ptszVal[0]) {
						SetDlgItemText(hwndDlg, IDC_MESSAGE, dbv.ptszVal);
						EnableWindow(GetDlgItem(hwndDlg, IDOK), TRUE);
						UpdateReadChars(hwndDlg, dat->hwndStatus);
						PostMessage(GetDlgItem(hwndDlg, IDC_MESSAGE), EM_SETSEL, -1, -1);
					}
					db_free(&dbv);
				}
			}
			SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_SETEVENTMASK, 0, ENM_CHANGE);
			{
				int flag = newData->noActivate ? RWPF_HIDDEN : 0;
				int savePerContact = db_get_b(NULL, SRMMMOD, SRMSGSET_SAVEPERCONTACT, SRMSGDEFSET_SAVEPERCONTACT);
				if (Utils_RestoreWindowPositionEx(hwndDlg, flag, savePerContact ? dat->hContact : NULL, SRMMMOD, "")) {
					if (savePerContact) {
						if (Utils_RestoreWindowPositionEx(hwndDlg, flag | RWPF_NOMOVE, NULL, SRMMMOD, ""))
							SetWindowPos(hwndDlg, 0, 0, 0, 450, 300, SWP_NOZORDER | SWP_NOMOVE | SWP_SHOWWINDOW);
					}
					else SetWindowPos(hwndDlg, 0, 0, 0, 450, 300, SWP_NOZORDER | SWP_NOMOVE | SWP_SHOWWINDOW);
				}
				if (!savePerContact && db_get_b(NULL, SRMMMOD, SRMSGSET_CASCADE, SRMSGDEFSET_CASCADE))
					WindowList_Broadcast(g_dat.hMessageWindowList, DM_CASCADENEWWINDOW, (WPARAM)hwndDlg, (LPARAM)& dat->windowWasCascaded);
			}
			if (newData->noActivate) {
				SetWindowPos(hwndDlg, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
				SetTimer(hwndDlg, TIMERID_FLASHWND, TIMEOUT_FLASHWND, NULL);
			}
			else {
				SetWindowPos(hwndDlg, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
				SetForegroundWindow(hwndDlg);
				SetFocus(GetDlgItem(hwndDlg, IDC_MESSAGE));
			}
		}

		SendMessage(hwndDlg, DM_GETAVATAR, 0, 0);

		NotifyLocalWinEvent(dat->hContact, hwndDlg, MSG_WINDOW_EVT_OPEN);
		return FALSE;

	case WM_CONTEXTMENU:
		if (dat->hwndStatus && dat->hwndStatus == (HWND)wParam) {
			POINT pt, pt2;
			GetCursorPos(&pt);
			pt2.x = pt.x; pt2.y = pt.y;
			ScreenToClient(dat->hwndStatus, &pt);

			// no popup menu for status icons - this is handled via NM_RCLICK notification and the plugins that added the icons
			RECT rc;
			SendMessage(dat->hwndStatus, SB_GETRECT, SendMessage(dat->hwndStatus, SB_GETPARTS, 0, 0) - 1, (LPARAM)&rc);
			if (pt.x >= rc.left)
				break;

			HMENU hMenu = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT, (WPARAM)dat->hContact, 0);

			TrackPopupMenu(hMenu, 0, pt2.x, pt2.y, 0, hwndDlg, NULL);
			DestroyMenu(hMenu);
		}
		break;

		// Mod from tabsrmm
	case WM_DROPFILES:
		if (dat->szProto == NULL) break;
		if (!(CallProtoService(dat->szProto, PS_GETCAPS, PFLAGNUM_1, 0)&PF1_FILESEND)) break;
		if (dat->wStatus == ID_STATUS_OFFLINE) break;
		if (dat->hContact != NULL) {
			TCHAR szFilename[MAX_PATH];
			HDROP hDrop = (HDROP)wParam;
			int fileCount = DragQueryFile(hDrop, -1, NULL, 0), totalCount = 0, i;
			TCHAR** ppFiles = NULL;
			for (i = 0; i < fileCount; i++) {
				DragQueryFile(hDrop, i, szFilename, SIZEOF(szFilename));
				AddToFileList(&ppFiles, &totalCount, szFilename);
			}
			CallServiceSync(MS_FILE_SENDSPECIFICFILEST, (WPARAM)dat->hContact, (LPARAM)ppFiles);
			for (i = 0; ppFiles[i]; i++)
				mir_free(ppFiles[i]);
			mir_free(ppFiles);
		}
		break;

	case HM_AVATARACK:
		ShowAvatar(hwndDlg, dat);
		break;

	case DM_AVATARCALCSIZE:
		if (dat->avatarPic == NULL || !(g_dat.flags & SMF_AVATAR)) {
			dat->avatarWidth = 50;
			dat->avatarHeight = 50;
			ShowWindow(GetDlgItem(hwndDlg, IDC_AVATAR), SW_HIDE);
			return 0;
		}
		else {
			BITMAP bminfo;
			GetObject(dat->avatarPic, sizeof(bminfo), &bminfo);
			dat->avatarWidth = bminfo.bmWidth + 2;
			dat->avatarHeight = bminfo.bmHeight + 2;
			if (dat->limitAvatarH&&dat->avatarHeight > dat->limitAvatarH) {
				dat->avatarWidth = bminfo.bmWidth * dat->limitAvatarH / bminfo.bmHeight + 2;
				dat->avatarHeight = dat->limitAvatarH + 2;
			}
			ShowWindow(GetDlgItem(hwndDlg, IDC_AVATAR), SW_SHOW);
		}
		break;

	case DM_UPDATESIZEBAR:
		dat->minEditBoxSize.cx = dat->minEditInit.right - dat->minEditInit.left;
		dat->minEditBoxSize.cy = dat->minEditInit.bottom - dat->minEditInit.top;
		if (g_dat.flags&SMF_AVATAR) {
			SendMessage(hwndDlg, DM_AVATARCALCSIZE, 0, 0);
			if (dat->avatarPic && dat->minEditBoxSize.cy <= dat->avatarHeight)
				dat->minEditBoxSize.cy = dat->avatarHeight;
		}
		break;

	case DM_AVATARSIZECHANGE:
		{
			RECT rc;
			GetWindowRect(GetDlgItem(hwndDlg, IDC_MESSAGE), &rc);
			if (rc.bottom - rc.top < dat->minEditBoxSize.cy)
				SendMessage(hwndDlg, DM_SPLITTERMOVED, rc.top - (rc.bottom - rc.top - dat->minEditBoxSize.cy - 4), (LPARAM)GetDlgItem(hwndDlg, IDC_SPLITTER));

			SendMessage(hwndDlg, WM_SIZE, 0, 0);
		}
		break;

	case DM_GETAVATAR:
		{
			PROTO_AVATAR_INFORMATIONT ai = { sizeof(ai), dat->hContact };
			CallProtoService(dat->szProto, PS_GETAVATARINFOT, GAIF_FORCE, (LPARAM)&ai);

			ShowAvatar(hwndDlg, dat);
			SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, 1);
		}
		break;

	case DM_TYPING:
		dat->nTypeSecs = (INT_PTR)lParam > 0 ? (int)lParam : 0;
		break;

	case DM_UPDATEWINICON:
		if (db_get_b(NULL, SRMMMOD, SRMSGSET_STATUSICON, SRMSGDEFSET_STATUSICON)) {
			Window_FreeIcon_IcoLib(hwndDlg);

			if (dat->szProto) {
				WORD wStatus = db_get_w(dat->hContact, dat->szProto, "Status", ID_STATUS_OFFLINE);
				SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadSkinnedProtoIconBig(dat->szProto, wStatus));
				SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadSkinnedProtoIcon(dat->szProto, wStatus));
				break;
			}
		}
		SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadSkinnedIconBig(SKINICON_EVENT_MESSAGE));
		SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadSkinnedIcon(SKINICON_EVENT_MESSAGE));
		break;

	case DM_USERNAMETOCLIP:
		if (dat->hContact) {
			TCHAR buf[128] = _T("");
			CONTACTINFO ci = { sizeof(ci) };
			ci.hContact = dat->hContact;
			ci.szProto = dat->szProto;
			ci.dwFlag = CNF_UNIQUEID | CNF_TCHAR;
			if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)& ci)) {
				switch (ci.type) {
				case CNFT_ASCIIZ:
					_tcsncpy_s(buf, ci.pszVal, _TRUNCATE);
					mir_free(ci.pszVal);
					break;

				case CNFT_DWORD:
					mir_sntprintf(buf, SIZEOF(buf), _T("%u"), ci.dVal);
					break;
				}
			}
			if (buf[0] && OpenClipboard(hwndDlg)) {
				EmptyClipboard();
				HGLOBAL hData = GlobalAlloc(GMEM_MOVEABLE, _tcslen(buf) * sizeof(TCHAR)+1);
				_tcscpy((TCHAR*)GlobalLock(hData), buf);
				GlobalUnlock(hData);
				SetClipboardData(CF_UNICODETEXT, hData);
				CloseClipboard();
			}
		}
		break;

	case DM_UPDATELASTMESSAGE:
		if (!dat->hwndStatus || dat->nTypeSecs)
			break;

		if (dat->lastMessage) {
			TCHAR date[64], time[64], fmt[128];
			tmi.printTimeStamp(NULL, dat->lastMessage, _T("d"), date, SIZEOF(date), 0);
			tmi.printTimeStamp(NULL, dat->lastMessage, _T("t"), time, SIZEOF(time), 0);
			mir_sntprintf(fmt, SIZEOF(fmt), TranslateT("Last message received on %s at %s."), date, time);
			SendMessage(dat->hwndStatus, SB_SETTEXT, 0, (LPARAM)fmt);
		}
		else SendMessage(dat->hwndStatus, SB_SETTEXT, 0, (LPARAM)_T(""));

		SendMessage(dat->hwndStatus, SB_SETICON, 0, (LPARAM)NULL);
		break;

	case DM_OPTIONSAPPLIED:
		SetDialogToType(hwndDlg);
		if (dat->hBkgBrush)
			DeleteObject(dat->hBkgBrush);
		{
			COLORREF colour = db_get_dw(NULL, SRMMMOD, SRMSGSET_BKGCOLOUR, SRMSGDEFSET_BKGCOLOUR);
			dat->hBkgBrush = CreateSolidBrush(colour);
			SendDlgItemMessage(hwndDlg, IDC_LOG, EM_SETBKGNDCOLOR, 0, colour);
			SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_SETBKGNDCOLOR, 0, colour);

			// avatar stuff
			dat->avatarPic = NULL;
			dat->limitAvatarH = 0;
			if (CallProtoService(dat->szProto, PS_GETCAPS, PFLAGNUM_4, 0) & PF4_AVATARS)
				dat->limitAvatarH = db_get_b(NULL, SRMMMOD, SRMSGSET_LIMITAVHEIGHT, SRMSGDEFSET_LIMITAVHEIGHT) ?
				db_get_dw(NULL, SRMMMOD, SRMSGSET_AVHEIGHT, SRMSGDEFSET_AVHEIGHT) : 0;

			if (!wParam)
				SendMessage(hwndDlg, DM_GETAVATAR, 0, 0);
		}
		InvalidateRect(GetDlgItem(hwndDlg, IDC_MESSAGE), NULL, FALSE);
		{
			LOGFONT lf;
			CHARFORMAT cf = { 0 };
			if (dat->hFont) DeleteObject(dat->hFont);
			LoadMsgDlgFont(MSGFONTID_MESSAGEAREA, &lf, &cf.crTextColor);
			dat->hFont = CreateFontIndirect(&lf);
			SendDlgItemMessage(hwndDlg, IDC_MESSAGE, WM_SETFONT, (WPARAM)dat->hFont, MAKELPARAM(TRUE, 0));

			cf.cbSize = sizeof(CHARFORMAT);
			cf.dwMask = CFM_COLOR;
			SendDlgItemMessage(hwndDlg, IDC_MESSAGE, EM_SETCHARFORMAT, SCF_ALL, (WPARAM)&cf);
		}

		// configure message history for proper RTL formatting
		{
			PARAFORMAT2 pf2;
			memset(&pf2, 0, sizeof(pf2));
			pf2.cbSize = sizeof(pf2);

			pf2.wEffects = PFE_RTLPARA;
			pf2.dwMask = PFM_RTLPARA;
			SetDlgItemText(hwndDlg, IDC_LOG, _T(""));
			SendDlgItemMessage(hwndDlg, IDC_LOG, EM_SETPARAFORMAT, 0, (LPARAM)&pf2);
			pf2.wEffects = 0;
			SendDlgItemMessage(hwndDlg, IDC_LOG, EM_SETPARAFORMAT, 0, (LPARAM)&pf2);
			SendDlgItemMessage(hwndDlg, IDC_LOG, EM_SETLANGOPTIONS, 0, (LPARAM)SendDlgItemMessage(hwndDlg, IDC_LOG, EM_GETLANGOPTIONS, 0, 0) & ~IMF_AUTOKEYBOARD);
		}
		SendMessage(hwndDlg, DM_REMAKELOG, 0, 0);
		SendMessage(hwndDlg, DM_UPDATEWINICON, 0, 0);
		break;

	case DM_UPDATETITLE:
		{
			TCHAR newtitle[256];
			if (dat->hContact && dat->szProto) {
				int statusIcon = db_get_b(NULL, SRMMMOD, SRMSGSET_STATUSICON, SRMSGDEFSET_STATUSICON);

				dat->wStatus = db_get_w(dat->hContact, dat->szProto, "Status", ID_STATUS_OFFLINE);
				TCHAR *contactName = pcli->pfnGetContactDisplayName(dat->hContact, 0);

				TCHAR buf[128] = _T("");
				if (strcmp(dat->szProto, META_PROTO)) {
					CONTACTINFO ci = { 0 };
					ci.cbSize = sizeof(ci);
					ci.hContact = dat->hContact;
					ci.szProto = dat->szProto;
					ci.dwFlag = CNF_DISPLAYUID | CNF_TCHAR;
					if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)&ci)) {
						switch (ci.type) {
						case CNFT_ASCIIZ:
							_tcsncpy_s(buf, ci.pszVal, _TRUNCATE);
							mir_free(ci.pszVal);
							break;
						case CNFT_DWORD:
							mir_sntprintf(buf, SIZEOF(buf), _T("%u"), ci.dVal);
							break;
						}
					}
				}
				if (buf[0])
					SetDlgItemText(hwndDlg, IDC_NAME, buf);
				else
					SetDlgItemText(hwndDlg, IDC_NAME, contactName);

				TCHAR *szStatus = pcli->pfnGetStatusModeDescription(dat->szProto == NULL ? ID_STATUS_OFFLINE : db_get_w(dat->hContact, dat->szProto, "Status", ID_STATUS_OFFLINE), 0);
				if (statusIcon)
					mir_sntprintf(newtitle, SIZEOF(newtitle), _T("%s - %s"), contactName, TranslateT("Message session"));
				else
					mir_sntprintf(newtitle, SIZEOF(newtitle), _T("%s (%s): %s"), contactName, szStatus, TranslateT("Message session"));
					
				DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *)wParam;
				if (!cws || (!strcmp(cws->szModule, dat->szProto) && !strcmp(cws->szSetting, "Status"))) {
					InvalidateRect(GetDlgItem(hwndDlg, IDC_PROTOCOL), NULL, TRUE);
					if (statusIcon)
						SendMessage(hwndDlg, DM_UPDATEWINICON, 0, 0);
				}

				dat->wOldStatus = dat->wStatus;
			}
			else
				mir_tstrncpy(newtitle, TranslateT("Message session"), SIZEOF(newtitle));

			TCHAR oldtitle[256];
			GetWindowText(hwndDlg, oldtitle, SIZEOF(oldtitle));
			if (_tcscmp(newtitle, oldtitle)) { //swt() flickers even if the title hasn't actually changed
				SetWindowText(hwndDlg, newtitle);
				SendMessage(hwndDlg, WM_SIZE, 0, 0);
			}
		}
		break;

	case DM_NEWTIMEZONE:
		dat->hTimeZone = tmi.createByContact(dat->hContact, 0, TZF_KNOWNONLY);
		dat->wMinute = 61;
		SendMessage(hwndDlg, WM_SIZE, 0, 0);
		break;

	case DM_GETWINDOWSTATE:
		{
			UINT state = 0;
			state |= MSG_WINDOW_STATE_EXISTS;
			if (IsWindowVisible(hwndDlg))
				state |= MSG_WINDOW_STATE_VISIBLE;
			if (GetForegroundWindow() == hwndDlg)
				state |= MSG_WINDOW_STATE_FOCUS;
			if (IsIconic(hwndDlg))
				state |= MSG_WINDOW_STATE_ICONIC;
			SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, state);
		}
		return TRUE;

	case DM_CASCADENEWWINDOW:
		if ((HWND)wParam != hwndDlg) {
			RECT rcThis, rcNew;
			GetWindowRect(hwndDlg, &rcThis);
			GetWindowRect((HWND)wParam, &rcNew);
			if (abs(rcThis.left - rcNew.left) < 3 && abs(rcThis.top - rcNew.top) < 3) {
				int offset = GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFRAME);
				SetWindowPos((HWND)wParam, 0, rcNew.left + offset, rcNew.top + offset, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
				*(int *)lParam = 1;
			}
		}
		break;

	case WM_ACTIVATE:
		if (LOWORD(wParam) != WA_ACTIVE)
			break;

		SetFocus(GetDlgItem(hwndDlg, IDC_MESSAGE));
		// fall through
	case WM_MOUSEACTIVATE:
		if (KillTimer(hwndDlg, TIMERID_FLASHWND))
			FlashWindow(hwndDlg, FALSE);
		break;

	case WM_GETMINMAXINFO:
		{
			MINMAXINFO* mmi = (MINMAXINFO *)lParam;
			RECT rcWindow, rcLog;
			GetWindowRect(hwndDlg, &rcWindow);
			GetWindowRect(GetDlgItem(hwndDlg, IDC_LOG), &rcLog);
			mmi->ptMinTrackSize.x = rcWindow.right - rcWindow.left - ((rcLog.right - rcLog.left) - dat->minEditBoxSize.cx);
			mmi->ptMinTrackSize.y = rcWindow.bottom - rcWindow.top - ((rcLog.bottom - rcLog.top) - dat->minEditBoxSize.cy);
		}
		return 0;

	case WM_SIZE:
		if (!IsIconic(hwndDlg)) {
			BOOL bottomScroll = TRUE;

			if (dat->hwndStatus) {
				SendMessage(dat->hwndStatus, WM_SIZE, 0, 0);
				SetupStatusBar(hwndDlg, dat);
			}

			if (GetWindowLongPtr(GetDlgItem(hwndDlg, IDC_LOG), GWL_STYLE) & WS_VSCROLL) {
				SCROLLINFO si = { 0 };
				si.cbSize = sizeof(si);
				si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
				GetScrollInfo(GetDlgItem(hwndDlg, IDC_LOG), SB_VERT, &si);
				bottomScroll = (si.nPos + (int)si.nPage + 5) >= si.nMax;
			}

			UTILRESIZEDIALOG urd = { sizeof(urd) };
			urd.hInstance = g_hInst;
			urd.hwndDlg = hwndDlg;
			urd.lParam = (LPARAM)dat;
			urd.lpTemplate = MAKEINTRESOURCEA(IDD_MSG);
			urd.pfnResizer = MessageDialogResize;
			CallService(MS_UTILS_RESIZEDIALOG, 0, (LPARAM)& urd);

			// The statusbar sometimes draws over these 2 controls so
			// redraw them
			if (dat->hwndStatus) {
				RedrawWindow(GetDlgItem(hwndDlg, IDOK), NULL, NULL, RDW_INVALIDATE);
				RedrawWindow(GetDlgItem(hwndDlg, IDC_MESSAGE), NULL, NULL, RDW_INVALIDATE);
			}
			if ((g_dat.flags & SMF_AVATAR) && dat->avatarPic)
				RedrawWindow(GetDlgItem(hwndDlg, IDC_AVATAR), NULL, NULL, RDW_INVALIDATE);

			if (bottomScroll)
				PostMessage(hwndDlg, DM_SCROLLLOGTOBOTTOM, 0, 0);
		}
		break;

	case DM_SPLITTERMOVED:
		if ((HWND)lParam == GetDlgItem(hwndDlg, IDC_SPLITTER)) {
			HWND hwndLog = GetDlgItem(hwndDlg, IDC_LOG);

			RECT rc, rcLog;
			GetClientRect(hwndDlg, &rc);
			GetWindowRect(hwndLog, &rcLog);

			POINT pt;
			pt.x = 0;
			pt.y = wParam;
			ScreenToClient(hwndDlg, &pt);

			int oldSplitterY = dat->splitterPos;
			dat->splitterPos = rc.bottom - pt.y + 23;
			GetWindowRect(GetDlgItem(hwndDlg, IDC_MESSAGE), &rc);
			if (rc.bottom - rc.top + (dat->splitterPos - oldSplitterY) < dat->minEditBoxSize.cy)
				dat->splitterPos = oldSplitterY + dat->minEditBoxSize.cy - (rc.bottom - rc.top);
			if (rcLog.bottom - rcLog.top - (dat->splitterPos - oldSplitterY) < dat->minEditBoxSize.cy)
				dat->splitterPos = oldSplitterY - dat->minEditBoxSize.cy + (rcLog.bottom - rcLog.top);

			SendMessage(hwndDlg, WM_SIZE, 0, 0);
		}
		break;

	case DM_REMAKELOG:
		StreamInEvents(hwndDlg, dat->hDbEventFirst, -1, 0);
		break;

	case DM_APPENDTOLOG:   //takes wParam=hDbEvent
		StreamInEvents(hwndDlg, wParam, 1, 1);
		break;

	case DM_SCROLLLOGTOBOTTOM:
		{
			HWND hwndLog = GetDlgItem(hwndDlg, IDC_LOG);
			if (GetWindowLongPtr(hwndLog, GWL_STYLE) & WS_VSCROLL) {
				SCROLLINFO si = { 0 };
				si.cbSize = sizeof(si);
				si.fMask = SIF_PAGE | SIF_RANGE;
				GetScrollInfo(hwndLog, SB_VERT, &si);
				si.fMask = SIF_POS;
				si.nPos = si.nMax - si.nPage;
				SetScrollInfo(hwndLog, SB_VERT, &si, TRUE);
				SendMessage(hwndLog, WM_VSCROLL, MAKEWPARAM(SB_BOTTOM, 0), 0);
			}
		}
		break;

	case HM_DBEVENTADDED:
		if (wParam == dat->hContact) {
			MEVENT hDbEvent = lParam;
			if (dat->hDbEventFirst == NULL)
				dat->hDbEventFirst = hDbEvent;

			DBEVENTINFO dbei = { sizeof(dbei) };
			db_event_get(hDbEvent, &dbei);
			bool isMessage = (dbei.eventType == EVENTTYPE_MESSAGE), isSent = ((dbei.flags & DBEF_SENT) != 0);
			if (DbEventIsShown(&dbei)) {
				// Sounds *only* for sent messages, not for custom events
				if (isMessage && !isSent) {
					if (GetForegroundWindow() == hwndDlg)
						SkinPlaySound("RecvMsgActive");
					else
						SkinPlaySound("RecvMsgInactive");
				}
				if (isMessage && !isSent) {
					dat->lastMessage = dbei.timestamp;
					SendMessage(hwndDlg, DM_UPDATELASTMESSAGE, 0, 0);
				}
				if (hDbEvent != dat->hDbEventFirst && db_event_next(dat->hContact, hDbEvent) == NULL)
					SendMessage(hwndDlg, DM_APPENDTOLOG, lParam, 0);
				else
					SendMessage(hwndDlg, DM_REMAKELOG, 0, 0);

				// Flash window *only* for messages, not for custom events
				if (isMessage && !isSent) {
					if (GetActiveWindow() == hwndDlg && GetForegroundWindow() == hwndDlg) {
						HWND hwndLog = GetDlgItem(hwndDlg, IDC_LOG);
						if (GetWindowLongPtr(hwndLog, GWL_STYLE) & WS_VSCROLL) {
							SCROLLINFO si = { 0 };
							si.cbSize = sizeof(si);
							si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
							GetScrollInfo(hwndLog, SB_VERT, &si);
							if ((si.nPos + (int)si.nPage + 5) < si.nMax)
								SetTimer(hwndDlg, TIMERID_FLASHWND, TIMEOUT_FLASHWND, NULL);
						}
					}
					else SetTimer(hwndDlg, TIMERID_FLASHWND, TIMEOUT_FLASHWND, NULL);
				}
			}
		}
		break;

	case WM_TIMECHANGE:
		PostMessage(hwndDlg, DM_NEWTIMEZONE, 0, 0);
		PostMessage(hwndDlg, DM_REMAKELOG, 0, 0);
		break;

	case WM_TIMER:
		if (wParam == TIMERID_FLASHWND) {
			FlashWindow(hwndDlg, TRUE);
			if (dat->nFlash > 2 * g_dat.nFlashMax) {
				KillTimer(hwndDlg, TIMERID_FLASHWND);
				FlashWindow(hwndDlg, FALSE);
				dat->nFlash = 0;
			}
			dat->nFlash++;
		}
		else if (wParam == TIMERID_TYPE) {
			ShowTime(dat);
			if (dat->nTypeMode == PROTOTYPE_SELFTYPING_ON && GetTickCount() - dat->nLastTyping > TIMEOUT_TYPEOFF)
				NotifyTyping(dat, PROTOTYPE_SELFTYPING_OFF);

			if (dat->showTyping) {
				if (dat->nTypeSecs) {
					dat->nTypeSecs--;
					if (GetForegroundWindow() == hwndDlg)
						SendMessage(hwndDlg, DM_UPDATEWINICON, 0, 0);
				}
				else {
					SendMessage(hwndDlg, DM_UPDATELASTMESSAGE, 0, 0);
					if (g_dat.flags & SMF_SHOWTYPINGWIN)
						SendMessage(hwndDlg, DM_UPDATEWINICON, 0, 0);
					dat->showTyping = 0;
				}
			}
			else {
				if (dat->nTypeSecs) {
					TCHAR szBuf[256];
					TCHAR* szContactName = pcli->pfnGetContactDisplayName(dat->hContact, 0);
					HICON hTyping = LoadSkinnedIcon(SKINICON_OTHER_TYPING);

					mir_sntprintf(szBuf, SIZEOF(szBuf), TranslateT("%s is typing a message..."), szContactName);
					dat->nTypeSecs--;

					SendMessage(dat->hwndStatus, SB_SETTEXT, 0, (LPARAM)szBuf);
					SendMessage(dat->hwndStatus, SB_SETICON, 0, (LPARAM)hTyping);
					if ((g_dat.flags & SMF_SHOWTYPINGWIN) && GetForegroundWindow() != hwndDlg) {
						HICON hIcon = (HICON)SendMessage(hwndDlg, WM_GETICON, ICON_SMALL, 0);
						SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)hTyping);
						Skin_ReleaseIcon(hIcon);
					}
					dat->showTyping = 1;
				}
			}
		}
		break;

	case WM_MEASUREITEM:
		{
			LPMEASUREITEMSTRUCT mis = (LPMEASUREITEMSTRUCT)lParam;
			if (mis->CtlType == ODT_MENU)
				return CallService(MS_CLIST_MENUMEASUREITEM, wParam, lParam);
		}
		break;

	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
			if (dis->CtlType == ODT_MENU)
				return CallService(MS_CLIST_MENUDRAWITEM, wParam, lParam);
			if (dis->hwndItem == dat->hwndStatus) {
				DrawStatusIcons(dat->hContact, dis->hDC, dis->rcItem, 2);
				return TRUE;
			}
			if (dis->CtlID == IDC_PROTOCOL) {
				if (dat->szProto) {
					int dwStatus = db_get_w(dat->hContact, dat->szProto, "Status", ID_STATUS_OFFLINE);
					HICON hIcon = LoadSkinnedProtoIcon(dat->szProto, dwStatus);
					if (hIcon) {
						if (db_get_dw(dat->hContact, dat->szProto, "IdleTS", 0)) {
							HIMAGELIST hImageList = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 | ILC_MASK, 1, 0);
							ImageList_AddIcon(hImageList, hIcon);
							ImageList_DrawEx(hImageList, 0, dis->hDC, dis->rcItem.left, dis->rcItem.top, 0, 0, CLR_NONE, CLR_NONE, ILD_SELECTED);
							ImageList_Destroy(hImageList);
						}
						else DrawIconEx(dis->hDC, dis->rcItem.left, dis->rcItem.top, hIcon, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0, NULL, DI_NORMAL);

						Skin_ReleaseIcon(hIcon);
						return TRUE;
					}
				}
			}
			else if (dis->CtlID == IDC_AVATAR && dat->avatarPic && (g_dat.flags & SMF_AVATAR)) {
				HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
				HPEN hOldPen = (HPEN)SelectObject(dis->hDC, hPen);
				Rectangle(dis->hDC, 0, 0, dat->avatarWidth, dat->avatarHeight);
				SelectObject(dis->hDC, hOldPen);
				DeleteObject(hPen);

				BITMAP bminfo;
				GetObject(dat->avatarPic, sizeof(bminfo), &bminfo);

				HDC hdcMem = CreateCompatibleDC(dis->hDC);
				HBITMAP hbmMem = (HBITMAP)SelectObject(hdcMem, dat->avatarPic);

				SetStretchBltMode(dis->hDC, HALFTONE);
				StretchBlt(dis->hDC, 1, 1, dat->avatarWidth - 2, dat->avatarHeight - 2, hdcMem, 0, 0,
							  bminfo.bmWidth, bminfo.bmHeight, SRCCOPY);

				SelectObject(hdcMem, hbmMem);
				DeleteDC(hdcMem);
				return TRUE;
			}
		}
		break;

	case WM_COMMAND:
		if (!lParam && CallService(MS_CLIST_MENUPROCESSCOMMAND, MAKEWPARAM(LOWORD(wParam), MPCF_CONTACTMENU), (LPARAM)dat->hContact))
			break;

		switch (LOWORD(wParam)) {
		case IDOK:
			if (IsWindowEnabled(GetDlgItem(hwndDlg, IDOK))) {
				int bufSize = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_MESSAGE)) + 1;
				TCHAR *temp = (TCHAR*)alloca(bufSize * sizeof(TCHAR));
				GetDlgItemText(hwndDlg, IDC_MESSAGE, temp, bufSize);
				if (!temp[0])
					break;

				int sendId = SendMessageDirect(rtrimt(temp), dat->hContact, dat->szProto);
				if (sendId) {
					dat->cmdList.insert(mir_tstrdup(temp));

					dat->cmdListInd = -1;
					if (dat->nTypeMode == PROTOTYPE_SELFTYPING_ON)
						NotifyTyping(dat, PROTOTYPE_SELFTYPING_OFF);

					EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);
					SetFocus(GetDlgItem(hwndDlg, IDC_MESSAGE));

					SetDlgItemText(hwndDlg, IDC_MESSAGE, _T(""));

					if (g_dat.flags & SMF_AUTOCLOSE)
						DestroyWindow(hwndDlg);
					else if (g_dat.flags & SMF_AUTOMIN)
						ShowWindow(hwndDlg, SW_MINIMIZE);
				}
				return TRUE;
			}
			break;

		case IDCANCEL:
			DestroyWindow(hwndDlg);
			return TRUE;

		case IDC_USERMENU:
		case IDC_NAME:
			if (GetKeyState(VK_SHIFT) & 0x8000)    // copy user name
				SendMessage(hwndDlg, DM_USERNAMETOCLIP, 0, 0);
			else {
				RECT rc;
				HMENU hMenu = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT, (WPARAM)dat->hContact, 0);
				GetWindowRect(GetDlgItem(hwndDlg, LOWORD(wParam)), &rc);
				TrackPopupMenu(hMenu, 0, rc.left, rc.bottom, 0, hwndDlg, NULL);
				DestroyMenu(hMenu);
			}
			break;

		case IDC_HISTORY:
			CallService(MS_HISTORY_SHOWCONTACTHISTORY, (WPARAM)dat->hContact, 0);
			break;

		case IDC_DETAILS:
			CallService(MS_USERINFO_SHOWDIALOG, (WPARAM)dat->hContact, 0);
			break;

		case IDC_ADD:
			{
				ADDCONTACTSTRUCT acs = { 0 };
				acs.hContact = dat->hContact;
				acs.handleType = HANDLE_CONTACT;
				acs.szProto = 0;
				CallService(MS_ADDCONTACT_SHOW, (WPARAM)hwndDlg, (LPARAM)& acs);
			}
			if (!db_get_b(dat->hContact, "CList", "NotOnList", 0))
				ShowWindow(GetDlgItem(hwndDlg, IDC_ADD), FALSE);
			break;

		case IDC_MESSAGE:
			if (HIWORD(wParam) == EN_CHANGE) {
				int len = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_MESSAGE));
				UpdateReadChars(hwndDlg, dat->hwndStatus);
				EnableWindow(GetDlgItem(hwndDlg, IDOK), len != 0);
				if (!(GetKeyState(VK_CONTROL) & 0x8000) && !(GetKeyState(VK_SHIFT) & 0x8000)) {
					dat->nLastTyping = GetTickCount();
					if (len) {
						if (dat->nTypeMode == PROTOTYPE_SELFTYPING_OFF)
							NotifyTyping(dat, PROTOTYPE_SELFTYPING_ON);
					}
					else if (dat->nTypeMode == PROTOTYPE_SELFTYPING_ON)
						NotifyTyping(dat, PROTOTYPE_SELFTYPING_OFF);
				}
			}
			break;
		}
		break;

	case WM_NOTIFY:
		if (dat && ((LPNMHDR)lParam)->hwndFrom == dat->hwndStatus) {
			if (((LPNMHDR)lParam)->code == NM_CLICK || ((LPNMHDR)lParam)->code == NM_RCLICK) {
				NMMOUSE *nm = (NMMOUSE *)lParam;
				RECT rc;

				SendMessage(dat->hwndStatus, SB_GETRECT, SendMessage(dat->hwndStatus, SB_GETPARTS, 0, 0) - 1, (LPARAM)&rc);
				if (nm->pt.x >= rc.left)
					CheckStatusIconClick(dat->hContact, dat->hwndStatus, nm->pt, rc, 2, ((LPNMHDR)lParam)->code == NM_RCLICK ? MBCF_RIGHTBUTTON : 0);
				return TRUE;
			}
		}

		switch (((LPNMHDR)lParam)->idFrom) {
		case IDC_LOG:
			switch (((LPNMHDR)lParam)->code) {
			case EN_MSGFILTER:
				switch (((MSGFILTER *)lParam)->msg) {
				HCURSOR hCur;
				case WM_LBUTTONDOWN:
					hCur = GetCursor();
					if (hCur == LoadCursor(NULL, IDC_SIZENS) || hCur == LoadCursor(NULL, IDC_SIZEWE) || hCur == LoadCursor(NULL, IDC_SIZENESW) || hCur == LoadCursor(NULL, IDC_SIZENWSE)) {
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
						return TRUE;
					}
					break;

				case WM_MOUSEMOVE:
					hCur = GetCursor();
					if (hCur == LoadCursor(NULL, IDC_SIZENS) || hCur == LoadCursor(NULL, IDC_SIZEWE) || hCur == LoadCursor(NULL, IDC_SIZENESW) || hCur == LoadCursor(NULL, IDC_SIZENWSE))
						SetCursor(LoadCursor(NULL, IDC_ARROW));
					break;

				case WM_RBUTTONUP:
					POINT pt;
					CHARRANGE sel, all = { 0, -1 };
					HMENU hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_CONTEXT));
					HMENU hSubMenu = GetSubMenu(hMenu, 0);
					TranslateMenu(hSubMenu);
					SendMessage(((NMHDR *)lParam)->hwndFrom, EM_EXGETSEL, 0, (LPARAM)& sel);
					if (sel.cpMin == sel.cpMax)
						EnableMenuItem(hSubMenu, IDM_COPY, MF_BYCOMMAND | MF_GRAYED);
					pt.x = (short)LOWORD(((ENLINK *)lParam)->lParam);
					pt.y = (short)HIWORD(((ENLINK *)lParam)->lParam);
					ClientToScreen(((NMHDR *)lParam)->hwndFrom, &pt);

					switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwndDlg, NULL)) {
					case IDM_COPY:
						SendMessage(((NMHDR *)lParam)->hwndFrom, WM_COPY, 0, 0);
						break;
					case IDM_COPYALL:
						SendMessage(((NMHDR *)lParam)->hwndFrom, EM_EXSETSEL, 0, (LPARAM)& all);
						SendMessage(((NMHDR *)lParam)->hwndFrom, WM_COPY, 0, 0);
						SendMessage(((NMHDR *)lParam)->hwndFrom, EM_EXSETSEL, 0, (LPARAM)& sel);
						break;
					case IDM_SELECTALL:
						SendMessage(((NMHDR *)lParam)->hwndFrom, EM_EXSETSEL, 0, (LPARAM)& all);
						break;
					case IDM_CLEAR:
						SetDlgItemText(hwndDlg, IDC_LOG, _T(""));
						dat->hDbEventFirst = NULL;
						break;
					}
					DestroyMenu(hSubMenu);
					DestroyMenu(hMenu);
					SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
					return TRUE;
				}
				break;

			case EN_VSCROLL:
				if (LOWORD(wParam) == IDC_LOG && GetWindowLongPtr((HWND)lParam, GWL_STYLE) & WS_VSCROLL) {
					SCROLLINFO si = { 0 };
					si.cbSize = sizeof(si);
					si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
					GetScrollInfo((HWND)lParam, SB_VERT, &si);
					if ((si.nPos + (int)si.nPage + 5) >= si.nMax)
						if (KillTimer(hwndDlg, TIMERID_FLASHWND))
							FlashWindow(hwndDlg, FALSE);
				}
				break;

			case EN_LINK:
				switch (((ENLINK *)lParam)->msg) {
				case WM_SETCURSOR:
					SetCursor(hCurHyperlinkHand);
					SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
					return TRUE;

				case WM_RBUTTONDOWN:
				case WM_LBUTTONUP:
					CHARRANGE sel;
					SendDlgItemMessage(hwndDlg, IDC_LOG, EM_EXGETSEL, 0, (LPARAM)& sel);
					if (sel.cpMin != sel.cpMax)
						break;

					TEXTRANGE tr;
					tr.chrg = ((ENLINK *)lParam)->chrg;
					tr.lpstrText = (TCHAR*)_alloca((tr.chrg.cpMax - tr.chrg.cpMin + 8) * sizeof(TCHAR));
					SendDlgItemMessage(hwndDlg, IDC_LOG, EM_GETTEXTRANGE, 0, (LPARAM)& tr);
					if (_tcschr(tr.lpstrText, '@') != NULL && _tcschr(tr.lpstrText, ':') == NULL && _tcschr(tr.lpstrText, '/') == NULL) {
						memmove(tr.lpstrText + 7, tr.lpstrText, (tr.chrg.cpMax - tr.chrg.cpMin + 1) * sizeof(TCHAR));
						memcpy(tr.lpstrText, _T("mailto:"), 7 * sizeof(TCHAR));
					}

					if (((ENLINK *)lParam)->msg == WM_RBUTTONDOWN) {
						POINT pt;
						HMENU hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDR_CONTEXT));
						HMENU hSubMenu = GetSubMenu(hMenu, 1);
						TranslateMenu(hSubMenu);
						pt.x = (short)LOWORD(((ENLINK *)lParam)->lParam);
						pt.y = (short)HIWORD(((ENLINK *)lParam)->lParam);
						ClientToScreen(((NMHDR *)lParam)->hwndFrom, &pt);

						switch (TrackPopupMenu(hSubMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwndDlg, NULL)) {
						case IDM_OPENLINK:
							ShellExecute(NULL, _T("open"), tr.lpstrText, NULL, NULL, SW_SHOW);
							break;

						case IDM_COPYLINK:
							if (OpenClipboard(hwndDlg)) {
								EmptyClipboard();
								HGLOBAL hData = GlobalAlloc(GMEM_MOVEABLE, (_tcslen(tr.lpstrText) + 1) * sizeof(TCHAR));
								_tcscpy((TCHAR*)GlobalLock(hData), tr.lpstrText);
								GlobalUnlock(hData);
								SetClipboardData(CF_UNICODETEXT, hData);
								CloseClipboard();
							}
							break;
						}

						DestroyMenu(hMenu);
						SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, TRUE);
						return TRUE;
					}

					ShellExecute(NULL, _T("open"), tr.lpstrText, NULL, NULL, SW_SHOW);
					SetFocus(GetDlgItem(hwndDlg, IDC_MESSAGE));
					break;
				}
			}
		}
		break;

	case DM_STATUSICONCHANGE:
		SendMessage(dat->hwndStatus, SB_SETTEXT, (WPARAM)(SBT_OWNERDRAW | (SendMessage(dat->hwndStatus, SB_GETPARTS, 0, 0) - 1)), 0);
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		if (dat) {
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
			NotifyLocalWinEvent(dat->hContact, hwndDlg, MSG_WINDOW_EVT_CLOSING);

			bool bSavePerContact = db_get_b(NULL, SRMMMOD, SRMSGSET_SAVEPERCONTACT, SRMSGDEFSET_SAVEPERCONTACT) != 0;

			// save string from the editor
			if (dat->hContact) {
				int len = GetWindowTextLength(GetDlgItem(hwndDlg, IDC_MESSAGE)) + 1;
				TCHAR *msg = (TCHAR*)alloca(sizeof(TCHAR)* len);
				GetDlgItemText(hwndDlg, IDC_MESSAGE, msg, len);
				if (msg[0])
					db_set_ts(dat->hContact, SRMSGMOD, DBSAVEDMSG, msg);
				else
					db_unset(dat->hContact, SRMSGMOD, DBSAVEDMSG);
			}
			KillTimer(hwndDlg, TIMERID_TYPE);
			if (dat->nTypeMode == PROTOTYPE_SELFTYPING_ON)
				NotifyTyping(dat, PROTOTYPE_SELFTYPING_OFF);

			if (dat->hBkgBrush)
				DeleteObject(dat->hBkgBrush);
			if (dat->hwndStatus)
				DestroyWindow(dat->hwndStatus);

			for (int i = 0; i < dat->cmdList.getCount(); i++)
				mir_free(dat->cmdList[i]);
			dat->cmdList.destroy();

			WindowList_Remove(g_dat.hMessageWindowList, hwndDlg);
			db_set_dw(bSavePerContact ? dat->hContact : NULL, SRMMMOD, "splitterPos", dat->splitterPos);

			if (dat->hFont)	{
				DeleteObject(dat->hFont);
				dat->hFont = NULL;
			}

			MCONTACT hContact = (bSavePerContact) ? dat->hContact : NULL;

			WINDOWPLACEMENT wp = { sizeof(wp) };
			GetWindowPlacement(hwndDlg, &wp);
			if (!dat->windowWasCascaded) {
				db_set_dw(hContact, SRMMMOD, "x", wp.rcNormalPosition.left);
				db_set_dw(hContact, SRMMMOD, "y", wp.rcNormalPosition.top);
			}
			db_set_dw(hContact, SRMMMOD, "width", wp.rcNormalPosition.right - wp.rcNormalPosition.left);
			db_set_dw(hContact, SRMMMOD, "height", wp.rcNormalPosition.bottom - wp.rcNormalPosition.top);

			NotifyLocalWinEvent(dat->hContact, hwndDlg, MSG_WINDOW_EVT_CLOSE);
			if (dat->hContact && db_get_b(NULL, SRMMMOD, SRMSGSET_DELTEMP, SRMSGDEFSET_DELTEMP))
				if (db_get_b(dat->hContact, "CList", "NotOnList", 0))
					CallService(MS_DB_CONTACT_DELETE, (WPARAM)dat->hContact, 0);

			Button_FreeIcon_IcoLib(hwndDlg, IDC_ADD);
			Button_FreeIcon_IcoLib(hwndDlg, IDC_DETAILS);
			Button_FreeIcon_IcoLib(hwndDlg, IDC_HISTORY);
			Button_FreeIcon_IcoLib(hwndDlg, IDC_USERMENU);
			Window_FreeIcon_IcoLib(hwndDlg);
			delete dat;
		}
		break;
	}
	return FALSE;
}
