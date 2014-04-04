/*
 * Miranda-IM Vypress Chat/quickChat plugins
 * Copyright (C) Saulius Menkevicius
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: service.c,v 1.30 2005/03/15 02:03:47 bobas Exp $
 */

#include "miranda.h"
#include "libvqproto/vqproto.h"

#include "main.h"
#include "service.h"
#include "resource.h"
#include "user.h"
#include "userlist.h"
#include "contacts.h"
#include "options.h"
#include "chatroom.h"
#include "chanlist.h"
#include "util.h"
#include "msgloop.h"
#include "skin.h"

/* defines */
#define SVC_CONTACT_CHANSETTINGS "/ChanSettings"
#define SVC_CONTACT_BEEP	"/SendBeep"
#define SVC_MENU_SET_NICKNAME	"/SetNickname"
#define SVC_MENU_JOIN_CHANNEL	"/JoinChannel"

/* static data
 */
static HANDLE	s_hook_ModulesLoaded;
static HANDLE	s_hook_menuPreBuild;
static HANDLE	s_hook_contactDeleted;

static HANDLE	s_hContactMenuBeep, s_hContactChanSettings;
static HANDLE	s_hMenuJoinChannel, s_hMenuSetNickname;
static HWND	s_hwndJoinChannel, s_hwndSetNickname;
static char *	s_joinChannelChanlist = NULL;

static vqp_link_t s_vqpLink;

/* static routines
 */
static int
service_GetCaps(WPARAM wParam, LPARAM lParam)
{
	switch(wParam) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_MODEMSG | PF1_BASICSEARCH;

	case PFLAGNUM_2:
		return PF2_ONLINE|PF2_SHORTAWAY|PF2_HEAVYDND|PF2_LONGAWAY;
	
	case PFLAGNUM_3:
		/* status modes where away can be set */
		return PF2_HEAVYDND|PF2_LONGAWAY;

	case PFLAG_UNIQUEIDTEXT:
		return (int)"Nickname";
		
	case PFLAG_UNIQUEIDSETTING:
		return (int)"Nick";

	case PFLAG_MAXLENOFMESSAGE:
		return VQP_MAX_PACKET_SIZE; /* an approximation */
	}

	return 0;
}

static int
service_GetName(WPARAM wParam, LPARAM lParam)
{
	strncpy((char*)lParam, VQCHAT_PROTO_NAME, wParam);
	return 0;
}

static int
service_LoadIcon(WPARAM wParam, LPARAM lParam)
{
	UINT id;

	switch(wParam) {
	case PLI_PROTOCOL|PLIF_LARGE:
		id = IDI_VQCHAT_PROTO_LARGE;
		break;
		
	case PLI_PROTOCOL|PLIF_SMALL:
		id = IDI_VQCHAT_PROTO;
		break;

	case PLI_ONLINE|PLIF_SMALL:
		id = IDI_VQCHAT_PROTO_ONLINE;
		break;

	case PLI_OFFLINE|PLIF_SMALL:
		id = IDI_VQCHAT_PROTO_OFFLINE;
		break;

	default:
		return 0;
	}

	return (int)LoadImage(
		g_hDllInstance, MAKEINTRESOURCE(id), IMAGE_ICON,
		GetSystemMetrics((wParam & PLIF_SMALL) ? SM_CXSMICON: SM_CXICON),
		GetSystemMetrics((wParam & PLIF_SMALL) ? SM_CYSMICON: SM_CYICON),
		0);
}

static int
service_SetStatus(WPARAM wParam, LPARAM lParam)
{
	int oldStatus = user_status();
		
	if(!user_set_status(wParam)) {
		ProtoBroadcastAck(
			VQCHAT_PROTO, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS,
			(HANDLE)oldStatus, (LPARAM)user_status());
	}

	return 0;
}

static int
service_GetStatus(WPARAM wParam, LPARAM lParam)
{
	return user_status();
}

static int
service_SetAwayMsg(WPARAM wParam, LPARAM lParam)
{
	/*int status_mode = wParam;*/
	LPCSTR szMessage = (LPCSTR)lParam;	/* note that szMessage can be NULL! */
	char * message;

	if(user_offline())
		return 1;

	/* convert to utf and set */
	message = util_loc2utf(szMessage ? szMessage: "");
	user_set_awaymsg(message);
	free(message);

	return 0;	/* success */
}

static void CALLBACK
service_BasicSearch_ResultAPC(ULONG_PTR dwParam)
{
	if(dwParam) {
		/* user found */
		PROTOSEARCHRESULT psr;

		memset(&psr, 0, sizeof(psr));
		psr.cbSize = sizeof(psr);
		psr.nick = (char*)dwParam;

		ProtoBroadcastAck(VQCHAT_PROTO, NULL, ACKTYPE_SEARCH,
			ACKRESULT_DATA, (HANDLE)1, (LPARAM)&psr);

		free((char*)dwParam);
	}

	/* search finished */
	ProtoBroadcastAck(VQCHAT_PROTO, NULL, ACKTYPE_SEARCH,
		ACKRESULT_SUCCESS, (HANDLE)1, 0);
}

/* service_BasicSearch:
 *	implement case insensitive search
 */
static int
service_BasicSearch(WPARAM wParam, LPARAM lParam)
{
	char * nickname;
	
	if(user_offline())
		return 0;

	nickname = util_loc2utf((char*)lParam);
	QueueUserAPC(
		service_BasicSearch_ResultAPC, g_hMainThread,
		(ULONG_PTR)(userlist_user_exists(nickname) ? strdup(nickname): 0));
	free(nickname);
	
	return 1;	/* search started (imaginary handle 1) */
}

static int
service_AddToList(WPARAM wParam, LPARAM lParam)
{
	int added;
	char * nickname;
	
	/* wParam = flags */
	PROTOSEARCHRESULT * psresult = (PROTOSEARCHRESULT*)lParam;

	if(user_offline())
		return 0;

	nickname = util_loc2utf(psresult->nick);
	added = (int)contacts_add_contact(nickname, 1);
	free(nickname);

	return added;
}

static int
service_GetContactInfo(WPARAM wParam, LPARAM lParam)
{
	CCSDATA * ccs = (CCSDATA*)lParam;

	if(user_offline())
		return 1;
	
	return contacts_get_contact_info(ccs->hContact, ccs->wParam);
}

static int
service_contact_send_message(WPARAM wParam, LPARAM lParam)
{
	CCSDATA * ccs = (CCSDATA*)lParam;
	char * message;
	int result;

	message = util_loc2utf((const char*)ccs->lParam);
	result = contacts_send_contact_message(ccs->hContact, message, wParam, 0);
	free(message);
	
	return result;
}

static int
service_contact_send_messageW(WPARAM wParam, LPARAM lParam)
{
	CCSDATA * ccs = (CCSDATA*)lParam;
	int result, len;
	char * message;

	len = strlen((const char *) ccs->lParam);
	message = util_uni2utf((wchar_t *)((const char *)ccs->lParam + len + 1));

	result = contacts_send_contact_message(ccs->hContact, message, wParam, 0);
	free(message);

	return result;
}

/* service_contact_received_message:
 *	invoked after our PSR_MESSAGE from contacts.c:contacts_input_contact_message()
 *	gets routed through all protocol plugins, and gets back to us
 */
static int
service_contact_received_message(WPARAM wParam, LPARAM lParam)
{
	CCSDATA * ccs = (CCSDATA *)lParam;
	PROTORECVEVENT * pre = (PROTORECVEVENT *)ccs->lParam;
	DBEVENTINFO dbei;

	memset(&dbei, 0, sizeof(dbei));
	dbei.cbSize = sizeof(dbei);
	dbei.szModule = VQCHAT_PROTO;
	dbei.timestamp = pre->timestamp;
	dbei.flags = (pre->flags & PREF_CREATEREAD) ? DBEF_READ: 0;
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.cbBlob = strlen(pre->szMessage) + 1;
	if(pre->flags & PREF_UNICODE)
		dbei.cbBlob *= (sizeof(wchar_t) + 1);
	dbei.pBlob = (PBYTE) pre->szMessage;
	
	CallService(MS_DB_EVENT_ADD, (WPARAM)ccs->hContact, (LPARAM)&dbei);

	return 0;	/* success */
}

static int
service_contact_beep(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;
	if(hContact && contacts_is_user_contact(hContact))
		contacts_send_beep(hContact);
	
	return 0;
}

static int
service_contact_channel_settings(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;
	if(hContact && contacts_is_chatroom_contact(hContact)) {
		DBVARIANT dbv;
		if(!db_get(hContact, VQCHAT_PROTO, "Nick", &dbv)) {
			/* show channel settings (skip '#' at the beginning) */
			char * channel = util_loc2utf(dbv.pszVal + 1);
			chatroom_channel_show_settings_dlg(channel);

			free(channel);
			db_free(&dbv);
		}
	}

	return 0;
}

static int
service_prebuild_contact_menu(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;

	if(hContact) {
		CLISTMENUITEM clmi;
		memset(&clmi, 0, sizeof(clmi));
		clmi.cbSize = sizeof(clmi);

		/* beep */
		if(contacts_is_user_contact(hContact)) {
			clmi.flags = CMIM_FLAGS
				| (contacts_get_contact_status(hContact)
							==ID_STATUS_OFFLINE
					? CMIF_GRAYED: 0);
		} else {
			clmi.flags = CMIM_FLAGS | CMIF_HIDDEN;
		}
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)s_hContactMenuBeep, (LPARAM)&clmi);

		/* channel settings */
		clmi.flags = CMIM_FLAGS
			| (!contacts_is_chatroom_contact(hContact)
							? CMIF_HIDDEN: 0);
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)s_hContactChanSettings, (LPARAM)&clmi);
	}

	return 0;
}

static int
service_contact_deleted(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;

	/* leave the channel */
	if(hContact!=NULL && contacts_is_chatroom_contact(hContact)) {
		DBVARIANT dbv;

		if(!db_get(hContact, VQCHAT_PROTO, "Nick", &dbv)) {
			/* channel name must begin with '#' */
			if(dbv.pszVal[0]=='#') {
				char * channel = util_loc2utf(dbv.pszVal + 1);
				chatroom_channel_part(channel, 0);
				free(channel);
			}
			
			db_free(&dbv);
		}
	}
	
	return 0;
}

void service_join_channel_merge_chanlist(const char * user_chanlist)
{
	if(s_hwndJoinChannel) {
		char * chanlist = chanlist_copy(user_chanlist),
			* channel;

		while((channel = chanlist_shift(&chanlist)) != NULL) {
			if(!chanlist_contains(*user_p_chanlist(), channel)
					&& !chanlist_contains(s_joinChannelChanlist, channel)) {
				/* add '#'+channel to combo box list */
				char * mod_channel = malloc(strlen(channel) + 2);
				wchar_t * u_channel;
				mod_channel[0] = '#';
				strcpy(mod_channel + 1, channel);
				u_channel = util_utf2uni(mod_channel);
				free(mod_channel);

				SendMessageW(
					GetDlgItem(s_hwndJoinChannel, IDC_JC_COMBO_CHANNEL),
					CB_ADDSTRING, 0, (LPARAM)u_channel);
				free(u_channel);

				/* add channel to the list of already visible channels
				 * in the combo list
				 */
				s_joinChannelChanlist = chanlist_add(
								s_joinChannelChanlist, channel);
			}
			free(channel);
		}
	}
}

static BOOL CALLBACK service_join_channel_dlg_wndproc(
	HWND hwnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	int len;
	char * channel;
	char * r_user_nickname;
	
	switch(nMsg) {
	case WM_INITDIALOG:
		/* set dialog icon */
		SendMessage(
			hwnd, WM_SETICON, ICON_SMALL,
			(LPARAM)LoadIcon(g_hDllInstance, MAKEINTRESOURCE(IDI_CHATROOM)));

		/* clear channel list and send channel list req*/
		SendMessage(GetDlgItem(hwnd, IDC_JC_COMBO_CHANNEL), CB_RESETCONTENT, 0, 0);

		r_user_nickname = util_utf2vqp(user_codepage(), user_nickname());
		msgloop_send(vqp_msg_channel_list_req(s_vqpLink, r_user_nickname), 0);
		free(r_user_nickname);

		chanlist_free(s_joinChannelChanlist);
		s_joinChannelChanlist = NULL;

		/* set focus on current combo box control */
		SetFocus(GetDlgItem(hwnd, IDC_JC_COMBO_CHANNEL));

		/* disable ok button, as there is no text in combo box */
		EnableWindow(GetDlgItem(hwnd, IDOK), FALSE);
		break;

	case WM_COMMAND:
		switch(HIWORD(wParam)) {
		case BN_CLICKED:
			switch(LOWORD(wParam)) {
			case IDOK:
				/* join the specified channel */
				channel = util_GetDlgItemTextUtf(hwnd, IDC_JC_COMBO_CHANNEL);
				chatroom_channel_join(channel[0]=='#' ? channel + 1: channel, 0);
				free(channel);

				/* close the dialog */
				SendMessage(hwnd, WM_CLOSE, 0, 0);
				break;
			}
			break;
			
		case CBN_EDITCHANGE:
			switch(LOWORD(wParam)) {
			case IDC_JC_COMBO_CHANNEL:
				len = GetWindowTextLengthW(GetDlgItem(hwnd, IDC_JC_COMBO_CHANNEL));
				EnableWindow(GetDlgItem(hwnd, IDOK), len!=0);
				break;
			}
			break;
			
		case CBN_SELCHANGE:
			switch(LOWORD(wParam)) {
			case IDC_JC_COMBO_CHANNEL:
				/* enable the 'join' button after
				 * the user selects a channel from the list
				 */
				EnableWindow(GetDlgItem(hwnd, IDOK), TRUE);
				break;
			}
			break;
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		s_hwndJoinChannel = NULL;

		chanlist_free(s_joinChannelChanlist);
		s_joinChannelChanlist = NULL;
		break;
	}
	return FALSE;
}

static int service_menu_join_channel(WPARAM wParam, LPARAM lParam)
{
	if(!s_hwndJoinChannel) {
		s_hwndJoinChannel = CreateDialog(
			g_hDllInstance, MAKEINTRESOURCE(IDD_JC), NULL,
			service_join_channel_dlg_wndproc
		);
		ShowWindow(s_hwndJoinChannel, SW_SHOW);
	}
	SetActiveWindow(s_hwndJoinChannel);

	return 0;
}

static BOOL CALLBACK
service_set_nickname_dlg_wndproc(
	HWND hwnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
	int len;
	char * nickname;
	
	switch(nMsg) {
	case WM_INITDIALOG:
		/* set dialog icon */
		SendMessage(
			hwnd, WM_SETICON, ICON_BIG,
			(LPARAM)(HICON)LoadImage(
				g_hDllInstance, MAKEINTRESOURCE(IDI_VQCHAT_PROTO_LARGE),
				IMAGE_ICON, GetSystemMetrics(SM_CXICON),
				GetSystemMetrics(SM_CYICON), LR_SHARED)
		);

		/* set current nickname */
		util_SetDlgItemTextUtf(hwnd, IDC_SN_EDIT_NICKNAME, user_nickname());

		/* select the entire nickname */
		SendMessage(
			GetDlgItem(hwnd, IDC_SN_EDIT_NICKNAME),
			EM_SETSEL, (WPARAM)0, (LPARAM)-1);
		SetFocus(GetDlgItem(hwnd, IDC_SN_EDIT_NICKNAME));
		break;

	case WM_COMMAND:
		switch(HIWORD(wParam)) {
		case BN_CLICKED:
			switch(LOWORD(wParam)) {
			case IDOK:
				/* set nickname */
				nickname = util_GetDlgItemTextUtf(hwnd, IDC_SN_EDIT_NICKNAME);
				user_set_nickname(nickname, 1);
				free(nickname);

				/* FALLTHROUGH */
		
			case IDCANCEL:
				SendMessage(hwnd, WM_CLOSE, 0, 0);
				break;
			}
			break;
			
		case EN_CHANGE:
			switch(LOWORD(wParam)) {
			case IDC_SN_EDIT_NICKNAME:
				/* disable the ok button, if we have
				 * an invalid nickname entered
				 */
				len = GetWindowTextLengthW(GetDlgItem(hwnd, IDC_SN_EDIT_NICKNAME));
				EnableWindow(GetDlgItem(hwnd, IDOK), len!=0);
				break;
			}
			break;
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		s_hwndSetNickname = NULL;
		break;
	}
	return FALSE;
}

static int
service_menu_set_nickname(WPARAM wParam, LPARAM lParam)
{
	if(!s_hwndSetNickname) {
		s_hwndSetNickname = CreateDialog(
			g_hDllInstance, MAKEINTRESOURCE(IDD_SN), NULL,
			service_set_nickname_dlg_wndproc
		);
		ShowWindow(s_hwndSetNickname, SW_SHOW);
	}
	SetActiveWindow(s_hwndSetNickname);

	return 0;
}

static void
service_init_menus()
{
	CLISTMENUITEM mi;
	memset(&mi, 0, sizeof(mi));
	mi.cbSize = sizeof(mi);

	/* normal contacts: beep */
	mi.pszName = "Alert Beep";
	mi.hIcon = LoadIcon(g_hDllInstance, MAKEINTRESOURCE(IDI_CONTACT_BELL));
	mi.pszService = VQCHAT_PROTO SVC_CONTACT_BEEP;
	mi.pszContactOwner = VQCHAT_PROTO;
	mi.popupPosition = 500090000;
	mi.pszPopupName = VQCHAT_PROTO_NAME;
	s_hContactMenuBeep = (HANDLE)CallService(
		MS_CLIST_ADDCONTACTMENUITEM, (WPARAM)0, (LPARAM)&mi);

	/* menu item: set nickname */
	mi.pszName = "Set &nickname";
	mi.hIcon = LoadIcon(g_hDllInstance, MAKEINTRESOURCE(IDI_VQCHAT_PROTO));
	mi.pszService = VQCHAT_PROTO SVC_MENU_SET_NICKNAME;
	mi.popupPosition = 500090000;
	mi.pszPopupName = VQCHAT_PROTO_NAME;
	s_hMenuSetNickname = (HANDLE)CallService(
		MS_CLIST_ADDMAINMENUITEM, (WPARAM)0, (LPARAM)&mi);
	s_hwndSetNickname = NULL;

	if(chatroom_module_installed()) {
		/* chatrooms: channel settings */
		mi.pszName = "Channel settings";
		mi.hIcon = LoadIcon(g_hDllInstance, MAKEINTRESOURCE(IDI_CHATROOM));
		mi.pszService = VQCHAT_PROTO SVC_CONTACT_CHANSETTINGS;
		mi.pszContactOwner = VQCHAT_PROTO;
		mi.popupPosition = 500090000;
		mi.pszPopupName = VQCHAT_PROTO_NAME;
		s_hContactChanSettings = (HANDLE)CallService(
			MS_CLIST_ADDCONTACTMENUITEM, (WPARAM)0, (LPARAM)&mi);

		/* menu item: join channel */
		mi.pszName = "&Join a channel";
		mi.hIcon = LoadIcon(g_hDllInstance, MAKEINTRESOURCE(IDI_CHATROOM));
		mi.pszService = VQCHAT_PROTO SVC_MENU_JOIN_CHANNEL;
		mi.popupPosition = 500090000;
		mi.pszPopupName = VQCHAT_PROTO_NAME;
		mi.flags = CMIM_FLAGS | CMIF_GRAYED;
		s_hMenuJoinChannel = (HANDLE)CallService(
			MS_CLIST_ADDMAINMENUITEM, (WPARAM)0, (LPARAM)&mi);
		s_hwndJoinChannel = NULL;
	}
}

static int
service_hook_modules_loaded(WPARAM wParam, LPARAM lParam)
{
	options_hook_modules_loaded();
	skin_hook_modules_loaded();
	user_hook_modules_loaded();
	contacts_hook_modules_loaded();
	chatroom_hook_modules_loaded();

	service_init_menus();

	s_hook_menuPreBuild = HookEvent(
			ME_CLIST_PREBUILDCONTACTMENU, service_prebuild_contact_menu);
	s_hook_contactDeleted = HookEvent(ME_DB_CONTACT_DELETED, service_contact_deleted);

	return 0;
}

/* exported routines
 */
void service_register_services()
{
	/* register protocol service funcs */
	CreateServiceFunction(VQCHAT_PROTO PS_GETCAPS, service_GetCaps);
	CreateServiceFunction(VQCHAT_PROTO PS_GETNAME, service_GetName);
	CreateServiceFunction(VQCHAT_PROTO PS_LOADICON, service_LoadIcon);
	CreateServiceFunction(VQCHAT_PROTO PS_SETSTATUS, service_SetStatus);
	CreateServiceFunction(VQCHAT_PROTO PS_GETSTATUS, service_GetStatus);
	CreateServiceFunction(VQCHAT_PROTO PS_SETAWAYMSG, service_SetAwayMsg);
	CreateServiceFunction(VQCHAT_PROTO PS_BASICSEARCH, service_BasicSearch);
	CreateServiceFunction(VQCHAT_PROTO PS_ADDTOLIST, service_AddToList);
	
	CreateServiceFunction(VQCHAT_PROTO PSS_GETINFO, service_GetContactInfo);

	CreateServiceFunction(VQCHAT_PROTO PSS_MESSAGE, service_contact_send_message);
	CreateServiceFunction(VQCHAT_PROTO PSS_MESSAGE"W", service_contact_send_messageW);
	CreateServiceFunction(VQCHAT_PROTO PSR_MESSAGE, service_contact_received_message);

	/* contact menu services */
	CreateServiceFunction(
		VQCHAT_PROTO SVC_CONTACT_BEEP,
		service_contact_beep
	);
	CreateServiceFunction(
		VQCHAT_PROTO SVC_CONTACT_CHANSETTINGS,
		service_contact_channel_settings);

	/* main menu services */
	CreateServiceFunction(
		VQCHAT_PROTO SVC_MENU_SET_NICKNAME,
		service_menu_set_nickname
	);
	CreateServiceFunction(
		VQCHAT_PROTO SVC_MENU_JOIN_CHANNEL,
		service_menu_join_channel);
}

void service_hook_all()
{
	s_hook_ModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED, service_hook_modules_loaded);
}

void service_uninit()
{
	/* unhook service module's hooks */
	UnhookEvent(s_hook_ModulesLoaded);
}

void service_connected(vqp_link_t link)
{
	CLISTMENUITEM clmi;
	memset(&clmi, 0, sizeof(clmi));
	clmi.cbSize = sizeof(clmi);

	/* enable 'Join channel menu item' */
	clmi.flags = CMIM_FLAGS;
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)s_hMenuJoinChannel, (LPARAM)&clmi);

	/* set link */
	s_vqpLink = link;
}

void service_disconnected()
{
	CLISTMENUITEM clmi;
	memset(&clmi, 0, sizeof(clmi));
	clmi.cbSize = sizeof(clmi);

	/* disable 'Join channel menu item' */
	clmi.flags = CMIM_FLAGS | CMIF_GRAYED;
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)s_hMenuJoinChannel, (LPARAM)&clmi);
}

