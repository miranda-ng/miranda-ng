/*
Plugin of Miranda IM for communicating with users of the MSN Messenger protocol.

Copyright (c) 2012-2017 Miranda NG Team
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

#include "stdafx.h"
#include "msn_proto.h"

static HGENMENU hBlockMenuItem, hLiveSpaceMenuItem, hNetmeetingMenuItem, hChatInviteMenuItem, hOpenInboxMenuItem;

// Block command callback function
INT_PTR CMsnProto::MsnBlockCommand(WPARAM hContact, LPARAM)
{
	if (msnLoggedIn) {
		char tEmail[MSN_MAX_EMAIL_LEN];
		if (db_get_static(hContact, m_szModuleName, "wlid", tEmail, sizeof(tEmail))
				&& db_get_static(hContact, m_szModuleName, "e-mail", tEmail, sizeof(tEmail)))
			return 0;

		if (Lists_IsInList(LIST_BL, tEmail))
			delSetting(hContact, "ApparentMode");
		else
			setWord(hContact, "ApparentMode", ID_STATUS_OFFLINE);
	}
	return 0;
}

// MsnGotoInbox - goes to the Inbox folder at the live.com
INT_PTR CMsnProto::MsnGotoInbox(WPARAM, LPARAM)
{
	MCONTACT hContact = MSN_HContactFromEmail(MyOptions.szEmail);
	if (hContact)
		pcli->pfnRemoveEvent(hContact, 1);

	MsnInvokeMyURL(true, "http://mail.live.com?rru=inbox");
	return 0;
}

INT_PTR CMsnProto::MsnSendHotmail(WPARAM hContact, LPARAM)
{
	char szEmail[MSN_MAX_EMAIL_LEN];
	if (MSN_IsMeByContact(hContact, szEmail))
		MsnGotoInbox(0, 0);
	else if (msnLoggedIn)
		MsnInvokeMyURL(true, CMStringA().Format("http://mail.live.com?rru=compose?to=%s", ptrA(mir_urlEncode(szEmail))));

	return 0;
}

// MsnSetupAlerts - goes to the alerts section at the live.com
INT_PTR CMsnProto::MsnSetupAlerts(WPARAM, LPARAM)
{
	MsnInvokeMyURL(false, "http://alerts.live.com");
	return 0;
}

// MsnViewProfile - view a contact's profile
INT_PTR CMsnProto::MsnViewProfile(WPARAM hContact, LPARAM)
{
	char buf[64], *cid;

	if (hContact == NULL)
		cid = mycid;
	else {
		cid = buf;
		if (db_get_static(hContact, m_szModuleName, "CID", buf, 30))
			return 0;
	}

	char tUrl[256];
	mir_snprintf(tUrl, "http://cid-%I64X.profiles.live.com", _atoi64(cid));
	MsnInvokeMyURL(false, tUrl);
	return 0;
}

// MsnEditProfile - goes to the Profile section at the live.com
INT_PTR CMsnProto::MsnEditProfile(WPARAM, LPARAM)
{
	MsnViewProfile(0, 0);
	return 0;
}

// MsnInviteCommand - invite command callback function
INT_PTR CMsnProto::MsnInviteCommand(WPARAM, LPARAM)
{
	DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_CHATROOM_INVITE), NULL, DlgInviteToChat,
		LPARAM(new InviteChatParam(NULL, NULL, this)));
	return 0;
}

// MsnRebuildContactMenu - gray or ungray the block menus according to contact's status
int CMsnProto::OnPrebuildContactMenu(WPARAM hContact, LPARAM)
{
	if (!MSN_IsMyContact(hContact))
		return 0;

	char szEmail[MSN_MAX_EMAIL_LEN];
	bool isMe = MSN_IsMeByContact(hContact, szEmail);
	if (szEmail[0]) {
		int listId = Lists_GetMask(szEmail);
		bool noChat = !(listId & LIST_FL) || isMe || isChatRoom(hContact);

		Menu_ModifyItem(hBlockMenuItem, (listId & LIST_BL) ? LPGENW("&Unblock") : LPGENW("&Block"));
		Menu_ShowItem(hBlockMenuItem, !noChat);

		Menu_ModifyItem(hOpenInboxMenuItem, isMe ? LPGENW("Open &Hotmail Inbox") : LPGENW("Send &Hotmail E-mail"));
		Menu_ShowItem(hOpenInboxMenuItem, emailEnabled);

		Menu_ShowItem(hChatInviteMenuItem, !noChat);
	}

	return 0;
}

int CMsnProto::OnContactDoubleClicked(WPARAM hContact, LPARAM)
{
	if (emailEnabled && MSN_IsMeByContact(hContact)) {
		MsnSendHotmail(hContact, 0);
		return 1;
	}
	return 0;
}

// Menus initialization
void CMsnProto::MsnInitMainMenu(void)
{
	CMenuItem mi;
	mi.root = Menu_GetProtocolRoot(this);

	mi.pszService = MSN_INVITE;
	CreateProtoService(mi.pszService, &CMsnProto::MsnInviteCommand);
	mi.position = 201002;
	mi.hIcolibItem = GetIconHandle(IDI_INVITE);
	mi.name.a = LPGEN("Create &Chat");
	menuItemsMain[0] = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	mi.pszService = MS_GOTO_INBOX;
	CreateProtoService(mi.pszService, &CMsnProto::MsnGotoInbox);
	mi.position = 201003;
	mi.hIcolibItem = GetIconHandle(IDI_INBOX);
	mi.name.a = LPGEN("Display &Hotmail Inbox");
	menuItemsMain[1] = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	mi.pszService = MS_EDIT_PROFILE;
	CreateProtoService(mi.pszService, &CMsnProto::MsnEditProfile);
	mi.position = 201004;
	mi.hIcolibItem = GetIconHandle(IDI_PROFILE);
	mi.name.a = LPGEN("View &Profile");
	menuItemsMain[2] = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	mi.pszService = MS_EDIT_ALERTS;
	CreateProtoService(mi.pszService, &CMsnProto::MsnSetupAlerts);
	mi.position = 201004;
	mi.hIcolibItem = GetIconHandle(IDI_PROFILE);
	mi.name.a = LPGEN("Setup Live &Alerts");
	menuItemsMain[3] = Menu_AddProtoMenuItem(&mi, m_szModuleName);

	MSN_EnableMenuItems(m_iStatus >= ID_STATUS_ONLINE);
}

void CMsnProto::MSN_EnableMenuItems(bool bEnable)
{
	for (int i = 0; i < _countof(menuItemsMain); i++)
		Menu_ModifyItem(menuItemsMain[i], NULL, INVALID_HANDLE_VALUE, bEnable ? 0 : CMIF_GRAYED);

	if (bEnable)
		Menu_ShowItem(menuItemsMain[1], emailEnabled);
}

//////////////////////////////////////////////////////////////////////////////////////

static CMsnProto* GetProtoInstanceByHContact(MCONTACT hContact)
{
	char* szProto = GetContactProto(hContact);
	if (szProto == NULL)
		return NULL;

	for (int i = 0; i < g_Instances.getCount(); i++)
		if (!mir_strcmp(szProto, g_Instances[i].m_szModuleName))
			return &g_Instances[i];

	return NULL;
}

static INT_PTR MsnMenuBlockCommand(WPARAM wParam, LPARAM lParam)
{
	CMsnProto* ppro = GetProtoInstanceByHContact(wParam);
	return (ppro) ? ppro->MsnBlockCommand(wParam, lParam) : 0;
}

static INT_PTR MsnMenuViewProfile(WPARAM wParam, LPARAM lParam)
{
	CMsnProto* ppro = GetProtoInstanceByHContact(wParam);
	return (ppro) ? ppro->MsnViewProfile(wParam, lParam) : 0;
}

static INT_PTR MsnMenuSendHotmail(WPARAM wParam, LPARAM lParam)
{
	CMsnProto* ppro = GetProtoInstanceByHContact(wParam);
	return (ppro) ? ppro->MsnSendHotmail(wParam, lParam) : 0;
}

static int MSN_OnPrebuildContactMenu(WPARAM wParam, LPARAM lParam)
{
	CMsnProto* ppro = GetProtoInstanceByHContact(wParam);
	if (ppro)
		ppro->OnPrebuildContactMenu(wParam, lParam);
	else {
		Menu_ShowItem(hBlockMenuItem, false);
		Menu_ShowItem(hLiveSpaceMenuItem, false);
		Menu_ShowItem(hChatInviteMenuItem, false);
		Menu_ShowItem(hOpenInboxMenuItem, false);
	}

	return 0;
}

void MSN_InitContactMenu(void)
{
	char servicefunction[100];
	mir_strcpy(servicefunction, "MSN");
	char* tDest = servicefunction + mir_strlen(servicefunction);

	CMenuItem mi;
	mi.pszService = servicefunction;

	SET_UID(mi, 0xc6169b8f, 0x53ab, 0x4242, 0xbe, 0x90, 0xe2, 0x4a, 0xa5, 0x73, 0x88, 0x32);
	mir_strcpy(tDest, MSN_BLOCK);
	CreateServiceFunction(servicefunction, MsnMenuBlockCommand);
	mi.position = -500050000;
	mi.hIcolibItem = GetIconHandle(IDI_MSNBLOCK);
	mi.name.a = LPGEN("&Block");
	hBlockMenuItem = Menu_AddContactMenuItem(&mi);

	SET_UID(mi, 0x7f7e4c24, 0x821c, 0x450f, 0x93, 0x76, 0xbe, 0x65, 0xe9, 0x2f, 0xb6, 0xc2);
	mir_strcpy(tDest, MSN_VIEW_PROFILE);
	CreateServiceFunction(servicefunction, MsnMenuViewProfile);
	mi.position = -500050003;
	mi.hIcolibItem = GetIconHandle(IDI_PROFILE);
	mi.name.a = LPGEN("View &Profile");
	hLiveSpaceMenuItem = Menu_AddContactMenuItem(&mi);

	SET_UID(mi,0x25a007c0, 0x8dc7, 0x4284, 0x8a, 0x5e, 0x2, 0x83, 0x17, 0x5d, 0x52, 0xea);
	mir_strcpy(tDest, "/SendHotmail");
	CreateServiceFunction(servicefunction, MsnMenuSendHotmail);
	mi.position = -2000010005;
	mi.flags = CMIF_HIDDEN;
	mi.hIcolibItem = Skin_GetIconHandle(SKINICON_OTHER_SENDEMAIL);
	mi.name.a = LPGEN("Open &Hotmail Inbox");
	hOpenInboxMenuItem = Menu_AddContactMenuItem(&mi);

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, MSN_OnPrebuildContactMenu);
}

void MSN_RemoveContactMenus(void)
{
	Menu_RemoveItem(hBlockMenuItem);
	Menu_RemoveItem(hLiveSpaceMenuItem);
	Menu_RemoveItem(hChatInviteMenuItem);
	Menu_RemoveItem(hOpenInboxMenuItem);
}
