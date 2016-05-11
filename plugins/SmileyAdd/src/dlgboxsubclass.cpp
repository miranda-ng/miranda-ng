/*
Miranda SmileyAdd Plugin
Copyright (C) 2005 - 2012 Boris Krasnovskiy All Rights Reserved
Copyright (C) 2003 - 2004 Rein-Peter de Boer

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

static mir_cs csWndList;

static LRESULT CALLBACK MessageDlgSubclass(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// type definitions 

class MsgWndData : public MZeroedObject
{
public:
	HWND hwnd;
	char ProtocolName[52];
	HWND REdit;
	HWND MEdit;
	HWND LButton;
	mutable HWND hSmlButton;
	mutable HBITMAP hSmlBmp;
	mutable HICON hSmlIco;
	int idxLastChar;
	MCONTACT hContact;
	bool doSmileyReplace;
	bool doSmileyButton;
	bool isSplit;
	bool isSend;

	MsgWndData()
	{
	}

	~MsgWndData()
	{
		clear();
	}

	void clear(void)
	{
		if (hSmlBmp != NULL) {
			DeleteObject(hSmlBmp);
			hSmlBmp = NULL;
		}
		if (hSmlIco != NULL) {
			DestroyIcon(hSmlIco);
			hSmlIco = NULL;
		}
		if (hSmlButton != NULL) {
			DestroyWindow(hSmlButton);
			hSmlButton = NULL;
		}		
	}

	RECT CalcSmileyButtonPos(void)
	{
		RECT rect;
		GetWindowRect(LButton, &rect);

		POINT pt;
		pt.y = rect.top;

		MUUID muidScriver = { 0x84636f78, 0x2057, 0x4302, { 0x8a, 0x65, 0x23, 0xa1, 0x6d, 0x46, 0x84, 0x4c } };
		int iShift = (IsPluginLoaded(muidScriver)) ? 28 : -28;

		if ((GetWindowLongPtr(LButton, GWL_STYLE) & WS_VISIBLE) != 0)
			pt.x = rect.left + iShift;
		else
			pt.x = rect.left;

		ScreenToClient(GetParent(LButton), &pt);
		rect.bottom += pt.y - rect.top;
		rect.right += pt.x - rect.left;
		rect.top = pt.y;
		rect.left = pt.x;
		return rect;
	}

	void CreateSmileyButton(void)
	{
		doSmileyButton = opt.ButtonStatus != 0;

		SmileyPackType *SmileyPack = GetSmileyPack(ProtocolName, hContact);
		doSmileyButton &= SmileyPack != NULL && SmileyPack->VisibleSmileyCount() != 0;

		doSmileyReplace = true;
		doSmileyButton &= db_get_b(NULL, "SRMM", "ShowButtonLine", TRUE) != 0;

		if (ProtocolName[0] != 0) {
			INT_PTR cap = CallProtoService(ProtocolName, PS_GETCAPS, PFLAGNUM_1, 0);
			doSmileyButton &= ((cap & (PF1_IMSEND | PF1_CHAT)) != 0);
			doSmileyReplace &= ((cap & (PF1_IMRECV | PF1_CHAT)) != 0);
		}

		if (doSmileyButton && opt.PluginSupportEnabled) {
			// create smiley button
			RECT rect = CalcSmileyButtonPos();

			hSmlButton = CreateWindowEx(
				WS_EX_LEFT | WS_EX_NOPARENTNOTIFY | WS_EX_TOPMOST,
				MIRANDABUTTONCLASS,
				_T("S"),
				WS_CHILD | WS_VISIBLE | WS_TABSTOP,
				rect.left,
				rect.top,
				rect.bottom - rect.top + 1,
				rect.bottom - rect.top + 1,
				GetParent(LButton),
				(HMENU)IDC_SMLBUTTON,
				NULL, NULL);

			// Conversion to bitmap done to prevent Miranda from scaling the image
			SmileyType *sml = FindButtonSmiley(SmileyPack);
			if (sml != NULL) {
				hSmlBmp = sml->GetBitmap(GetSysColor(COLOR_BTNFACE), 0, 0);
				SendMessage(hSmlButton, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hSmlBmp);
			}
			else {
				hSmlIco = GetDefaultIcon();
				SendMessage(hSmlButton, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hSmlIco);
			}

			SendMessage(hSmlButton, BUTTONADDTOOLTIP, (WPARAM)LPGEN("Show smiley selection window"), 0);
			SendMessage(hSmlButton, BUTTONSETASFLATBTN, TRUE, 0);
		}
	}
};

static LIST<MsgWndData> g_MsgWndList(10, HandleKeySortT);

int UpdateSrmmDlg(WPARAM wParam, LPARAM)
{
	mir_cslock lck(csWndList);
	
	for (int i = 0; i < g_MsgWndList.getCount(); ++i) {
		if (wParam == 0 || g_MsgWndList[i]->hContact == wParam) {
			SendMessage(g_MsgWndList[i]->hwnd, WM_SETREDRAW, FALSE, 0);
			SendMessage(g_MsgWndList[i]->hwnd, DM_OPTIONSAPPLIED, 0, 0);
			SendMessage(g_MsgWndList[i]->hwnd, WM_SETREDRAW, TRUE, 0);
		}
	}
	return 0;
}

// find the dialog info in the stored list
static MsgWndData* IsMsgWnd(HWND hwnd)
{
	mir_cslock lck(csWndList);
	return g_MsgWndList.find((MsgWndData*)&hwnd);
}

// global subclass function for all dialogs
static LRESULT CALLBACK MessageDlgSubclass(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	MsgWndData *dat = IsMsgWnd(hwnd);
	if (dat == NULL)
		return 0;

	switch (uMsg) {
	case DM_OPTIONSAPPLIED:
		dat->clear();
		dat->CreateSmileyButton();
		break;

	case DM_APPENDTOLOG:
		if (opt.PluginSupportEnabled) {
			// get length of text now before things can get added...
			GETTEXTLENGTHEX gtl;
			gtl.codepage = 1200;
			gtl.flags = GTL_PRECISE | GTL_NUMCHARS;
			dat->idxLastChar = (int)SendMessage(dat->REdit, EM_GETTEXTLENGTHEX, (WPARAM)&gtl, 0);
		}
		break;
	}

	LRESULT result = mir_callNextSubclass(hwnd, MessageDlgSubclass, uMsg, wParam, lParam);
	if (!opt.PluginSupportEnabled)
		return result;

	switch (uMsg) {
	case WM_DESTROY:
		{
			mir_cslock lck(csWndList);
			int ind = g_MsgWndList.getIndex((MsgWndData*)&hwnd);
			if (ind != -1) {
				delete g_MsgWndList[ind];
				g_MsgWndList.remove(ind);
			}
		}
		break;

	case WM_SIZE:
		if (dat->doSmileyButton) {
			RECT rect = dat->CalcSmileyButtonPos();
			SetWindowPos(dat->hSmlButton, NULL, rect.left, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
		}
		break;

	case DM_APPENDTOLOG:
		if (dat->doSmileyReplace) {
			SmileyPackCType *smcp;
			SmileyPackType *SmileyPack = GetSmileyPack(dat->ProtocolName, dat->hContact, &smcp);
			if (SmileyPack != NULL) {
				const CHARRANGE sel = { dat->idxLastChar, LONG_MAX };
				ReplaceSmileys(dat->REdit, SmileyPack, smcp, sel, false, false, false);
			}
		}
		break;

	case DM_REMAKELOG:
		if (dat->doSmileyReplace) {
			SmileyPackCType *smcp;
			SmileyPackType *SmileyPack = GetSmileyPack(dat->ProtocolName, dat->hContact, &smcp);
			if (SmileyPack != NULL) {
				static const CHARRANGE sel = { 0, LONG_MAX };
				ReplaceSmileys(dat->REdit, SmileyPack, smcp, sel, false, false, false);
			}
		}
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_SMLBUTTON && HIWORD(wParam) == BN_CLICKED) {
			RECT rect;
			GetWindowRect(dat->hSmlButton, &rect);

			SmileyToolWindowParam *stwp = new SmileyToolWindowParam;
			stwp->pSmileyPack = GetSmileyPack(dat->ProtocolName, dat->hContact);
			stwp->hWndParent = hwnd;
			stwp->hWndTarget = dat->MEdit;
			stwp->targetMessage = EM_REPLACESEL;
			stwp->targetWParam = TRUE;
			stwp->direction = 0;
			stwp->xPosition = rect.left;
			stwp->yPosition = rect.top + 24;
			mir_forkthread(SmileyToolThread, stwp);
		}

		if (LOWORD(wParam) == MI_IDC_ADD && HIWORD(wParam) == BN_CLICKED && dat->doSmileyButton) {
			RECT rect = dat->CalcSmileyButtonPos();
			SetWindowPos(dat->hSmlButton, NULL, rect.left, rect.top,	0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
		}
		break;
	}

	return result;
}

static int MsgDlgHook(WPARAM, LPARAM lParam)
{
	const MessageWindowEventData *wndEvtData = (MessageWindowEventData*)lParam;
	switch (wndEvtData->uType) {
	case MSG_WINDOW_EVT_OPENING:
		if (wndEvtData->cbSize >= sizeof(MessageWindowEventData)) {
			MsgWndData *msgwnd = new MsgWndData();
			msgwnd->hwnd = wndEvtData->hwndWindow;
			msgwnd->hContact = wndEvtData->hContact;
			msgwnd->REdit = wndEvtData->hwndLog;
			msgwnd->MEdit = wndEvtData->hwndInput;
			msgwnd->LButton = GetDlgItem(wndEvtData->hwndWindow, MI_IDC_ADD);
			if (msgwnd->LButton == NULL)
				msgwnd->LButton = GetDlgItem(wndEvtData->hwndWindow, 5019);

			// Get the protocol for this contact to display correct smileys.
			char *protonam = GetContactProto(DecodeMetaContact(msgwnd->hContact));
			if (protonam)
				strncpy_s(msgwnd->ProtocolName, protonam, _TRUNCATE);

			mir_subclassWindow(msgwnd->hwnd, MessageDlgSubclass);
			msgwnd->CreateSmileyButton();
			{
				mir_cslock lck(csWndList);
				g_MsgWndList.insert(msgwnd);
			}

			SetRichOwnerCallback(wndEvtData->hwndWindow, wndEvtData->hwndInput, wndEvtData->hwndLog);

			if (wndEvtData->hwndLog)
				SetRichCallback(wndEvtData->hwndLog, wndEvtData->hContact, false, false);
			if (wndEvtData->hwndInput)
				SetRichCallback(wndEvtData->hwndInput, wndEvtData->hContact, false, false);
		}
		break;

	case MSG_WINDOW_EVT_OPEN:
		if (wndEvtData->cbSize >= sizeof(MessageWindowEventData)) {
			SetRichOwnerCallback(wndEvtData->hwndWindow, wndEvtData->hwndInput, wndEvtData->hwndLog);
			if (wndEvtData->hwndLog)
				SetRichCallback(wndEvtData->hwndLog, wndEvtData->hContact, true, true);
			if (wndEvtData->hwndInput) {
				SetRichCallback(wndEvtData->hwndInput, wndEvtData->hContact, true, true);
				SendMessage(wndEvtData->hwndInput, WM_REMAKERICH, 0, 0);
			}
		}
		break;

	case MSG_WINDOW_EVT_CLOSE:
		if (wndEvtData->cbSize >= sizeof(MessageWindowEventData) && wndEvtData->hwndLog) {
			CloseRichCallback(wndEvtData->hwndLog);
			CloseRichOwnerCallback(wndEvtData->hwndWindow);
		}
		break;
	}
	return 0;
}

void InstallDialogBoxHook(void)
{
	HookEvent(ME_MSG_WINDOWEVENT, MsgDlgHook);
}

void RemoveDialogBoxHook(void)
{
	mir_cslock lck(csWndList);
	for (int i = 0; i < g_MsgWndList.getCount(); i++)
		delete g_MsgWndList[i];
	g_MsgWndList.destroy();
}
