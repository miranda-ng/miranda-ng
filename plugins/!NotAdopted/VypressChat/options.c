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
 * $Id: options.c,v 1.21 2005/04/11 21:44:15 bobas Exp $
 */

#include "miranda.h"

#include "main.h"
#include "user.h"
#include "util.h"
#include "options.h"
#include "resource.h"
#include "contacts.h"
#include "userlist.h"
#include "chanlist.h"
#include "chatroom.h"

/* static data
 */
static HANDLE s_hook_opt_initialise;
static HANDLE s_hook_userinfo_initialise;

/* static routines
 */

static BOOL CALLBACK
options_user_dlgproc(HWND hdlg, UINT msg, WPARAM wp, LPARAM lp)
{
	static BOOL dlg_initalizing;
	
	HWND dlgitem;
	char * str;
	
	switch(msg) {
	case WM_INITDIALOG:
		/* controls may not send PSM_CHANGED for now:
		 * we're initalizing the dialog
		 */
		dlg_initalizing = TRUE;
		
		/* set nickname entry */
		util_SetDlgItemTextUtf(hdlg, IDC_USER_EDIT_NICKNAME, user_nickname());

		/* setup gender combo box */
		dlgitem = GetDlgItem(hdlg, IDC_USER_COMBO_GENDER);
		SendMessage(dlgitem, CB_RESETCONTENT, 0, 0);
		SendMessage(dlgitem, CB_ADDSTRING, 0, (LPARAM)"Male");
		SendMessage(dlgitem, CB_ADDSTRING, 0, (LPARAM)"Female");
		SendMessage(dlgitem, CB_SETCURSEL,
				user_gender()==VQP_GENDER_MALE ? 0: 1, (LPARAM)0);

		/* set UUID entry */
#ifdef VYPRESSCHAT
		str = vqp_uuid_to_string(user_p_uuid());
		SetDlgItemTextA(hdlg, IDC_USER_EDIT_UUID, str);
		free(str);
#else
		EnableWindow(GetDlgItem(hdlg, IDC_USER_EDIT_UUID), FALSE);
#endif
		/* set other options */
		CheckDlgButton(hdlg, IDC_USER_MSGONALERTBEEP,
			db_byte_get(NULL, VQCHAT_PROTO, "MsgOnAlertBeep", 0)
				? BST_CHECKED: BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_USER_NICKNAMEONTOPIC,
			db_byte_get(NULL, VQCHAT_PROTO, "NicknameOnTopic", 0)
				? BST_CHECKED: BST_UNCHECKED);
		CheckDlgButton(hdlg, IDC_USER_NEWPREFERMSG,
			db_byte_get(NULL, VQCHAT_PROTO, "ContactsPreferMsg", 0)
				? BST_CHECKED: BST_UNCHECKED);
		
		/* controls can sen send PSM_CHANGED now */
		dlg_initalizing = FALSE;
		break;

	case WM_COMMAND:
		switch(HIWORD(wp)) {
		case BN_CLICKED:
			if(!dlg_initalizing)
				SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
			break;
			
		case EN_CHANGE:
			switch(LOWORD(wp)) {
			case IDC_USER_EDIT_NICKNAME:
			case IDC_USER_EDIT_UUID:
				if(!dlg_initalizing)
					SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
				break;
			}
			break;

		case CBN_SELCHANGE:
			switch(LOWORD(wp)) {
			case IDC_USER_COMBO_GENDER:
				if(!dlg_initalizing)
					SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
				break;
			}
			break;
		}
		break;
		
	case WM_NOTIFY:
		if(((LPNMHDR)lp)->code == PSN_APPLY) {
			/* apply changes
			 */
#ifdef VYPRESSCHAT
			char * str;
			vqp_uuid_t uuid;
#endif
			
			/* apply nickname */
			str = util_GetDlgItemTextUtf(hdlg, IDC_USER_EDIT_NICKNAME);
			if(user_set_nickname(str, TRUE)) {
				/* XXX: show error message */
				free(str);
				SetFocus(GetDlgItem(hdlg, IDC_USER_EDIT_NICKNAME));
				return TRUE;
			}
			free(str);

			/* apply gender */
			user_set_gender(
				SendMessage(
					GetDlgItem(hdlg, IDC_USER_COMBO_GENDER),
					CB_GETCURSEL, 0, 0
					) == 0
						? VQP_GENDER_MALE: VQP_GENDER_FEMALE,
				TRUE
			);
			
#ifdef VYPRESSCHAT
			/* store uuid */
			str = util_GetDlgItemTextUtf(hdlg, IDC_USER_EDIT_UUID);
			if(vqp_uuid_from_string(&uuid, str)) {
				/* restore old uuid */
				char * old_str = vqp_uuid_to_string(user_p_uuid());
				util_SetDlgItemTextUtf(hdlg, IDC_USER_EDIT_UUID, old_str);
				free(old_str);

				/* XXX: show error message */
				free(str);
				SetFocus(GetDlgItem(hdlg, IDC_USER_EDIT_UUID));
				return TRUE;
			}
			free(str);
#endif
			db_byte_set(NULL, VQCHAT_PROTO, "MsgOnAlertBeep",
				IsDlgButtonChecked(hdlg, IDC_USER_MSGONALERTBEEP) ? 1: 0);
			db_byte_set(NULL, VQCHAT_PROTO, "NicknameOnTopic",
				IsDlgButtonChecked(hdlg, IDC_USER_NICKNAMEONTOPIC) ? 1: 0);
			db_byte_set(NULL, VQCHAT_PROTO, "ContactsPreferMsg",
				IsDlgButtonChecked(hdlg, IDC_USER_NEWPREFERMSG) ? 1: 0);

		}
		break;
	}
	return FALSE;
}

static BOOL
options_network_dlgproc_bcast_add(HANDLE hdlg)
{
	DWORD ip;
	HWND ip_cntl = GetDlgItem(hdlg, IDC_CONN_BCAST_INPUT),
		list_cntl = GetDlgItem(hdlg, IDC_CONN_BCAST_LIST);
	size_t list_sz, pos;
	char * ip_string;
	
	/* check that we've a valid broadcast mask entered */
	if(SendMessage(ip_cntl, IPM_GETADDRESS, 0, (LPARAM)&ip) != 4)
		return FALSE;

	if(!ip) return FALSE;
	
	/* check that there are no such ips entered into the list already */
	list_sz = SendMessage(list_cntl, LB_GETCOUNT, 0, 0);
	for(pos = 0; pos < list_sz; pos++) {
		DWORD another_ip = SendMessage(list_cntl, LB_GETITEMDATA, pos, 0);
		if(another_ip == ip)
			return FALSE;
	}

	/* add new item into the list */
	ip_string = malloc(16);
	sprintf(ip_string, "%u.%u.%u.%u",
		(unsigned)(ip >> 24) & 0xff, (unsigned)(ip >> 16) & 0xff,
		(unsigned)(ip >> 8) & 0xff, (unsigned)(ip >> 0) & 0xff);

	pos = SendMessage(list_cntl, LB_ADDSTRING, 0, (LPARAM)ip_string);
	SendMessage(list_cntl, LB_SETITEMDATA, (WPARAM)pos, (LPARAM)ip);

	free(ip_string);
	return TRUE;
}

static BOOL CALLBACK
options_network_dlgproc(HWND hdlg, UINT msg, WPARAM wp, LPARAM lp)
{
	static BOOL dlg_initalizing;

	BOOL ival_read;
	int ival;
	DWORD ip, * ip_list;
	size_t ip_list_sz, pos;

	switch(msg) {
	case WM_INITDIALOG:
		/* may not send PSM_CHANGED for now */
		dlg_initalizing = TRUE;

		/*
		 * "Connection Type" frame
		 */

		/* set port */
		SetDlgItemInt(
			hdlg, IDC_CONN_EDIT_PORT,
			db_word_get(NULL, VQCHAT_PROTO, "Port", VQCHAT_VQP_DEF_PORT),
			FALSE);

		/* set active connection type radio
		 */
		if(db_byte_get(NULL, VQCHAT_PROTO, "ProtoConn", 2) == 1) {
			CheckRadioButton(
				hdlg, IDC_CONN_RADIO_MULTICAST, IDC_CONN_RADIO_IPX,
				IDC_CONN_RADIO_IPX);
		} else {
			if(db_dword_get(NULL, VQCHAT_PROTO, "ProtoOpt", VQCHAT_VQP_DEF_PROTO_OPT)
					& VQP_PROTOCOL_OPT_MULTICAST) {
				CheckRadioButton(
					hdlg, IDC_CONN_RADIO_MULTICAST, IDC_CONN_RADIO_IPX,
					IDC_CONN_RADIO_MULTICAST);
			} else {
				CheckRadioButton(
					hdlg, IDC_CONN_RADIO_MULTICAST, IDC_CONN_RADIO_IPX,
					IDC_CONN_RADIO_BROADCAST);
			}
		}

		/* set multicast scope */
		SetDlgItemInt(
			hdlg, IDC_CONN_EDIT_SCOPE,
			db_byte_get(NULL, VQCHAT_PROTO, "MulticastScope", VQCHAT_VQP_DEF_SCOPE),
			FALSE);

		/* set multicast address */
		SendMessage(
			GetDlgItem(hdlg, IDC_CONN_IP_MULTICAST),
			IPM_SETADDRESS, 0, (LPARAM)db_dword_get(NULL, VQCHAT_PROTO,
							"Multicast", VQCHAT_VQP_DEF_MULTICAST));

		/* fill in broadcast masks list */
		ip_list = db_dword_list(NULL, VQCHAT_PROTO, "BroadcastMasks", &ip_list_sz);
		if(ip_list) {
			char * ip_string = malloc(16);
			size_t i;
			HWND item = GetDlgItem(hdlg, IDC_CONN_BCAST_LIST);
			SendMessageW(item, LB_RESETCONTENT, 0, 0);

			for(i = 0; i < ip_list_sz; i++) {
				LRESULT pos;

				ip = ip_list[i];
				sprintf(ip_string, "%u.%u.%u.%u",
					(unsigned)(ip >> 24) & 0xff, (unsigned)(ip >> 16) & 0xff,
					(unsigned)(ip >> 8) & 0xff, (unsigned)(ip >> 0) & 0xff);

				pos = SendMessage(item, LB_ADDSTRING, 0, (LPARAM)ip_string);
				SendMessage(item, LB_SETITEMDATA, (WPARAM)pos, (LPARAM)ip);
			}
			free(ip_string);
			free(ip_list);
		}
		EnableWindow(GetDlgItem(hdlg, IDC_CONN_BCAST_REMOVE), FALSE);
		
		/*
		 * "Miscellaneous Options" frame
		 */

		/* set user list update period */
		SetDlgItemInt(
			hdlg, IDC_CONN_EDIT_REFRESH,
			db_byte_get(NULL, VQCHAT_PROTO, "UserlistTimeout",
							VQCHAT_DEF_REFRESH_TIMEOUT),
			FALSE);

#ifdef VYPRESSCHAT
		/* set use/do not use utf-8 per default */
		CheckDlgButton(
			hdlg, IDC_CONN_CHECK_UTF8,
			user_codepage()==VQP_CODEPAGE_UTF8 ? BST_CHECKED: BST_UNCHECKED);
#else
		EnableWindow(GetDlgItem(hdlg, IDC_CONN_CHECK_UTF8), FALSE);
		EnableWindow(GetDlgItem(hdlg, IDC_CONN_LABEL_UTF8), FALSE);
#endif

		/* can send PSM_CHANGED now */
		dlg_initalizing = FALSE;
		break;

	case WM_COMMAND:
		switch(HIWORD(wp)) {
		case BN_CLICKED:
			switch(LOWORD(wp)) {
			case IDC_CONN_BTN_DEF_MULTICAST:
				SetDlgItemInt(hdlg, IDC_CONN_EDIT_PORT, VQCHAT_VQP_DEF_PORT, FALSE);
				SendMessage(GetDlgItem(hdlg, IDC_CONN_IP_MULTICAST),
					IPM_SETADDRESS, 0, (LPARAM)VQCHAT_VQP_DEF_MULTICAST);
				SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
				break;

			case IDC_CONN_BTN_DEF_PORT:
				SetDlgItemInt(hdlg, IDC_CONN_EDIT_PORT, VQCHAT_VQP_DEF_PORT, FALSE);
				SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
				break;

			case IDC_CONN_BCAST_ADD:
				if(options_network_dlgproc_bcast_add(hdlg))
					SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
				break;

			case IDC_CONN_BCAST_REMOVE:
				pos = SendMessage(GetDlgItem(hdlg, IDC_CONN_BCAST_LIST),
						LB_GETCURSEL, 0, 0);
				if(pos != LB_ERR) {
					SendMessage(GetDlgItem(hdlg, IDC_CONN_BCAST_LIST),
						LB_DELETESTRING, pos, 0);

					EnableWindow(
						GetDlgItem(hdlg, IDC_CONN_BCAST_REMOVE),
						SendMessage(
							GetDlgItem(hdlg, IDC_CONN_BCAST_LIST),
							LB_GETCURSEL, 0, 0
						) != LB_ERR
					);

					SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
				}
				break;

			case IDC_CONN_CHECK_UTF8:
			case IDC_CONN_RADIO_MULTICAST:
			case IDC_CONN_RADIO_BROADCAST:
			case IDC_CONN_RADIO_IPX:
				if(!dlg_initalizing)
					SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
				break;
			}
			break;
			
		case EN_CHANGE:
			if(!dlg_initalizing)
				SendMessage(GetParent(hdlg), PSM_CHANGED, 0, 0);
			break;

		case LBN_SELCHANGE:
			switch(LOWORD(wp)) {
			case IDC_CONN_BCAST_LIST:
				EnableWindow(
					GetDlgItem(hdlg, IDC_CONN_BCAST_REMOVE),
					SendMessage(
						GetDlgItem(hdlg, IDC_CONN_BCAST_LIST), LB_GETCURSEL,
						0, 0
					) != LB_ERR);
				break;
			}
			break;
		}
		break;

	case WM_NOTIFY:
		if(((LPNMHDR)lp)->code == PSN_APPLY) {
			DWORD * list;
			size_t list_sz, i;
			HWND item;
			
			/* set port value */
			ival = GetDlgItemInt(hdlg, IDC_CONN_EDIT_PORT, &ival_read, FALSE);
			if(!ival_read) {
				/* XXX: error message */
				SetFocus(GetDlgItem(hdlg, IDC_CONN_EDIT_PORT));
				return TRUE;
			}
			db_word_set(NULL, VQCHAT_PROTO, "Port", ival);

			/* set active connection type */
			if(IsDlgButtonChecked(hdlg, IDC_CONN_RADIO_IPX)) {
				/* IPX */
				db_byte_set(NULL, VQCHAT_PROTO, "ProtoConn", 1);
			} else {
				/* UDP (broadcast or multicast) */
				db_byte_set(NULL, VQCHAT_PROTO, "ProtoConn", 0);
				db_dword_set(
					NULL, VQCHAT_PROTO, "ProtoOpt",
					(db_dword_get(NULL, VQCHAT_PROTO,
							"ProtoOpt", VQCHAT_VQP_DEF_PROTO_OPT)
						& ~VQP_PROTOCOL_OPT_MULTICAST)
					| (IsDlgButtonChecked(hdlg, IDC_CONN_RADIO_MULTICAST)
						? VQP_PROTOCOL_OPT_MULTICAST: 0)
				);
			}

			/* set multicast scope value */
			ival = GetDlgItemInt(hdlg, IDC_CONN_EDIT_SCOPE, &ival_read, FALSE);
			if(!ival_read) {
				/* XXX: error message */
				SetFocus(GetDlgItem(hdlg, IDC_CONN_EDIT_SCOPE));
				return TRUE;
			}
			db_byte_set(NULL, VQCHAT_PROTO, "MulticastScope", ival);

			/* set multicast address */
			ip = 0;
			SendMessage(GetDlgItem(hdlg, IDC_CONN_IP_MULTICAST),
				IPM_GETADDRESS, 0, (LPARAM)&ip);
			if(ip == 0) {
				/* XXX: error message */
				SetFocus(GetDlgItem(hdlg, IDC_CONN_IP_MULTICAST));
				return TRUE;
			}
			db_dword_set(NULL, VQCHAT_PROTO, "Multicast", ip);

			/* store broadcast masks */
			item = GetDlgItem(hdlg, IDC_CONN_BCAST_LIST);
			list_sz = SendMessage(item, LB_GETCOUNT, 0, 0);
			if(list_sz) {
				list = malloc(sizeof(DWORD) * list_sz);
				for(i = 0; i < list_sz; i++)
					list[i] = SendMessage(item, LB_GETITEMDATA, i, 0);

				db_blob_set(NULL, VQCHAT_PROTO, "BroadcastMasks",
						list, sizeof(DWORD) * list_sz);
				free(list);
			} else {
				db_unset(NULL, VQCHAT_PROTO, "BroadcastMasks");
			}

			/* "Miscellaneous Options" frame
			 */

			/* set user list refresh period value */
			ival = GetDlgItemInt(hdlg, IDC_CONN_EDIT_REFRESH, &ival_read, FALSE);
			if(!ival_read) {
				/* XXX: error message */
				SetFocus(GetDlgItem(hdlg, IDC_CONN_EDIT_REFRESH));
				return TRUE;
			}
			db_byte_set(NULL, VQCHAT_PROTO, "UserlistTimeout", ival);

#ifdef VYPRESSCHAT
			/* set default encoding value */
			user_set_codepage(
				IsDlgButtonChecked(hdlg, IDC_CONN_CHECK_UTF8)
					? VQP_CODEPAGE_UTF8: VQP_CODEPAGE_LOCALE,
				TRUE
			);
#endif
		}
		break;
	}
	return FALSE;
}

static int options_hook_opt_initialise(
	WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp;
	
	memset(&odp, 0, sizeof(odp));
	odp.cbSize = sizeof(odp);
	odp.position = -800000000;
	odp.hInstance = g_hDllInstance;

	/* add user configuration page
	 */
	odp.pszTemplate = MAKEINTRESOURCE(IDD_USER);
	odp.pszGroup = Translate("Network");
	odp.pszTitle = Translate(VQCHAT_PROTO_NAME);
	odp.pfnDlgProc = options_user_dlgproc;
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);

	/* add network configuration page
	 */
	odp.pszTemplate = MAKEINTRESOURCE(IDD_CONN);
	odp.pszGroup = Translate("Network");
	odp.pszTitle = Translate(VQCHAT_PROTO_NAME " Network");
	odp.pfnDlgProc = options_network_dlgproc;
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);

	return 0; /* success */
}

static __inline void
options_userinfo_set_dlgitem(
	HWND hwnd, UINT item,
	HANDLE hContact, enum contact_property_enum property)
{
	char * str = contacts_get_contact_property(hContact, property);
	wchar_t * u_str = util_utf2uni(str);
	free(str);
	SetDlgItemTextW(hwnd, item, u_str);
	free(u_str);
}

static void options_userinfo_set_chanlist(
	HWND hdlg, UINT item, const char * c_chanlist)
{
	char * channel, * chanlist = chanlist_copy(c_chanlist);

	SendMessage(GetDlgItem(hdlg, item), LB_RESETCONTENT, 0, 0);

	while((channel = chanlist_shift(&chanlist)) != NULL) {
		char * mod_channel = malloc(strlen(channel) + 2);
		wchar_t * w_channel;
		
		/* prepend '#' to the channel name */
		mod_channel[0] = '#';
		strcpy(mod_channel + 1, channel);
		w_channel = util_utf2uni(mod_channel);
		free(mod_channel);
		free(channel);

		SendMessageW(GetDlgItem(hdlg, item), LB_ADDSTRING, 0, (LPARAM)w_channel);
		free(w_channel);
	}
}

static CALLBACK BOOL
options_userinfo_dlgproc(HWND hdlg, UINT nmsg, WPARAM wp, LPARAM lp)
{
	switch(nmsg) {
	case WM_COMMAND:
		if(HIWORD(wp)==LBN_DBLCLK && LOWORD(wp)==IDC_USERINFO_CHANNELS) {
			HWND hlist = GetDlgItem(hdlg, IDC_USERINFO_CHANNELS);
			
			/* join the channel user has clicked on
			 */
			int cursel = SendMessage(hlist, LB_GETCURSEL, 0, 0);
			if(cursel != LB_ERR) {
				int t_len = SendMessage(hlist, LB_GETTEXTLEN, cursel, 0);
				wchar_t * strbuf = malloc(sizeof(wchar_t) * (t_len + 1));
				char * channel;

				SendMessageW(hlist, LB_GETTEXT, cursel, (LPARAM)strbuf);
				channel = util_uni2utf(strbuf + 1);	/* skip '#' */
				free(strbuf);

				/* join the channel permanently */
				chatroom_channel_join(channel, 0);
				free(channel);
			}
		}
		break;
		
	case WM_NOTIFY:
		if(((LPNMHDR)lp)->code == PSN_INFOCHANGED) {
			char * nick;
			HANDLE hContact = (HANDLE)((LPPSHNOTIFY)lp)->lParam;
			ASSERT_RETURNVALIFFAIL(contacts_is_user_contact(hContact), FALSE);

			/* get contact nickname */
			nick = contacts_get_nickname(hContact);

			/* set node address */
			if(userlist_user_exists(nick)) {
				char * node_str = util_vqpaddr2str(userlist_user_addr(nick));
				SetDlgItemTextA(hdlg, IDC_USERINFO_NODE, node_str);
				free(node_str);
			} else {
				SetDlgItemTextA(hdlg, IDC_USERINFO_NODE, "(unknown)");
			}

			/* set string properties */
			options_userinfo_set_dlgitem(
				hdlg, IDC_USERINFO_COMPUTER, hContact, CONTACT_COMPUTER);
			options_userinfo_set_dlgitem(
				hdlg, IDC_USERINFO_USER, hContact, CONTACT_USER);
			options_userinfo_set_dlgitem(
				hdlg, IDC_USERINFO_PLATFORM, hContact, CONTACT_PLATFORM);
			options_userinfo_set_dlgitem(
				hdlg, IDC_USERINFO_WORKGROUP, hContact, CONTACT_WORKGROUP);
			options_userinfo_set_dlgitem(
				hdlg, IDC_USERINFO_SOFTWARE, hContact, CONTACT_SOFTWARE);

			/* fill in channel list */
			options_userinfo_set_chanlist(
				hdlg, IDC_USERINFO_CHANNELS, userlist_user_chanlist(nick));

			/* free nickname */
			free(nick);
		}
		break;
	}

	return FALSE;
}

static CALLBACK BOOL
options_useropt_dlgproc(HWND hdlg, UINT nmsg, WPARAM wp, LPARAM lp)
{
	HANDLE hContact = (HANDLE)GetWindowLongPtr(hdlg, GWLP_USERDATA);
	char * nick;

	switch(nmsg) {
	case WM_INITDIALOG:
		SendMessage(GetDlgItem(hdlg, IDC_USEROPT_PREFERS), CB_ADDSTRING,
			0, (LPARAM)"Private chats");
		SendMessage(GetDlgItem(hdlg, IDC_USEROPT_PREFERS), CB_ADDSTRING,
			0, (LPARAM)"Messages");
		break;

	case WM_COMMAND:
		switch(HIWORD(wp)) {
		case BN_CLICKED:
			switch(LOWORD(wp)) {
			case IDC_USEROPT_SET:
				nick = util_GetDlgItemTextUtf(hdlg, IDC_USEROPT_NICKNAME);
				if(strlen(nick))
					contacts_set_contact_nickname(hContact, nick);

				free(nick);
				break;
				
			case IDC_USEROPT_LOCKNICK:
				db_byte_set(hContact, VQCHAT_PROTO, "LockNick",
					IsDlgButtonChecked(hdlg, IDC_USEROPT_LOCKNICK) ? 1: 0);
				break;
			}
			break;

		case CBN_SELCHANGE:
			switch(LOWORD(wp)) {
			case IDC_USEROPT_PREFERS:
				nick = util_GetDlgItemTextUtf(hdlg, IDC_USEROPT_NICKNAME);
				if(userlist_user_swversion(nick) < VQP_MAKE_SWVERSION(2, 0)) {
					db_byte_set(
						hContact, VQCHAT_PROTO, "PreferMsg",
						SendMessage(
							GetDlgItem(hdlg, IDC_USEROPT_PREFERS),
							CB_GETCURSEL, 0, 0
						) == 1
					);
				}
				free(nick);
				break;
			}
			break;
		}
		break;

	case WM_NOTIFY:
		if(((LPNMHDR)lp)->code == PSN_INFOCHANGED) {
			char * nick;
			HANDLE hContact = (HANDLE)((LPPSHNOTIFY)lp)->lParam;

			ASSERT_RETURNVALIFFAIL(contacts_is_user_contact(hContact), FALSE);

			/* save current contact handle */
			SetWindowLongPtr(hdlg, GWLP_USERDATA, (UINT_PTR)hContact);

			/* set current contact nickname */
			nick = contacts_get_nickname(hContact);
			util_SetDlgItemTextUtf(hdlg, IDC_USEROPT_NICKNAME, nick);

			/* set if contact prefers msg */
			if(userlist_user_swversion(nick) < VQP_MAKE_SWVERSION(2, 0)) {
				EnableWindow(GetDlgItem(hdlg, IDC_USEROPT_PREFERS), TRUE);
				SendMessage(
					GetDlgItem(hdlg, IDC_USEROPT_PREFERS),
					CB_SETCURSEL,
					db_byte_get(hContact, VQCHAT_PROTO, "PreferMsg", 0) ? 1: 0,
					0);
			} else {
				/* vypress chat 2.0+ doesn't have private chats */
				EnableWindow(GetDlgItem(hdlg, IDC_USEROPT_PREFERS), FALSE);
				SendMessage(GetDlgItem(hdlg, IDC_USEROPT_PREFERS),
								CB_SETCURSEL, 1, 0);
			}

			free(nick);

			/* set if we want the contact nickname to not change */
			CheckDlgButton(hdlg, IDC_USEROPT_LOCKNICK,
				db_byte_get(hContact, VQCHAT_PROTO, "LockNick", 0)
					? BST_CHECKED: BST_UNCHECKED);
		}
		break;
	}

	return FALSE;
}

static int options_hook_userinfo_initialise(WPARAM wp, LPARAM lp)
{
	OPTIONSDIALOGPAGE odp;
	HANDLE hContact = (HANDLE)lp;

	if(hContact) {
		if(contacts_is_user_contact(hContact)) {
			/* we show additional property sheets for user contacts only
			 */
	
			/* basic user info page */
			memset(&odp, 0, sizeof(odp));
			odp.cbSize = sizeof(odp);
			odp.position = -1900000000;
			odp.pszTitle = VQCHAT_PROTO_NAME;
			odp.hInstance = g_hDllInstance;
			odp.pfnDlgProc = options_userinfo_dlgproc;
			odp.pszTemplate = MAKEINTRESOURCE(IDD_USERINFO);
			CallService(MS_USERINFO_ADDPAGE, wp, (LPARAM)&odp);

			/* user ID page */ 
			odp.pszTitle = VQCHAT_PROTO_NAME " options";
			odp.pfnDlgProc = options_useropt_dlgproc;
			odp.pszTemplate = MAKEINTRESOURCE(IDD_USEROPT);
			CallService(MS_USERINFO_ADDPAGE, wp, (LPARAM)&odp);
		}
	} else {
		/* XXX: add tabs for options of 'myself' */
	}

	return 0;
}

/* options_load_settings:
 *	loads settings from database
 *	(and presets some stuff if not already present in db)
 */
static void options_load_settings()
{
	DBVARIANT dbv;
	int rc;

	/* get user name */
	rc = db_get(NULL, VQCHAT_PROTO, "Nick", &dbv);
	if(!rc && dbv.type==DBVT_ASCIIZ) {
		if(strlen(dbv.pszVal) != 0) {
			/* ok, got name from db */
			user_set_nickname(dbv.pszVal, FALSE);
		}
		db_free(&dbv);
	}

	/* get user gender */
	rc = db_byte_get(NULL, VQCHAT_PROTO, "Gender", 0);
	if(rc != 'M' && rc != 'F') {
		user_set_gender(VQP_GENDER_MALE, TRUE);
	} else {
		user_set_gender(rc == 'F' ? VQP_GENDER_FEMALE: VQP_GENDER_MALE, FALSE);
	}

	/* emit msg by default on alert beep */
	rc = db_byte_get(NULL, VQCHAT_PROTO, "MsgOnAlertBeep", 10);
	if(rc != 0 && rc != 1)
		db_byte_set(NULL, VQCHAT_PROTO, "MsgOnAlertBeep", 1);

	/* append nickname after topic */
	rc = db_byte_get(NULL, VQCHAT_PROTO, "NicknameOnTopic", 10);
	if(rc != 0 && rc != 1)
		db_byte_set(NULL, VQCHAT_PROTO, "NicknameOnTopic", 1);

	/* new contacts prefer private messages */
	rc = db_byte_get(NULL, VQCHAT_PROTO, "ContactsPreferMsg", 10);
	if(rc != 0 && rc != 1)
		db_byte_set(NULL, VQCHAT_PROTO, "ContactsPreferMsg", 0);

	/* check if port is set */
	if(db_word_get(NULL, VQCHAT_PROTO, "Port", 0) == 0)
		db_word_set(NULL, VQCHAT_PROTO, "Port", VQCHAT_VQP_DEF_PORT);

	/* check that protocol options are set */
	if(db_dword_get(NULL, VQCHAT_PROTO, "ProtoOpt", (DWORD)-1) == (DWORD)-1)
		db_dword_set(NULL, VQCHAT_PROTO, "ProtoOpt", VQCHAT_VQP_DEF_PROTO_OPT);

	/* check connection type (the default is IP) */
	rc = db_byte_get(NULL, VQCHAT_PROTO, "ProtoConn", 10);
	if(rc != 0 && rc != 1)
		db_byte_set(NULL, VQCHAT_PROTO, "ProtoConn", 0);

	/* load (or generate a new) user uuid */
	rc = 1;	/* will get reset to 0, if we already have valid uuid set */
	if(!db_get(NULL, VQCHAT_PROTO, "Uuid", &dbv)) {
		if(dbv.type == DBVT_ASCIIZ) {
			/* parse uuid from string */
			vqp_uuid_t uuid;
			if(!vqp_uuid_from_string(&uuid, dbv.pszVal)) {
				/* set uuid */
				user_set_uuid(&uuid, FALSE);
				rc = 0;
			}
		}
	}
	if(rc) {
		/* generate new uuid and store it in database */
		vqp_uuid_t uuid;
		vqp_uuid_create(&uuid);
		user_set_uuid(&uuid, TRUE);
	}
	
	/* load multicast settings */
	if(db_dword_get(NULL, VQCHAT_PROTO, "Multicast", 0) == 0)
		db_dword_set(NULL, VQCHAT_PROTO, "Multicast", VQCHAT_VQP_DEF_MULTICAST);

	if(db_byte_get(NULL, VQCHAT_PROTO, "MulticastScope", 0) == 0)
		db_byte_set(NULL, VQCHAT_PROTO, "MulticastScope", VQCHAT_VQP_DEF_SCOPE);
				
	/* check user list timeout value */
	if(db_get(NULL, VQCHAT_PROTO, "UserlistTimeout", &dbv) || dbv.type!=DBVT_BYTE) {
		/* setting not set, use default */
		db_byte_set(NULL, VQCHAT_PROTO,
				"UserlistTimeout", VQCHAT_DEF_REFRESH_TIMEOUT);
	} else {
		/* check if timeout value is within valid range */
		if(dbv.type!=DBVT_BYTE) {
			db_unset(NULL, VQCHAT_PROTO, "UserlistTimeout");
			db_byte_set(NULL, VQCHAT_PROTO,
				"UserlistTimeout", VQCHAT_DEF_REFRESH_TIMEOUT);
		} else if((unsigned char)dbv.cVal < VQCHAT_MIN_REFRESH_TIMEOUT) {
			db_byte_set(NULL, VQCHAT_PROTO,
				"UserlistTimeout", VQCHAT_MIN_REFRESH_TIMEOUT);
		} else if((unsigned char)dbv.cVal > VQCHAT_MAX_REFRESH_TIMEOUT) {
			db_byte_set(NULL, VQCHAT_PROTO,
				"UserlistTimeout", VQCHAT_MAX_REFRESH_TIMEOUT);
		}
	}

	/* get default codepage */
#ifdef VYPRESSCHAT
	rc = db_byte_get(NULL, VQCHAT_PROTO, "Codepage", 2);
	if(rc != 0 && rc != 1) {
		/* default to utf8 encoding on vypress chat networks */
		user_set_codepage(VQP_CODEPAGE_UTF8, TRUE);
	} else {
		/* codepage setting was read successfully */
		user_set_codepage(rc==0 ? VQP_CODEPAGE_LOCALE: VQP_CODEPAGE_UTF8, 0);
	}
#else
	/* we always default to locale encoding on quickchat networks */
	user_set_codepage(VQP_CODEPAGE_LOCALE, FALSE);
#endif

	/* load user's channel list */
	rc = db_get(NULL, VQCHAT_PROTO, "Chanlist", &dbv);
	
	if(!rc && !strlen(dbv.pszVal)) {
		/* chanlist is valid, but empty */
		user_set_chanlist(NULL, 0);
	} else {
		if(rc || (!rc && dbv.type!=DBVT_ASCIIZ)
				|| (!rc && !chanlist_is_valid(dbv.pszVal, 1)))
		{
			/* user list is unset or invalid - set to default:
			 * "#Main" channel */
			char * default_chanlist = chanlist_add(NULL, VQCHAT_MAIN_CHANNEL);
			user_set_chanlist(default_chanlist, TRUE);
			chanlist_free(default_chanlist);
		} else {
			/* chanlist is valid: set it */
			user_set_chanlist(dbv.pszVal, FALSE);
		}
	}
	if(!rc) db_free(&dbv);

	/* convert broadcasts masks from old-style
	 */
	if(!db_get(NULL, VQCHAT_PROTO, "Broadcast#0", &dbv) && dbv.type==DBVT_DWORD) {
		if(dbv.dVal)
			db_dword_list_add(NULL, VQCHAT_PROTO, "BroadcastMasks", dbv.dVal, TRUE);
		db_unset(NULL, VQCHAT_PROTO, "Broadcast#0");
	}
	if(!db_get(NULL, VQCHAT_PROTO, "Broadcast#1", &dbv) && dbv.type==DBVT_DWORD) {
		if(dbv.dVal)
			db_dword_list_add(NULL, VQCHAT_PROTO, "BroadcastMasks", dbv.dVal, TRUE);
		db_unset(NULL, VQCHAT_PROTO, "Broadcast#1");
	}
	if(!db_get(NULL, VQCHAT_PROTO, "Broadcast#2", &dbv) && dbv.type==DBVT_DWORD) {
		if(dbv.dVal)
			db_dword_list_add(NULL, VQCHAT_PROTO, "BroadcastMasks", dbv.dVal, TRUE);
		db_unset(NULL, VQCHAT_PROTO, "Broadcast#2");
	}
}


/* exported routines
 */

void options_init()
{
}

void options_hook_modules_loaded()
{
	options_load_settings();

	s_hook_opt_initialise = HookEvent(ME_OPT_INITIALISE, options_hook_opt_initialise);
	s_hook_userinfo_initialise
		= HookEvent(ME_USERINFO_INITIALISE, options_hook_userinfo_initialise);
}

void options_uninit()
{
	UnhookEvent(s_hook_opt_initialise);
	UnhookEvent(s_hook_userinfo_initialise);
}

void options_show_network_options()
{
	OPENOPTIONSDIALOG ood;
	memset(&ood, 0, sizeof(ood));
	ood.cbSize = sizeof(ood);
	ood.pszGroup = Translate("Network");
	ood.pszPage = Translate(VQCHAT_PROTO_NAME " Network");

	CallService(MS_OPT_OPENOPTIONS, 0, (LPARAM)&ood);
}

void options_show_user_options()
{
	OPENOPTIONSDIALOG ood;
	memset(&ood, 0, sizeof(ood));
	ood.cbSize = sizeof(ood);
	ood.pszGroup = Translate("Network");
	ood.pszPage = Translate(VQCHAT_PROTO_NAME);

	CallService(MS_OPT_OPENOPTIONS, 0, (LPARAM)&ood);
}

