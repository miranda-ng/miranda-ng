/*
IRC plugin for Miranda IM

Copyright (C) 2003-05 Jurgen Persson
Copyright (C) 2007-09 George Hazan

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

#include "irc.h"

BOOL bChatInstalled = FALSE, m_bMbotInstalled = FALSE;

void CIrcProto::InitMainMenus(void)
{
	char temp[ MAXMODULELABELLENGTH ];
	char *d = temp + sprintf( temp, m_szModuleName );

	CLISTMENUITEM mi = { 0 };
	mi.cbSize = sizeof( mi );
	mi.pszService = temp;

	if ( bChatInstalled ) {
		HGENMENU hRoot = MO_GetProtoRootMenu( m_szModuleName );
		if ( hRoot == NULL ) {
			// Root popupmenuitem
			mi.ptszName = m_tszUserName;
			mi.position = -1999901010;
			mi.hParentMenu = HGENMENU_ROOT;
			mi.flags = CMIF_ICONFROMICOLIB | CMIF_ROOTPOPUP | CMIF_TCHAR | CMIF_KEEPUNTRANSLATED;
			mi.icolibItem = GetIconHandle(IDI_MAIN);
			hRoot = hMenuRoot = ( HGENMENU )CallService( MS_CLIST_ADDPROTOMENUITEM,  (WPARAM)0, (LPARAM)&mi);
		}
		else {
			if (hMenuRoot)
				CallService( MS_CLIST_REMOVEMAINMENUITEM, ( WPARAM )hMenuRoot, 0 );
			hMenuRoot = NULL;
		}
		
		mi.flags = CMIF_ICONFROMICOLIB | CMIF_CHILDPOPUP;
		mi.pszName = LPGEN("&Quick connect");
		mi.icolibItem = GetIconHandle(IDI_QUICK);
		strcpy( d, IRC_QUICKCONNECT );
		mi.position = 201001;
		mi.hParentMenu = hRoot;
		hMenuQuick = ( HGENMENU )CallService( MS_CLIST_ADDPROTOMENUITEM, (WPARAM)0, (LPARAM)&mi);

		if (m_iStatus != ID_STATUS_OFFLINE) mi.flags |= CMIF_GRAYED;

		mi.pszName = LPGEN("&Join channel");
		mi.icolibItem = LoadSkinnedIconHandle(SKINICON_CHAT_JOIN);//GetIconHandle(IDI_JOIN);
		strcpy( d, IRC_JOINCHANNEL );
		mi.position = 201002;
		hMenuJoin = ( HGENMENU )CallService( MS_CLIST_ADDPROTOMENUITEM, (WPARAM)0, (LPARAM)&mi);

		mi.pszName = LPGEN("&Change your nickname");
		mi.icolibItem = GetIconHandle(IDI_RENAME);
		strcpy( d, IRC_CHANGENICK );
		mi.position = 201003;
		hMenuNick = ( HGENMENU )CallService( MS_CLIST_ADDPROTOMENUITEM, (WPARAM)0, (LPARAM)&mi);

		mi.pszName = LPGEN("Show the &list of available channels");
		mi.icolibItem = GetIconHandle(IDI_LIST);
		strcpy( d, IRC_SHOWLIST );
		mi.position = 201004;
		hMenuList = ( HGENMENU )CallService( MS_CLIST_ADDPROTOMENUITEM, (WPARAM)0, (LPARAM)&mi);

		if (m_useServer) mi.flags &= ~CMIF_GRAYED;
		mi.pszName = LPGEN("&Show the server window");
		mi.icolibItem = GetIconHandle(IDI_SERVER);
		strcpy( d, IRC_SHOWSERVER );
		mi.position = 201005;
		hMenuServer = ( HGENMENU )CallService( MS_CLIST_ADDPROTOMENUITEM, (WPARAM)0, (LPARAM)&mi);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

static HGENMENU hUMenuChanSettings, hUMenuWhois, hUMenuDisconnect, hUMenuIgnore;
static HANDLE hPreBuildContactMenu, hMenuChanSettings, hMenuWhois, hMenuDisconnect, hMenuIgnore;

static CIrcProto* IrcGetInstanceByHContact(HANDLE hContact)
{
	char* szProto = ( char* )CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
	if (szProto == NULL)
		return NULL;

	for (int i = 0; i < g_Instances.getCount(); i++)
		if (!strcmp(szProto, g_Instances[i]->m_szModuleName))
			return g_Instances[i];

	return NULL;
}

static INT_PTR IrcMenuChanSettings(WPARAM wParam, LPARAM lParam)
{
	CIrcProto* ppro = IrcGetInstanceByHContact((HANDLE)wParam);
	return (ppro) ? ppro->OnMenuChanSettings(wParam, lParam) : 0;
}

static INT_PTR IrcMenuWhois(WPARAM wParam, LPARAM lParam)
{
	CIrcProto* ppro = IrcGetInstanceByHContact((HANDLE)wParam);
	return (ppro) ? ppro->OnMenuWhois(wParam, lParam) : 0;
}

static INT_PTR IrcMenuDisconnect(WPARAM wParam, LPARAM lParam)
{
	CIrcProto* ppro = IrcGetInstanceByHContact((HANDLE)wParam);
	return (ppro) ? ppro->OnMenuDisconnect(wParam, lParam) : 0;
}

static INT_PTR IrcMenuIgnore(WPARAM wParam, LPARAM lParam)
{
	CIrcProto* ppro = IrcGetInstanceByHContact((HANDLE)wParam);
	return (ppro) ? ppro->OnMenuIgnore(wParam, lParam) : 0;
}

int IrcPrebuildContactMenu( WPARAM wParam, LPARAM lParam )
{
	CLISTMENUITEM clmi = {0};
	clmi.cbSize = sizeof( CLISTMENUITEM );
	clmi.flags = CMIM_FLAGS | CMIF_HIDDEN;

	CallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hUMenuChanSettings, ( LPARAM )&clmi );
	CallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hUMenuWhois,        ( LPARAM )&clmi );
	CallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hUMenuDisconnect,   ( LPARAM )&clmi );
	CallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hUMenuIgnore,       ( LPARAM )&clmi );
	
	CIrcProto* ppro = IrcGetInstanceByHContact((HANDLE)wParam);
	return (ppro) ? ppro->OnMenuPreBuild(wParam, lParam) : 0;
}

void InitContactMenus(void)
{
	char temp[MAXMODULELABELLENGTH];
	char *d = temp + sprintf(temp, "IRC");

	CLISTMENUITEM mi = {0};
	mi.cbSize = sizeof(mi);
	mi.pszService = temp;
	mi.flags = CMIF_ICONFROMICOLIB;

	mi.pszName = LPGEN("Channel &settings");
	mi.icolibItem = GetIconHandle(IDI_MANAGER);
	strcpy(d, IRC_UM_CHANSETTINGS);
	mi.popupPosition = 500090002;
	hUMenuChanSettings = (HGENMENU)CallService(MS_CLIST_ADDCONTACTMENUITEM, (WPARAM)0, (LPARAM)&mi);
	hMenuChanSettings = CreateServiceFunction(temp, IrcMenuChanSettings);

	mi.pszName = LPGEN("&WhoIs info");
	mi.icolibItem = GetIconHandle(IDI_WHOIS);
	strcpy(d, IRC_UM_WHOIS);
	mi.popupPosition = 500090001;
	hUMenuWhois = (HGENMENU)CallService(MS_CLIST_ADDCONTACTMENUITEM, (WPARAM)0, (LPARAM)&mi);
	hMenuWhois = CreateServiceFunction(temp, IrcMenuWhois);

	mi.pszName = LPGEN("Di&sconnect");
	mi.icolibItem = GetIconHandle(IDI_DELETE);
	strcpy(d, IRC_UM_DISCONNECT);
	mi.popupPosition = 500090001;
	hUMenuDisconnect = (HGENMENU)CallService(MS_CLIST_ADDCONTACTMENUITEM, (WPARAM)0, (LPARAM)&mi);
	hMenuDisconnect = CreateServiceFunction(temp, IrcMenuDisconnect);

	mi.pszName = LPGEN("&Add to ignore list");
	mi.icolibItem = GetIconHandle(IDI_BLOCK);
	strcpy(d, IRC_UM_IGNORE);
	mi.popupPosition = 500090002;
	hUMenuIgnore = (HGENMENU)CallService( MS_CLIST_ADDCONTACTMENUITEM, (WPARAM)0, (LPARAM)&mi);
	hMenuIgnore = CreateServiceFunction( temp, IrcMenuIgnore );

	hPreBuildContactMenu = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, IrcPrebuildContactMenu);
}

void UninitContactMenus(void)
{
	UnhookEvent(hPreBuildContactMenu);
	CallService(MS_CLIST_REMOVECONTACTMENUITEM, (WPARAM)hUMenuChanSettings, 0);
	CallService(MS_CLIST_REMOVECONTACTMENUITEM, (WPARAM)hUMenuWhois, 0);
	CallService(MS_CLIST_REMOVECONTACTMENUITEM, (WPARAM)hUMenuDisconnect, 0);
	CallService(MS_CLIST_REMOVECONTACTMENUITEM, (WPARAM)hUMenuIgnore, 0);
	DestroyServiceFunction(hMenuChanSettings);
	DestroyServiceFunction(hMenuWhois);
	DestroyServiceFunction(hMenuDisconnect);
	DestroyServiceFunction(hMenuIgnore);
}

INT_PTR __cdecl CIrcProto::OnDoubleclicked(WPARAM, LPARAM lParam)
{
	if (!lParam)
		return 0;

	CLISTEVENT* pcle = (CLISTEVENT*)lParam;

	if ( getByte((HANDLE) pcle->hContact, "DCC", 0) != 0) {
		DCCINFO* pdci = ( DCCINFO* )pcle->lParam;
		CMessageBoxDlg* dlg = new CMessageBoxDlg( this, pdci );
		dlg->Show();
		HWND hWnd = dlg->GetHwnd();
		TCHAR szTemp[500];
		mir_sntprintf( szTemp, SIZEOF(szTemp), TranslateT("%s (%s) is requesting a client-to-client chat connection."),
			pdci->sContactName.c_str(), pdci->sHostmask.c_str());
		SetDlgItemText( hWnd, IDC_TEXT, szTemp );
		ShowWindow( hWnd, SW_SHOW );
		return 1;
	}
	return 0;
}

int __cdecl CIrcProto::OnContactDeleted(WPARAM wp, LPARAM)
{
	HANDLE hContact = ( HANDLE )wp;
	if ( !hContact )
		return 0;

	DBVARIANT dbv;
	if ( !getTString( hContact, "Nick", &dbv )) {
		int type = getByte( hContact, "ChatRoom", 0 );
		if ( type != 0 ) {
			GCEVENT gce = {0};
			GCDEST gcd = {0};
			CMString S = _T("");
			if (type == GCW_CHATROOM)
				S = MakeWndID( dbv.ptszVal );
			if (type == GCW_SERVER)
				S = SERVERWINDOW;
			gce.cbSize = sizeof(GCEVENT);
			gce.dwItemData = 0;
			gcd.iType = GC_EVENT_CONTROL;
			gcd.pszModule = m_szModuleName;
			gce.dwFlags = GC_TCHAR;
			gce.pDest = &gcd;
			gcd.ptszID = ( TCHAR* )S.c_str();
			int i = CallChatEvent( SESSION_TERMINATE, (LPARAM)&gce);
			if (i && type == GCW_CHATROOM)
				PostIrcMessage( _T("/PART %s %s"), dbv.ptszVal, m_userInfo);
		}
		else {
			BYTE bDCC = getByte(( HANDLE )wp, "DCC", 0) ;
			if ( bDCC ) {
				CDccSession* dcc = FindDCCSession((HANDLE)wp);
				if ( dcc )
					dcc->Disconnect();
		}	}

		DBFreeVariant(&dbv);
	}
	return 0;
}

INT_PTR __cdecl CIrcProto::OnJoinChat(WPARAM wp, LPARAM)
{
	if (!wp )
		return 0;

	DBVARIANT dbv;
	if ( !getTString(( HANDLE )wp, "Nick", &dbv)) {
		if ( getByte(( HANDLE )wp, "ChatRoom", 0) == GCW_CHATROOM)
			PostIrcMessage( _T("/JOIN %s"), dbv.ptszVal);
		DBFreeVariant(&dbv);
	}
	return 0;
}

INT_PTR __cdecl CIrcProto::OnLeaveChat(WPARAM wp, LPARAM)
{
	if (!wp )
		return 0;

	DBVARIANT dbv;
	if ( !getTString(( HANDLE )wp, "Nick", &dbv)) {
		if ( getByte(( HANDLE )wp, "ChatRoom", 0) == GCW_CHATROOM) {
			PostIrcMessage( _T("/PART %s %s"), dbv.ptszVal, m_userInfo);

			GCEVENT gce = {0};
			GCDEST gcd = {0};
			CMString S = MakeWndID(dbv.ptszVal);
			gce.cbSize = sizeof(GCEVENT);
			gce.dwFlags = GC_TCHAR;
			gcd.iType = GC_EVENT_CONTROL;
			gcd.pszModule = m_szModuleName;
			gce.pDest = &gcd;
			gcd.ptszID = ( TCHAR* )S.c_str();
			CallChatEvent( SESSION_TERMINATE, (LPARAM)&gce);
		}
		DBFreeVariant(&dbv);
	}
	return 0;
}

INT_PTR __cdecl CIrcProto::OnMenuChanSettings(WPARAM wp, LPARAM)
{
	if (!wp )
		return 0;

	HANDLE hContact = (HANDLE) wp;
	DBVARIANT dbv;
	if ( !getTString( hContact, "Nick", &dbv )) {
		PostIrcMessageWnd(dbv.ptszVal, NULL, _T("/CHANNELMANAGER"));
		DBFreeVariant(&dbv);
	}
	return 0;
}

INT_PTR __cdecl CIrcProto::OnMenuWhois(WPARAM wp, LPARAM)
{
	if ( !wp )
		return 0;

	DBVARIANT dbv;

	if ( !getTString(( HANDLE )wp, "Nick", &dbv)) {
		PostIrcMessage( _T("/WHOIS %s %s"), dbv.ptszVal, dbv.ptszVal);
		DBFreeVariant(&dbv);
	}
	return 0;
}

INT_PTR __cdecl CIrcProto::OnMenuDisconnect(WPARAM wp, LPARAM)
{
	CDccSession* dcc = FindDCCSession((HANDLE)wp);
	if ( dcc )
		dcc->Disconnect();
	return 0;
}

INT_PTR __cdecl CIrcProto::OnMenuIgnore(WPARAM wp, LPARAM)
{
	if ( !wp )
		return 0;

	HANDLE hContact = (HANDLE) wp;
	DBVARIANT dbv;
	if ( !getTString( hContact, "Nick", &dbv )) {
		if ( getByte(( HANDLE )wp, "ChatRoom", 0) == 0 ) {
			char* host = NULL;
			DBVARIANT dbv1;
			if ( !getString((HANDLE) wp, "Host", &dbv1))
				host = dbv1.pszVal;

			if ( host ) {
				String S;
				if (m_ignoreChannelDefault)
					S = "+qnidcm";
				else
					S = "+qnidc";
				PostIrcMessage( _T("/IGNORE %%question=\"%s\",\"%s\",\"*!*@") _T(TCHAR_STR_PARAM) _T("\" %s"),
					TranslateT("Please enter the hostmask (nick!user@host) \nNOTE! Contacts on your contact list are never ignored"),
					TranslateT("Ignore"), host, S.c_str());
				DBFreeVariant(&dbv1);
			}
		}
		DBFreeVariant(&dbv);
	}
	return 0;
}

INT_PTR __cdecl CIrcProto::OnJoinMenuCommand(WPARAM, LPARAM)
{
	if ( !m_joinDlg ) {
		m_joinDlg = new CJoinDlg( this );
		m_joinDlg->Show();
	}

	SetDlgItemText( m_joinDlg->GetHwnd(), IDC_CAPTION, TranslateT("Join channel"));
	SetWindowText( GetDlgItem( m_joinDlg->GetHwnd(), IDC_TEXT), TranslateT("Please enter a channel to join"));
	SendMessage( GetDlgItem( m_joinDlg->GetHwnd(), IDC_ENICK), EM_SETSEL, 0,MAKELPARAM(0,-1));
	ShowWindow( m_joinDlg->GetHwnd(), SW_SHOW);
	SetActiveWindow( m_joinDlg->GetHwnd());
	return 0;
}

INT_PTR __cdecl CIrcProto::OnQuickConnectMenuCommand(WPARAM, LPARAM)
{
	if ( !m_quickDlg ) {
		m_quickDlg = new CQuickDlg( this );
		m_quickDlg->Show();

		SetWindowText( m_quickDlg->GetHwnd(), TranslateT( "Quick connect" ));
		SetDlgItemText( m_quickDlg->GetHwnd(), IDC_TEXT, TranslateT( "Please select IRC network and enter the password if needed" ));
		SetDlgItemText( m_quickDlg->GetHwnd(), IDC_CAPTION, TranslateT( "Quick connect" ));
		WindowSetIcon( m_quickDlg->GetHwnd(), IDI_QUICK );
	}

	ShowWindow( m_quickDlg->GetHwnd(), SW_SHOW );
	SetActiveWindow( m_quickDlg->GetHwnd() );
	return 0;
}

INT_PTR __cdecl CIrcProto::OnShowListMenuCommand(WPARAM, LPARAM)
{
	PostIrcMessage( _T("/LIST"));
	return 0;
}

INT_PTR __cdecl CIrcProto::OnShowServerMenuCommand(WPARAM, LPARAM)
{
	GCEVENT gce = {0};
	GCDEST gcd = {0};
	gcd.iType = GC_EVENT_CONTROL;
	gcd.ptszID = SERVERWINDOW;
	gce.dwFlags = GC_TCHAR;
	gcd.pszModule = m_szModuleName;
	gce.cbSize = sizeof(GCEVENT);
	gce.pDest = &gcd;
	CallChatEvent( WINDOW_VISIBLE, (LPARAM)&gce);
	return 0;
}

INT_PTR __cdecl CIrcProto::OnChangeNickMenuCommand(WPARAM, LPARAM)
{
	if ( !m_nickDlg ) {
		m_nickDlg = new CNickDlg( this );
		m_nickDlg->Show();
	}

	SetDlgItemText( m_nickDlg->GetHwnd(), IDC_CAPTION, TranslateT("Change nick name"));
	SetWindowText( GetDlgItem( m_nickDlg->GetHwnd(), IDC_TEXT), TranslateT("Please enter a unique nickname"));
	m_nickDlg->m_Enick.SetText( m_info.sNick.c_str());
	m_nickDlg->m_Enick.SendMsg( CB_SETEDITSEL, 0, MAKELPARAM(0,-1));
	ShowWindow( m_nickDlg->GetHwnd(), SW_SHOW);
	SetActiveWindow( m_nickDlg->GetHwnd());
	return 0;
}

static void DoChatFormatting( TCHAR* pszText )
{
	TCHAR* p1 = pszText;
	int iFG = -1;
	int iRemoveChars;
	TCHAR InsertThis[50];

	while (*p1 != '\0') {
		iRemoveChars = 0;
		InsertThis[0] = 0;

		if ( *p1 == '%' ) {
			switch ( p1[1] ) {
			case 'B':
			case 'b':
				lstrcpy(InsertThis, _T("\002"));
				iRemoveChars = 2;
				break;
			case 'I':
			case 'i':
				lstrcpy(InsertThis, _T("\026"));
				iRemoveChars = 2;
				break;
			case 'U':
			case 'u':
				lstrcpy(InsertThis, _T("\037"));
				iRemoveChars = 2;
				break;
			case 'c':
				{
					lstrcpy(InsertThis, _T("\003"));
					iRemoveChars = 2;

					TCHAR szTemp[3];
					lstrcpyn(szTemp, p1 + 2, 3);
					iFG = _ttoi(szTemp);
				}
				break;
			case 'C':
				if ( p1[2] == '%' && p1[3] == 'F') {
					lstrcpy(InsertThis, _T("\00399,99"));
					iRemoveChars = 4;
				}
				else {
					lstrcpy(InsertThis, _T("\00399"));
					iRemoveChars = 2;
				}
				iFG = -1;
				break;
			case 'f':
				if (p1 - 3 >= pszText && p1[-3] == '\003')
					lstrcpy(InsertThis, _T(","));
				else if ( iFG >= 0 )
					mir_sntprintf(InsertThis, SIZEOF(InsertThis), _T("\003%u,"), iFG);
				else
					lstrcpy(InsertThis, _T("\00399,"));

				iRemoveChars = 2;
				break;

			case 'F':
				if (iFG >= 0)
					mir_sntprintf(InsertThis, SIZEOF(InsertThis), _T("\003%u,99"), iFG);
				else
					lstrcpy(InsertThis, _T("\00399,99"));
				iRemoveChars = 2;
				break;

			case '%':
				lstrcpy(InsertThis, _T("%"));
				iRemoveChars = 2;
				break;

			default:
				iRemoveChars = 2;
				break;
			}

			MoveMemory(p1 + lstrlen(InsertThis), p1 + iRemoveChars, sizeof(TCHAR)*(lstrlen(p1) - iRemoveChars + 1));
			CopyMemory(p1, InsertThis, sizeof(TCHAR)*lstrlen(InsertThis));
			if (iRemoveChars || lstrlen(InsertThis))
				p1 += lstrlen(InsertThis);
			else
				p1++;
		}
		else p1++;
}	}

int __cdecl CIrcProto::GCEventHook(WPARAM wParam,LPARAM lParam)
{
	GCHOOK *gchook= (GCHOOK*) lParam;
	GCHOOK *gchtemp = NULL;
	GCHOOK *gch = NULL;
	CMString S = _T("");

	EnterCriticalSection(&m_gchook);

	// handle the hook
	if ( gchook ) {
		if (!lstrcmpiA(gchook->pDest->pszModule, m_szModuleName)) {

			// first see if the scripting module should modify or stop this event
			if (m_bMbotInstalled && m_scriptingEnabled && wParam == NULL) {
				gchtemp = (GCHOOK *)mir_alloc(sizeof(GCHOOK));
				gchtemp->pDest = (GCDEST *)mir_alloc(sizeof(GCDEST));
				gchtemp->pDest->iType = gchook->pDest->iType;
				gchtemp->dwData = gchook->dwData;

				if ( gchook->pDest->ptszID ) {
					gchtemp->pDest->ptszID = mir_tstrdup( gchook->pDest->ptszID );
					TCHAR* pTemp = _tcschr(gchtemp->pDest->ptszID, ' ');
					if ( pTemp )
						*pTemp = '\0';
				}
				else gchtemp->pDest->ptszID = NULL;
				//MBOT CORRECTIONS
//*
				gchook->pDest->pszModule = mir_strdup( gchook->pDest->pszModule );
				gchook->ptszText = mir_tstrdup( gchook->ptszText );
				gchook->ptszUID = mir_tstrdup( gchook->ptszUID );
/*/
				gchtemp->pDest->pszModule = mir_strdup( gchook->pDest->pszModule );
				gchtemp->ptszText = mir_tstrdup( gchook->ptszText );
				gchtemp->ptszUID = mir_tstrdup( gchook->ptszUID );
*/
				if ( Scripting_TriggerMSPGuiOut(gchtemp) && gchtemp)
					gch = gchtemp;
				else
					gch = NULL;
			}
			else gch = gchook;

			if ( gch ) {
				TCHAR* p1 = mir_tstrdup( gch->pDest->ptszID );
				TCHAR* p2 = _tcsstr( p1, _T(" - "));
				if ( p2 )
					*p2 = '\0';

				switch( gch->pDest->iType ) {
				case GC_SESSION_TERMINATE:
					FreeWindowItemData(p1, (CHANNELINFO*)gch->dwData);
					break;

				case GC_USER_MESSAGE:
					if (gch && gch->pszText && lstrlen(gch->ptszText) > 0 ) {
						TCHAR* pszText = new TCHAR[lstrlen(gch->ptszText)+1000];
						lstrcpy(pszText, gch->ptszText);
						DoChatFormatting(pszText);
						PostIrcMessageWnd(p1, NULL, pszText);
						delete []pszText;
					}
					break;

				case GC_USER_CHANMGR:
					PostIrcMessageWnd(p1, NULL, _T("/CHANNELMANAGER"));
					break;

				case GC_USER_PRIVMESS:
					{
						TCHAR szTemp[4000];
						mir_sntprintf(szTemp, SIZEOF(szTemp), _T("/QUERY %s"), gch->ptszUID );
						PostIrcMessageWnd(p1, NULL, szTemp);
					}
					break;

				case GC_USER_LOGMENU:
					switch( gch->dwData ) {
					case 1:
						OnChangeNickMenuCommand(NULL, NULL);
						break;
					case 2:
						PostIrcMessageWnd(p1, NULL, _T("/CHANNELMANAGER"));
						break;

					case 3:
						PostIrcMessage( _T("/PART %s %s"), p1, m_userInfo );
						{	GCEVENT gce = {0};
							GCDEST gcd = {0};
							S = MakeWndID(p1);
							gce.cbSize = sizeof(GCEVENT);
							gcd.iType = GC_EVENT_CONTROL;
							gcd.pszModule = m_szModuleName;
							gce.dwFlags = GC_TCHAR;
							gce.pDest = &gcd;
							gcd.ptszID = ( TCHAR* )S.c_str();
							CallChatEvent( SESSION_TERMINATE, (LPARAM)&gce);
						}
						break;
					case 4:		// show server window
						PostIrcMessageWnd(p1, NULL, _T("/SERVERSHOW"));
						break;
/*					case 5:		// nickserv register nick
						PostIrcMessage( _T("/nickserv REGISTER %%question=\"%s\",\"%s\""),
							TranslateT("Please enter your authentification code"), TranslateT("Authentificate nick") );
						break;
*/					case 6:		// nickserv Identify
						PostIrcMessage( _T("/nickserv AUTH %%question=\"%s\",\"%s\""),
							TranslateT("Please enter your authentification code"), TranslateT("Authentificate nick") );
						break;
					case 7:		// nickserv drop nick
						if (MessageBox(0, TranslateT("Are you sure you want to unregister your current nick?"), TranslateT("Delete nick"),
								MB_ICONERROR + MB_YESNO + MB_DEFBUTTON2) == IDYES)
							PostIrcMessage( _T("/nickserv DROP"));
						break;
					case 8:		// nickserv Identify
						{
							CQuestionDlg* dlg = new CQuestionDlg( this );
							dlg->Show();
							HWND question_hWnd = dlg->GetHwnd();
							HWND hEditCtrl = GetDlgItem( question_hWnd, IDC_EDIT);
							SetDlgItemText( question_hWnd, IDC_CAPTION, TranslateT("Identify nick"));
							SetWindowText( GetDlgItem( question_hWnd, IDC_TEXT), TranslateT("Please enter your password") );
							SetDlgItemText( question_hWnd, IDC_HIDDENEDIT, _T("/nickserv IDENTIFY %question=\"%s\",\"%s\""));
							SetWindowLong(GetDlgItem( question_hWnd, IDC_EDIT), GWL_STYLE,
								(LONG)GetWindowLong(GetDlgItem( question_hWnd, IDC_EDIT), GWL_STYLE) | ES_PASSWORD);
							SendMessage(hEditCtrl, EM_SETPASSWORDCHAR,(WPARAM)_T('*'),0 );
							SetFocus(hEditCtrl);
							dlg->Activate();
						}
						break;
					case 9:		// nickserv remind password
						{
							DBVARIANT dbv;
							if ( !getTString( "Nick", &dbv )) {
								PostIrcMessage( _T("/nickserv SENDPASS %s"), dbv.ptszVal);
								DBFreeVariant( &dbv );
						}	}
						break;
					case 10:		// nickserv set new password
						PostIrcMessage( _T("/nickserv SET PASSWORD %%question=\"%s\",\"%s\""),
							TranslateT("Please enter your new password"), TranslateT("Set new password") );
						break;
					case 11:		// nickserv set language
						PostIrcMessage( _T("/nickserv SET LANGUAGE %%question=\"%s\",\"%s\""),
							TranslateT("Please enter desired languageID (numeric value, depends on server)"), TranslateT("Change language of NickServ messages") );
						break;
					case 12:		// nickserv set homepage
						PostIrcMessage( _T("/nickserv SET URL %%question=\"%s\",\"%s\""),
						TranslateT("Please enter URL that will be linked to your nick"), TranslateT("Set URL, linked to nick") );
						break;
					case 13:		// nickserv set email
						PostIrcMessage( _T("/nickserv SET EMAIL %%question=\"%s\",\"%s\""),
							TranslateT("Please enter your e-mail, that will be linked to your nick"), TranslateT("Set e-mail, linked to nick") );
						break;
					case 14:		// nickserv set info
						PostIrcMessage( _T("/nickserv SET INFO %%question=\"%s\",\"%s\""),
							TranslateT("Please enter some information about your nick"), TranslateT("Set information for nick") );
						break;
					case 15:		// nickserv kill unauth off
						PostIrcMessage( _T("/nickserv SET KILL OFF"));
							break;
					case 16:		// nickserv kill unauth on
						PostIrcMessage( _T("/nickserv SET KILL ON"));
							break;
					case 17:		// nickserv kill unauth quick
						PostIrcMessage( _T("/nickserv SET KILL QUICK"));
							break;
					case 18:		// nickserv hide nick from /LIST
						PostIrcMessage( _T("/nickserv SET PRIVATE ON"));
						break;
					case 19:		// nickserv show nick to /LIST
						PostIrcMessage( _T("/nickserv SET PRIVATE OFF"));
						break;
					case 20:		// nickserv Hide e-mail from info
						PostIrcMessage( _T("/nickserv SET HIDE EMAIL ON"));
							break;
					case 21:		// nickserv Show e-mail in info
						PostIrcMessage( _T("/nickserv SET HIDE EMAIL OFF"));
							break;
					case 22:		// nickserv Set security for nick
						PostIrcMessage( _T("/nickserv SET SECURE ON"));
							break;
					case 23:		// nickserv Remove security for nick
						PostIrcMessage( _T("/nickserv SET SECURE OFF"));
							break;
					case 24:		// nickserv Link nick to current
						PostIrcMessage( _T("/nickserv LINK %%question=\"%s\",\"%s\""),
							TranslateT("Please enter nick you want to link to your current nick"), TranslateT("Link another nick to current nick") );
						break;
					case 25:		// nickserv Unlink nick from current
						PostIrcMessage( _T("/nickserv LINK %%question=\"%s\",\"%s\""),
							TranslateT("Please enter nick you want to unlink from your current nick"), TranslateT("Unlink another nick from current nick") );
						break;
					case 26:		// nickserv Set main nick
						PostIrcMessage( _T("/nickserv LINK %%question=\"%s\",\"%s\""),
							TranslateT("Please enter nick you want to set as your main nick"), TranslateT("Set main nick") );
						break;
					case 27:		// nickserv list all linked nicks
						PostIrcMessage( _T("/nickserv LISTLINKS"));
							break;
					case 28:		// nickserv list all channels owned
						PostIrcMessage( _T("/nickserv LISTCHANS"));
							break;
					}
					break;

				case GC_USER_NICKLISTMENU:
					switch(gch->dwData) {
					case 1:
						PostIrcMessage( _T("/MODE %s +o %s"), p1, gch->ptszUID );
						break;
					case 2:
						PostIrcMessage( _T("/MODE %s -o %s"), p1, gch->ptszUID );
						break;
					case 3:
						PostIrcMessage( _T("/MODE %s +v %s"), p1, gch->ptszUID );
						break;
					case 4:
						PostIrcMessage( _T("/MODE %s -v %s"), p1, gch->ptszUID );
						break;
					case 5:
						PostIrcMessage( _T("/KICK %s %s"), p1, gch->ptszUID );
						break;
					case 6:
						PostIrcMessage( _T("/KICK %s %s %%question=\"%s\",\"%s\",\"%s\""),
							p1, gch->ptszUID, TranslateT("Please enter the reason"), TranslateT("Kick"), TranslateT("Jerk") );
						break;
					case 7:
						DoUserhostWithReason(1, _T("B") + (CMString)p1, true, _T("%s"), gch->ptszUID );
						break;
					case 8:
						DoUserhostWithReason(1, _T("K") + (CMString)p1, true, _T("%s"), gch->ptszUID );
						break;
					case 9:
						DoUserhostWithReason(1, _T("L") + (CMString)p1, true, _T("%s"), gch->ptszUID );
						break;
					case 10:
						PostIrcMessage( _T("/WHOIS %s %s"), gch->ptszUID, gch->ptszUID );
						break;
				//	case 11:
				//		DoUserhostWithReason(1, "I", true, "%s", gch->ptszUID );
				//		break;
				//	case 12:
				//		DoUserhostWithReason(1, "J", true, "%s", gch->ptszUID );
				//		break;
					case 13:
						PostIrcMessage( _T("/DCC CHAT %s"), gch->ptszUID );
						break;
					case 14:
						PostIrcMessage( _T("/DCC SEND %s"), gch->ptszUID );
						break;
					case 15:
						DoUserhostWithReason(1, _T("I"), true, _T("%s"), gch->ptszUID );
						break;
					case 16:
						PostIrcMessage( _T("/MODE %s +h %s"), p1, gch->ptszUID );
						break;
					case 17:
						PostIrcMessage( _T("/MODE %s -h %s"), p1, gch->ptszUID );
						break;
					case 18:
						PostIrcMessage( _T("/MODE %s +q %s"), p1, gch->ptszUID );
						break;
					case 19:
						PostIrcMessage( _T("/MODE %s -q %s"), p1, gch->ptszUID );
						break;
					case 20:
						PostIrcMessage( _T("/MODE %s +a %s"), p1, gch->ptszUID );
						break;
					case 21:
						PostIrcMessage( _T("/MODE %s -a %s"), p1, gch->ptszUID );
						break;
					case 22:
						PostIrcMessage( _T("/NOTICE %s %%question=\"%s\",\"%s\""),
							gch->ptszUID, TranslateT("Please enter the notice text"), TranslateT("Send notice") );
						break;
					case 23:
						PostIrcMessage( _T("/INVITE %s %%question=\"%s\",\"%s\""),
							gch->ptszUID, TranslateT("Please enter the channel name to invite to"), TranslateT("Invite to channel") );
						break;
					case 30:
						{
							PROTOSEARCHRESULT psr = { 0 };
							psr.cbSize = sizeof(psr);
							psr.flags = PSR_TCHAR;
							psr.id = gch->ptszUID;
							psr.nick = gch->ptszUID;

							ADDCONTACTSTRUCT acs = { 0 };
							acs.handleType = HANDLE_SEARCHRESULT;
							acs.szProto = m_szModuleName;
							acs.psr = &psr;
							CallService( MS_ADDCONTACT_SHOW, (WPARAM)NULL, (LPARAM)&acs);
						}
						break;
					case 31:	//slap
						{
							TCHAR tszTemp[4000];
							mir_sntprintf(tszTemp, SIZEOF(tszTemp), _T("/slap %s"), gch->ptszUID);
							PostIrcMessageWnd(p1, NULL, tszTemp);
						}
						break;
					case 32:  //nickserv info
						{
							TCHAR tszTemp[4000];
							mir_sntprintf(tszTemp, SIZEOF(tszTemp), _T("/nickserv INFO %s ALL"), gch->ptszUID);
							PostIrcMessageWnd(p1, NULL, tszTemp);
						}
						break;
					case 33:  //nickserv ghost
						{
							TCHAR tszTemp[4000];
							mir_sntprintf(tszTemp, SIZEOF(tszTemp), _T("/nickserv GHOST %s"), gch->ptszUID);
							PostIrcMessageWnd(p1, NULL, tszTemp);
						}
						break;
					}
					break;
				}
				mir_free( p1 );
	}	}	}

	if ( gchtemp ) {
		mir_free(gchtemp->pszUID);
		mir_free(gchtemp->pszText);
		mir_free(gchtemp->pDest->ptszID);
		mir_free(gchtemp->pDest->pszModule);
		mir_free(gchtemp->pDest);
		mir_free(gchtemp);
	}
	LeaveCriticalSection(&m_gchook);
	return 0;
}

int __cdecl CIrcProto::GCMenuHook(WPARAM, LPARAM lParam)
{
	GCMENUITEMS *gcmi= (GCMENUITEMS*) lParam;
	if ( gcmi ) {
		if ( !lstrcmpiA( gcmi->pszModule, m_szModuleName )) {
			if ( gcmi->Type == MENU_ON_LOG ) {
				if ( lstrcmpi( gcmi->pszID, SERVERWINDOW)) {
					static gc_item Item[] = {
						{ TranslateT("&Change your nickname" ),		1, MENU_ITEM,		FALSE},
						{ TranslateT("Channel &settings" ),			2, MENU_ITEM,		FALSE},
						{ _T(""),									0, MENU_SEPARATOR,	FALSE},
						{ TranslateT("NickServ"),					0, MENU_NEWPOPUP,	FALSE},
						{ TranslateT("Register nick" ),				5, MENU_POPUPITEM,	TRUE},
						{ TranslateT("Auth nick" ),					6, MENU_POPUPITEM,	FALSE},
						{ TranslateT("Delete nick" ),				7, MENU_POPUPITEM,	FALSE},
						{ TranslateT("Identify nick" ),				8, MENU_POPUPITEM,	FALSE},
						{ TranslateT("Remind password " ),			9, MENU_POPUPITEM,	FALSE},
						{ TranslateT("Set new password" ),			10, MENU_POPUPITEM,	TRUE},
						{ TranslateT("Set language" ),				11, MENU_POPUPITEM,	FALSE},
						{ TranslateT("Set homepage" ),				12, MENU_POPUPITEM,	FALSE},
						{ TranslateT("Set e-mail" ),				13, MENU_POPUPITEM,	FALSE},
						{ TranslateT("Set info" ),					14, MENU_POPUPITEM,	FALSE},
						{ _T("" ),									0,	MENU_POPUPSEPARATOR,FALSE},
						{ TranslateT("Hide e-mail from info" ),		20, MENU_POPUPITEM,	FALSE},
						{ TranslateT("Show e-mail in info" ),		21, MENU_POPUPITEM,	FALSE},
						{ _T("" ),									0,	MENU_POPUPSEPARATOR,FALSE},
						{ TranslateT("Set security for nick" ),		22, MENU_POPUPITEM,	FALSE},
						{ TranslateT("Remove security for nick" ),	23, MENU_POPUPITEM,	FALSE},
						{ _T("" ),									0,	MENU_POPUPSEPARATOR,FALSE},
						{ TranslateT("Link nick to current" ),		24, MENU_POPUPITEM,	FALSE},
						{ TranslateT("Unlink nick from current" ),	25, MENU_POPUPITEM,	FALSE},
						{ TranslateT("Set main nick" ),				26, MENU_POPUPITEM,	FALSE},
						{ TranslateT("List all your nicks" ),		27, MENU_POPUPITEM,	FALSE},
						{ TranslateT("List your channels" ),		28, MENU_POPUPITEM,	FALSE},
						{ _T("" ),									0,	MENU_POPUPSEPARATOR,FALSE},
						{ TranslateT("Kill unauthorized: off" ),	15, MENU_POPUPITEM,	FALSE},
						{ TranslateT("Kill unauthorized: on" ),		16, MENU_POPUPITEM,	FALSE},
						{ TranslateT("Kill unauthorized: quick" ),	17, MENU_POPUPITEM,	FALSE},
						{ _T("" ),									0,	MENU_POPUPSEPARATOR,FALSE},
						{ TranslateT("Hide nick from list" ),		18, MENU_POPUPITEM,	FALSE},
						{ TranslateT("Show nick to list" ),			19, MENU_POPUPITEM,	FALSE},
						{ TranslateT("Show the server &window" ),	4, MENU_ITEM,		FALSE},
						{ _T(""),									0, MENU_SEPARATOR,	FALSE},
						{ TranslateT("&Leave the channel" ),		3, MENU_ITEM,		FALSE}
						};
						gcmi->nItems = SIZEOF(Item);
						gcmi->Item = &Item[0];
				}
				else gcmi->nItems = 0;
			}

			if (gcmi->Type == MENU_ON_NICKLIST) {
				CONTACT user ={ (TCHAR*)gcmi->pszUID, NULL, NULL, false, false, false};
				HANDLE hContact = CList_FindContact(&user);

				static gc_item Item[] = {
					{ TranslateT("&WhoIs info"),         10, MENU_ITEM,           FALSE },		//0
					{ TranslateT("&Invite to channel"),  23, MENU_ITEM,           FALSE },
					{ TranslateT("Send &notice"),        22, MENU_ITEM,           FALSE },
					{ TranslateT("&Slap"),		         31, MENU_ITEM,           FALSE },
					{ TranslateT("Nickserv info"),		 32, MENU_ITEM,           FALSE },
					{ TranslateT("Nickserv kill ghost"), 33, MENU_ITEM,           FALSE },		//5
					{ TranslateT("&Control"),             0, MENU_NEWPOPUP,       FALSE },
					{ TranslateT("Give Owner"),          18, MENU_POPUPITEM,      FALSE },		//7
					{ TranslateT("Take Owner"),          19, MENU_POPUPITEM,      FALSE },
					{ TranslateT("Give Admin"),          20, MENU_POPUPITEM,      FALSE },
					{ TranslateT("Take Admin"),          21, MENU_POPUPITEM,      FALSE },		//10
					{ TranslateT("Give &Op"),             1, MENU_POPUPITEM,      FALSE },
					{ TranslateT("Take O&p"),             2, MENU_POPUPITEM,      FALSE },
					{ TranslateT("Give &Halfop"),        16, MENU_POPUPITEM,      FALSE },
					{ TranslateT("Take H&alfop"),        17, MENU_POPUPITEM,      FALSE },
					{ TranslateT("Give &Voice"),          3, MENU_POPUPITEM,      FALSE },		//15
					{ TranslateT("Take V&oice"),          4, MENU_POPUPITEM,      FALSE },
					{ _T(""),                             0, MENU_POPUPSEPARATOR, FALSE },
					{ TranslateT("&Kick"),                5, MENU_POPUPITEM,      FALSE },
					{ TranslateT("Ki&ck (reason)"),       6, MENU_POPUPITEM,      FALSE },
					{ TranslateT("&Ban"),                 7, MENU_POPUPITEM,      FALSE },		//20
					{ TranslateT("Ban'&n kick"),          8, MENU_POPUPITEM,      FALSE },
					{ TranslateT("Ban'n kick (&reason)"), 9, MENU_POPUPITEM,      FALSE },
					{ TranslateT("&Direct Connection"),   0, MENU_NEWPOPUP,       FALSE },
					{ TranslateT("Request &Chat"),       13, MENU_POPUPITEM,      FALSE },
					{ TranslateT("Send &File"),          14, MENU_POPUPITEM,      FALSE },		//25
					{ TranslateT("Add to &ignore list"), 15, MENU_ITEM,           FALSE },
					{ _T(""),                            12, MENU_SEPARATOR,      FALSE },
					{ TranslateT("&Add User"),           30, MENU_ITEM,           FALSE }
				};

				gcmi->nItems = SIZEOF(Item);
				gcmi->Item = &Item[0];
				BOOL bIsInList = (hContact && DBGetContactSettingByte(hContact, "CList", "NotOnList", 0) == 0);
				gcmi->Item[gcmi->nItems-1].bDisabled = bIsInList;

				unsigned long ulAdr = 0;
				if (m_manualHost)
					ulAdr = ConvertIPToInteger(m_mySpecifiedHostIP);
				else
					ulAdr = ConvertIPToInteger(m_IPFromServer?m_myHost:m_myLocalHost);
				gcmi->Item[23].bDisabled = ulAdr == 0?TRUE:FALSE;		//DCC submenu

				TCHAR stzChanName[100];
				const TCHAR* temp = _tcschr( gcmi->pszID, ' ' );
				int len = min((( temp == NULL ) ? lstrlen( gcmi->pszID ) : ( int )( temp - gcmi->pszID + 1 )), SIZEOF(stzChanName)-1 );
				lstrcpyn( stzChanName, gcmi->pszID, len );
				stzChanName[ len ] = 0;
				CHANNELINFO* wi = (CHANNELINFO *)DoEvent(GC_EVENT_GETITEMDATA,stzChanName, NULL, NULL, NULL, NULL, NULL, false, false, 0);
				BOOL bServOwner  = strchr(sUserModes.c_str(), 'q') == NULL?FALSE:TRUE;
				BOOL bServAdmin  = strchr(sUserModes.c_str(), 'a') == NULL?FALSE:TRUE;
				BOOL bOwner  = bServOwner?((wi->OwnMode>>4)&01):FALSE;
				BOOL bAdmin  = bServAdmin?((wi->OwnMode>>3)&01):FALSE;
				BOOL bOp	 = strchr(sUserModes.c_str(), 'o') == NULL?FALSE:((wi->OwnMode>>2)&01);
				BOOL bHalfop = strchr(sUserModes.c_str(), 'h') == NULL?FALSE:((wi->OwnMode>>1)&01);

				BOOL bForceEnable = GetAsyncKeyState(VK_CONTROL);

				gcmi->Item[6].bDisabled /* "Control" submenu */ = !(bForceEnable|| bHalfop || bOp || bAdmin || bOwner);
				gcmi->Item[7].uType = gcmi->Item[8].uType  = /* +/- Owner */ bServOwner?MENU_POPUPITEM:0;
				gcmi->Item[9].uType = gcmi->Item[10].uType = /* +/- Admin */ bServAdmin?MENU_POPUPITEM:0;
				gcmi->Item[7].bDisabled  = gcmi->Item[8].bDisabled  = gcmi->Item[9].bDisabled  = gcmi->Item[10].bDisabled = /* +/- Owner/Admin */
					!(bForceEnable || bOwner);
				gcmi->Item[11].bDisabled = gcmi->Item[12].bDisabled = gcmi->Item[13].bDisabled = gcmi->Item[14].bDisabled = /* +/- Op/hop */
					!(bForceEnable || bOp || bAdmin || bOwner);
	}	}	}

	return 0;
}

int __cdecl CIrcProto::OnPreShutdown(WPARAM, LPARAM)
{
	EnterCriticalSection(&cs);

	if ( m_perform && IsConnected() )
		if ( DoPerform( "Event: Disconnect" ))
			Sleep( 200 );

	DisconnectAllDCCSessions( true );

	if ( IsConnected() )
		Disconnect();
	if ( m_listDlg )
		m_listDlg->Close();
	if ( m_nickDlg )
		m_nickDlg->Close();
	if ( m_joinDlg )
		m_joinDlg->Close();

	LeaveCriticalSection(&cs);
	return 0;
}

int __cdecl CIrcProto::OnMenuPreBuild(WPARAM wParam, LPARAM)
{
	DBVARIANT dbv;
	HANDLE hContact = ( HANDLE )wParam;
	if ( !hContact )
		return 0;

	CLISTMENUITEM clmi = { 0 };
	clmi.cbSize = sizeof( clmi );
	clmi.flags = CMIM_FLAGS | CMIM_NAME | CMIM_ICON;

	char *szProto = ( char* ) CallService( MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) wParam, 0);
	if ( szProto && !lstrcmpiA(szProto, m_szModuleName)) {
		bool bIsOnline = getWord(hContact, "Status", ID_STATUS_OFFLINE)== ID_STATUS_OFFLINE ? false : true;
		if ( getByte(hContact, "ChatRoom", 0) == GCW_CHATROOM) {
			// context menu for chatrooms
			clmi.flags = CMIM_FLAGS | CMIF_NOTOFFLINE;
			CallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hUMenuChanSettings, ( LPARAM )&clmi );
		}
		else if ( !getTString( hContact, "Default", &dbv )) {
			// context menu for contact
			BYTE bDcc = getByte( hContact, "DCC", 0) ;

			clmi.flags = CMIM_FLAGS | CMIF_HIDDEN;
			CallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hUMenuChanSettings,		( LPARAM )&clmi );

			clmi.flags = CMIM_FLAGS;
			if ( bDcc ) {
				// for DCC contact
				clmi.flags = CMIM_FLAGS | CMIF_NOTOFFLINE;
				CallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hUMenuDisconnect,		( LPARAM )&clmi );
			}
			else {
				// for normal contact
				clmi.flags = CMIM_FLAGS | CMIF_NOTOFFLINE;
				if ( !IsConnected() )
					clmi.flags = CMIM_FLAGS | CMIF_HIDDEN;
				CallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hUMenuWhois, ( LPARAM )&clmi );

				if (bIsOnline) {
					DBVARIANT dbv3;
					if ( !getString( hContact, "Host", &dbv3) ) {
						if (dbv3.pszVal[0] == 0)  
							clmi.flags = CMIM_FLAGS | CMIF_HIDDEN;
						DBFreeVariant( &dbv3 );
					}
				}
				CallService( MS_CLIST_MODIFYMENUITEM, ( WPARAM )hUMenuIgnore, ( LPARAM )&clmi );
			}
			DBFreeVariant( &dbv );
	}	}

	return 0;
}

int __cdecl CIrcProto::OnDbSettingChanged(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = ( HANDLE ) wParam;
	if ( hContact == NULL || !IsConnected() )
		return 0;

	DBCONTACTWRITESETTING* cws = ( DBCONTACTWRITESETTING* )lParam;
	if ( strcmp( cws->szModule, "CList" ))
		return 0;

	if ( cws->value.type != DBVT_DELETED && !( cws->value.type==DBVT_BYTE && cws->value.bVal==0 ))
		return 0;

	if ( !strcmp( cws->szSetting, "NotOnList" ) ) {
		DBVARIANT dbv;
		if ( !getTString( hContact, "Nick", &dbv )) {
			if ( getByte( "MirVerAutoRequest", 1) )
				PostIrcMessage( _T("/PRIVMSG %s \001VERSION\001"), dbv.ptszVal );
			DBFreeVariant( &dbv );
	}	}
	return 0;
}
void __cdecl CIrcProto::ConnectServerThread( void* )
{
	InterlockedIncrement((long *) &m_bConnectThreadRunning);
	InterlockedIncrement((long *) &m_bConnectRequested);
	while ( !Miranda_Terminated() && m_bConnectRequested > 0 ) {
		while(m_bConnectRequested > 0)
			InterlockedDecrement((long *) &m_bConnectRequested);
		if (IsConnected()) {
			Sleep(200);
			Disconnect();
		}

		m_info.bNickFlag = false;
		int Temp = m_iStatus;
		m_iStatus = ID_STATUS_CONNECTING;
		nickflag = true;
		ProtoBroadcastAck(m_szModuleName,NULL,ACKTYPE_STATUS,ACKRESULT_SUCCESS,(HANDLE)Temp,ID_STATUS_CONNECTING);
		Sleep(100);
		EnterCriticalSection(&cs);
		Connect(si);
		LeaveCriticalSection(&cs);
		if (IsConnected()) {
			KillChatTimer( RetryTimer );

			if ( m_mySpecifiedHost[0] )
				ircFork( &CIrcProto::ResolveIPThread, new IPRESOLVE( m_mySpecifiedHost, IP_MANUAL ));

			DoEvent(GC_EVENT_CHANGESESSIONAME, SERVERWINDOW, NULL, m_info.sNetwork.c_str(), NULL, NULL, NULL, FALSE, TRUE);
		}
		else {
			Temp = m_iDesiredStatus;
			m_iStatus = m_iDesiredStatus = ID_STATUS_OFFLINE;
			ProtoBroadcastAck(m_szModuleName, NULL, ACKTYPE_LOGIN, ACKRESULT_FAILED, NULL, LOGINERR_NONETWORK );
			ProtoBroadcastAck(m_szModuleName,NULL,ACKTYPE_STATUS,ACKRESULT_SUCCESS,(HANDLE)Temp,ID_STATUS_OFFLINE);
			Sleep(100);
	}	}

	InterlockedDecrement((long *) &m_bConnectThreadRunning);
}

void __cdecl CIrcProto::DisconnectServerThread( void* )
{
	EnterCriticalSection( &cs );
	KillChatTimer( RetryTimer );
	if ( IsConnected() )
		Disconnect();
	LeaveCriticalSection( &cs );
	return;
}

void CIrcProto::ConnectToServer(void)
{
	m_portCount = StrToIntA(m_portStart);
	si.sServer = GetWord(m_serverName, 0);
	si.iPort = m_portCount;
	si.sNick = m_nick;
	si.sUserID = m_userID;
	si.sFullName = m_name;
	si.sPassword = m_password;
	si.bIdentServer =  ((m_ident) ? (true) : (false));
	si.iIdentServerPort = StrToInt(m_identPort);
	si.sIdentServerType = m_identSystem;
	si.m_iSSL = m_iSSL;
	{	TCHAR* p = mir_a2t( m_network );
		si.sNetwork = p;
		mir_free(p);
	}
	m_iRetryCount = 1;
	KillChatTimer(RetryTimer);
	if (m_retry) {
		if (StrToInt(m_retryWait)<10)
			lstrcpy(m_retryWait, _T("10"));
		SetChatTimer(RetryTimer, StrToInt(m_retryWait)*1000, RetryTimerProc);
	}

	bPerformDone = false;
	bTempDisableCheck = false;
	bTempForceCheck = false;
	m_iTempCheckTime = 0;
	sChannelPrefixes = _T("&#");
	sUserModes = "ov";
	sUserModePrefixes = _T("@+");
	sChannelModes = "btnimklps";

	if (!m_bConnectThreadRunning)
		ircFork( &CIrcProto::ConnectServerThread, 0 );
	else if (m_bConnectRequested < 1)
		InterlockedIncrement((long *) &m_bConnectRequested);

	TCHAR szTemp[300];
	mir_sntprintf(szTemp, SIZEOF(szTemp), _T("\0033%s \002%s\002 (") _T(TCHAR_STR_PARAM) _T(": %u)"),
		TranslateT("Connecting to"), si.sNetwork.c_str(), si.sServer.c_str(), si.iPort);
	DoEvent(GC_EVENT_INFORMATION, SERVERWINDOW, NULL, szTemp, NULL, NULL, NULL, true, false);
}

void CIrcProto::DisconnectFromServer(void)
{
	GCEVENT gce = {0};
	GCDEST gcd = {0};

	if ( m_perform && IsConnected() )
		DoPerform( "Event: Disconnect" );

	gcd.iType = GC_EVENT_CONTROL;
	gcd.ptszID = NULL; // all windows
	gcd.pszModule = m_szModuleName;
	gce.cbSize = sizeof(GCEVENT);
	gce.pDest = &gcd;

	CallChatEvent( SESSION_TERMINATE, (LPARAM)&gce);
	ircFork( &CIrcProto::DisconnectServerThread, 0 );
}

/////////////////////////////////////////////////////////////////////////////////////////
// GetMyAwayMsg - obtain the current away message

INT_PTR __cdecl CIrcProto::GetMyAwayMsg(WPARAM wParam,LPARAM lParam)
{
	if (( int )wParam != m_iStatus )
		return 0;

	const TCHAR* p = m_statusMessage.c_str();

	return (lParam & SGMA_UNICODE) ? (INT_PTR)mir_t2u(p) : (INT_PTR)mir_t2a(p);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Service function creation

VOID CALLBACK RetryTimerProc( HWND, UINT, UINT_PTR idEvent, DWORD )
{
	CIrcProto* ppro = GetTimerOwner( idEvent );
	if ( !ppro )
		return;

	if ( ppro->m_iRetryCount <= StrToInt( ppro->m_retryCount) && ppro->m_retry ) {
		ppro->m_portCount++;
		if ( ppro->m_portCount > StrToIntA( ppro->m_portEnd ) || StrToIntA( ppro->m_portEnd ) == 0 )
			ppro->m_portCount = StrToIntA( ppro->m_portStart );
		ppro->si.iPort = ppro->m_portCount;

		TCHAR szTemp[300];
		mir_sntprintf(szTemp, SIZEOF(szTemp), _T("\0033%s \002%s\002 (") _T(TCHAR_STR_PARAM) _T(": %u, try %u)"),
			TranslateT("Reconnecting to"), ppro->si.sNetwork.c_str(), ppro->si.sServer.c_str(), ppro->si.iPort, ppro->m_iRetryCount);

		ppro->DoEvent(GC_EVENT_INFORMATION, SERVERWINDOW, NULL, szTemp, NULL, NULL, NULL, true, false);

		if ( !ppro->m_bConnectThreadRunning )
			ppro->ircFork( &CIrcProto::ConnectServerThread, 0 );
		else
			ppro->m_bConnectRequested = true;

		ppro->m_iRetryCount++;
	}
	else ppro->KillChatTimer( ppro->RetryTimer );
}

// logs text into NetLib (stolen from Jabber ;) )
void CIrcProto::DoNetlibLog( const char* fmt, ... )
{
	va_list vararg;
	va_start( vararg, fmt );
	char* str = ( char* )alloca( 32000 );
	mir_vsnprintf( str, 32000, fmt, vararg );
	va_end( vararg );

	CallService( MS_NETLIB_LOG, ( WPARAM )hNetlib, ( LPARAM )str );
}
