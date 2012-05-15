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

extern void   Chat_Load();
extern void   Chat_Unload();
extern int    Chat_ModulesLoaded(WPARAM wParam,LPARAM lParam);
extern int    Chat_FontsChanged(WPARAM wParam,LPARAM lParam);
extern int    Chat_SmileyOptionsChanged(WPARAM wParam,LPARAM lParam);
extern int    Chat_PreShutdown(WPARAM wParam,LPARAM lParam);
extern int    Chat_IconsChanged(WPARAM wParam,LPARAM lParam);

extern int OptInitialise(WPARAM wParam, LPARAM lParam);
extern int FontServiceFontsChanged(WPARAM wParam, LPARAM lParam);
int StatusIconPressed(WPARAM wParam, LPARAM lParam);

static void InitREOleCallback(void);

HCURSOR hCurSplitNS, hCurSplitWE, hCurHyperlinkHand, hDragCursor;

HANDLE hMsgMenuItem, hHookWinEvt=NULL, hHookWinPopup=NULL;;

extern HINSTANCE g_hInst;
extern HWND GetParentWindow(HANDLE hContact, BOOL bChat);

PSLWA pSetLayeredWindowAttributes;

#ifdef __MINGW32__
// RichEdit interface GUIDs
const CLSID IID_IRichEditOle=
{ 0x00020D00, 0x00, 0x00,
    { 0xC0, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x46 } };

const CLSID IID_IRichEditOleCallback=
{ 0x00020D03, 0x00, 0x00,
    { 0xC0, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x46 } };

#endif

#define EVENTTYPE_SCRIVER 2010
#define SCRIVER_DB_GETEVENTTEXT "Scriver/GetText"

static int SRMMStatusToPf2(int status)
{
    switch (status) {
        case ID_STATUS_ONLINE:
            return PF2_ONLINE;
        case ID_STATUS_AWAY:
            return PF2_SHORTAWAY;
        case ID_STATUS_DND:
            return PF2_HEAVYDND;
        case ID_STATUS_NA:
            return PF2_LONGAWAY;
        case ID_STATUS_OCCUPIED:
            return PF2_LIGHTDND;
        case ID_STATUS_FREECHAT:
            return PF2_FREECHAT;
        case ID_STATUS_INVISIBLE:
            return PF2_INVISIBLE;
        case ID_STATUS_ONTHEPHONE:
            return PF2_ONTHEPHONE;
        case ID_STATUS_OUTTOLUNCH:
            return PF2_OUTTOLUNCH;
        case ID_STATUS_OFFLINE:
            return MODEF_OFFLINE;
    }
    return 0;
}

int IsAutoPopup(HANDLE hContact) {
	if (g_dat->flags & SMF_AUTOPOPUP) {
		char *szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
		if (strcmp(szProto, "MetaContacts") == 0 ) {
			hContact = (HANDLE)CallService(MS_MC_GETMOSTONLINECONTACT,(WPARAM)hContact, 0);
			if (hContact != NULL) {
				szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hContact,0);
			}
		}
		if (szProto && (g_dat->openFlags & SRMMStatusToPf2(CallProtoService(szProto, PS_GETSTATUS, 0, 0)))) {
			return 1;
		}
	}
	return 0;
}

static INT_PTR ReadMessageCommand(WPARAM wParam, LPARAM lParam)
{
	NewMessageWindowLParam newData = { 0 };
	HWND hwndExisting;
	HWND hParent;

	hwndExisting = WindowList_Find(g_dat->hMessageWindowList, ((CLISTEVENT *) lParam)->hContact);
	if (hwndExisting == NULL) {
		newData.hContact = ((CLISTEVENT *) lParam)->hContact;
		hParent = GetParentWindow(newData.hContact, FALSE);
		CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_MSG), hParent, DlgProcMessage, (LPARAM) & newData);
	} else {
		SendMessage(GetParent(hwndExisting), CM_POPUPWINDOW, 0, (LPARAM) hwndExisting);
	}
	return 0;
}

static int MessageEventAdded(WPARAM wParam, LPARAM lParam)
{
	DBEVENTINFO dbei = {0};
	HWND hwnd;

	dbei.cbSize = sizeof(dbei);
	CallService(MS_DB_EVENT_GET, lParam, (LPARAM) & dbei);
	if (dbei.eventType == EVENTTYPE_MESSAGE && (dbei.flags & DBEF_READ))
		return 0;
	hwnd = WindowList_Find(g_dat->hMessageWindowList, (HANDLE) wParam);
	if (hwnd) {
		SendMessage(hwnd, HM_DBEVENTADDED, wParam, lParam);
	}
	if (dbei.flags & DBEF_SENT || !DbEventIsMessageOrCustom(&dbei))
		return 0;

	CallServiceSync(MS_CLIST_REMOVEEVENT, wParam, (LPARAM) 1);
	/* does a window for the contact exist? */
	if (hwnd == NULL) {
		/* new message */
		SkinPlaySound("AlertMsg");
		if (IsAutoPopup((HANDLE) wParam)) {
			HWND hParent;
			NewMessageWindowLParam newData = { 0 };
			newData.hContact = (HANDLE) wParam;
			hParent = GetParentWindow(newData.hContact, FALSE);
			newData.flags = NMWLP_INCOMING;
			CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_MSG), hParent, DlgProcMessage, (LPARAM) & newData);
			return 0;
		}
	}
	if (hwnd == NULL || !IsWindowVisible(GetParent(hwnd))) 
	{
		CLISTEVENT cle = {0};
		TCHAR *contactName;
		TCHAR toolTip[256];

		cle.cbSize = sizeof(cle);
		cle.flags = CLEF_TCHAR;
		cle.hContact = (HANDLE) wParam;
		cle.hDbEvent = (HANDLE) lParam;
		cle.hIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
		cle.pszService = "SRMsg/ReadMessage";
		contactName = (TCHAR *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, wParam, GCDNF_TCHAR);
		mir_sntprintf(toolTip, SIZEOF(toolTip), TranslateT("Message from %s"), contactName);
		cle.ptszTooltip = toolTip;
		CallService(MS_CLIST_ADDEVENT, 0, (LPARAM) & cle);
	}
	return 0;
}

#if defined(_UNICODE)
static INT_PTR SendMessageCommandW(WPARAM wParam, LPARAM lParam)
{
   HWND hwnd;
   NewMessageWindowLParam newData = { 0 };

   {
      /* does the HCONTACT's protocol support IM messages? */
      char *szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0);
      if (szProto) {
         if (!CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IMSEND)
            return 1;
      }
      else {
         /* unknown contact */
         return 1;
      }                       //if
   }

   if ((hwnd = WindowList_Find(g_dat->hMessageWindowList, (HANDLE) wParam))) {
      if (lParam) {
         HWND hEdit;
         hEdit = GetDlgItem(hwnd, IDC_MESSAGE);
		 SendMessage(hEdit, EM_SETSEL, -1, SendMessage(hEdit, WM_GETTEXTLENGTH, 0, 0));
/*
		 SETTEXTEX  st;
		 st.flags = ST_SELECTION;
		 st.codepage = 1200;
		 SendMessage(hEdit, EM_SETTEXTEX, (WPARAM) &st, (LPARAM)lParam);
*/
         SendMessage(hEdit, EM_REPLACESEL, FALSE, (LPARAM) (TCHAR *) lParam);
      }
      SendMessage(GetParent(hwnd), CM_POPUPWINDOW, 0, (LPARAM) hwnd);
   } else {
      HWND hParent;
      newData.hContact = (HANDLE) wParam;
      newData.szInitialText = (const char *) lParam;
      newData.isWchar = 1;
      hParent = GetParentWindow(newData.hContact, FALSE);
      CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_MSG), hParent, DlgProcMessage, (LPARAM) & newData);
   }
   return 0;
}
#endif

static INT_PTR SendMessageCommand(WPARAM wParam, LPARAM lParam)
{
   HWND hwnd;
   NewMessageWindowLParam newData = { 0 };

   {
      char *szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0);
      //logInfo("Show message window for: %s (%s)", CallService(MS_CLIST_GETCONTACTDISPLAYNAME, wParam, 0), szProto);
      if (szProto) {
	      /* does the HCONTACT's protocol support IM messages? */
         if (!CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IMSEND)
            return 1;
      }
      else {
         /* unknown contact */
         return 1;
      }                       //if
   }

   if ((hwnd = WindowList_Find(g_dat->hMessageWindowList, (HANDLE) wParam))) {
      if (lParam) {
       HWND hEdit;
         hEdit = GetDlgItem(hwnd, IDC_MESSAGE);
		 SendMessage(hEdit, EM_SETSEL, -1, SendMessage(hEdit, WM_GETTEXTLENGTH, 0, 0));
/*
		 SETTEXTEX  st;
  		 st.flags = ST_SELECTION;
		 st.codepage = CP_ACP;
		 SendMessage(hEdit, EM_SETTEXTEX, (WPARAM) &st, (LPARAM)lParam);
*/
         SendMessageA(hEdit, EM_REPLACESEL, FALSE, (LPARAM) (char *) lParam);
      }
      SendMessage(GetParent(hwnd), CM_POPUPWINDOW, 0, (LPARAM) hwnd);
   } else {
      HWND hParent;
      newData.hContact = (HANDLE) wParam;
      newData.szInitialText = (const char *) lParam;
      newData.isWchar = 0;
      hParent = GetParentWindow(newData.hContact, FALSE);
      CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_MSG), hParent, DlgProcMessage, (LPARAM) & newData);
   }
   return 0;
}

static INT_PTR TypingMessageCommand(WPARAM wParam, LPARAM lParam)
{
   CLISTEVENT *cle = (CLISTEVENT *) lParam;

   if (!cle)
      return 0;
   SendMessageCommand((WPARAM) cle->hContact, 0);
   return 0;
}

static int TypingMessage(WPARAM wParam, LPARAM lParam)
{
   HWND hwnd;

   if (!(g_dat->flags2&SMF2_SHOWTYPING))
      return 0;
   if ((hwnd = WindowList_Find(g_dat->hMessageWindowList, (HANDLE) wParam))) {
      SendMessage(hwnd, DM_TYPING, 0, lParam);
   } else if ((int) lParam && (g_dat->flags2&SMF2_SHOWTYPINGTRAY)) {
      TCHAR szTip[256];

      mir_sntprintf(szTip, SIZEOF(szTip), TranslateT("%s is typing a message"), (TCHAR *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, wParam, GCDNF_TCHAR));
      if (ServiceExists(MS_CLIST_SYSTRAY_NOTIFY) && !(g_dat->flags2&SMF2_SHOWTYPINGCLIST)) {
         MIRANDASYSTRAYNOTIFY tn;
         tn.szProto = NULL;
         tn.cbSize = sizeof(tn);
         tn.tszInfoTitle = TranslateT("Typing Notification");
         tn.tszInfo = szTip;
#ifdef UNICODE
		 tn.dwInfoFlags = NIIF_INFO | NIIF_INTERN_UNICODE;
#else
		 tn.dwInfoFlags = NIIF_INFO;
#endif
		 tn.uTimeout = 1000 * 4;
         CallService(MS_CLIST_SYSTRAY_NOTIFY, 0, (LPARAM) & tn);
      }
      else {
		CLISTEVENT cle =  {0};

		cle.cbSize = sizeof(cle);
		cle.hContact = (HANDLE) wParam;
		cle.hDbEvent = (HANDLE) 1;
		cle.flags = CLEF_ONLYAFEW | CLEF_TCHAR;
		cle.hIcon = GetCachedIcon("scriver_TYPING");
		cle.pszService = "SRMsg/TypingMessage";
		cle.ptszTooltip = szTip;
		CallServiceSync(MS_CLIST_REMOVEEVENT, wParam, (LPARAM) 1);
		CallServiceSync(MS_CLIST_ADDEVENT, wParam, (LPARAM) & cle);
      }
   }
   return 0;
}

static int MessageSettingChanged(WPARAM wParam, LPARAM lParam)
{
   DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *) lParam;
   char *szProto;

   szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0);
   if (lstrcmpA(cws->szModule, "CList") && (szProto == NULL || lstrcmpA(cws->szModule, szProto)))
      return 0;
   WindowList_Broadcast(g_dat->hMessageWindowList, DM_CLISTSETTINGSCHANGED, wParam, lParam);
   return 0;
}

static int ContactDeleted(WPARAM wParam, LPARAM lParam)
{
   HWND hwnd;

   if ((hwnd = WindowList_Find(g_dat->hMessageWindowList, (HANDLE) wParam))) {
      SendMessage(hwnd, WM_CLOSE, 0, 0);
   }
   return 0;
}

static void RestoreUnreadMessageAlerts(void)
{
   CLISTEVENT cle = { 0 };
   DBEVENTINFO dbei = { 0 };
   TCHAR toolTip[256];
   int windowAlreadyExists;
   HANDLE hDbEvent, hContact;

   dbei.cbSize = sizeof(dbei);
   cle.cbSize = sizeof(cle);
   cle.hIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
   cle.pszService = "SRMsg/ReadMessage";
   cle.flags = CLEF_TCHAR;
   cle.ptszTooltip = toolTip;

   hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
   while (hContact) {
      hDbEvent = (HANDLE) CallService(MS_DB_EVENT_FINDFIRSTUNREAD, (WPARAM) hContact, 0);
      while (hDbEvent) {
         dbei.cbBlob = 0;
         CallService(MS_DB_EVENT_GET, (WPARAM) hDbEvent, (LPARAM) & dbei);
         if (!(dbei.flags & (DBEF_SENT | DBEF_READ)) && DbEventIsMessageOrCustom(&dbei)) {
            windowAlreadyExists = WindowList_Find(g_dat->hMessageWindowList, hContact) != NULL;
            if (windowAlreadyExists)
               continue;

			if (IsAutoPopup(hContact) && !windowAlreadyExists) {
               HWND hParent;
               NewMessageWindowLParam newData = { 0 };
               newData.hContact = hContact;
               newData.flags = NMWLP_INCOMING;
			   hParent = GetParentWindow(newData.hContact, FALSE);
               CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_MSG), hParent, DlgProcMessage, (LPARAM) & newData);
//               CreateDialogParam(g_hInst, MAKEINTRESOURCE(IDD_MSG), NULL, DlgProcMessage, (LPARAM) & newData);
            }
            else {
               cle.hContact = hContact;
               cle.hDbEvent = hDbEvent;
               mir_sntprintf(toolTip, SIZEOF(toolTip), TranslateT("Message from %s"), (char *) CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM) hContact, GCDNF_TCHAR));
               CallService(MS_CLIST_ADDEVENT, 0, (LPARAM) & cle);
            }
         }
         hDbEvent = (HANDLE) CallService(MS_DB_EVENT_FINDNEXT, (WPARAM) hDbEvent, 0);
      }
      hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
   }
}

static INT_PTR GetWindowAPI(WPARAM wParam, LPARAM lParam)
{
   return PLUGIN_MAKE_VERSION(0,0,0,3);
}

static INT_PTR GetWindowClass(WPARAM wParam, LPARAM lParam)
{
   char *szBuf = (char*)wParam;
   int size = (int)lParam;
   mir_snprintf(szBuf, size, "Scriver");
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
   if (hwnd == NULL)
	   hwnd = SM_FindWindowByContact(mwid->hContact);
   mwd->uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;
   mwd->hwndWindow = hwnd;
   mwd->local = 0;
   mwd->uState = SendMessage(hwnd, DM_GETWINDOWSTATE, 0, 0);
   return 0;
}

static int PrebuildContactMenu(WPARAM wParam, LPARAM lParam) {
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

static int AvatarChanged(WPARAM wParam, LPARAM lParam) {
   if (wParam == 0) {         // protocol picture has changed...
      WindowList_Broadcast(g_dat->hMessageWindowList, DM_AVATARCHANGED, wParam, lParam);
   } else {
      HWND hwnd = WindowList_Find(g_dat->hMessageWindowList, (HANDLE)wParam);
      SendMessage(hwnd, DM_AVATARCHANGED, wParam, lParam);
   }
    return 0;
}

static void RegisterStatusIcons() {
	StatusIconData sid;
	sid.cbSize = sizeof(sid);
	sid.szModule = SRMMMOD;

	sid.dwId = 1;
	sid.hIcon = CopyIcon(GetCachedIcon("scriver_TYPING"));
	sid.hIconDisabled = CopyIcon(GetCachedIcon("scriver_TYPINGOFF"));
	sid.flags = MBF_HIDDEN;
	sid.szTooltip = NULL;
	AddStickyStatusIcon((WPARAM) 0, (LPARAM) &sid);

#if !defined( _UNICODE )
	sid.dwId = 0;
	sid.hIcon = CopyIcon(GetCachedIcon("scriver_UNICODEOFF"));
	sid.hIconDisabled = CopyIcon(GetCachedIcon("scriver_UNICODEOFF"));
	sid.flags = 0;
	sid.szTooltip = NULL;
	AddStickyStatusIcon((WPARAM) 0, (LPARAM) &sid);
#endif
}

void ChangeStatusIcons() {
	StatusIconData sid;
	sid.cbSize = sizeof(sid);
	sid.szModule = SRMMMOD;
	sid.dwId = 0;
	sid.hIcon = CopyIcon(GetCachedIcon("scriver_UNICODEON"));
	sid.hIconDisabled = CopyIcon(GetCachedIcon("scriver_UNICODEOFF"));
	sid.flags = 0;
	sid.szTooltip = NULL;
	ModifyStatusIcon((WPARAM)NULL, (LPARAM) &sid);

	sid.dwId = 1;
	sid.hIcon = CopyIcon(GetCachedIcon("scriver_TYPING"));
	sid.hIconDisabled = CopyIcon(GetCachedIcon("scriver_TYPINGOFF"));
	sid.flags = MBF_HIDDEN;
	sid.szTooltip = NULL;
	ModifyStatusIcon((WPARAM)NULL, (LPARAM) &sid);
}

int StatusIconPressed(WPARAM wParam, LPARAM lParam) {
//	HANDLE hContact = (HANDLE) wParam;
	StatusIconClickData *sicd = (StatusIconClickData *) lParam;
	HWND hwnd = WindowList_Find(g_dat->hMessageWindowList, (HANDLE)wParam);
	if (hwnd == NULL) {
		hwnd = SM_FindWindowByContact((HANDLE)wParam);

	}
	if (hwnd != NULL) {
		if (!strcmp(SRMMMOD, sicd->szModule)) {
			if (sicd->dwId == 0 && g_dat->hMenuANSIEncoding) {
				if (sicd->flags & MBCF_RIGHTBUTTON) {
					int codePage = (int) SendMessage(hwnd, DM_GETCODEPAGE, 0, 0);
					if (codePage != 1200) {
						int i, iSel;
						for (i = 0; i < GetMenuItemCount(g_dat->hMenuANSIEncoding); i++) {
							CheckMenuItem (g_dat->hMenuANSIEncoding, i, MF_BYPOSITION | MF_UNCHECKED);
						}
						if (codePage == CP_ACP) {
							CheckMenuItem(g_dat->hMenuANSIEncoding, 0, MF_BYPOSITION | MF_CHECKED);
						} else {
							CheckMenuItem(g_dat->hMenuANSIEncoding, codePage, MF_BYCOMMAND | MF_CHECKED);
						}
						iSel = TrackPopupMenu(g_dat->hMenuANSIEncoding, TPM_RETURNCMD, sicd->clickLocation.x, sicd->clickLocation.y, 0, GetParent(hwnd), NULL);
						if (iSel >= 500) {
							if (iSel == 500) iSel = CP_ACP;
							SendMessage(hwnd, DM_SETCODEPAGE, 0, iSel);
						}
					}
				}
			} else {
				SendMessage(hwnd, DM_SWITCHTYPING, 0, 0);
			}
		}
	}
	return 0;
}


static int OnModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	CLISTMENUITEM mi;
	ReloadGlobals();
	RegisterIcons();
	RegisterFontServiceFonts();
	RegisterKeyBindings();
	LoadGlobalIcons();
	LoadMsgLogIcons();
	ZeroMemory(&mi, sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.position = -2000090000;
	if ( ServiceExists( MS_SKIN2_GETICONBYHANDLE )) {
		mi.flags = CMIF_ICONFROMICOLIB | CMIF_DEFAULT;
		mi.icolibItem = LoadSkinnedIconHandle( SKINICON_EVENT_MESSAGE );
	}
	else {
		mi.flags = CMIF_DEFAULT;
		mi.hIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
	}
	mi.pszName = LPGEN("&Message");
	mi.pszService = MS_MSG_SENDMESSAGE;
	hMsgMenuItem = (HANDLE) CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM) & mi);
	CallService(MS_SKIN2_RELEASEICON,(WPARAM)mi.hIcon, 0);

	HookEvent_Ex(ME_SMILEYADD_OPTIONSCHANGED, SmileySettingsChanged);
	HookEvent_Ex(ME_IEVIEW_OPTIONSCHANGED, SmileySettingsChanged);
	HookEvent_Ex(ME_AV_AVATARCHANGED, AvatarChanged);
	HookEvent_Ex(ME_FONT_RELOAD, FontServiceFontsChanged);
	HookEvent_Ex(ME_MSG_ICONPRESSED, StatusIconPressed);

	RestoreUnreadMessageAlerts();
	Chat_ModulesLoaded(wParam, lParam);
	RegisterStatusIcons();
	return 0;
}

int OnSystemPreshutdown(WPARAM wParam, LPARAM lParam)
{
	Chat_PreShutdown(wParam, lParam);
	WindowList_BroadcastAsync(g_dat->hMessageWindowList, WM_CLOSE, 0, 0);
	DeinitStatusIcons();
	return 0;
}

int OnUnloadModule(void)
{
	Chat_Unload();
	DestroyCursor(hCurSplitNS);
	DestroyCursor(hCurHyperlinkHand);
	DestroyCursor(hCurSplitWE);
	DestroyCursor(hDragCursor);
	UnhookEvents_Ex();
	DestroyServices_Ex();
	DestroyHookableEvent(hHookWinEvt);
	DestroyHookableEvent(hHookWinPopup);
	ReleaseIcons();
	FreeMsgLogIcons();
	FreeLibrary(GetModuleHandleA("riched20.dll"));
	OleUninitialize();
	RichUtil_Unload();
	FreeGlobals();
	return 0;
}

int OnLoadModule(void) {
	HMODULE	hDLL = 0;
	if (LoadLibraryA("riched20.dll") == NULL) {
		if (IDYES !=
			MessageBox(0,
					TranslateT
					("Miranda could not load the built-in message module, riched20.dll is missing. If you are using Windows 95 or WINE please make sure you have riched20.dll installed. Press 'Yes' to continue loading Miranda."),
					TranslateT("Information"), MB_YESNO | MB_ICONINFORMATION))
			return 1;
		return 0;
	}
	hDLL = GetModuleHandle(_T("user32"));
	pSetLayeredWindowAttributes = (PSLWA) GetProcAddress(hDLL,"SetLayeredWindowAttributes");

	InitGlobals();
	RichUtil_Load();
	OleInitialize(NULL);
	InitREOleCallback();
	InitStatusIcons();

	HookEvent_Ex(ME_OPT_INITIALISE, OptInitialise);
	HookEvent_Ex(ME_DB_EVENT_ADDED, MessageEventAdded);
	HookEvent_Ex(ME_DB_CONTACT_SETTINGCHANGED, MessageSettingChanged);
	HookEvent_Ex(ME_DB_CONTACT_DELETED, ContactDeleted);
	HookEvent_Ex(ME_SYSTEM_MODULESLOADED, OnModulesLoaded);
	HookEvent_Ex(ME_SKIN_ICONSCHANGED, IconsChanged);
	HookEvent_Ex(ME_PROTO_CONTACTISTYPING, TypingMessage);
	HookEvent_Ex(ME_SYSTEM_PRESHUTDOWN, OnSystemPreshutdown);
	HookEvent_Ex(ME_CLIST_PREBUILDCONTACTMENU, PrebuildContactMenu);

	CreateServiceFunction_Ex(MS_MSG_SENDMESSAGE, SendMessageCommand);
 #if defined(_UNICODE)
	CreateServiceFunction_Ex(MS_MSG_SENDMESSAGEW, SendMessageCommandW);
 #endif
	CreateServiceFunction_Ex(MS_MSG_GETWINDOWAPI, GetWindowAPI);
	CreateServiceFunction_Ex(MS_MSG_GETWINDOWCLASS, GetWindowClass);
	CreateServiceFunction_Ex(MS_MSG_GETWINDOWDATA, GetWindowData);
	CreateServiceFunction_Ex("SRMsg/ReadMessage", ReadMessageCommand);
	CreateServiceFunction_Ex("SRMsg/TypingMessage", TypingMessageCommand);

	hHookWinEvt = CreateHookableEvent(ME_MSG_WINDOWEVENT);
	hHookWinPopup = CreateHookableEvent(ME_MSG_WINDOWPOPUP);
	SkinAddNewSoundEx("RecvMsgActive", LPGEN("Instant messages"), LPGEN("Incoming (Focused Window)"));
	SkinAddNewSoundEx("RecvMsgInactive", LPGEN("Instant messages"), LPGEN("Incoming (Unfocused Window)"));
	SkinAddNewSoundEx("AlertMsg", LPGEN("Instant messages"), LPGEN("Incoming (New Session)"));
	SkinAddNewSoundEx("SendMsg", LPGEN("Instant messages"), LPGEN("Outgoing"));
	hCurSplitNS = LoadCursor(NULL, IDC_SIZENS);
	hCurSplitWE = LoadCursor(NULL, IDC_SIZEWE);
	hCurHyperlinkHand = LoadCursor(NULL, IDC_HAND);
	if (hCurHyperlinkHand == NULL)
		hCurHyperlinkHand = LoadCursor(g_hInst, MAKEINTRESOURCE(IDC_HYPERLINKHAND));
	hDragCursor = LoadCursor(g_hInst,  MAKEINTRESOURCE(IDC_DRAGCURSOR));


	Chat_Load();
	return 0;
}

static IRichEditOleCallbackVtbl reOleCallbackVtbl;
struct CREOleCallback reOleCallback;
static IRichEditOleCallbackVtbl reOleCallbackVtbl2;
struct CREOleCallback reOleCallback2;

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
   MultiByteToWideChar(CP_ACP, 0, szName, -1, szwName, sizeof(szwName) / sizeof(szwName[0]));
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

static STDMETHODIMP_(HRESULT) CREOleCallback_QueryAcceptData2(struct CREOleCallback *lpThis, LPDATAOBJECT lpdataobj, CLIPFORMAT * lpcfFormat, DWORD reco, BOOL fReally, HGLOBAL hMetaPict)
{
	*lpcfFormat = CF_TEXT;
   return S_OK;
}


static void InitREOleCallback(void)
{
   reOleCallback.lpVtbl = &reOleCallbackVtbl;
   reOleCallback.lpVtbl->AddRef = (ULONG(__stdcall *) (IRichEditOleCallback *)) CREOleCallback_AddRef;
   reOleCallback.lpVtbl->Release = (ULONG(__stdcall *) (IRichEditOleCallback *)) CREOleCallback_Release;
   reOleCallback.lpVtbl->QueryInterface = (HRESULT(__stdcall *) (IRichEditOleCallback *, REFIID, PVOID *)) CREOleCallback_QueryInterface;
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

   reOleCallback2.lpVtbl = &reOleCallbackVtbl2;
   reOleCallback2.lpVtbl->AddRef = (ULONG(__stdcall *) (IRichEditOleCallback *)) CREOleCallback_AddRef;
   reOleCallback2.lpVtbl->Release = (ULONG(__stdcall *) (IRichEditOleCallback *)) CREOleCallback_Release;
   reOleCallback2.lpVtbl->QueryInterface = (HRESULT(__stdcall *) (IRichEditOleCallback *, REFIID, PVOID *)) CREOleCallback_QueryInterface;
   reOleCallback2.lpVtbl->ContextSensitiveHelp = (HRESULT(__stdcall *) (IRichEditOleCallback *, BOOL)) CREOleCallback_ContextSensitiveHelp;
   reOleCallback2.lpVtbl->DeleteObject = (HRESULT(__stdcall *) (IRichEditOleCallback *, LPOLEOBJECT)) CREOleCallback_DeleteObject;
   reOleCallback2.lpVtbl->GetClipboardData = (HRESULT(__stdcall *) (IRichEditOleCallback *, CHARRANGE *, DWORD, LPDATAOBJECT *)) CREOleCallback_GetClipboardData;
   reOleCallback2.lpVtbl->GetContextMenu = (HRESULT(__stdcall *) (IRichEditOleCallback *, WORD, LPOLEOBJECT, CHARRANGE *, HMENU *)) CREOleCallback_GetContextMenu;
   reOleCallback2.lpVtbl->GetDragDropEffect = (HRESULT(__stdcall *) (IRichEditOleCallback *, BOOL, DWORD, LPDWORD)) CREOleCallback_GetDragDropEffect;
   reOleCallback2.lpVtbl->GetInPlaceContext = (HRESULT(__stdcall *) (IRichEditOleCallback *, LPOLEINPLACEFRAME *, LPOLEINPLACEUIWINDOW *, LPOLEINPLACEFRAMEINFO))
      CREOleCallback_GetInPlaceContext;
   reOleCallback2.lpVtbl->GetNewStorage = (HRESULT(__stdcall *) (IRichEditOleCallback *, LPSTORAGE *)) CREOleCallback_GetNewStorage;
   reOleCallback2.lpVtbl->QueryAcceptData = (HRESULT(__stdcall *) (IRichEditOleCallback *, LPDATAOBJECT, CLIPFORMAT *, DWORD, BOOL, HGLOBAL)) CREOleCallback_QueryAcceptData2;
   reOleCallback2.lpVtbl->QueryInsertObject = (HRESULT(__stdcall *) (IRichEditOleCallback *, LPCLSID, LPSTORAGE, LONG)) CREOleCallback_QueryInsertObject;
   reOleCallback2.lpVtbl->ShowContainerUI = (HRESULT(__stdcall *) (IRichEditOleCallback *, BOOL)) CREOleCallback_ShowContainerUI;
   reOleCallback2.refCount = 0;
}
