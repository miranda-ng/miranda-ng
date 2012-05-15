/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.
Copyright (c) 2006-2012 Boris Krasnovskiy.
Copyright (c) 2003-2005 George Hazan.
Copyright (c) 2002-2003 Richard Hughes (original version).

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "msn_global.h"
#include "msn_proto.h"

static HANDLE hPrebuildMenuHook;
static HGENMENU 
	hBlockMenuItem, 
	hLiveSpaceMenuItem, 
	hNetmeetingMenuItem, 
	hChatInviteMenuItem, 
	hOpenInboxMenuItem;
HANDLE hNetMeeting, hBlockCom, hSendHotMail, hInviteChat, hViewProfile;

/////////////////////////////////////////////////////////////////////////////////////////
// Block command callback function

INT_PTR CMsnProto::MsnBlockCommand(WPARAM wParam, LPARAM)
{
	if (msnLoggedIn) 
	{
		const HANDLE hContact = (HANDLE)wParam;

		char tEmail[MSN_MAX_EMAIL_LEN];
		getStaticString(hContact, "e-mail", tEmail, sizeof(tEmail));

		if (Lists_IsInList(LIST_BL, tEmail))
			deleteSetting(hContact, "ApparentMode");
		else
			setWord(hContact, "ApparentMode", ID_STATUS_OFFLINE);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MsnGotoInbox - goes to the Inbox folder at the live.com

INT_PTR CMsnProto::MsnGotoInbox(WPARAM, LPARAM)
{
	HANDLE hContact = MSN_HContactFromEmail(MyOptions.szEmail);
	if (hContact) CallService(MS_CLIST_REMOVEEVENT, (WPARAM)hContact, (LPARAM) 1);

	MsnInvokeMyURL(true, "http://mail.live.com?rru=inbox");
	return 0;
}

INT_PTR CMsnProto::MsnSendHotmail(WPARAM wParam, LPARAM)
{
	const HANDLE hContact = (HANDLE)wParam;
	char szEmail[MSN_MAX_EMAIL_LEN];

	if (MSN_IsMeByContact(hContact, szEmail))
		MsnGotoInbox(0, 0);
	else if (msnLoggedIn)
	{
		char szEmailEnc[MSN_MAX_EMAIL_LEN*3], szUrl[256];

		UrlEncode(szEmail, szEmailEnc, sizeof(szEmailEnc));
		mir_snprintf(szUrl, sizeof(szUrl), "http://mail.live.com?rru=compose?to=%s", szEmailEnc);
		MsnInvokeMyURL(true, szUrl);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MsnSetupAlerts - goes to the alerts section at the live.com

INT_PTR CMsnProto::MsnSetupAlerts(WPARAM, LPARAM)
{
	MsnInvokeMyURL(false, "http://alerts.live.com");
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MsnViewProfile - view a contact's profile

INT_PTR CMsnProto::MsnViewProfile(WPARAM wParam, LPARAM)
{
	const HANDLE hContact = (HANDLE)wParam;
	char buf[64], *cid;

	if (hContact == NULL)
		cid = mycid;
	else 
	{
		cid = buf;
		if (getStaticString(hContact, "CID", buf, 30))
			return 0;
	}

	char tUrl[256];
	mir_snprintf(tUrl, sizeof(tUrl), "http://cid-%I64X.profiles.live.com", _atoi64(cid));
	MsnInvokeMyURL(false, tUrl);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MsnEditProfile - goes to the Profile section at the live.com

INT_PTR CMsnProto::MsnEditProfile(WPARAM, LPARAM)
{
	MsnViewProfile(0, 0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MsnInviteCommand - invite command callback function

INT_PTR CMsnProto::MsnInviteCommand(WPARAM, LPARAM)
{
	DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_CHATROOM_INVITE), NULL, DlgInviteToChat, 
		LPARAM(new InviteChatParam(NULL, NULL, this)));
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MsnRebuildContactMenu - gray or ungray the block menus according to contact's status

int CMsnProto::OnPrebuildContactMenu(WPARAM wParam, LPARAM)
{
	const HANDLE hContact = (HANDLE)wParam;
	char szEmail[MSN_MAX_EMAIL_LEN];

	CLISTMENUITEM mi = {0};
	mi.cbSize = sizeof(mi);

	if (!MSN_IsMyContact(hContact)) return 0;

	bool isMe = MSN_IsMeByContact(hContact, szEmail);
	if (szEmail[0]) 
	{
		int listId = Lists_GetMask(szEmail);
		
		bool noChat = !(listId & LIST_FL) || isMe || getByte(hContact, "ChatRoom", 0);

		mi.flags = CMIM_NAME | CMIM_FLAGS | CMIF_ICONFROMICOLIB;
		if (noChat) mi.flags |= CMIF_HIDDEN;
		mi.pszName = (char*)((listId & LIST_BL) ? "&Unblock" : "&Block");
		MSN_CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hBlockMenuItem, (LPARAM)&mi);

		mi.flags = CMIM_NAME | CMIM_FLAGS | CMIF_ICONFROMICOLIB;
		if (!emailEnabled) mi.flags |= CMIF_HIDDEN;
		mi.pszName = isMe ? LPGEN("Open &Hotmail Inbox") : LPGEN("Send &Hotmail E-mail");
		MSN_CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hOpenInboxMenuItem, (LPARAM)&mi);

		mi.flags = CMIM_FLAGS | CMIF_ICONFROMICOLIB | CMIF_NOTOFFLINE;
		if (noChat) mi.flags |= CMIF_HIDDEN;
		MSN_CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hNetmeetingMenuItem, (LPARAM)&mi);
		MSN_CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hChatInviteMenuItem, (LPARAM)&mi);
	}

	return 0;
}

int CMsnProto::OnContactDoubleClicked(WPARAM wParam, LPARAM)
{
	const HANDLE hContact = (HANDLE)wParam;

	if (emailEnabled && MSN_IsMeByContact(hContact)) 
	{
		MsnSendHotmail(wParam, 0);
		return 1;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MsnSendNetMeeting - Netmeeting callback function

INT_PTR CMsnProto::MsnSendNetMeeting(WPARAM wParam, LPARAM)
{
	if (!msnLoggedIn) return 0;

	HANDLE hContact = HANDLE(wParam);

	char szEmail[MSN_MAX_EMAIL_LEN];
	if (MSN_IsMeByContact(hContact, szEmail)) return 0;

	ThreadData* thread = MSN_GetThreadByContact(szEmail);

	if (thread == NULL) {
		MessageBox(NULL, TranslateT("You must be talking to start Netmeeting"), TranslateT("MSN Protocol"), MB_OK | MB_ICONERROR);
		return 0;
	}

	char msg[1024];

	mir_snprintf(msg, sizeof(msg),
		"Content-Type: text/x-msmsgsinvite; charset=UTF-8\r\n\r\n"
		"Application-Name: NetMeeting\r\n"
		"Application-GUID: {44BBA842-CC51-11CF-AAFA-00AA00B6015C}\r\n"
		"Session-Protocol: SM1\r\n"
		"Invitation-Command: INVITE\r\n"
		"Invitation-Cookie: %i\r\n"
		"Session-ID: {1A879604-D1B8-11D7-9066-0003FF431510}\r\n\r\n",
		MSN_GenRandom());

	thread->sendMessage('N', NULL, 1, msg, MSG_DISABLE_HDR);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
//	SetNicknameCommand - sets nick name

static INT_PTR CALLBACK DlgProcSetNickname(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			TranslateDialogDefault(hwndDlg);

			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
			CMsnProto* proto = (CMsnProto*)lParam;

			SendMessage(hwndDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIconEx("main", true));
			SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIconEx("main"));
			SendMessage(GetDlgItem(hwndDlg, IDC_NICKNAME), EM_LIMITTEXT, 129, 0);

			DBVARIANT dbv;
			if (!proto->getTString("Nick", &dbv)) {
				SetDlgItemText(hwndDlg, IDC_NICKNAME, dbv.ptszVal);
				MSN_FreeVariant(&dbv);
			}
			return TRUE;
		}
		case WM_COMMAND:
			switch(wParam)
			{
			case IDOK: 
				{
					CMsnProto* proto = (CMsnProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
					if (proto->msnLoggedIn) 
					{
						TCHAR str[130];
						GetDlgItemText(hwndDlg, IDC_NICKNAME, str, SIZEOF(str));
						proto->MSN_SendNickname(str);
					}
				}

				case IDCANCEL:
					DestroyWindow(hwndDlg);
					break;
			}
			break;

		case WM_CLOSE:
			DestroyWindow(hwndDlg);
			break;

		case WM_DESTROY:
			ReleaseIconEx("main");
			ReleaseIconEx("main", true);
			break;
	}
	return FALSE;
}

INT_PTR CMsnProto::SetNicknameUI(WPARAM, LPARAM)
{
	HWND hwndSetNickname = CreateDialogParam (hInst, MAKEINTRESOURCE(IDD_SETNICKNAME), 
		NULL, DlgProcSetNickname, (LPARAM)this);

	SetForegroundWindow(hwndSetNickname);
	SetFocus(hwndSetNickname);
	ShowWindow(hwndSetNickname, SW_SHOW);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////
// Menus initialization

void CMsnProto::MsnInitMainMenu(void)
{
	char servicefunction[100];
	strcpy(servicefunction, m_szModuleName);
	char* tDest = servicefunction + strlen(servicefunction);

	CLISTMENUITEM mi = {0};
	mi.cbSize = sizeof(mi);

	HGENMENU hRoot = MO_GetProtoRootMenu(m_szModuleName);
	if (hRoot == NULL)
	{
		mi.popupPosition = 500085000;
		mi.hParentMenu = HGENMENU_ROOT;
		mi.flags = CMIF_ICONFROMICOLIB | CMIF_ROOTPOPUP | CMIF_TCHAR | CMIF_KEEPUNTRANSLATED;
		mi.icolibItem = GetIconHandle(IDI_MSN);
		mi.ptszName = m_tszUserName;
		hRoot = mainMenuRoot = (HGENMENU)MSN_CallService(MS_CLIST_ADDPROTOMENUITEM,  (WPARAM)0, (LPARAM)&mi);
	}
	else
	{
		MsnRemoveMainMenus();
		mainMenuRoot = NULL;
	}

	mi.flags = CMIF_ICONFROMICOLIB | CMIF_CHILDPOPUP;
	mi.hParentMenu = hRoot;
	mi.pszService = servicefunction;

	strcpy(tDest, MS_SET_NICKNAME_UI);
	CreateProtoService(MS_SET_NICKNAME_UI, &CMsnProto::SetNicknameUI);
	mi.position = 201001;
	mi.icolibItem = GetIconHandle(IDI_MSN);
	mi.pszName = LPGEN("Set &Nickname");
	menuItemsMain[0] = (HGENMENU)MSN_CallService(MS_CLIST_ADDPROTOMENUITEM, 0, (LPARAM)&mi);

	strcpy(tDest, MSN_INVITE);
	CreateProtoService(MSN_INVITE, &CMsnProto::MsnInviteCommand);
	mi.position = 201002;
	mi.icolibItem = GetIconHandle(IDI_INVITE);
	mi.pszName = LPGEN("Create &Chat");
	menuItemsMain[0] = (HGENMENU)MSN_CallService(MS_CLIST_ADDPROTOMENUITEM, 0, (LPARAM)&mi);

	strcpy(tDest, MS_GOTO_INBOX);
	CreateProtoService(MS_GOTO_INBOX, &CMsnProto::MsnGotoInbox);
	mi.position = 201003;
	mi.icolibItem = GetIconHandle(IDI_INBOX);
	mi.pszName = LPGEN("Display &Hotmail Inbox");
	menuItemsMain[1] = (HGENMENU)MSN_CallService(MS_CLIST_ADDPROTOMENUITEM, 0, (LPARAM)&mi);

	strcpy(tDest, MS_EDIT_PROFILE);
	CreateProtoService(MS_EDIT_PROFILE, &CMsnProto::MsnEditProfile);
	mi.position = 201004;
	mi.icolibItem = GetIconHandle(IDI_PROFILE);
	mi.pszName = LPGEN("View &Profile");
	menuItemsMain[2] = (HGENMENU)MSN_CallService(MS_CLIST_ADDPROTOMENUITEM, 0, (LPARAM)&mi);

	strcpy(tDest, MS_EDIT_ALERTS);
	CreateProtoService(MS_EDIT_ALERTS, &CMsnProto::MsnSetupAlerts);
	mi.position = 201004;
	mi.icolibItem = GetIconHandle(IDI_PROFILE);
	mi.pszName = LPGEN("Setup Live &Alerts");
	menuItemsMain[3] = (HGENMENU)MSN_CallService(MS_CLIST_ADDPROTOMENUITEM, 0, (LPARAM)&mi);

	MSN_EnableMenuItems(m_iStatus >= ID_STATUS_ONLINE);
}

void CMsnProto::MsnRemoveMainMenus(void)
{
	if (mainMenuRoot) 
		MSN_CallService(MS_CLIST_REMOVEMAINMENUITEM, (WPARAM)mainMenuRoot, 0);
}

void  CMsnProto::MSN_EnableMenuItems(bool bEnable)
{
	CLISTMENUITEM mi = {0};
	mi.cbSize = sizeof(mi);
	mi.flags = CMIM_FLAGS;
	if (!bEnable)
		mi.flags |= CMIF_GRAYED;

	for (unsigned i=0; i < SIZEOF(menuItemsMain); i++)
	{
		if (menuItemsMain[i] != NULL)
			MSN_CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)menuItemsMain[i], (LPARAM)&mi);
	}

	if (bEnable)
	{
		mi.flags = CMIM_FLAGS;
		if (!emailEnabled) mi.flags |= CMIF_HIDDEN;
		MSN_CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)menuItemsMain[1], (LPARAM)&mi);
	}
}

//////////////////////////////////////////////////////////////////////////////////////

static CMsnProto* GetProtoInstanceByHContact(HANDLE hContact)
{
	char* szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
	if (szProto == NULL)
		return NULL;

	for (int i = 0; i < g_Instances.getCount(); i++)
		if (!strcmp(szProto, g_Instances[i].m_szModuleName))
			return &g_Instances[i];

	return NULL;
}

static INT_PTR MsnMenuBlockCommand(WPARAM wParam, LPARAM lParam)
{
	CMsnProto* ppro = GetProtoInstanceByHContact((HANDLE)wParam);
	return (ppro) ? ppro->MsnBlockCommand(wParam, lParam) : 0;
}

static INT_PTR MsnMenuViewProfile(WPARAM wParam, LPARAM lParam)
{
	CMsnProto* ppro = GetProtoInstanceByHContact((HANDLE)wParam);
	return (ppro) ? ppro->MsnViewProfile(wParam, lParam) : 0;
}

static INT_PTR MsnMenuSendNetMeeting(WPARAM wParam, LPARAM lParam)
{
	CMsnProto* ppro = GetProtoInstanceByHContact((HANDLE)wParam);
	return (ppro) ? ppro->MsnSendNetMeeting(wParam, lParam) : 0;
}

static INT_PTR MsnMenuSendHotmail(WPARAM wParam, LPARAM lParam)
{
	CMsnProto* ppro = GetProtoInstanceByHContact((HANDLE)wParam);
	return (ppro) ? ppro->MsnSendHotmail(wParam, lParam) : 0;
}

static void sttEnableMenuItem(HANDLE hMenuItem, bool bEnable)
{
	CLISTMENUITEM clmi = {0};
	clmi.cbSize = sizeof(CLISTMENUITEM);
	clmi.flags = CMIM_FLAGS;
	if (!bEnable)
		clmi.flags |= CMIF_HIDDEN;

	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)hMenuItem, (LPARAM)&clmi);
}

static int MSN_OnPrebuildContactMenu(WPARAM wParam, LPARAM lParam)
{
	CMsnProto* ppro = GetProtoInstanceByHContact((HANDLE)wParam);
	if (ppro)
		ppro->OnPrebuildContactMenu(wParam, lParam);
	else
	{
		sttEnableMenuItem(hBlockMenuItem, false);
		sttEnableMenuItem(hLiveSpaceMenuItem, false);
		sttEnableMenuItem(hNetmeetingMenuItem, false);
		sttEnableMenuItem(hChatInviteMenuItem, false);
		sttEnableMenuItem(hOpenInboxMenuItem, false);
	}

	return 0;
}

void MSN_InitContactMenu(void)
{
	char servicefunction[100];
	strcpy(servicefunction, "MSN");
	char* tDest = servicefunction + strlen(servicefunction);

	CLISTMENUITEM mi = {0};
	mi.cbSize = sizeof(mi);
	mi.flags = CMIF_ICONFROMICOLIB;
	mi.pszService = servicefunction;

	strcpy(tDest, MSN_BLOCK);
	hBlockCom = CreateServiceFunction(servicefunction, MsnMenuBlockCommand);
	mi.position = -500050000;
	mi.icolibItem = GetIconHandle(IDI_MSNBLOCK);
	mi.pszName = LPGEN("&Block");
	hBlockMenuItem = (HGENMENU)MSN_CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);

	strcpy(tDest, MSN_VIEW_PROFILE);
	hViewProfile = CreateServiceFunction(servicefunction, MsnMenuViewProfile);
	mi.position = -500050003;
	mi.icolibItem = GetIconHandle(IDI_PROFILE);
	mi.pszName = LPGEN("View &Profile");
	hLiveSpaceMenuItem = (HGENMENU)MSN_CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);

	strcpy(tDest, MSN_NETMEETING);
	hNetMeeting = CreateServiceFunction(servicefunction, MsnMenuSendNetMeeting);
	mi.flags = CMIF_ICONFROMICOLIB | CMIF_NOTOFFLINE;
	mi.position = -500050002;
	mi.icolibItem = GetIconHandle(IDI_NETMEETING);
	mi.pszName = LPGEN("&Start Netmeeting");
	hNetmeetingMenuItem = (HGENMENU)MSN_CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM)&mi);

	strcpy(tDest, "/SendHotmail");
	hSendHotMail = CreateServiceFunction(servicefunction, MsnMenuSendHotmail);
	mi.position = -2000010005;
	mi.flags = CMIF_ICONFROMICOLIB | CMIF_HIDDEN;
	mi.icolibItem = LoadSkinnedIconHandle(SKINICON_OTHER_SENDEMAIL);
	mi.pszName = LPGEN("Open &Hotmail Inbox");
	hOpenInboxMenuItem = (HGENMENU)CallService(MS_CLIST_ADDCONTACTMENUITEM, 0, (LPARAM) &mi);

	hPrebuildMenuHook = HookEvent(ME_CLIST_PREBUILDCONTACTMENU, MSN_OnPrebuildContactMenu);
}

void MSN_RemoveContactMenus(void)
{
	UnhookEvent(hPrebuildMenuHook);

	MSN_CallService(MS_CLIST_REMOVECONTACTMENUITEM, (WPARAM)hBlockMenuItem, 0);
	MSN_CallService(MS_CLIST_REMOVECONTACTMENUITEM, (WPARAM)hLiveSpaceMenuItem, 0);
	MSN_CallService(MS_CLIST_REMOVECONTACTMENUITEM, (WPARAM)hNetmeetingMenuItem, 0);
	MSN_CallService(MS_CLIST_REMOVECONTACTMENUITEM, (WPARAM)hChatInviteMenuItem, 0);
	MSN_CallService(MS_CLIST_REMOVECONTACTMENUITEM, (WPARAM)hOpenInboxMenuItem, 0);

	DestroyServiceFunction(hNetMeeting);
	DestroyServiceFunction(hBlockCom);
	DestroyServiceFunction(hSendHotMail);
	DestroyServiceFunction(hInviteChat);
	DestroyServiceFunction(hViewProfile);
}
