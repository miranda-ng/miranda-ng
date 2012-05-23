/*
Copyright 2000-2012 Miranda IM project,
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

/* Missing MinGW GUIDs */
#ifdef __MINGW32__
const CLSID IID_IRichEditOle = { 0x00020D00, 0x00, 0x00, { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };
const CLSID IID_IRichEditOleCallback = { 0x00020D03, 0x00, 0x00, { 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } };
#endif

static void InitREOleCallback(void);

HCURSOR hCurSplitNS, hCurSplitWE, hCurHyperlinkHand;
HANDLE hHookWinEvt, hHookWinPopup, hMsgMenuItem;
static HANDLE hServices[7];
static HANDLE hHooks[8];

static int SRMMStatusToPf2(int status)
{
	switch (status) {
		case ID_STATUS_ONLINE:     return PF2_ONLINE;
		case ID_STATUS_AWAY:       return PF2_SHORTAWAY;
		case ID_STATUS_DND:        return PF2_HEAVYDND;
		case ID_STATUS_NA:         return PF2_LONGAWAY;
		case ID_STATUS_OCCUPIED:   return PF2_LIGHTDND;
		case ID_STATUS_FREECHAT:   return PF2_FREECHAT;
		case ID_STATUS_INVISIBLE:  return PF2_INVISIBLE;
		case ID_STATUS_ONTHEPHONE: return PF2_ONTHEPHONE;
		case ID_STATUS_OUTTOLUNCH: return PF2_OUTTOLUNCH;
		case ID_STATUS_OFFLINE:    return MODEF_OFFLINE;
	}
	return 0;
}

static int MessageEventAdded(WPARAM wParam, LPARAM lParam)
{
	DBEVENTINFO dbei = {0};
	HWND hwnd;

	dbei.cbSize = sizeof(dbei);
	CallService(MS_DB_EVENT_GET, lParam, (LPARAM) & dbei);

	if (dbei.flags & (DBEF_SENT | DBEF_READ) || !(dbei.eventType == EVENTTYPE_MESSAGE || DbEventIsForMsgWindow(&dbei)))
		return 0;

	CallServiceSync(MS_CLIST_REMOVEEVENT, wParam, (LPARAM) 1);
	/* does a window for the contact exist? */
	hwnd = WindowList_Find(g_dat->hMessageWindowList, (HANDLE) wParam);
	if (hwnd) 
	{
		if (!DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_DONOTSTEALFOCUS, SRMSGDEFSET_DONOTSTEALFOCUS))
		{
			ShowWindow(hwnd, SW_RESTORE);
			SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
			SetForegroundWindow(hwnd);
			SkinPlaySound("RecvMsgActive");
		}
		else
		{
			if (GetForegroundWindow() == hwnd)
				SkinPlaySound("RecvMsgActive");
			else 
				SkinPlaySound("RecvMsgInactive");
		}
		return 0;
	}
	/* new message */
	SkinPlaySound("AlertMsg");
	{
		char *szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) wParam, 0);
		if (szProto && (g_dat->openFlags & SRMMStatusToPf2(CallProtoService(szProto, PS_GETSTATUS, 0, 0))))
		{
			struct NewMessageWindowLParam newData = { 0 };
			newData.hContact = (HANDLE) wParam;
			newData.noActivate = DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_DONOTSTEALFOCUS, SRMSGDEFSET_DONOTSTEALFOCUS);
			CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_MSG), NULL, DlgProcMessage, (LPARAM) & newData);
			return 0;
		}
	}
	{
		TCHAR toolTip[256], *contactName;
		CLISTEVENT cle = {0};
		cle.cbSize = sizeof(cle);
		cle.hContact = (HANDLE) wParam;
		cle.hDbEvent = (HANDLE) lParam;
		cle.flags = CLEF_TCHAR;
		cle.hIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
		cle.pszService = "SRMsg/ReadMessage";
		contactName = (TCHAR*) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, wParam, GCDNF_TCHAR);
		mir_sntprintf(toolTip, SIZEOF(toolTip), TranslateT("Message from %s"), contactName);
		cle.ptszTooltip = toolTip;
		CallService(MS_CLIST_ADDEVENT, 0, (LPARAM) & cle);
	}
	return 0;
}

INT_PTR SendMessageCmd(HANDLE hContact, char* msg, int isWchar)
{
	char *szProto;
	HWND hwnd;

	/* does the HCONTACT's protocol support IM messages? */
	szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	if (!szProto || (!CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IMSEND))
		return 1;

	if (hwnd = WindowList_Find(g_dat->hMessageWindowList, hContact)) 
	{
		if (msg) 
		{
			HWND hEdit;
			hEdit = GetDlgItem(hwnd, IDC_MESSAGE);
			SendMessage(hEdit, EM_SETSEL, -1, SendMessage(hEdit, WM_GETTEXTLENGTH, 0, 0));
			if (isWchar)
				SendMessage(hEdit, EM_REPLACESEL, FALSE, (LPARAM)msg);
			else
				SendMessageA(hEdit, EM_REPLACESEL, FALSE, (LPARAM)msg);
		}
		ShowWindow(hwnd, SW_RESTORE);
		SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
		SetForegroundWindow(hwnd);
	}
	else 
	{
		struct NewMessageWindowLParam newData = { 0 };
		newData.hContact = hContact;
		newData.szInitialText = msg;
		newData.isWchar = isWchar;
		hwnd = CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_MSG), NULL, DlgProcMessage, (LPARAM)&newData);
	}
	return 0;
}

#if defined(_UNICODE)
static INT_PTR SendMessageCommand_W(WPARAM wParam, LPARAM lParam)
{
	return SendMessageCmd((HANDLE)wParam, (char*)lParam, TRUE);
}
#endif

static INT_PTR SendMessageCommand(WPARAM wParam, LPARAM lParam)
{
	return SendMessageCmd((HANDLE)wParam, (char*)lParam, FALSE);
}

static INT_PTR ReadMessageCommand(WPARAM wParam, LPARAM lParam)
{
	CLISTEVENT *cle = (CLISTEVENT *) lParam;

	if (cle)
		SendMessageCmd(cle->hContact, NULL, 0);

	return 0;
}

static int TypingMessage(WPARAM wParam, LPARAM lParam)
{
	HWND hwnd;
	int foundWin = 0;

	if (!(g_dat->flags&SMF_SHOWTYPING))
		return 0;
	if (hwnd = WindowList_Find(g_dat->hMessageWindowList, (HANDLE) wParam)) {
		SendMessage(hwnd, DM_TYPING, 0, lParam);
		foundWin = 1;
	}
	if (lParam && !foundWin && (g_dat->flags&SMF_SHOWTYPINGTRAY)) {
		TCHAR szTip[256];
		mir_sntprintf(szTip, SIZEOF(szTip), TranslateT("%s is typing a message"), (TCHAR *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, wParam, GCDNF_TCHAR));

		if (ServiceExists(MS_CLIST_SYSTRAY_NOTIFY) && !(g_dat->flags&SMF_SHOWTYPINGCLIST)) {
			MIRANDASYSTRAYNOTIFY tn = {0};
			tn.cbSize = sizeof(tn);
			tn.tszInfoTitle = TranslateT("Typing Notification");
			tn.tszInfo = szTip;
			tn.dwInfoFlags = NIIF_INFO;
#ifdef _UNICODE
			tn.dwInfoFlags |= NIIF_INTERN_UNICODE;
#endif			
			tn.uTimeout = 1000 * 4;
			CallService(MS_CLIST_SYSTRAY_NOTIFY, 0, (LPARAM) & tn);
		}
		else {
			CLISTEVENT cle = {0};

			cle.cbSize = sizeof(cle);
			cle.hContact = (HANDLE) wParam;
			cle.hDbEvent = (HANDLE) 1;
			cle.flags = CLEF_ONLYAFEW | CLEF_TCHAR;
			cle.hIcon = LoadSkinnedIcon( SKINICON_OTHER_TYPING );
			cle.pszService = "SRMsg/ReadMessage";
			cle.ptszTooltip = szTip;
			CallServiceSync(MS_CLIST_REMOVEEVENT, wParam, (LPARAM) 1);
			CallServiceSync(MS_CLIST_ADDEVENT, wParam, (LPARAM) & cle);
			CallService(MS_SKIN2_RELEASEICON,(WPARAM)cle.hIcon, 0);
		}
	}
	return 0;
}

static int MessageSettingChanged(WPARAM wParam, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *) lParam;
	HANDLE hContact = (HANDLE)wParam;

	if (cws->szModule == NULL) return 0;

	if (!strcmp(cws->szModule, "CList"))
		WindowList_Broadcast(g_dat->hMessageWindowList, DM_UPDATETITLE, (WPARAM) cws, 0);
	else if (hContact)
	{
		if (cws->szSetting && !strcmp(cws->szSetting, "Timezone"))
			WindowList_Broadcast(g_dat->hMessageWindowList, DM_NEWTIMEZONE, (WPARAM) cws, 0);
		else
		{
			char * szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0);
			if (szProto && !strcmp(cws->szModule, szProto))
				WindowList_Broadcast(g_dat->hMessageWindowList, DM_UPDATETITLE, (WPARAM) cws, 0);
		}
	}
	return 0;
}

static int ContactDeleted(WPARAM wParam, LPARAM lParam)
{
	HWND hwnd;
	if (hwnd = WindowList_Find(g_dat->hMessageWindowList, (HANDLE) wParam))
		SendMessage(hwnd, WM_CLOSE, 0, 0);

	return 0;
}

static void RestoreUnreadMessageAlerts(void)
{
	CLISTEVENT cle = {0};
	DBEVENTINFO dbei = {0};
	TCHAR toolTip[256];
	int windowAlreadyExists;
	HANDLE hDbEvent, hContact;
	int autoPopup;

	dbei.cbSize = sizeof(dbei);
	cle.cbSize = sizeof(cle);
	cle.hIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
	cle.pszService = "SRMsg/ReadMessage";
	cle.flags = CLEF_TCHAR;
	cle.ptszTooltip = toolTip;

	hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while (hContact) 
	{
		hDbEvent = (HANDLE) CallService(MS_DB_EVENT_FINDFIRSTUNREAD, (WPARAM) hContact, 0);
		while (hDbEvent) 
		{
			autoPopup = 0;
			dbei.cbBlob = 0;
			CallService(MS_DB_EVENT_GET, (WPARAM) hDbEvent, (LPARAM) & dbei);
			if (!(dbei.flags & (DBEF_SENT | DBEF_READ)) && ( dbei.eventType == EVENTTYPE_MESSAGE || DbEventIsForMsgWindow(&dbei))) 
			{
				windowAlreadyExists = WindowList_Find(g_dat->hMessageWindowList, hContact) != NULL;
				if (windowAlreadyExists)
					continue;
				{
					char *szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
					if (szProto && (g_dat->openFlags & SRMMStatusToPf2(CallProtoService(szProto, PS_GETSTATUS, 0, 0)))) 
					{
						autoPopup = 1;
					}
				}
				if (autoPopup && !windowAlreadyExists)
				{
					struct NewMessageWindowLParam newData = {0};
					newData.hContact = hContact;
					newData.noActivate = DBGetContactSettingByte(NULL, SRMMMOD, SRMSGSET_DONOTSTEALFOCUS, SRMSGDEFSET_DONOTSTEALFOCUS);
					CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_MSG), NULL, DlgProcMessage, (LPARAM) & newData);
				}
				else 
				{
					cle.hContact = hContact;
					cle.hDbEvent = hDbEvent;
					mir_sntprintf(toolTip, SIZEOF(toolTip), TranslateT("Message from %s"), (TCHAR *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM) hContact, GCDNF_TCHAR));
					CallService(MS_CLIST_ADDEVENT, 0, (LPARAM) & cle);
				}
			}
			hDbEvent = (HANDLE) CallService(MS_DB_EVENT_FINDNEXT, (WPARAM) hDbEvent, 0);
		}
		hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
	}
}

void RegisterSRMMFonts( void );

static int FontsChanged(WPARAM wParam,LPARAM lParam)
{
	WindowList_Broadcast(g_dat->hMessageWindowList, DM_OPTIONSAPPLIED, 0, 0);
	return 0;
}

static int SplitmsgModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	RegisterSRMMFonts();
	LoadMsgLogIcons();
	{
		CLISTMENUITEM mi = { 0 };
		mi.cbSize = sizeof(mi);
		mi.position = -2000090000;
		mi.flags = CMIF_ICONFROMICOLIB | CMIF_DEFAULT;
		mi.icolibItem = LoadSkinnedIconHandle( SKINICON_EVENT_MESSAGE );
		mi.pszName = LPGEN("&Message");
		mi.pszService = MS_MSG_SENDMESSAGE;
		hMsgMenuItem = ( HANDLE )CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM) & mi);
	}
	HookEvent(ME_FONT_RELOAD, FontsChanged);

	RestoreUnreadMessageAlerts();
	return 0;
}

int PreshutdownSendRecv(WPARAM wParam, LPARAM lParam)
{
	WindowList_BroadcastAsync(g_dat->hMessageWindowList, WM_CLOSE, 0, 0);
	DeinitStatusIcons();
	return 0;
}

int SplitmsgShutdown(void)
{
    int i;

	DestroyCursor(hCurSplitNS);
	DestroyCursor(hCurHyperlinkHand);
	DestroyCursor(hCurSplitWE);
	
	for (i=0; i < SIZEOF(hHooks); ++i) 
		if (hHooks[i])
			UnhookEvent(hHooks[i]);

	for ( i=0; i < SIZEOF(hServices); ++i)
		if (hServices[i])
			DestroyServiceFunction(hServices[i]);

	FreeMsgLogIcons();
	FreeLibrary(GetModuleHandleA("riched20"));
	OleUninitialize();
	RichUtil_Unload();
	msgQueue_destroy();
	FreeGlobals();
	return 0;
}

static int IconsChanged(WPARAM wParam, LPARAM lParam)
{
	FreeMsgLogIcons();
	LoadMsgLogIcons();
	WindowList_Broadcast(g_dat->hMessageWindowList, DM_REMAKELOG, 0, 0);
	// change all the icons
	WindowList_Broadcast(g_dat->hMessageWindowList, DM_UPDATEWINICON, 0, 0);
	return 0;
}

static int PrebuildContactMenu(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;
	if ( hContact ) {
		char* szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);

		CLISTMENUITEM clmi = {0};
		clmi.cbSize = sizeof( CLISTMENUITEM );
		clmi.flags = CMIM_FLAGS | CMIF_DEFAULT | CMIF_HIDDEN;

		if ( szProto ) {
			// leave this menu item hidden for chats
			if ( !DBGetContactSettingByte( hContact, szProto, "ChatRoom", 0 ))
				if ( CallProtoService( szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IMSEND )
					clmi.flags &= ~CMIF_HIDDEN;
		}

		CallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hMsgMenuItem, ( LPARAM )&clmi );
	}
	return 0;
}

static INT_PTR GetWindowAPI(WPARAM wParam, LPARAM lParam)
{
	return PLUGIN_MAKE_VERSION(0,0,0,4);
}

static INT_PTR GetWindowClass(WPARAM wParam, LPARAM lParam)
{
	char *szBuf = (char*)wParam;
	int size = (int)lParam;
	mir_snprintf(szBuf, size, SRMMMOD);
	return 0;
}

static INT_PTR GetWindowData(WPARAM wParam, LPARAM lParam)
{
	MessageWindowInputData *mwid = (MessageWindowInputData*)wParam;
	MessageWindowData *mwd = (MessageWindowData*)lParam;
	HWND hwnd;

	if (mwid==NULL||mwd==NULL) return 1;
	if (mwid->cbSize!=sizeof(MessageWindowInputData)||mwd->cbSize!=sizeof(MessageWindowData)) return 1;
	if (mwid->hContact==NULL) return 1;
	if (mwid->uFlags!=MSG_WINDOW_UFLAG_MSG_BOTH) return 1;
	hwnd = WindowList_Find(g_dat->hMessageWindowList, mwid->hContact);
	mwd->uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;
	mwd->hwndWindow = hwnd;
	mwd->local = 0;
	mwd->uState = SendMessage(hwnd, DM_GETWINDOWSTATE, 0, 0);
	return 0;
}

int LoadSendRecvMessageModule(void)
{
	if (LoadLibraryA("riched20.dll") == NULL) {
		if (IDYES !=
			MessageBox(0,
			TranslateT
			("Miranda could not load the built-in message module, riched20.dll is missing. If you are using Windows 95 or WINE please make sure you have riched20.dll installed. Press 'Yes' to continue loading Miranda."),
			TranslateT("Information"), MB_YESNO | MB_ICONINFORMATION))
			return 1;
		return 0;
	}

	InitGlobals();
	RichUtil_Load();
	OleInitialize(NULL);
	InitREOleCallback();
	InitOptions();
	msgQueue_init();

	hHooks[0] = HookEvent(ME_DB_EVENT_ADDED, MessageEventAdded);
	hHooks[1] = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, MessageSettingChanged);
	hHooks[2] = HookEvent(ME_DB_CONTACT_DELETED, ContactDeleted);
	hHooks[3] = HookEvent(ME_SYSTEM_MODULESLOADED, SplitmsgModulesLoaded);
	hHooks[4] = HookEvent(ME_SKIN_ICONSCHANGED, IconsChanged);
	hHooks[5] = HookEvent(ME_PROTO_CONTACTISTYPING, TypingMessage);
	hHooks[6] = HookEvent(ME_SYSTEM_PRESHUTDOWN, PreshutdownSendRecv);
	hHooks[7] = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PrebuildContactMenu);

	hServices[0] = CreateServiceFunction(MS_MSG_SENDMESSAGE, SendMessageCommand);
#ifdef _UNICODE
	hServices[1] = CreateServiceFunction(MS_MSG_SENDMESSAGEW, SendMessageCommand_W);
#endif
	hServices[2] = CreateServiceFunction(MS_MSG_GETWINDOWAPI, GetWindowAPI);
	hServices[3] = CreateServiceFunction(MS_MSG_GETWINDOWCLASS, GetWindowClass);
	hServices[4] = CreateServiceFunction(MS_MSG_GETWINDOWDATA, GetWindowData);
	hServices[5] = CreateServiceFunction("SRMsg/ReadMessage", ReadMessageCommand);

	hHookWinEvt   = CreateHookableEvent(ME_MSG_WINDOWEVENT);
	hHookWinPopup = CreateHookableEvent(ME_MSG_WINDOWPOPUP);
	
	SkinAddNewSoundEx("RecvMsgActive", LPGEN("Instant messages"), LPGEN("Incoming (Focused Window)"));
	SkinAddNewSoundEx("RecvMsgInactive", LPGEN("Instant messages"), LPGEN("Incoming (Unfocused Window)"));
	SkinAddNewSoundEx("AlertMsg", LPGEN("Instant messages"), LPGEN("Incoming (New Session)"));
	SkinAddNewSoundEx("SendMsg", LPGEN("Instant messages"), LPGEN("Outgoing"));
	SkinAddNewSoundEx("SendError", LPGEN("Instant messages"), LPGEN("Message send error"));
	
	hCurSplitNS = LoadCursor(NULL, IDC_SIZENS);
	hCurSplitWE = LoadCursor(NULL, IDC_SIZEWE);
	hCurHyperlinkHand = LoadCursor(NULL, IDC_HAND);
	if (hCurHyperlinkHand == NULL)
		hCurHyperlinkHand = LoadCursor(g_hInst, MAKEINTRESOURCE(IDC_HYPERLINKHAND));

	InitStatusIcons();
	return 0;
}

static IRichEditOleCallbackVtbl reOleCallbackVtbl;
struct CREOleCallback reOleCallback;

static STDMETHODIMP_(ULONG) CREOleCallback_QueryInterface(struct CREOleCallback *lpThis, REFIID riid, LPVOID * ppvObj)
{
	if (IsEqualIID(riid, &IID_IRichEditOleCallback)) {
		*ppvObj = lpThis;
		lpThis->lpVtbl->AddRef((IRichEditOleCallback *) lpThis);
		return S_OK;
	}
	*ppvObj = NULL;
	return E_NOINTERFACE;
}

static STDMETHODIMP_(ULONG) CREOleCallback_AddRef(struct CREOleCallback *lpThis)
{
	if (lpThis->refCount == 0) {
		if (S_OK != StgCreateDocfile(NULL, STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE | STGM_DELETEONRELEASE, 0, &lpThis->pictStg))
			lpThis->pictStg = NULL;
		lpThis->nextStgId = 0;
	}
	return ++lpThis->refCount;
}

static STDMETHODIMP_(ULONG) CREOleCallback_Release(struct CREOleCallback *lpThis)
{
	if (--lpThis->refCount == 0) {
		if (lpThis->pictStg)
			lpThis->pictStg->lpVtbl->Release(lpThis->pictStg);
	}
	return lpThis->refCount;
}

static STDMETHODIMP_(HRESULT) CREOleCallback_ContextSensitiveHelp(struct CREOleCallback *lpThis, BOOL fEnterMode)
{
	return S_OK;
}

static STDMETHODIMP_(HRESULT) CREOleCallback_DeleteObject(struct CREOleCallback *lpThis, LPOLEOBJECT lpoleobj)
{
	return S_OK;
}

static STDMETHODIMP_(HRESULT) CREOleCallback_GetClipboardData(struct CREOleCallback *lpThis, CHARRANGE * lpchrg, DWORD reco, LPDATAOBJECT * lplpdataobj)
{
	return E_NOTIMPL;
}

static STDMETHODIMP_(HRESULT) CREOleCallback_GetContextMenu(struct CREOleCallback *lpThis, WORD seltype, LPOLEOBJECT lpoleobj, CHARRANGE * lpchrg, HMENU * lphmenu)
{
	return E_INVALIDARG;
}

static STDMETHODIMP_(HRESULT) CREOleCallback_GetDragDropEffect(struct CREOleCallback *lpThis, BOOL fDrag, DWORD grfKeyState, LPDWORD pdwEffect)
{
	return S_OK;
}

static STDMETHODIMP_(HRESULT) CREOleCallback_GetInPlaceContext(struct CREOleCallback *lpThis, LPOLEINPLACEFRAME * lplpFrame, LPOLEINPLACEUIWINDOW * lplpDoc, LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
	return E_INVALIDARG;
}

static STDMETHODIMP_(HRESULT) CREOleCallback_GetNewStorage(struct CREOleCallback *lpThis, LPSTORAGE * lplpstg)
{
	WCHAR szwName[64];
	char szName[64];
	wsprintfA(szName, "s%u", lpThis->nextStgId++);
	MultiByteToWideChar(CP_ACP, 0, szName, -1, szwName, SIZEOF(szwName));
	if (lpThis->pictStg == NULL)
		return STG_E_MEDIUMFULL;
	return lpThis->pictStg->lpVtbl->CreateStorage(lpThis->pictStg, szwName, STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, 0, lplpstg);
}

static STDMETHODIMP_(HRESULT) CREOleCallback_QueryAcceptData(struct CREOleCallback *lpThis, LPDATAOBJECT lpdataobj, CLIPFORMAT * lpcfFormat, DWORD reco, BOOL fReally, HGLOBAL hMetaPict)
{
	return S_OK;
}

static STDMETHODIMP_(HRESULT) CREOleCallback_QueryInsertObject(struct CREOleCallback *lpThis, LPCLSID lpclsid, LPSTORAGE lpstg, LONG cp)
{
	return S_OK;
}

static STDMETHODIMP_(HRESULT) CREOleCallback_ShowContainerUI(struct CREOleCallback *lpThis, BOOL fShow)
{
	return S_OK;
}

static void InitREOleCallback(void)
{
	reOleCallback.lpVtbl = &reOleCallbackVtbl;
	reOleCallback.lpVtbl->AddRef = (ULONG(__stdcall *) (IRichEditOleCallback *)) CREOleCallback_AddRef;
	reOleCallback.lpVtbl->Release = (ULONG(__stdcall *) (IRichEditOleCallback *)) CREOleCallback_Release;
	reOleCallback.lpVtbl->QueryInterface = (ULONG(__stdcall *) (IRichEditOleCallback *, REFIID, PVOID *)) CREOleCallback_QueryInterface;
	reOleCallback.lpVtbl->ContextSensitiveHelp = (HRESULT(__stdcall *) (IRichEditOleCallback *, BOOL)) CREOleCallback_ContextSensitiveHelp;
	reOleCallback.lpVtbl->DeleteObject = (HRESULT(__stdcall *) (IRichEditOleCallback *, LPOLEOBJECT)) CREOleCallback_DeleteObject;
	reOleCallback.lpVtbl->GetClipboardData = (HRESULT(__stdcall *) (IRichEditOleCallback *, CHARRANGE *, DWORD, LPDATAOBJECT *)) CREOleCallback_GetClipboardData;
	reOleCallback.lpVtbl->GetContextMenu = (HRESULT(__stdcall *) (IRichEditOleCallback *, WORD, LPOLEOBJECT, CHARRANGE *, HMENU *)) CREOleCallback_GetContextMenu;
	reOleCallback.lpVtbl->GetDragDropEffect = (HRESULT(__stdcall *) (IRichEditOleCallback *, BOOL, DWORD, LPDWORD)) CREOleCallback_GetDragDropEffect;
	reOleCallback.lpVtbl->GetInPlaceContext = (HRESULT(__stdcall *) (IRichEditOleCallback *, LPOLEINPLACEFRAME *, LPOLEINPLACEUIWINDOW *, LPOLEINPLACEFRAMEINFO))
		CREOleCallback_GetInPlaceContext;
	reOleCallback.lpVtbl->GetNewStorage = (HRESULT(__stdcall *) (IRichEditOleCallback *, LPSTORAGE *)) CREOleCallback_GetNewStorage;
	reOleCallback.lpVtbl->QueryAcceptData = (HRESULT(__stdcall *) (IRichEditOleCallback *, LPDATAOBJECT, CLIPFORMAT *, DWORD, BOOL, HGLOBAL)) CREOleCallback_QueryAcceptData;
	reOleCallback.lpVtbl->QueryInsertObject = (HRESULT(__stdcall *) (IRichEditOleCallback *, LPCLSID, LPSTORAGE, LONG)) CREOleCallback_QueryInsertObject;
	reOleCallback.lpVtbl->ShowContainerUI = (HRESULT(__stdcall *) (IRichEditOleCallback *, BOOL)) CREOleCallback_ShowContainerUI;
	reOleCallback.refCount = 0;
}
