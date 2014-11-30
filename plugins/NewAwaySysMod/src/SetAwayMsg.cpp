/*
	New Away System plugin for Miranda IM
	Copyright (c) 2005-2007 Chervov Dmitry
	Copyright (c) 2004-2005 Iksaif Entertainment
	Copyright (c) 2002-2003 Goblineye Entertainment

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


#include "Common.h"
#include "m_button.h"
#include "m_icolib.h"
#include "ContactList.h"
#include "MsgTree.h"
#include "Properties.h"
#include "m_NewAwaySys.h"

#define EXTRAIMGLIST_DOT 0
#define EXTRAIMGLIST_IGNORE 1
#define EXTRAIMGLIST_MSG 2
#define EXTRAIMGLIST_AUTOREPLY_ON 3
#define EXTRAIMGLIST_AUTOREPLY_OFF 4

#define EXTRACOLUMN_IGNORE 0
#define EXTRACOLUMN_REPLY 1

#define MINSPLITTERPOS 176
#define MINYDLGSIZE 100

#define SAM_TIMER_ID 1

#define UM_SAM_INITCLIST (WM_USER + 100)

#define CLSEL_DAT_CONTACT ((PTREEITEMARRAY)(-1)) // these constants are used to specify selection when the contact list is not loaded
#define CLSEL_DAT_NOTHING ((PTREEITEMARRAY)(-2))

static WNDPROC g_OrigEditMsgProc;
static WNDPROC g_OrigSplitterProc;
static WNDPROC g_OrigCListProc;
static int g_VariablesButtonDX;
static int g_MsgSplitterX, g_ContactSplitterX;
static int MinMsgSplitterX, MinContactSplitterX;
static int MinMsgEditSize; // used to calculate minimal X size of the dialog

static LRESULT CALLBACK MsgEditSubclassProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg) {
	case WM_CHAR:
		if (GetKeyState(VK_CONTROL) & 0x8000) {
			if (wParam == '\n') { // ctrl-enter
				PostMessage(GetParent(hWnd), WM_COMMAND, IDC_OK, 0);
				return 0;
			}
			if (wParam == 1) { // ctrl-a
				SendMessage(hWnd, EM_SETSEL, 0, -1);
				return 0;
			}
			if (wParam == 23) { // ctrl-w
				SendMessage(GetParent(hWnd), WM_CLOSE, 0, 0);
				return 0;
			}
			if (wParam == 127) { // ctrl-backspace
				DWORD start, end;
				SendMessage(hWnd, EM_GETSEL, (WPARAM)&end, NULL);
				SendMessage(hWnd, WM_KEYDOWN, VK_LEFT, 0);
				SendMessage(hWnd, EM_GETSEL, (WPARAM)&start, NULL);
				int nLen = GetWindowTextLength(hWnd);
				TCHAR *text = (TCHAR*)malloc((nLen + 1) * sizeof(TCHAR));
				GetWindowText(hWnd, text, nLen + 1);
				MoveMemory(text + start, text + end, sizeof(TCHAR)* (_tcslen(text) + 1 - end));
				SetWindowText(hWnd, text);
				free(text);
				SendMessage(hWnd, EM_SETSEL, start, start);
				SendMessage(GetParent(hWnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hWnd), EN_CHANGE), (LPARAM)hWnd);
				return 0;
			}
		}
		break;
	
	case WM_KEYDOWN:
		SendMessage(GetParent(hWnd), UM_SAM_KILLTIMER, 0, 0);
		break;
	}
	return CallWindowProc(g_OrigEditMsgProc, hWnd, Msg, wParam, lParam);
}

// used splitter code from TabSRMM as a base
static LRESULT CALLBACK SplitterSubclassProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg) {
	case WM_NCHITTEST:
		return HTCLIENT;

	case WM_SETCURSOR:
		SetCursor(LoadCursor(NULL, IDC_SIZEWE));
		return true;

	case WM_LBUTTONDOWN:
		SetCapture(hWnd);
		return false;

	case WM_MOUSEMOVE:
		if (GetCapture() == hWnd) {
			RECT rc;
			GetClientRect(hWnd, &rc);
			SendMessage(GetParent(hWnd), UM_SAM_SPLITTERMOVED, (short)LOWORD(GetMessagePos()) + rc.right / 2, (LPARAM)GetDlgCtrlID(hWnd));
		}
		return 0;

	case WM_LBUTTONUP:
		ReleaseCapture();
		return false;
	}
	return CallWindowProc(g_OrigSplitterProc, hWnd, Msg, wParam, lParam);
}

static LRESULT CALLBACK CListSubclassProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	CCList *dat = CWndUserData(GetParent(hWnd)).GetCList();
	switch (Msg) {
	case WM_MOUSEMOVE:
		{
			DWORD hitFlags;
			POINT pt = { (short)LOWORD(lParam), (short)HIWORD(lParam) };
			if (dat->HitTest(&pt, &hitFlags) && hitFlags & MCLCHT_ONITEMEXTRA)
				lParam = 0; // reset mouse coordinates, so TreeView's wndproc will not draw any item in a hovered state
		}
		break;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
		DWORD hitFlags;
		POINT pt = { (short)LOWORD(lParam), (short)HIWORD(lParam) };
		if (dat->HitTest(&pt, &hitFlags) && hitFlags & MCLCHT_ONITEMEXTRA) {
			SetFocus(hWnd);
			NMHDR nmhdr;
			nmhdr.code = NM_CLICK;
			nmhdr.hwndFrom = hWnd;
			nmhdr.idFrom = GetDlgCtrlID(hWnd);
			SendMessage(GetParent(hWnd), WM_NOTIFY, 0, (LPARAM)&nmhdr);
			return DefWindowProc(hWnd, Msg, wParam, lParam);
		}
	}
	return CallWindowProc(g_OrigCListProc, hWnd, Msg, wParam, lParam);
}

static int SetAwayMsgDlgResize(HWND hwndDlg, LPARAM lParam, UTILRESIZECONTROL *urc)
{
	COptPage *SetAwayMsgPage = (COptPage*)lParam;
	int bShowMsgTree = SetAwayMsgPage->GetValue(IDS_SAWAYMSG_SHOWMSGTREE), X;
	int bShowContactTree = SetAwayMsgPage->GetValue(IDS_SAWAYMSG_SHOWCONTACTTREE);
	switch (urc->wId) {
	case IDC_SAWAYMSG_MSGDATA:
		urc->rcItem.right = urc->dlgOriginalSize.cx - 2;
		if (bShowContactTree)
			urc->rcItem.right -= g_ContactSplitterX;
		urc->rcItem.left = (bShowMsgTree) ? (g_MsgSplitterX + 2) : 2;
		return RD_ANCHORX_WIDTH | RD_ANCHORY_HEIGHT;

	case IDC_SAWAYMSG_TREE:
		urc->rcItem.right = g_MsgSplitterX - 2;
		return RD_ANCHORX_LEFT | RD_ANCHORY_HEIGHT;

	case IDC_SAWAYMSG_CONTACTSTREE:
		urc->rcItem.left = urc->dlgOriginalSize.cx - g_ContactSplitterX + 2;
		return RD_ANCHORX_RIGHT | RD_ANCHORY_HEIGHT;

	case IDC_OK:
		X = bShowMsgTree ? min(max(g_MsgSplitterX, MinMsgSplitterX), urc->dlgNewSize.cx - ((bShowContactTree ? MinContactSplitterX : 0) + MinMsgEditSize)) : 0;
		OffsetRect(&urc->rcItem, X + 2 - urc->rcItem.left, 0);
		return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM;

	case IDC_SAWAYMSG_EDITMSGS:
	case IDC_SAWAYMSG_SAVEMSG:
	case IDC_SAWAYMSG_SAVEASNEW:
	case IDC_SAWAYMSG_NEWCATEGORY:
	case IDC_SAWAYMSG_DELETE:
		return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM;

	case IDC_SAWAYMSG_VARS:
	case IDC_SAWAYMSG_OPTIONS:
		X = bShowContactTree ? max(min(g_ContactSplitterX, urc->dlgNewSize.cx - (bShowMsgTree ? max(g_MsgSplitterX, MinMsgSplitterX) : 0) - MinMsgEditSize), MinContactSplitterX) : 0;
		OffsetRect(&urc->rcItem, urc->dlgOriginalSize.cx - X - 2 - urc->rcItem.right - ((urc->wId == IDC_SAWAYMSG_VARS) ? g_VariablesButtonDX : 0), 0);
		return RD_ANCHORX_RIGHT | RD_ANCHORY_BOTTOM;

	case IDC_SAWAYMSG_STATIC_IGNOREICON:
	case IDC_SAWAYMSG_STATIC_REPLYICON:
		urc->rcItem.left = urc->rcItem.right - 16;
		urc->rcItem.top = urc->rcItem.bottom - 16;
		return RD_ANCHORX_RIGHT | RD_ANCHORY_BOTTOM;

	case IDC_SAWAYMSG_IGNOREREQ:
	case IDC_SAWAYMSG_SENDMSG:
		return RD_ANCHORX_RIGHT | RD_ANCHORY_BOTTOM;

	case IDC_SAWAYMSG_MSGSPLITTER:
		OffsetRect(&urc->rcItem, g_MsgSplitterX - (urc->rcItem.left + ++urc->rcItem.right) / 2, 0);
		return RD_ANCHORX_LEFT | RD_ANCHORY_HEIGHT;

	case IDC_SAWAYMSG_CONTACTSPLITTER:
		OffsetRect(&urc->rcItem, urc->dlgOriginalSize.cx - g_ContactSplitterX - (urc->rcItem.left + urc->rcItem.right) / 2, 0);
		return RD_ANCHORX_RIGHT | RD_ANCHORY_HEIGHT;
	}

	return RD_ANCHORX_LEFT | RD_ANCHORY_BOTTOM;
}

__inline int DBValueToReplyIcon(int Value)
{
	switch (Value) {
		case VAL_USEDEFAULT: return EXTRAIMGLIST_DOT;
		case 0: return EXTRAIMGLIST_AUTOREPLY_OFF;
		default: return EXTRAIMGLIST_AUTOREPLY_ON;
	}
}

int GetRealReplyIcon(CCList *CList, HTREEITEM hItem)
{
	_ASSERT(CList);
	MCONTACT hContact = CList->GethContact(hItem);
	int ItemType = CList->GetItemType(hItem);
	char *szProto = (char*)CList->GetItemParam(hItem);
	return (ItemType == MCLCIT_GROUP) ? CList->GetItemParam(hItem) : DBValueToReplyIcon((ItemType == MCLCIT_CONTACT) ? (int)CContactSettings(0, hContact).Autoreply : CProtoSettings(szProto).Autoreply);
}


void SetExtraIcon(CCList *CList, int nColumn, HTREEITEM hItem, int nIcon)
{
	_ASSERT(CList);
	int ItemType = CList->GetItemType(hItem);
	MCONTACT hContact = CList->GethContact(hItem);
	if (ItemType == MCLCIT_CONTACT) {
		if (nIcon == -1) { // means we need to retrieve it from the db by ourselves
			if (nColumn == EXTRACOLUMN_IGNORE)
				nIcon = CContactSettings(0, hContact).Ignore ? EXTRAIMGLIST_IGNORE : EXTRAIMGLIST_DOT;
			else {
				_ASSERT(nColumn == EXTRACOLUMN_REPLY);
				nIcon = DBValueToReplyIcon(CContactSettings(0, hContact).Autoreply);
			}
		}
		else { // save it back to the db
			if (nColumn == EXTRACOLUMN_IGNORE)
				CContactSettings(0, hContact).Ignore = nIcon == EXTRAIMGLIST_IGNORE;
			else {
				_ASSERT(nColumn == EXTRACOLUMN_REPLY);
				CContactSettings(0, hContact).Autoreply = (nIcon == EXTRAIMGLIST_DOT) ? VAL_USEDEFAULT : (nIcon == EXTRAIMGLIST_AUTOREPLY_ON);
			}
		}
		if (nColumn == EXTRACOLUMN_IGNORE && nIcon != EXTRAIMGLIST_IGNORE)
			nIcon = (CContactSettings(0, hContact).GetMsgFormat(GMF_PERSONAL) == NULL) ? EXTRAIMGLIST_DOT : EXTRAIMGLIST_MSG;
	}
	else if (ItemType == MCLCIT_INFO) {
		char *szProto = (char*)CList->GetItemParam(hItem);
		if (nColumn == EXTRACOLUMN_REPLY) {
			if (nIcon == -1)
				nIcon = DBValueToReplyIcon(CProtoSettings(szProto).Autoreply);
			else
				CProtoSettings(szProto).Autoreply = (nIcon == EXTRAIMGLIST_DOT) ? VAL_USEDEFAULT : (nIcon == EXTRAIMGLIST_AUTOREPLY_ON);

			if (!szProto && nIcon == EXTRAIMGLIST_DOT)
				nIcon = EXTRAIMGLIST_AUTOREPLY_OFF;
		}
		else nIcon = (CProtoSettings(szProto).GetMsgFormat(GMF_TEMPORARY | GMF_PERSONAL) == NULL) ? EXTRAIMGLIST_DOT : EXTRAIMGLIST_MSG;
	}
	
	int Ignore = (nColumn == EXTRACOLUMN_IGNORE) ? (nIcon == EXTRAIMGLIST_IGNORE) : ((ItemType == MCLCIT_CONTACT) ? CContactSettings(0, hContact).Ignore : ((ItemType == MCLCIT_GROUP) ? CList->GetExtraImage(hItem, EXTRACOLUMN_IGNORE) : false));
	if (Ignore) {
		if (nColumn == EXTRACOLUMN_IGNORE)
			CList->SetExtraImage(hItem, EXTRACOLUMN_REPLY, CLC_EXTRAICON_EMPTY);
		else
			nIcon = CLC_EXTRAICON_EMPTY;
	}
	else {
		int nReplyIcon;
		if (ItemType == MCLCIT_CONTACT)
			nReplyIcon = DBValueToReplyIcon(CContactSettings(0, hContact).Autoreply);
		else if (ItemType == MCLCIT_GROUP)
			nReplyIcon = GetRealReplyIcon(CList, hItem);
		else {
			_ASSERT(ItemType == MCLCIT_INFO);
			char *szProto = (char*)CList->GetItemParam(hItem);
			nReplyIcon = DBValueToReplyIcon(CProtoSettings(szProto).Autoreply);
		}
		
		if (nColumn == EXTRACOLUMN_IGNORE)
			CList->SetExtraImage(hItem, EXTRACOLUMN_REPLY, nReplyIcon);
		else if (nIcon == CLC_EXTRAICON_EMPTY)
			nIcon = nReplyIcon;
	}
	CList->SetExtraImage(hItem, nColumn, nIcon);
}


void SetCListGroupIcons(SetAwayMsgData *dat, CCList *CList)
{
	_ASSERT(CList);
	HTREEITEM hItem = CList->GetNextItem(MCLGN_LAST, NULL); // start from last item, so every item is processed before its parents
	if (!hItem)
		return;

	if (CList->GetItemType(hItem) != MCLCIT_GROUP) {
		hItem = CList->GetNextItem(MCLGN_PREV | MCLGN_GROUP | MCLGN_MULTILEVEL, hItem);
		if (!hItem)
			return;
	}
	
	do {
		HTREEITEM hCurItem = CList->GetNextItem(MCLGN_CHILD, hItem);
		if (hCurItem) {
			int IgnoreIcon = CList->GetExtraImage(hCurItem, EXTRACOLUMN_IGNORE);
			int AutoreplyIcon = GetRealReplyIcon(CList, hCurItem);
			if (IgnoreIcon == EXTRAIMGLIST_MSG)
				IgnoreIcon = EXTRAIMGLIST_DOT;
	
			while ((hCurItem = CList->GetNextItem(MCLGN_NEXT | MCLGN_ANY, hCurItem)) && (IgnoreIcon != EXTRAIMGLIST_DOT || AutoreplyIcon != EXTRAIMGLIST_DOT)) {
				if (CList->GetExtraImage(hCurItem, EXTRACOLUMN_IGNORE) != EXTRAIMGLIST_IGNORE)
					IgnoreIcon = EXTRAIMGLIST_DOT;

				int CurReplyIcon = GetRealReplyIcon(CList, hCurItem);
				if (CurReplyIcon != AutoreplyIcon)
					AutoreplyIcon = EXTRAIMGLIST_DOT;
			}
			CList->SetItemParam(hItem, AutoreplyIcon); // store Reply icon in item's Param, so that we can always get real reply icon for groups later, even if CLC_EXTRAICON_EMPTY is set instead of it
			SetExtraIcon(CList, EXTRACOLUMN_IGNORE, hItem, IgnoreIcon);
			SetExtraIcon(CList, EXTRACOLUMN_REPLY, hItem, AutoreplyIcon);
		}
	}
		while (hItem = CList->GetNextItem(MCLGN_PREV | MCLGN_GROUP | MCLGN_MULTILEVEL, hItem));
}


int GetSelContactsNum(CCList *CList, PTREEITEMARRAY Selection = NULL, bool *bOnlyInfo = NULL) // "SelContacts" mean not only contacts, but everything with "personal" status messages and settings - i.e. "All contacts" and protocol items are counted too.
{
	if (!CList)
		return (Selection == CLSEL_DAT_NOTHING) ? 0 : 1; // Selection == NULL means we need to retrieve current selection by ourselves, and current selection is always CLSEL_DAT_CONTACT in this case

	if (!Selection)
		Selection = CList->GetSelection();

	int nContacts = 0;
	if (bOnlyInfo)
		*bOnlyInfo = true;

	for (int i = 0; i < Selection->GetSize(); i++) {
		int ItemType = CList->GetItemType((*Selection)[i]);
		if (bOnlyInfo && ItemType != MCLCIT_INFO)
			*bOnlyInfo = false;

		nContacts += ItemType == MCLCIT_CONTACT || ItemType == MCLCIT_INFO;
	}
	return nContacts;
}

void ApplySelContactsMessage(SetAwayMsgData* dat, CCList *CList, PTREEITEMARRAY Selection = NULL)
{
	TCString Message;
	HWND hwndDlg = g_SetAwayMsgPage.GetWnd();
	GetDlgItemText(hwndDlg, IDC_SAWAYMSG_MSGDATA, Message.GetBuffer(AWAY_MSGDATA_MAX), AWAY_MSGDATA_MAX);
	Message.ReleaseBuffer();
	if (!mir_tstrlen(Message))
		Message = NULL; // delete personal message if it's empty

	if (CList) {
		if (!Selection)
			Selection = CList->GetSelection();

		for (int i = 0; i < Selection->GetSize(); i++) {
			HTREEITEM hItem = (*Selection)[i];
			int ItemType = CList->GetItemType(hItem);
			if (ItemType == MCLCIT_CONTACT) {
				MCONTACT hContact = CList->GethContact(hItem);
				CContactSettings(0, hContact).SetMsgFormat(SMF_PERSONAL, Message);
			}
			else if (ItemType == MCLCIT_INFO) {
				char *szProto = (char*)CList->GetItemParam(hItem);
				CProtoSettings(szProto).SetMsgFormat(SMF_PERSONAL, (szProto || Message != NULL) ? Message : _T("")); // "szProto || Message != NULL..." means that we'll set an empty message instead of NULL for the global status, if the message is empty (NULL for the global status has a special meaning - SetMsgFormat will set the default message instead of NULL)
			}
			else continue;

			SetExtraIcon(CList, EXTRACOLUMN_IGNORE, hItem, -1); // refresh contact's personal message icon (MSG or DOT)
		}
	}
	else if (Selection != CLSEL_DAT_NOTHING) {
		if (dat->hInitContact)
			CContactSettings(0, dat->hInitContact).SetMsgFormat(SMF_PERSONAL, Message);
		else
			CProtoSettings(dat->szProtocol).SetMsgFormat(SMF_PERSONAL, (dat->szProtocol || Message != NULL) ? Message : _T(""));
	}
	SendDlgItemMessage(hwndDlg, IDC_SAWAYMSG_MSGDATA, EM_SETMODIFY, false, 0);
	SetDlgItemText(hwndDlg, IDC_OK, TranslateT("OK"));
}

void UpdateCheckboxesState(CCList *CList)
{
	_ASSERT(CList);
	PTREEITEMARRAY Selection = CList->GetSelection();
	int ReplyIcon = -1;
	int IgnoreIcon = -1;
	for (int i = 0; i < Selection->GetSize() && (IgnoreIcon != EXTRAIMGLIST_DOT || ReplyIcon != EXTRAIMGLIST_DOT); i++) {
		HTREEITEM hItem = (*Selection)[i];
		int ItemType = CList->GetItemType(hItem);
		if (ItemType == MCLCIT_CONTACT || ItemType == MCLCIT_INFO) {
			int CurIgnoreIcon = CList->GetExtraImage(hItem, EXTRACOLUMN_IGNORE);
			if (IgnoreIcon == -1)
				IgnoreIcon = CurIgnoreIcon;
			else if (CurIgnoreIcon != EXTRAIMGLIST_IGNORE)
				IgnoreIcon = EXTRAIMGLIST_DOT;

			int CurReplyIcon = CList->GetExtraImage(hItem, EXTRACOLUMN_REPLY);
			if (ReplyIcon == -1)
				ReplyIcon = CurReplyIcon;
			else if (CurReplyIcon != ReplyIcon)
				ReplyIcon = EXTRAIMGLIST_DOT;
		}
	}
	HWND hwndDlg = g_SetAwayMsgPage.GetWnd();
	CheckDlgButton(hwndDlg, IDC_SAWAYMSG_IGNOREREQ, (IgnoreIcon == EXTRAIMGLIST_IGNORE) ? BST_CHECKED : BST_UNCHECKED);
	EnableWindow(GetDlgItem(hwndDlg, IDC_SAWAYMSG_SENDMSG), IgnoreIcon != EXTRAIMGLIST_IGNORE);
	CheckDlgButton(hwndDlg, IDC_SAWAYMSG_SENDMSG, (ReplyIcon == EXTRAIMGLIST_AUTOREPLY_ON) ? BST_CHECKED : ((ReplyIcon == EXTRAIMGLIST_AUTOREPLY_OFF) ? BST_UNCHECKED : BST_INDETERMINATE));
}

HICON g_LoadIconEx(const char* name, bool big)
{
	char szSettingName[100];
	mir_snprintf(szSettingName, sizeof(szSettingName), "%s_%s", "", name);
	return Skin_GetIcon(szSettingName, big);
}

struct {
	int DlgItem, IconIndex;
	TCHAR* Text;
}
static Buttons[] =
{
	{ IDC_SAWAYMSG_SAVEMSG, ILI_SAVE, LPGENT("Save, replacing the selected message") },
	{ IDC_SAWAYMSG_SAVEASNEW, ILI_SAVEASNEW, LPGENT("Save as a new message") },
	{ IDC_SAWAYMSG_NEWCATEGORY, ILI_NEWCATEGORY, LPGENT("Create new category") },
	{ IDC_SAWAYMSG_DELETE, ILI_DELETE, LPGENT("Delete") },
	{ IDC_SAWAYMSG_VARS, ILI_NOICON, LPGENT("Open Variables help dialog") },
	{ IDC_SAWAYMSG_OPTIONS, ILI_SETTINGS, LPGENT("Show settings menu") }
};

struct {
	int DlgItemID;
	TCHAR* Text;
} Tooltips[] = {
	IDC_SAWAYMSG_IGNOREREQ, LPGENT("Don't send the status message to selected contact(s)"),
	IDC_SAWAYMSG_SENDMSG, LPGENT("Send an autoreply to selected contact(s)"),
};

INT_PTR CALLBACK SetAwayMsgDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static int SetMsgSplitterX, SetContactSplitterX;
	static SetAwayMsgData *dat;
	static HWND hWndTooltips;
	static int UpdateLock = 0;
	static int Countdown;
	static CMsgTree *MsgTree = NULL;
	static CCList *CList = NULL;

	NMCLIST nm;

	switch (uMsg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		{
			dat = (SetAwayMsgData*)lParam;
			g_SetAwayMsgPage.SetWnd(hwndDlg);
			g_SetAwayMsgPage.DBToMemToPage();

			HICON hTitleIconBigElse = LoadSkinnedIconBig(SKINICON_OTHER_MIRANDA);

			char *szProto = dat->hInitContact ? GetContactProto(dat->hInitContact) : dat->szProtocol;
			int Status = 0;
			Status = g_ProtoStates[dat->szProtocol].Status;
			HICON hTitleIcon = LoadSkinnedProtoIcon(szProto, Status);
			HICON hTitleIconBig = LoadSkinnedProtoIconBig(szProto, Status);

			if (hTitleIconBig == NULL || (HICON)CALLSERVICE_NOTFOUND == hTitleIconBig)
				SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)hTitleIconBigElse);
			else
				SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)hTitleIconBig);

			SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)hTitleIcon);
			SendDlgItemMessage(hwndDlg, IDC_SAWAYMSG_MSGDATA, EM_LIMITTEXT, AWAY_MSGDATA_MAX, 0);

			// init window size variables / resize the window
			RECT rc;
			POINT pt;
			GetWindowRect(GetDlgItem(hwndDlg, IDC_SAWAYMSG_MSGSPLITTER), &rc);
			pt.x = (rc.left + rc.right) / 2;
			pt.y = 0;
			ScreenToClient(hwndDlg, &pt);
			g_MsgSplitterX = pt.x;
			RECT rcContactSplitter;
			GetWindowRect(GetDlgItem(hwndDlg, IDC_SAWAYMSG_CONTACTSPLITTER), &rcContactSplitter);
			RECT rcMsgDlg;
			GetWindowRect(hwndDlg, &rcMsgDlg);
			g_ContactSplitterX = rcMsgDlg.right - rcContactSplitter.left - (rcContactSplitter.right - rcContactSplitter.left) / 2;
			RECT rcVars;
			GetWindowRect(GetDlgItem(hwndDlg, IDC_SAWAYMSG_VARS), &rcVars);
			g_VariablesButtonDX = rcContactSplitter.left - rcVars.right;

			RECT rcOK;
			GetWindowRect(GetDlgItem(hwndDlg, IDC_OK), &rcOK);
			MinMsgEditSize = rcOK.right - rc.left + rcContactSplitter.right - rcVars.left + 3;
			rc.left = MINSPLITTERPOS;

			pt.x = rc.right;
			pt.y = 0;
			ScreenToClient(hwndDlg, &pt);
			MinMsgSplitterX = pt.x;
			GetWindowRect(GetDlgItem(hwndDlg, IDC_SAWAYMSG_IGNOREREQ), &rc);
			MinContactSplitterX = rcMsgDlg.right - rc.left + 1;

			// [try] getting dialog position
			int DlgPosX = db_get_dw(NULL, MOD_NAME, SAM_DB_DLGPOSX, -1);
			int DlgPosY = db_get_dw(NULL, MOD_NAME, SAM_DB_DLGPOSY, -1);
			int DlgSizeX = db_get_dw(NULL, MOD_NAME, SAM_DB_DLGSIZEX, -1);
			int DlgSizeY = db_get_dw(NULL, MOD_NAME, SAM_DB_DLGSIZEY, -1);
			int MsgSplitterX = db_get_dw(NULL, MOD_NAME, SAM_DB_MSGSPLITTERPOS, -1);
			int ContactSplitterX = db_get_dw(NULL, MOD_NAME, SAM_DB_CONTACTSPLITTERPOS, -1);
			if (DlgPosX >= 0 && DlgPosY >= 0 && DlgSizeX > 0 && DlgSizeY > 0 && MsgSplitterX > 0 && ContactSplitterX > 0) {
				RECT rcWorkArea, rcIntersect;
				SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWorkArea, 0);
				RECT rcDlgPos = { DlgPosX, DlgPosY, DlgPosX + DlgSizeX, DlgPosY + DlgSizeY };
				if (!IntersectRect(&rcIntersect, &rcDlgPos, &rcWorkArea)) { // make sure the window will be visible
					DlgPosX = rcWorkArea.left;
					DlgPosY = rcWorkArea.top;
				}
				SetWindowPos(hwndDlg, NULL, DlgPosX, DlgPosY, DlgSizeX, DlgSizeY, SWP_NOZORDER);
				g_MsgSplitterX = MsgSplitterX;
				g_ContactSplitterX = ContactSplitterX;
			}

			SetMsgSplitterX = g_MsgSplitterX;
			SetContactSplitterX = g_ContactSplitterX;
			SendMessage(hwndDlg, WM_SIZE, 0, 0); // show/hide dialog controls accordingly to the settings

			// Attention: it's important to call NEW and DELETE in a proper order, as CMsgTree and CCList are setting their own WNDPROCs for the parent window, so we must prevent WNDPROC conflicts.
			MsgTree = new CMsgTree(GetDlgItem(hwndDlg, IDC_SAWAYMSG_TREE));
			CList = NULL;
			if (g_SetAwayMsgPage.GetValue(IDS_SAWAYMSG_SHOWCONTACTTREE))
				SendMessage(hwndDlg, UM_SAM_INITCLIST, 0, 0);
			else {
				nm.hdr.code = MCLN_SELCHANGED;
				nm.hdr.hwndFrom = GetDlgItem(hwndDlg, IDC_SAWAYMSG_CONTACTSTREE);
				nm.hdr.idFrom = IDC_SAWAYMSG_CONTACTSTREE;
				nm.OldSelection = CLSEL_DAT_NOTHING;
				nm.NewSelection = CLSEL_DAT_CONTACT;
				SendMessage(hwndDlg, WM_NOTIFY, 0, (LPARAM)&nm);
			}

			// init message tree
			if (g_MoreOptPage.GetDBValueCopy(IDC_MOREOPTDLG_RECENTMSGSCOUNT) && g_MoreOptPage.GetDBValueCopy(IDC_MOREOPTDLG_PERSTATUSMRM)) {
				char *szProto = dat->hInitContact ? GetContactProto(dat->hInitContact) : dat->szProtocol;
				int ID = GetRecentGroupID((szProto || !dat->hInitContact) ? g_ProtoStates[szProto].Status : ID_STATUS_AWAY);
				CBaseTreeItem* pTreeItem = MsgTree->GetNextItem(MTGN_CHILD | MTGN_BYID, (CBaseTreeItem*)g_Messages_RecentRootID);
				while (pTreeItem) {
					if (pTreeItem->Flags & TIF_GROUP) {
						if (pTreeItem->ID == ID)
							pTreeItem->Flags |= TIF_EXPANDED; // leave expanded only one appropriate subgroup of Recent Messages group
						else
							pTreeItem->Flags &= ~TIF_EXPANDED;

						MsgTree->UpdateItem(pTreeItem->ID);
					}
					pTreeItem = MsgTree->GetNextItem(MTGN_NEXT, pTreeItem);
				}
			}
			int Order;
			CProtoSettings(dat->szProtocol).GetMsgFormat(GMF_LASTORDEFAULT, &Order);
			if (Order >= 0) // so just select an appropriate message tree item; MSGDATA text is filled automatically through SELCHANGED notification
				MsgTree->SetSelection(Order, MTSS_BYORDER);

			if (dat->Message != NULL) { // this allows to override the default message
				SetDlgItemText(hwndDlg, IDC_SAWAYMSG_MSGDATA, dat->Message);
				SendDlgItemMessage(hwndDlg, IDC_SAWAYMSG_MSGDATA, EM_SETMODIFY, true, 0);
			}
			SendDlgItemMessage(hwndDlg, IDC_SAWAYMSG_MSGDATA, EM_SETSEL, 0, -1); // select all the text in the edit control

			// init timer
			Countdown = (dat->ISW_Flags & ISWF_NOCOUNTDOWN) ? -1 : g_MoreOptPage.GetDBValueCopy(IDC_MOREOPTDLG_WAITFORMSG);
			if (Countdown == -1) // infinite
				SetDlgItemText(hwndDlg, IDC_OK, TranslateT("OK"));
			else {
				SendMessage(hwndDlg, WM_TIMER, SAM_TIMER_ID, NULL);
				SetTimer(hwndDlg, SAM_TIMER_ID, 1000, NULL);
			}

			// init image buttons
			for (int i = 0; i < SIZEOF(Buttons); i++) {
				HWND hButton = GetDlgItem(hwndDlg, Buttons[i].DlgItem);
				SendMessage(hButton, BUTTONADDTOOLTIP, (WPARAM)TranslateTS(Buttons[i].Text), BATF_TCHAR);
				SendMessage(hButton, BUTTONSETASFLATBTN, TRUE, 0);
			}

			// init tooltips
			TOOLINFO ti = { 0 };
			hWndTooltips = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, _T(""), WS_POPUP, 0, 0, 0, 0, NULL, NULL, GetModuleHandle(NULL), NULL);
			ti.cbSize = sizeof(ti);
			ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
			for (int i = 0; i < SIZEOF(Tooltips); i++) {
				ti.uId = (UINT_PTR)GetDlgItem(hwndDlg, Tooltips[i].DlgItemID);
				ti.lpszText = TranslateTS(Tooltips[i].Text);
				SendMessage(hWndTooltips, TTM_ADDTOOL, 0, (LPARAM)&ti);
			}
			SendMessage(hwndDlg, UM_ICONSCHANGED, 0, 0);

			g_OrigCListProc = (WNDPROC)SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_SAWAYMSG_CONTACTSTREE), GWLP_WNDPROC, (LONG_PTR)CListSubclassProc);
			g_OrigEditMsgProc = (WNDPROC)SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_SAWAYMSG_MSGDATA), GWLP_WNDPROC, (LONG_PTR)MsgEditSubclassProc);
			g_OrigSplitterProc = (WNDPROC)SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_SAWAYMSG_MSGSPLITTER), GWLP_WNDPROC, (LONG_PTR)SplitterSubclassProc);
			SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_SAWAYMSG_CONTACTSPLITTER), GWLP_WNDPROC, (LONG_PTR)SplitterSubclassProc);
		}
		return false;

	case WM_NOTIFY:
		switch (((NMHDR*)lParam)->idFrom) {
		case IDC_SAWAYMSG_CONTACTSTREE:
			switch (((NMHDR*)lParam)->code) { // check the notification code
			case NM_CLICK:
				_ASSERT(CList);
				{
					HWND hTreeView = GetDlgItem(hwndDlg, IDC_SAWAYMSG_CONTACTSTREE);
					POINT pt = { (short)LOWORD(GetMessagePos()), (short)HIWORD(GetMessagePos()) };
					ScreenToClient(hTreeView, &pt);
					DWORD hitFlags;
					HTREEITEM hItem = CList->HitTest(&pt, &hitFlags);
					if (hitFlags & MCLCHT_ONITEMEXTRA) {
						int nColumn = HIBYTE(HIWORD(hitFlags));
						int CurIcon = CList->GetExtraImage(hItem, nColumn);
						int ItemType = CList->GetItemType(hItem);
						if (nColumn == EXTRACOLUMN_IGNORE)
							CurIcon = (CurIcon == EXTRAIMGLIST_IGNORE) ? EXTRAIMGLIST_DOT : EXTRAIMGLIST_IGNORE;
						else {
							_ASSERT(nColumn == EXTRACOLUMN_REPLY);
							switch (CurIcon) {
							case EXTRAIMGLIST_DOT: CurIcon = EXTRAIMGLIST_AUTOREPLY_OFF; break;
							case EXTRAIMGLIST_AUTOREPLY_OFF: CurIcon = EXTRAIMGLIST_AUTOREPLY_ON; break;
							case EXTRAIMGLIST_AUTOREPLY_ON: CurIcon = EXTRAIMGLIST_DOT; break;
							}
						}
						SetExtraIcon(CList, nColumn, hItem, CurIcon);
						if (ItemType == MCLCIT_GROUP) { // set all child items
							HTREEITEM hCurItem = CList->GetNextItem(MCLGN_CHILD, hItem);
							HTREEITEM hLimitItem = CList->GetNextItem(MCLGN_NEXT | MCLGN_ANY | MCLGN_NOTCHILD, hItem);
							while (hCurItem && hCurItem != hLimitItem) {
								SetExtraIcon(CList, nColumn, hCurItem, CurIcon);
								hCurItem = CList->GetNextItem(MCLGN_NEXT | MCLGN_ANY | MCLGN_MULTILEVEL, hCurItem);
							}
						}
						SetCListGroupIcons(dat, CList);
						UpdateCheckboxesState(CList);
					}
				}
				break;

			case MCLN_SELCHANGED:
				{
					PNMCLIST nm = (PNMCLIST)lParam;
					TCString BtnTitle(TranslateT("OK"));
					if (CList)
						UpdateCheckboxesState(CList);

					bool bOnlyInfo;
					bool bLeaveOldMessage = nm->OldSelection == nm->NewSelection; // OldSelection == NewSelection when we send MCLN_SELCHANGED from UM_SAM_PROTOSTATUSCHANGED; seems that it's better to leave old message then
					int nOldContacts = GetSelContactsNum(CList, nm->OldSelection);
					int nNewContacts = GetSelContactsNum(CList, nm->NewSelection, &bOnlyInfo);
					EnableWindow(GetDlgItem(hwndDlg, IDC_SAWAYMSG_IGNOREREQ), !bOnlyInfo);
					if (CList) {
						if (SendDlgItemMessage(hwndDlg, IDC_SAWAYMSG_MSGDATA, EM_GETMODIFY, 0, 0)) {
							int i, j;
							for (i = nm->OldSelection->GetSize() - 1; i >= 0; i--) {
								for (j = nm->NewSelection->GetSize() - 1; j >= 0; j--)
									if ((*nm->NewSelection)[j] == (*nm->OldSelection)[i])
										break;
								if (j < 0)
									break;
							}
							if (nNewContacts > 1)
								BtnTitle = TranslateT("Apply");

							// there's at least one item in the old selection that is not present in the new one
							if (i >= 0 && nOldContacts) {
								if ((nOldContacts == 1) || MessageBox(hwndDlg, TranslateT("Do you want to apply the message?"), TranslateT("New Away System"), MB_ICONQUESTION | MB_YESNO) == IDYES)
									ApplySelContactsMessage(dat, CList, nm->OldSelection);
							}
							else if (nOldContacts)
								bLeaveOldMessage = true; // don't change the edit control text yet - we're still writing...
						}
					}
					else if (nOldContacts && !nNewContacts)
						ApplySelContactsMessage(dat, CList, nm->OldSelection);

					// determine the right new message and window title for the edit control now
					TCString Message;
					int Status = 0;
					if (CList) {
						bool MessageDetermined = false;
						bool StatusDetermined = false;
						for (int i = 0; i < nm->NewSelection->GetSize(); i++) {
							HTREEITEM hItem = (*nm->NewSelection)[i];
							MCONTACT hContact;
							char *szProto;
							int ItemType = CList->GetItemType(hItem);
							if (ItemType == MCLCIT_CONTACT) {
								hContact = CList->GethContact(hItem);
								_ASSERT(hContact);
								szProto = GetContactProto(hContact);
								_ASSERT(szProto);
							}
							else if (ItemType == MCLCIT_INFO)
								szProto = (char*)CList->GetItemParam(hItem);

							if (ItemType == MCLCIT_CONTACT || ItemType == MCLCIT_INFO) {
								int CurStatus = g_ProtoStates[szProto].Status;
								if (!MessageDetermined) {
									TCString CurMessage((ItemType == MCLCIT_CONTACT) ? CContactSettings(0, hContact).GetMsgFormat(GMF_PERSONAL) : CProtoSettings(szProto).GetMsgFormat(GMF_TEMPORARY | GMF_PERSONAL));
									if (CurMessage == NULL)
										CurMessage = _T("");

									if (Message == NULL)
										Message = CurMessage;
									else if (CurMessage != (const TCHAR*)Message) {
										Message = _T("");
										BtnTitle = TranslateT("Apply");
										MessageDetermined = true;
									}
								}
								if (!StatusDetermined) {
									if (!Status)
										Status = CurStatus;
									else if (CurStatus != Status) {
										Status = 0;
										StatusDetermined = true;
									}
								}
							}
						}
					}
					else if (nNewContacts) {
						Status = g_ProtoStates[dat->szProtocol].Status;
						Message = dat->hInitContact ? CContactSettings(0, dat->hInitContact).GetMsgFormat(GMF_PERSONAL) : CProtoSettings(dat->szProtocol).GetMsgFormat(GMF_TEMPORARY | GMF_PERSONAL);
					}

					if (!bLeaveOldMessage)
						SetDlgItemText(hwndDlg, IDC_SAWAYMSG_MSGDATA, (Message == NULL) ? _T("") : Message);

					SetDlgItemText(hwndDlg, IDC_OK, BtnTitle);
					TCString WindowTitle(TranslateT("Set message for"));
					WindowTitle += _T(" \"");
					WindowTitle += Status ? pcli->pfnGetStatusModeDescription(Status, 0) : TranslateT("Statuses");
					if (nNewContacts == 1) {
						WindowTitle += TCString(TranslateT("\" ("));
						MCONTACT hContact = NULL;
						char *szProto = NULL;
						if (CList) {
							for (int i = 0; i < nm->NewSelection->GetSize(); i++) {
								HTREEITEM hItem = (*nm->NewSelection)[i];
								int ItemType = CList->GetItemType(hItem);
								if (ItemType == MCLCIT_CONTACT) {
									hContact = CList->GethContact((*nm->NewSelection)[i]);
									break;
								}
								if (ItemType == MCLCIT_INFO) {
									szProto = (char*)CList->GetItemParam(hItem);
									break;
								}
							}
						}
						else {
							hContact = dat->hInitContact;
							szProto = dat->szProtocol;
						}

						if (hContact) {
							if (IsAnICQProto(GetContactProto(hContact))) {
								WindowTitle += TranslateT("message for");
								WindowTitle += _T(" ");
							}
							else {
								WindowTitle += TranslateT("for");
								WindowTitle += _T(" ");
							}

							WindowTitle += (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR);
							if (!IsAnICQProto(GetContactProto(hContact))) {
								WindowTitle += _T(" ");
								WindowTitle += TranslateT("available autoreply only");
								WindowTitle += _T(")");
							}
							else WindowTitle += _T(")");
						}
						else {
							if (!szProto) {
								WindowTitle += TranslateT("all accounts");
								WindowTitle += _T(")");
							}
							else {
								PROTOACCOUNT *acc = ProtoGetAccount(szProto);
								WindowTitle += acc->tszAccountName;
								WindowTitle += _T(")");
							}
						}
					}
					else WindowTitle += _T("\"");

					SetWindowText(hwndDlg, WindowTitle);
				}
			}
			break;

		case IDC_SAWAYMSG_TREE:
			switch (((NMHDR*)lParam)->code) {
			case MTN_SELCHANGED:
				if (!UpdateLock) {
					PNMMSGTREE pnm = (PNMMSGTREE)lParam;
					if (pnm->ItemNew && !(pnm->ItemNew->Flags & (TIF_ROOTITEM | TIF_GROUP))) {
						SetDlgItemText(hwndDlg, IDC_SAWAYMSG_MSGDATA, ((CTreeItem*)pnm->ItemNew)->User_Str1);
						SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_SAWAYMSG_MSGDATA, EN_CHANGE), (LPARAM)GetDlgItem(hwndDlg, IDC_SAWAYMSG_MSGDATA));
						SendDlgItemMessage(hwndDlg, IDC_SAWAYMSG_MSGDATA, EM_SETMODIFY, true, 0);
						EnableWindow(GetDlgItem(hwndDlg, IDC_SAWAYMSG_SAVEMSG), true);
					}
					else EnableWindow(GetDlgItem(hwndDlg, IDC_SAWAYMSG_SAVEMSG), false);

					EnableWindow(GetDlgItem(hwndDlg, IDC_SAWAYMSG_DELETE), pnm->ItemNew && !(pnm->ItemNew->Flags & TIF_ROOTITEM));
				}
			}
			break;
		}
		return true;

	case WM_TIMER:
		if (wParam == SAM_TIMER_ID) {
			if (!Countdown) {
				SendMessage(hwndDlg, UM_SAM_APPLYANDCLOSE, 0, 0);
				return true;
			}
			TCHAR BtnTitle[64];
			_sntprintf(BtnTitle, sizeof(BtnTitle), TranslateT("Closing in %d"), Countdown);
			SetDlgItemText(hwndDlg, IDC_OK, BtnTitle);
			Countdown--;
		}
		break;

	case UM_SAM_INITCLIST:
		_ASSERT(!CList);
		CList = new CCList(GetDlgItem(hwndDlg, IDC_SAWAYMSG_CONTACTSTREE));
		{
			// init contact tree
			HIMAGELIST hil = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 | ILC_MASK, 5, 2);
			ImageList_AddIcon(hil, LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_DOT)));
			ImageList_AddIcon(hil, LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_IGNORE)));
			ImageList_AddIcon(hil, LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_MSGICON)));
			ImageList_AddIcon(hil, LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_SOE_ENABLED)));
			ImageList_AddIcon(hil, LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_SOE_DISABLED)));
			CList->SetExtraImageList(hil);

			HTREEITEM hSelItem;
			HTREEITEM hItem = hSelItem = CList->AddInfo(TranslateT("** All contacts **"), CLC_ROOT, CLC_ROOT, NULL, LoadSkinnedProtoIcon(NULL, g_ProtoStates[(char*)NULL].Status));
			int numAccs;
			PROTOACCOUNT **accs;
			ProtoEnumAccounts(&numAccs, &accs);
			for (int i = 0; i < numAccs; i++) {
				PROTOACCOUNT *p = accs[i];
				// don't forget to change Recent Message Save loop in the UM_SAM_APPLYANDCLOSE if you're changing something here
				if (CallProtoService(p->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND) {
					PROTOACCOUNT * acc = ProtoGetAccount(p->szModuleName);
					hItem = CList->AddInfo(TCString(_T("* ")) + acc->tszAccountName + _T(" *"), CLC_ROOT, hItem, (LPARAM)p->szModuleName, LoadSkinnedProtoIcon(p->szModuleName, g_ProtoStates[p->szModuleName].Status));
					if (dat->szProtocol && !strcmp(p->szModuleName, dat->szProtocol))
						hSelItem = hItem;
				}
			}

			MCONTACT hContact = db_find_first();
			CList->SetRedraw(false);
			do {
				char *szProto = GetContactProto(hContact);
				if (szProto) {
					int Flag1 = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0);
					if ((Flag1 & PF1_IM) == PF1_IM || Flag1 & PF1_INDIVMODEMSG) { // check if the protocol supports message sending/receiving or individual status messages before adding this contact
						HTREEITEM hItem = CList->AddContact(hContact);
						if (dat->hInitContact == hContact)
							hSelItem = hItem;
					}
				}
			}
				while (hContact = db_find_next(hContact));

			CList->SortContacts();
			hItem = CLC_ROOT;
			while (hItem = CList->GetNextItem(MCLGN_NEXT | MCLGN_CONTACT | MCLGN_INFO | MCLGN_MULTILEVEL, hItem)) {
				SetExtraIcon(CList, EXTRACOLUMN_IGNORE, hItem, -1);
				SetExtraIcon(CList, EXTRACOLUMN_REPLY, hItem, -1);
			}
			SetCListGroupIcons(dat, CList);
			CList->SetRedraw(true); // SetRedraw must be before SelectItem - otherwise SelectItem won't change scrollbar position to make the selected contact visible
			CList->SelectItem(hSelItem); // must be selected after setting all extra icons, to set checkboxes properly
		}
		break;

	case UM_SAM_APPLYANDCLOSE:
		KillTimer(hwndDlg, SAM_TIMER_ID);
		if (CList)
			CList->SelectItem(NULL);
		else {
			nm.hdr.code = MCLN_SELCHANGED;
			nm.hdr.hwndFrom = GetDlgItem(hwndDlg, IDC_SAWAYMSG_CONTACTSTREE);
			nm.hdr.idFrom = IDC_SAWAYMSG_CONTACTSTREE;
			nm.OldSelection = CLSEL_DAT_CONTACT;
			nm.NewSelection = CLSEL_DAT_NOTHING;
			SendMessage(hwndDlg, WM_NOTIFY, 0, (LPARAM)&nm);
		}
		MsgTree->Save();
		{
			// save Recent Messages
			int numAccs;
			PROTOACCOUNT **accs;
			ProtoEnumAccounts(&numAccs, &accs);
			for (int i = 0; i < numAccs; i++) {
				PROTOACCOUNT *p = accs[i];
				if (CallProtoService(p->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_MODEMSGSEND) {
					TCString Message(CProtoSettings(p->szModuleName).GetMsgFormat(GMF_PERSONAL)); // yes, we don't specify GMF_TEMPORARY here, because we don't need to save it
					if (Message != NULL)
						CProtoSettings(p->szModuleName).SetMsgFormat(SMF_LAST, Message); // if the user set a message for this protocol, save it to the recent messages
					ChangeProtoMessages(p->szModuleName, g_ProtoStates[p->szModuleName].Status, TCString(NULL)); // and actual setting of a status message for the protocol
				}
			}
			TCString Message(CProtoSettings().GetMsgFormat(GMF_PERSONAL));
			if (Message != NULL)
				CProtoSettings().SetMsgFormat(SMF_LAST, Message); // save the global message to the recent messages

			if (g_SetAwayMsgPage.GetValue(IDS_SAWAYMSG_AUTOSAVEDLGSETTINGS))
				SendMessage(hwndDlg, UM_SAM_SAVEDLGSETTINGS, 0, 0);

			if (dat->IsModeless)
				DestroyWindow(hwndDlg);
			else
				EndDialog(hwndDlg, 2);
		}
		return true;

	case UM_SAM_SAVEDLGSETTINGS:
		{
			RECT rcRect;
			GetWindowRect(hwndDlg, &rcRect);
			db_set_dw(NULL, MOD_NAME, SAM_DB_DLGPOSX, rcRect.left);
			db_set_dw(NULL, MOD_NAME, SAM_DB_DLGPOSY, rcRect.top);
			db_set_dw(NULL, MOD_NAME, SAM_DB_DLGSIZEX, rcRect.right - rcRect.left);
			db_set_dw(NULL, MOD_NAME, SAM_DB_DLGSIZEY, rcRect.bottom - rcRect.top);
			db_set_dw(NULL, MOD_NAME, SAM_DB_MSGSPLITTERPOS, g_MsgSplitterX);
			db_set_dw(NULL, MOD_NAME, SAM_DB_CONTACTSPLITTERPOS, g_ContactSplitterX);
			g_SetAwayMsgPage.PageToMemToDB();
		}
		break;

	case UM_SAM_REPLYSETTINGCHANGED:
		if (CList) {
			HTREEITEM hItem = CLC_ROOT;
			if (!wParam) { // it's the global autoreply setting
				while (hItem = CList->GetNextItem(MCLGN_NEXT | MCLGN_INFO | MCLGN_MULTILEVEL, hItem)) {
					if (!CList->GetItemParam(hItem)) { // we found the item
						SetExtraIcon(CList, EXTRACOLUMN_REPLY, hItem, -1); // update it
						break;
					}
				}
			}
			else { // it's a contact's autoreply setting
				while (hItem = CList->GetNextItem(MCLGN_NEXT | MCLGN_CONTACT | MCLGN_MULTILEVEL, hItem)) {
					MCONTACT hContact = CList->GethContact(hItem);
					if (CList->GethContact(hItem) == wParam) { // we found the item
						SetExtraIcon(CList, EXTRACOLUMN_REPLY, hItem, -1); // update it
						break;
					}
				}
			}
			UpdateCheckboxesState(CList);
		}
		break;

	case UM_SAM_PROTOSTATUSCHANGED:
		if (CList) {
			HTREEITEM hItem = CLC_ROOT;
			while (hItem = CList->GetNextItem(MCLGN_NEXT | MCLGN_CONTACT | MCLGN_INFO | MCLGN_MULTILEVEL, hItem)) {
				if (CList->GetItemType(hItem) == MCLCIT_INFO) {
					char *szProto = (char*)CList->GetItemParam(hItem);
					if (!wParam || !mir_strcmp(szProto, (char*)wParam)) {
						CList->SetInfoIcon(hItem, LoadSkinnedProtoIcon(szProto, g_ProtoStates[szProto].Status));
					}
				}
				SetExtraIcon(CList, EXTRACOLUMN_IGNORE, hItem, -1);
				SetExtraIcon(CList, EXTRACOLUMN_REPLY, hItem, -1);
			}
			SetCListGroupIcons(dat, CList);
		}
		nm.hdr.code = MCLN_SELCHANGED;
		nm.hdr.hwndFrom = GetDlgItem(hwndDlg, IDC_SAWAYMSG_CONTACTSTREE);
		nm.hdr.idFrom = IDC_SAWAYMSG_CONTACTSTREE;
		nm.OldSelection = nm.NewSelection = CList ? CList->GetSelection() : CLSEL_DAT_CONTACT;
		SendMessage(hwndDlg, WM_NOTIFY, 0, (LPARAM)&nm); // everything else is in the MCLN_SELCHANGED handler, so we'll just call it from here.
		break;

	case UM_ICONSCHANGED:
		for (int i = 0; i < SIZEOF(Buttons); i++)
			if (Buttons[i].IconIndex != ILI_NOICON)
				SendDlgItemMessage(hwndDlg, Buttons[i].DlgItem, BM_SETIMAGE, IMAGE_ICON, (LPARAM)g_IconList[Buttons[i].IconIndex]);

		variables_skin_helpbutton(hwndDlg, IDC_SAWAYMSG_VARS);
		break;

	case WM_COMMAND:
		switch (HIWORD(wParam)) {
		case EN_CHANGE:
			if (LOWORD(wParam) == IDC_SAWAYMSG_MSGDATA)
				SetDlgItemText(hwndDlg, IDC_OK, TranslateTS((GetSelContactsNum(CList) > 1) ? LPGENT("Apply") : LPGENT("OK")));
			break;

		case EN_KILLFOCUS:
			if (LOWORD(wParam) == IDC_SAWAYMSG_MSGDATA && GetForegroundWindow() == hwndDlg)
				SendMessage(hwndDlg, UM_SAM_KILLTIMER, 0, 0);
			break;

		case BN_CLICKED:
			switch (LOWORD(wParam)) {
			case IDC_SAWAYMSG_IGNOREREQ:
				_ASSERT(CList);
				{
					int Ignore = IsDlgButtonChecked(hwndDlg, IDC_SAWAYMSG_IGNOREREQ) == BST_CHECKED;
					EnableWindow(GetDlgItem(hwndDlg, IDC_SAWAYMSG_SENDMSG), !Ignore);
					PTREEITEMARRAY Selection = CList->GetSelection();
					for (int i = 0; i < Selection->GetSize(); i++)
						if (CList->GetItemType((*Selection)[i]) == MCLCIT_CONTACT)
							SetExtraIcon(CList, EXTRACOLUMN_IGNORE, (*Selection)[i], Ignore ? EXTRAIMGLIST_IGNORE : EXTRAIMGLIST_DOT);

					if (Selection->GetSize() == 1)
						CList->EnsureVisible((*Selection)[0]);

					SetCListGroupIcons(dat, CList);
				}
				break;

			case IDC_SAWAYMSG_SENDMSG:
				_ASSERT(CList);
				{
					int Reply = IsDlgButtonChecked(hwndDlg, IDC_SAWAYMSG_SENDMSG);
					int ReplyIcon = (Reply == BST_CHECKED) ? EXTRAIMGLIST_AUTOREPLY_ON : ((Reply == BST_UNCHECKED) ? EXTRAIMGLIST_AUTOREPLY_OFF : EXTRAIMGLIST_DOT);
					PTREEITEMARRAY Selection = CList->GetSelection();
					for (int i = 0; i < Selection->GetSize(); i++)
						SetExtraIcon(CList, EXTRACOLUMN_REPLY, (*Selection)[i], ReplyIcon);
					if (Selection->GetSize() == 1)
						CList->EnsureVisible((*Selection)[0]);
					SetCListGroupIcons(dat, CList);
				}
				break;

			case IDC_SAWAYMSG_OPTIONS:
				{
					HMENU hMenu = LoadMenu(g_hInstance, MAKEINTRESOURCE(IDR_SAM_OPTIONS));
					_ASSERT(hMenu);
					HMENU hPopupMenu = GetSubMenu(hMenu, 0);
					TranslateMenu(hPopupMenu);
					CheckMenuItem(hPopupMenu, IDM_SAM_OPTIONS_SHOWMSGTREE, MF_BYCOMMAND | (g_SetAwayMsgPage.GetValue(IDS_SAWAYMSG_SHOWMSGTREE) ? MF_CHECKED : MF_UNCHECKED));
					CheckMenuItem(hPopupMenu, IDM_SAM_OPTIONS_SHOWCONTACTTREE, MF_BYCOMMAND | (g_SetAwayMsgPage.GetValue(IDS_SAWAYMSG_SHOWCONTACTTREE) ? MF_CHECKED : MF_UNCHECKED));
					CheckMenuItem(hPopupMenu, IDM_SAM_OPTIONS_AUTOSAVEDLGSETTINGS, MF_BYCOMMAND | (g_SetAwayMsgPage.GetValue(IDS_SAWAYMSG_AUTOSAVEDLGSETTINGS) ? MF_CHECKED : MF_UNCHECKED));
					int VariablesInstalled = ServiceExists(MS_VARS_FORMATSTRING);
					CheckMenuItem(hPopupMenu, IDM_SAM_OPTIONS_DISABLEVARIABLES, MF_BYCOMMAND | ((g_SetAwayMsgPage.GetValue(IDS_SAWAYMSG_DISABLEVARIABLES) || !VariablesInstalled) ? MF_CHECKED : MF_UNCHECKED));
					if (!VariablesInstalled)
						EnableMenuItem(hPopupMenu, IDM_SAM_OPTIONS_DISABLEVARIABLES, MF_BYCOMMAND | MF_GRAYED);

					switch (TrackPopupMenu(hPopupMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, (short)LOWORD(GetMessagePos()), (short)HIWORD(GetMessagePos()), 0, hwndDlg, NULL)) {
					case IDM_SAM_OPTIONS_SHOWMSGTREE:
						{
							int bShow = !g_SetAwayMsgPage.GetValue(IDS_SAWAYMSG_SHOWMSGTREE);
							g_SetAwayMsgPage.SetValue(IDS_SAWAYMSG_SHOWMSGTREE, bShow);
							RECT rcDlg;
							GetWindowRect(hwndDlg, &rcDlg);
							rcDlg.left -= bShow ? g_MsgSplitterX : -g_MsgSplitterX;
							SendMessage(hwndDlg, WM_SIZING, WMSZ_LEFT, (LPARAM)&rcDlg);
							SetWindowPos(hwndDlg, NULL, rcDlg.left, rcDlg.top, rcDlg.right - rcDlg.left, rcDlg.bottom - rcDlg.top, SWP_NOZORDER);
							SendMessage(hwndDlg, WM_SIZE, 0, 0); // show/hide dialog controls accordingly
						}
						break;
					case IDM_SAM_OPTIONS_SHOWCONTACTTREE:
						{
							int bShow = !g_SetAwayMsgPage.GetValue(IDS_SAWAYMSG_SHOWCONTACTTREE);
							if (bShow && !CList) {
								nm.hdr.code = MCLN_SELCHANGED;
								nm.hdr.hwndFrom = GetDlgItem(hwndDlg, IDC_SAWAYMSG_CONTACTSTREE);
								nm.hdr.idFrom = IDC_SAWAYMSG_CONTACTSTREE;
								nm.OldSelection = CLSEL_DAT_CONTACT;
								nm.NewSelection = CLSEL_DAT_NOTHING;
								SendMessage(hwndDlg, WM_NOTIFY, 0, (LPARAM)&nm); // save current message
								SendMessage(hwndDlg, UM_SAM_INITCLIST, 0, 0);
							}
							g_SetAwayMsgPage.SetValue(IDS_SAWAYMSG_SHOWCONTACTTREE, bShow);
							RECT rcDlg;
							GetWindowRect(hwndDlg, &rcDlg);
							rcDlg.right += bShow ? g_ContactSplitterX : -g_ContactSplitterX;
							SendMessage(hwndDlg, WM_SIZING, WMSZ_RIGHT, (LPARAM)&rcDlg);
							SetWindowPos(hwndDlg, NULL, 0, 0, rcDlg.right - rcDlg.left, rcDlg.bottom - rcDlg.top, SWP_NOZORDER | SWP_NOMOVE);
							SendMessage(hwndDlg, WM_SIZE, 0, 0); // show/hide dialog controls accordingly
						}
						break;
					case IDM_SAM_OPTIONS_AUTOSAVEDLGSETTINGS:
						g_SetAwayMsgPage.SetDBValue(IDS_SAWAYMSG_AUTOSAVEDLGSETTINGS, !g_SetAwayMsgPage.GetValue(IDS_SAWAYMSG_AUTOSAVEDLGSETTINGS));
						break;
					case IDM_SAM_OPTIONS_SAVEDLGSETTINGSNOW:
						SendMessage(hwndDlg, UM_SAM_SAVEDLGSETTINGS, 0, 0);
						break;
					case IDM_SAM_OPTIONS_DISABLEVARIABLES:
						g_SetAwayMsgPage.SetDBValue(IDS_SAWAYMSG_DISABLEVARIABLES, !g_SetAwayMsgPage.GetValue(IDS_SAWAYMSG_DISABLEVARIABLES));
						break;
					}
					DestroyMenu(hMenu);
				}
				break;

			case IDC_SAWAYMSG_SAVEMSG:
				{
					CBaseTreeItem* TreeItem = MsgTree->GetSelection();
					if (TreeItem && !(TreeItem->Flags & TIF_ROOTITEM)) {
						MsgTree->EnsureVisible(TreeItem->hItem);
						TCString NewMsg;
						GetDlgItemText(hwndDlg, IDC_SAWAYMSG_MSGDATA, NewMsg.GetBuffer(AWAY_MSGDATA_MAX), AWAY_MSGDATA_MAX);
						NewMsg.ReleaseBuffer();
						if (((CTreeItem*)TreeItem)->User_Str1 != (const TCHAR*)NewMsg) {
							((CTreeItem*)TreeItem)->User_Str1 = NewMsg;
							MsgTree->SetModified(true);
						}
					}
				}
				break;

			case IDC_SAWAYMSG_SAVEASNEW:
				{
					TCString Text;
					GetDlgItemText(hwndDlg, IDC_SAWAYMSG_MSGDATA, Text.GetBuffer(AWAY_MSGDATA_MAX), AWAY_MSGDATA_MAX);
					Text.ReleaseBuffer();
					UpdateLock++;
					CTreeItem* TreeItem = MsgTree->AddMessage();
					UpdateLock--;
					TreeItem->User_Str1 = Text;
				}
				break;

			case IDC_SAWAYMSG_NEWCATEGORY:
				MsgTree->AddCategory();
				break;

			case IDC_SAWAYMSG_DELETE:
				MsgTree->EnsureVisible(MsgTree->GetSelection()->hItem);
				MsgTree->DeleteSelectedItem();
				break;

			case IDC_SAWAYMSG_VARS:
				my_variables_showhelp(hwndDlg, IDC_SAWAYMSG_MSGDATA);
				break;

			case IDC_OK:
				// save OK button title before resetting it in SetSelContactsMessage
				TCString BtnTitle;
				GetDlgItemText(hwndDlg, IDC_OK, BtnTitle.GetBuffer(64), 64);
				BtnTitle.ReleaseBuffer();
				ApplySelContactsMessage(dat, CList);
				if (BtnTitle != (const TCHAR*)TranslateT("Apply"))
					SendMessage(hwndDlg, UM_SAM_APPLYANDCLOSE, 0, 0);
			}
		}
		break;

	case UM_SAM_SPLITTERMOVED:
		switch (lParam) {
		RECT rc;
		POINT pt;
		int MaxSetSplitterX, MaxSplitterX;
		case IDC_SAWAYMSG_MSGSPLITTER:
			GetClientRect(hwndDlg, &rc);
			pt.x = wParam;
			pt.y = 0;
			ScreenToClient(hwndDlg, &pt);
			g_MsgSplitterX = (pt.x < MINSPLITTERPOS) ? MINSPLITTERPOS : pt.x;
			MaxSetSplitterX = rc.right - MINSPLITTERPOS + 2;
			MaxSplitterX = MaxSetSplitterX;
			if (g_SetAwayMsgPage.GetValue(IDS_SAWAYMSG_SHOWCONTACTTREE)) {
				MaxSetSplitterX -= g_ContactSplitterX;
				MaxSplitterX -= MINSPLITTERPOS;
			}
			if (g_MsgSplitterX > MaxSetSplitterX) {
				if (g_MsgSplitterX > MaxSplitterX) {
					g_ContactSplitterX = MINSPLITTERPOS;
					g_MsgSplitterX = MaxSplitterX;
				}
				else g_ContactSplitterX = MINSPLITTERPOS + MaxSplitterX - g_MsgSplitterX;
			}
			break;

		case IDC_SAWAYMSG_CONTACTSPLITTER:
			GetClientRect(hwndDlg, &rc);
			pt.x = wParam;
			pt.y = 0;
			ScreenToClient(hwndDlg, &pt);
			g_ContactSplitterX = (rc.right - pt.x < MINSPLITTERPOS) ? MINSPLITTERPOS : (rc.right - pt.x);
			MaxSetSplitterX = rc.right - MINSPLITTERPOS + 2;
			MaxSplitterX = MaxSetSplitterX;
			if (g_SetAwayMsgPage.GetValue(IDS_SAWAYMSG_SHOWMSGTREE)) {
				MaxSetSplitterX -= g_MsgSplitterX;
				MaxSplitterX -= MINSPLITTERPOS;
			}
			if (g_ContactSplitterX > MaxSetSplitterX) {
				if (g_ContactSplitterX > MaxSplitterX) {
					g_MsgSplitterX = MINSPLITTERPOS;
					g_ContactSplitterX = MaxSplitterX;
				}
				else g_MsgSplitterX = MINSPLITTERPOS + MaxSplitterX - g_ContactSplitterX;
			}
		}
		SetMsgSplitterX = g_MsgSplitterX;
		SetContactSplitterX = g_ContactSplitterX;
		SendMessage(hwndDlg, WM_SIZE, 0, 0);
		break;

	case WM_SIZING:
		{
			RECT *prcDlg = (RECT*)lParam;
			int MinSetXSize = MINSPLITTERPOS + 7;
			int MinXSize = MinMsgEditSize + 7;
			if (g_SetAwayMsgPage.GetValue(IDS_SAWAYMSG_SHOWMSGTREE)) {
				MinSetXSize += SetMsgSplitterX - 1;
				MinXSize += MinMsgSplitterX - 1;
			}
			if (g_SetAwayMsgPage.GetValue(IDS_SAWAYMSG_SHOWCONTACTTREE)) {
				MinSetXSize += SetContactSplitterX - 1;
				MinXSize += MinContactSplitterX - 1;
			}
			if (prcDlg->right - prcDlg->left < MinXSize) {
				if (wParam != WMSZ_LEFT && wParam != WMSZ_TOPLEFT && wParam != WMSZ_BOTTOMLEFT)
					prcDlg->right = prcDlg->left + MinXSize;
				else
					prcDlg->left = prcDlg->right - MinXSize;
			}
			if (prcDlg->right - prcDlg->left < MinSetXSize) {
				int Delta = MinSetXSize - (prcDlg->right - prcDlg->left);
				if (SetMsgSplitterX > MinMsgSplitterX) {
					int D2 = min(Delta, SetMsgSplitterX - MinMsgSplitterX);
					g_MsgSplitterX = SetMsgSplitterX - D2;
					Delta -= D2;
				}
				g_ContactSplitterX = SetContactSplitterX - Delta;
			}
			else {
				g_MsgSplitterX = SetMsgSplitterX;
				g_ContactSplitterX = SetContactSplitterX;
			}
			if (prcDlg->bottom - prcDlg->top < MINYDLGSIZE) {
				if (wParam != WMSZ_TOP && wParam != WMSZ_TOPLEFT && wParam != WMSZ_TOPRIGHT)
					prcDlg->bottom = prcDlg->top + MINYDLGSIZE;
				else
					prcDlg->top = prcDlg->bottom - MINYDLGSIZE;
			}
		}
		return true;

	case WM_SIZE:
		{
			UTILRESIZEDIALOG urd = { sizeof(urd) };
			urd.hInstance = g_hInstance;
			urd.hwndDlg = hwndDlg;
			urd.lParam = (LPARAM)&g_SetAwayMsgPage;
			urd.lpTemplate = MAKEINTRESOURCEA(IDD_SETAWAYMSG);
			urd.pfnResizer = SetAwayMsgDlgResize;
			CallService(MS_UTILS_RESIZEDIALOG, 0, (LPARAM)&urd);
			
			// means that we sent WM_SIZE message to apply new settings to the dialog; probably it's somewhat a misuse, but who cares ;-P
			if (!wParam && !lParam) {
				int bShow;
				bShow = g_SetAwayMsgPage.GetValue(IDS_SAWAYMSG_SHOWMSGTREE) ? SW_SHOW : SW_HIDE;
				ShowWindow(GetDlgItem(hwndDlg, IDC_SAWAYMSG_TREE), bShow);
				ShowWindow(GetDlgItem(hwndDlg, IDC_SAWAYMSG_SAVEMSG), bShow);
				ShowWindow(GetDlgItem(hwndDlg, IDC_SAWAYMSG_SAVEASNEW), bShow);
				ShowWindow(GetDlgItem(hwndDlg, IDC_SAWAYMSG_NEWCATEGORY), bShow);
				ShowWindow(GetDlgItem(hwndDlg, IDC_SAWAYMSG_DELETE), bShow);
				ShowWindow(GetDlgItem(hwndDlg, IDC_SAWAYMSG_MSGSPLITTER), bShow);
				bShow = g_SetAwayMsgPage.GetValue(IDS_SAWAYMSG_SHOWCONTACTTREE) ? SW_SHOW : SW_HIDE;
				ShowWindow(GetDlgItem(hwndDlg, IDC_SAWAYMSG_CONTACTSTREE), bShow);
				ShowWindow(GetDlgItem(hwndDlg, IDC_SAWAYMSG_IGNOREREQ), bShow);
				ShowWindow(GetDlgItem(hwndDlg, IDC_SAWAYMSG_SENDMSG), bShow);
				ShowWindow(GetDlgItem(hwndDlg, IDC_SAWAYMSG_CONTACTSPLITTER), bShow);
				ShowWindow(GetDlgItem(hwndDlg, IDC_SAWAYMSG_STATIC_IGNOREICON), bShow);
				ShowWindow(GetDlgItem(hwndDlg, IDC_SAWAYMSG_STATIC_REPLYICON), bShow);
			}
		} // go through
	case UM_SAM_KILLTIMER:
	case WM_LBUTTONDOWN:
	case WM_MOUSEACTIVATE:
	case WM_MOVING: // stops counting
		if (Countdown != -1) { // still counting
			KillTimer(hwndDlg, SAM_TIMER_ID);
			SetDlgItemText(hwndDlg, IDC_OK, TranslateT("OK"));
			Countdown = -1;
		}
		break;

	case WM_SETCURSOR:
		{
			HWND hTreeView = GetDlgItem(hwndDlg, IDC_SAWAYMSG_CONTACTSTREE);
			if ((HWND)wParam == hTreeView && LOWORD(lParam) == HTCLIENT) {
				_ASSERT(CList);
				DWORD hitFlags;
				POINT pt;
				pt.x = (short)LOWORD(GetMessagePos());
				pt.y = (short)HIWORD(GetMessagePos());
				ScreenToClient(hTreeView, &pt);
				CList->HitTest(&pt, &hitFlags);
				HCURSOR hCursor = NULL;
				if (hitFlags & (MCLCHT_ONITEM | MCLCHT_ONITEMEXTRA)) {
					SetClassLong(hTreeView, GCLP_HCURSOR, NULL);
					hCursor = LoadCursor(NULL, IDC_HAND); // set mouse cursor to a hand when hovering over items or their extra images
				}
				else SetClassLong(hTreeView, GCLP_HCURSOR, (LONG)LoadCursor(NULL, IDC_ARROW));

				if (!hCursor)
					hCursor = LoadCursor(NULL, IDC_ARROW);

				SetCursor(hCursor);
				return true;
			}
			else SetClassLong(hTreeView, GCLP_HCURSOR, (LONG)LoadCursor(NULL, IDC_ARROW));
		}
		break;

	case WM_CLOSE:
		KillTimer(hwndDlg, SAM_TIMER_ID);
		if (dat->IsModeless)
			DestroyWindow(hwndDlg);
		else
			EndDialog(hwndDlg, 2);
		break;

	case WM_DESTROY:
		delete dat;
		if (CList) {
			delete CList;
			CList = NULL;
		}
		delete MsgTree;
		MsgTree = NULL;
		g_SetAwayMsgPage.SetWnd(NULL);
		DestroyWindow(hWndTooltips);
		return false;
	}
	return false;
}
