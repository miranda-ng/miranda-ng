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

#include "general.h"

//***************************************************//
//              DISCLAIMER!!!
// we are not supposed to use this object, so be aware
typedef struct NewMessageWindowLParam 
{
	MCONTACT hContact;
	int isSend;
	const char *szInitialText;
} 
msgData;
// this is an undocumented object!!!!!!!
// subject to change in miranda versions...!!!!!!
//              DISCLAIMER!!!
//***************************************************//

static HHOOK g_hMessageHookPre = NULL;
static HANDLE g_hMutex = NULL;
static HANDLE g_hHookMsgWnd = NULL;

static LRESULT CALLBACK MessageDlgSubclass(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


//type definitions 
class MsgWndData 
{
public:
	HWND hwnd;
	char ProtocolName[52];
	HWND REdit;
	HWND QuoteB;
	HWND MEdit;
	HWND MOK;
	HWND LButton;
	mutable HWND hSmlButton;
	mutable HBITMAP hSmlBmp;
	mutable HICON hSmlIco;
	int idxLastChar;
	MCONTACT hContact;
	bool doSmileyReplace;
	bool doSmileyButton;
	bool OldButtonPlace;
	bool isSplit;
	bool isSend;

	MsgWndData()
	{
		ProtocolName[0] = 0;
		REdit = NULL;
		QuoteB = NULL;
		MEdit = NULL;
		MOK = NULL;
		LButton = NULL;
		hSmlButton = NULL;
		hSmlBmp = NULL;
		hSmlIco = NULL;
		idxLastChar = 0;
		hContact = NULL;
		doSmileyReplace = false;
		doSmileyButton = false;
		OldButtonPlace = false;
		isSplit = false;
		isSend = false;
	}

	MsgWndData(const MsgWndData &dsb)
	{
		*this = dsb; 
		dsb.hSmlBmp = NULL; 
		dsb.hSmlIco = NULL; 
		dsb.hSmlButton = NULL;
	}


	~MsgWndData()
	{
		clear();
	}

	void clear(void)
	{
		if (hSmlBmp != NULL) DeleteObject(hSmlBmp);
		if (hSmlIco != NULL) DestroyIcon(hSmlIco);
		if (hSmlButton != NULL) DestroyWindow(hSmlButton);
		hSmlBmp = NULL;
		hSmlIco = NULL;
		hSmlButton = NULL;
	}

	RECT CalcSmileyButtonPos(void)
	{
		RECT rect;
		POINT pt;

		if (OldButtonPlace) {
			if (isSplit && db_get_b(NULL, "SRMsg", "ShowQuote", FALSE)) {
				GetWindowRect(QuoteB, &rect);
				pt.x = rect.right + 12;
			}
			else {
				GetWindowRect(MEdit, &rect);
				pt.x = rect.left;
			}
			GetWindowRect(MOK, &rect);
			pt.y = rect.top;
		}
		else {
			GetWindowRect(LButton, &rect);
			pt.y = rect.top;

			if ((GetWindowLongPtr(LButton, GWL_STYLE) & WS_VISIBLE) != 0)
				pt.x = rect.left - 28;
			else
				pt.x = rect.left;
		}

		ScreenToClient(GetParent(LButton), &pt);
		rect.bottom += pt.y - rect.top; 
		rect.right  += pt.x - rect.left; 
		rect.top = pt.y;
		rect.left = pt.x;

		return rect;
	}

	//helper function
	//identifies the message dialog
	bool IsMessageSendDialog(HWND hwnd)
	{
		TCHAR szClassName[32] = _T("");

		GetClassName(hwnd, szClassName, SIZEOF(szClassName));
		if (mir_tstrcmp(szClassName, _T("#32770")))
			return false;  

		if ((REdit = GetDlgItem(hwnd, MI_IDC_LOG)) != NULL) {
			GetClassName(REdit, szClassName, SIZEOF(szClassName));
			if (mir_tstrcmp(szClassName, _T("RichEdit20A")) != 0 && 
				 mir_tstrcmp(szClassName, _T("RichEdit20W")) != 0 &&
				 mir_tstrcmp(szClassName, _T("RICHEDIT50W")) != 0)
				return false; 
		}
		else return false; 

		if ((MEdit = GetDlgItem(hwnd, MI_IDC_MESSAGE)) != NULL) {
			GetClassName(MEdit, szClassName, SIZEOF(szClassName));
			if (mir_tstrcmp(szClassName, _T("Edit")) != 0 &&  
				 mir_tstrcmp(szClassName, _T("RichEdit20A")) != 0 && 
				 mir_tstrcmp(szClassName, _T("RichEdit20W")) != 0 &&
				 mir_tstrcmp(szClassName, _T("RICHEDIT50W")) != 0)
				return false; 
		}
		else return false;

		QuoteB = GetDlgItem(hwnd, MI_IDC_QUOTE);

		if ((LButton = GetDlgItem(hwnd, MI_IDC_ADD)) == NULL)
			return false;

		if (GetDlgItem(hwnd, MI_IDC_NAME) == NULL) 
			return false;
		if ((MOK = GetDlgItem(hwnd, IDOK)) == NULL) 
			return false;

		return true;
	}

	void CreateSmileyButton(void)
	{
		doSmileyButton = opt.ButtonStatus != 0;
		OldButtonPlace = opt.ButtonStatus == 2;

		SmileyPackType* SmileyPack = GetSmileyPack(ProtocolName, hContact);
		doSmileyButton &= SmileyPack != NULL && SmileyPack->VisibleSmileyCount() != 0;

		bool showButtonLine;
		if (IsOldSrmm()) {
			isSplit = db_get_b(NULL,"SRMsg","Split", TRUE) != 0;

			doSmileyReplace = (isSplit || !isSend);
			doSmileyButton &= isSplit ||  isSend;
			showButtonLine = db_get_b(NULL, "SRMsg", "ShowButtonLine", TRUE) != 0;
		}
		else {
			doSmileyReplace = true;
			OldButtonPlace = false;
			showButtonLine = db_get_b(NULL, "SRMM", "ShowButtonLine", TRUE) != 0;
		}

		doSmileyButton &= OldButtonPlace || showButtonLine;

		if (ProtocolName[0] != 0) {
			INT_PTR cap = CallProtoService(ProtocolName, PS_GETCAPS, PFLAGNUM_1, 0);
			doSmileyButton  &= ((cap & (PF1_IMSEND | PF1_CHAT)) != 0);
			doSmileyReplace &= ((cap & (PF1_IMRECV | PF1_CHAT)) != 0);
		}

		if (doSmileyButton && opt.PluginSupportEnabled) {
			//create smiley button
			RECT rect = CalcSmileyButtonPos();

			hSmlButton = CreateWindowEx(
				WS_EX_LEFT | WS_EX_NOPARENTNOTIFY | WS_EX_TOPMOST,
				MIRANDABUTTONCLASS,
				_T("S"),
				WS_CHILD|WS_VISIBLE|WS_TABSTOP, // window style
				rect.left,                      // horizontal position of window
				rect.top,                       // vertical position of window
				rect.bottom - rect.top + 1,     // window width
				rect.bottom - rect.top + 1,     // window height
				GetParent(LButton),             // handle to parent or owner window
				(HMENU) IDC_SMLBUTTON,          // menu handle or child identifier
				NULL,                           // handle to application instance
				NULL);                          // window-creation data

			// Conversion to bitmap done to prevent Miranda from scaling the image
			SmileyType* sml = FindButtonSmiley(SmileyPack);
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

static int CompareMsgWndData(const MsgWndData* p1, const MsgWndData* p2)
{
	return (int)((INT_PTR)p1->hwnd - (INT_PTR)p2->hwnd);
}
static LIST<MsgWndData> g_MsgWndList(10, CompareMsgWndData);


bool IsOldSrmm(void)
{
	return ServiceExists(MS_MSG_GETWINDOWCLASS) == 0;
}


int UpdateSrmmDlg(WPARAM wParam, LPARAM /* lParam */)
{
	WaitForSingleObject(g_hMutex, 2000);
	for (int i=0; i < g_MsgWndList.getCount(); ++i) {
		if (wParam == 0 || g_MsgWndList[i]->hContact == wParam) {
			SendMessage(g_MsgWndList[i]->hwnd, WM_SETREDRAW, FALSE, 0);
			SendMessage(g_MsgWndList[i]->hwnd, DM_OPTIONSAPPLIED, 0, 0);
			SendMessage(g_MsgWndList[i]->hwnd, WM_SETREDRAW, TRUE, 0);
		}
	}
	ReleaseMutex(g_hMutex);

	return 0;
}	


//find the dialog info in the stored list
static MsgWndData* IsMsgWnd(HWND hwnd)
{
	WaitForSingleObject(g_hMutex, 2000);
	MsgWndData* res = g_MsgWndList.find((MsgWndData*)&hwnd);
	ReleaseMutex(g_hMutex);

	return res;
}


static void MsgWndDetect(HWND hwndDlg, MCONTACT hContact, msgData* datm)
{
	MsgWndData dat;

	if (dat.IsMessageSendDialog(hwndDlg)) {
		dat.hwnd = hwndDlg;
		if (datm != NULL) {
			dat.isSend = datm->isSend != 0;
			dat.hContact = datm->hContact;
		}
		else dat.hContact = hContact;

		// Get the protocol for this contact to display correct smileys.
		char *protonam = GetContactProto(DecodeMetaContact(dat.hContact));
		if (protonam) {
			mir_strncpy(dat.ProtocolName, protonam, sizeof(dat.ProtocolName));
			dat.ProtocolName[sizeof(dat.ProtocolName)-1] = 0;
		}

		WaitForSingleObject(g_hMutex, 2000);

		MsgWndData* msgwnd = g_MsgWndList.find((MsgWndData*)&hwndDlg);
		if (msgwnd == NULL) {
			msgwnd = new MsgWndData(dat);
			g_MsgWndList.insert(msgwnd);
		}
		else 
			msgwnd = NULL;
		ReleaseMutex(g_hMutex);

		if (msgwnd != NULL) {
			mir_subclassWindow(hwndDlg, MessageDlgSubclass);
			msgwnd->CreateSmileyButton();
			if (hContact == NULL)
				SetRichCallback(msgwnd->REdit, msgwnd->hContact, true, true);
		}
	}
}


//global subclass function for all dialogs
static LRESULT CALLBACK MessageDlgSubclass(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	MsgWndData* dat = IsMsgWnd(hwnd);
	if (dat == NULL)
		return 0;

	switch(uMsg) {
	case DM_OPTIONSAPPLIED:
		dat->clear();
		dat->CreateSmileyButton();
		break;

	case DM_APPENDTOLOG:
		if (opt.PluginSupportEnabled) {
			//get length of text now before things can get added...
			GETTEXTLENGTHEX gtl;
			gtl.codepage = 1200;
			gtl.flags = GTL_PRECISE | GTL_NUMCHARS;
			dat->idxLastChar = (int)SendMessage(dat->REdit, EM_GETTEXTLENGTHEX, (WPARAM) &gtl, 0);
		}
		break;
	}

	LRESULT result = mir_callNextSubclass(hwnd, MessageDlgSubclass, uMsg, wParam, lParam); 
	if (!opt.PluginSupportEnabled)
		return result;

	switch(uMsg) {
	case WM_DESTROY:
		WaitForSingleObject(g_hMutex, 2000);
		{
			int ind = g_MsgWndList.getIndex((MsgWndData*)&hwnd);
			if ( ind != -1 ) {
				delete g_MsgWndList[ind];
				g_MsgWndList.remove(ind);
			}
		}
		ReleaseMutex(g_hMutex);
		break;

	case WM_SIZE:
		if (dat->doSmileyButton) {
			RECT rect = dat->CalcSmileyButtonPos();
			SetWindowPos(dat->hSmlButton, NULL, rect.left, rect.top, 
				0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
		}
		break;

	case DM_APPENDTOLOG:
		if (dat->doSmileyReplace) {
			SmileyPackCType* smcp;
			SmileyPackType* SmileyPack = GetSmileyPack(dat->ProtocolName, dat->hContact, &smcp);
			if (SmileyPack != NULL) {
				const CHARRANGE sel = { dat->idxLastChar, LONG_MAX };
				ReplaceSmileys(dat->REdit, SmileyPack, smcp, sel, false, false, false);
			}
		}
		break;

	case DM_REMAKELOG:
		if (dat->doSmileyReplace) {
			SmileyPackCType* smcp;
			SmileyPackType* SmileyPack = GetSmileyPack(dat->ProtocolName, dat->hContact, &smcp);
			if (SmileyPack != NULL) {
				static const CHARRANGE sel = { 0, LONG_MAX };
				ReplaceSmileys(dat->REdit, SmileyPack, smcp, sel, false, false, false);
			}
		}
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_SMLBUTTON && HIWORD(wParam) == BN_CLICKED) {
			SmileyToolWindowParam *stwp = new SmileyToolWindowParam;
			stwp->pSmileyPack = GetSmileyPack(dat->ProtocolName, dat->hContact);

			stwp->hWndParent = hwnd;
			stwp->hWndTarget = dat->MEdit;
			stwp->targetMessage = EM_REPLACESEL;
			stwp->targetWParam = TRUE;

			RECT rect;
			GetWindowRect(dat->hSmlButton, &rect);

			if (dat->OldButtonPlace) {
				stwp->direction = 3;
				stwp->xPosition = rect.left;
				stwp->yPosition = rect.top + 4;
			}
			else {
				stwp->direction = 0;
				stwp->xPosition = rect.left;
				stwp->yPosition = rect.top + 24;
			}

			mir_forkthread(SmileyToolThread, stwp);
		}

		if (LOWORD(wParam) == MI_IDC_ADD && HIWORD(wParam) == BN_CLICKED && dat->doSmileyButton) {
			RECT rect = dat->CalcSmileyButtonPos();
			SetWindowPos(dat->hSmlButton, NULL, rect.left, rect.top, 
				0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
		}
		break;
	}

	return result;
}

static int MsgDlgHook(WPARAM, LPARAM lParam)
{
	const MessageWindowEventData *wndEvtData = (MessageWindowEventData*)lParam;
	switch(wndEvtData->uType) {
	case MSG_WINDOW_EVT_OPENING:
		MsgWndDetect(wndEvtData->hwndWindow, wndEvtData->hContact, NULL);
		if (wndEvtData->cbSize >= sizeof(MessageWindowEventData)) {
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


//global subclass function for all dialogs
static LRESULT CALLBACK MsgDlgHookProcPre(int code, WPARAM wParam, LPARAM lParam)
{
	const CWPSTRUCT *msg = (CWPSTRUCT*)lParam;

	if (code == HC_ACTION && msg->message == WM_INITDIALOG) 
		MsgWndDetect(msg->hwnd, NULL, (msgData*)msg->lParam);

	return CallNextHookEx(g_hMessageHookPre, code, wParam, lParam);
}


void InstallDialogBoxHook(void)
{
	g_hMutex = CreateMutex(NULL, FALSE, NULL);

	g_hHookMsgWnd = HookEvent(ME_MSG_WINDOWEVENT, MsgDlgHook);

	// Hook message API
	if (g_hHookMsgWnd == NULL)
		g_hMessageHookPre = SetWindowsHookEx(WH_CALLWNDPROC, MsgDlgHookProcPre, 
			NULL, GetCurrentThreadId());
}


void RemoveDialogBoxHook(void)
{
	if (g_hHookMsgWnd) UnhookEvent(g_hHookMsgWnd);
	if (g_hMessageHookPre) UnhookWindowsHookEx(g_hMessageHookPre);

	WaitForSingleObject(g_hMutex, 2000);
	for (int i=0; i<g_MsgWndList.getCount(); i++) 
		delete g_MsgWndList[i];
	g_MsgWndList.destroy();
	ReleaseMutex(g_hMutex);

	if (g_hMutex) CloseHandle(g_hMutex);

	g_hHookMsgWnd = NULL;
	g_hMessageHookPre = NULL;
	g_hMutex = NULL;
}
