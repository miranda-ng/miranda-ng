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
 * $Id: chatroom.c,v 1.44 2005/03/17 11:02:43 bobas Exp $
 */

#include <time.h>
#include "miranda.h"
#include "contrib/hashtable.h"
#include "contrib/strhashfunc.h"
#include "libvqproto/vqproto.h"

#include "main.h"
#include "user.h"
#include "userlist.h"
#include "chatroom.h"
#include "msgloop.h"
#include "chanlist.h"
#include "util.h"
#include "contacts.h"
#include "resource.h"

/* struct defs
 */

#define VQCHAT_WM_INITCS	(WM_USER + 1)

#define CHATROOM_NORMAL_STATUS	"Normal"

struct chatroom_status_entry {
	char	* topic;
};

/* static data
 */
static BOOL	s_fHaveChat;	/* TRUE, if we have chat.dll in miranda */
static HANDLE	s_hGcUserEvent, s_hGcMenuBuild;
static struct hashtable
		* s_channelHash;
static HWND	s_hwndChannelSettingsDlg;
static vqp_link_t s_vqpLink;
static UINT_PTR s_chatroom_connected_join_timer;

/* static routines
 */

static void chatroom_free_channelhash_value(void * value)
{
	struct chatroom_status_entry * entry = (struct chatroom_status_entry *)value;
	if(entry->topic)
		free(entry->topic);
	free(entry);
}

static char * chatroom_make_name(const char * channel)
{
	int len;
	char * channel_name;

	ASSERT_RETURNVALIFFAIL(VALIDPTR(channel), NULL);

	len = strlen(channel);
	channel_name = malloc(len + 2);
	channel_name[0] = '#';
	memcpy(channel_name + 1, channel, len + 1);

	return channel_name;
}

static void chatroom_send_net_join(const char * channel)
{
	char * r_channel, * r_nickname;

	ASSERT_RETURNIFFAIL(VALIDPTR(channel));
	
	r_channel = util_utf2vqp(user_codepage(), channel);
	r_nickname = util_utf2vqp(user_codepage(), user_nickname());

	/* send a join msg */
	msgloop_send(
		vqp_msg_channel_join(
			s_vqpLink, r_channel, r_nickname, user_vqp_status(), user_gender(),
			VQCHAT_VQP_SWVERSION, user_p_uuid(), user_codepage(), VQCHAT_VQP_DEF_COLOR),
		0);

	/* send request who-is-here-on-this-channel? */
	msgloop_send(vqp_msg_channel_whohere_req(s_vqpLink, r_channel, r_nickname), 0);

	free(r_channel);
	free(r_nickname);
}

static void chatroom_send_net_part(const char * channel)
{
	char * r_channel = util_utf2vqp(user_codepage(), channel),
		* r_nickname = util_utf2vqp(user_codepage(), user_nickname());

	msgloop_send(vqp_msg_channel_leave(s_vqpLink, r_channel, r_nickname, user_gender()), 1);
	free(r_channel);
	free(r_nickname);
}

static void chatroom_svc_gc_event_user_message(
	const char * channel, const char * message)
{
	/* chatroom_svc_gc_event_user_message:
	 *	handles a message the user has typed
	 *	and emits it onto the channel window and onto the
	 *	network
	 */
	char ** msg_lines;
	int i;

	ASSERT_RETURNIFFAIL(VALIDPTR(channel) && VALIDPTR(message));
	
	/* split the message into multiple lines (and skip the empty ones) */
	msg_lines = util_split_multiline(message, 0);

	/* emit message lines to the network */
	for(i = 0; msg_lines[i]; i++) {
		char * r_channel, * r_nickname, * r_text;
		enum vqp_codepage cp = user_codepage();
			
		/* emit this text on the channel window */
		chatroom_channel_user_text(channel, user_nickname(), msg_lines[i], 0);

		/* send the message to the network */
		msgloop_send(
			vqp_msg_channel_text(
				s_vqpLink, r_channel = util_utf2vqp(cp, channel),
				r_nickname = util_utf2vqp(cp, user_nickname()),
				r_text = util_utf2vqp(cp, msg_lines[i]), 0),
			0);
		free(r_channel);
		free(r_nickname);
		free(r_text);
	}

	/* free the string list */
	util_free_str_list(msg_lines);
}

static void chatroom_svc_gc_event_user_nicklistmenu(GCHOOK * pHook)
{
	PROTOSEARCHRESULT psr;
	ADDCONTACTSTRUCT acs;
	char * dst;
	
	switch(pHook->dwData) {
	case 1: /* "Beep" */
		dst = util_loc2utf(pHook->pszUID);
		if(!user_is_my_nickname(dst)) {
			char * r_dst, * r_nickname;
			enum vqp_codepage cp = userlist_user_codepage(dst);
			r_nickname = util_utf2vqp(cp, user_nickname());
			r_dst = util_utf2vqp(cp, dst);

			msgloop_send_to(
				vqp_msg_beep_signal(s_vqpLink, r_nickname, r_dst),
				0, userlist_user_addr(dst));
			free(r_nickname);
			free(r_dst);
		}
		free(dst);
		break;
		
	case 2: /* "Add User" */
		memset(&psr, 0, sizeof(psr));
		psr.cbSize = sizeof(psr);
		psr.nick = pHook->pszUID;
		memset(&acs, 0, sizeof(acs));
		acs.handleType = HANDLE_SEARCHRESULT;
		acs.szProto = VQCHAT_PROTO;
		acs.psr = &psr;

		CallService(MS_ADDCONTACT_SHOW, (WPARAM)NULL, (LPARAM)&acs);	
		break;
	}
}

static BOOL CALLBACK chatroom_channel_settings_wndproc(
	HWND hdlg, UINT nMsg,
	WPARAM wParam, LPARAM lParam)
{
	char * channel, * topic;
	
	switch(nMsg) {
	case WM_INITDIALOG:
		/* set dialog icon */
		SendMessage(
			hdlg, WM_SETICON, ICON_SMALL,
			(LPARAM)LoadIcon(g_hDllInstance,
					 MAKEINTRESOURCE(IDI_CHATROOM)));
		SendMessage(
			hdlg, WM_SETICON, ICON_BIG,
			(LPARAM)(HICON)LoadImage(
				g_hDllInstance, MAKEINTRESOURCE(IDI_VQCHAT_PROTO_LARGE),
				IMAGE_ICON, GetSystemMetrics(SM_CXICON),
				GetSystemMetrics(SM_CYICON), LR_SHARED)
		);
		break;
	
	case VQCHAT_WM_INITCS:
		channel = (char *)lParam;

		util_SetDlgItemTextUtf(hdlg, IDC_CS_EDIT_NAME, channel);
		util_SetDlgItemTextUtf(
			hdlg, IDC_CS_EDIT_TOPIC, chatroom_channel_get_topic(channel));

		SendDlgItemMessageW(hdlg, IDC_CS_EDIT_TOPIC, EM_SETSEL, (WPARAM)0, (LPARAM)-1);

		/* make dialog title */
		topic = malloc(strlen(channel) + 64);
		if(topic) {
			sprintf(topic, "#%s channel settings", channel);
			util_SetWindowTextUtf(hdlg, topic);
			free(topic);
		}

		/* disable the 'set-topic' button */
		EnableWindow(GetDlgItem(hdlg, IDC_CS_BTN_SET), FALSE);
		break;

	case WM_COMMAND:
		switch(HIWORD(wParam)) {
		case BN_CLICKED:
			switch(LOWORD(wParam)) {
			case IDC_CS_BTN_SET:
			case IDOK:
				/* set channel topic */
				channel = util_GetDlgItemTextUtf(hdlg, IDC_CS_EDIT_NAME);
				topic = util_GetDlgItemTextUtf(hdlg, IDC_CS_EDIT_TOPIC);
				if(channel && topic) {
					if(db_byte_get(NULL, VQCHAT_PROTO, "NicknameOnTopic", 0)) {
						char * new_topic = malloc(strlen(topic)
								+ strlen(user_nickname()) + 8);
						sprintf(new_topic, "%s (%s)",
								topic, user_nickname());
						chatroom_channel_topic_change(
								channel, new_topic, 1, 1);
						free(new_topic);
					} else {
						chatroom_channel_topic_change(channel, topic, 1, 1);
					}
				}
				if(topic) free(topic);
				if(channel) free(channel);
				
				/* disable the 'set-topic' button */
				EnableWindow(GetDlgItem(hdlg, IDC_CS_BTN_SET), FALSE);

				/* close the window (if pressed OK btn) */
				if(LOWORD(wParam) == IDOK)
					SendMessage(hdlg, WM_CLOSE, 0,0);
				break;
			}
			break;
			
		case EN_CHANGE:
			switch(LOWORD(wParam)) {
			case IDC_CS_EDIT_TOPIC:
				/* enable the set-topic button on topic
				 * entry change */
				EnableWindow(GetDlgItem(hdlg, IDC_CS_BTN_SET), TRUE);
				break;
			}
			break;
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hdlg);
		s_hwndChannelSettingsDlg = NULL;
		break;
	}

	return (FALSE);
}

static void chatroom_svc_gc_event_user_logmenu(GCHOOK * pHook)
{
	char * channel;
	
	switch(pHook->dwData) {
	case 1:
		channel = util_loc2utf(pHook->pDest->pszID);
		chatroom_channel_show_settings_dlg(channel);
		free(channel);
		break;
	}
}

static int chatroom_svc_gc_event(WPARAM wParam, LPARAM lParam)
{
	GCHOOK * pHook = (GCHOOK *)lParam;
	HANDLE hContact;
	char * channel, * text, * dst;

	ASSERT_RETURNVALIFFAIL(VALIDPTR(pHook) && VALIDPTR(pHook->pDest), 0);

	if(strcmp(pHook->pDest->pszModule, VQCHAT_PROTO))
		return 0;

	switch(pHook->pDest->iType) {
	case GC_USER_MESSAGE:
		channel = util_loc2utf(pHook->pDest->pszID);
		text = util_loc2utf(pHook->pszText);
		
		chatroom_svc_gc_event_user_message(channel, text);
		free(channel);
		free(text);
		break;

	case GC_USER_CHANMGR:
		channel = util_loc2utf(pHook->pDest->pszID);
		chatroom_channel_show_settings_dlg(channel);
		free(channel);
		break;

	case GC_USER_PRIVMESS:
		dst = util_loc2utf(pHook->pszUID);
		if(!user_is_my_nickname(dst)) {
			hContact = contacts_add_contact(dst, 0);
			if(hContact)
				CallService(MS_MSG_SENDMESSAGE, (WPARAM)hContact, 0);
		}
		free(dst);
		break;

	case GC_USER_NICKLISTMENU:
		chatroom_svc_gc_event_user_nicklistmenu(pHook);
		break;

	case GC_USER_LOGMENU:
		chatroom_svc_gc_event_user_logmenu(pHook);
		break;
	}
	
	return 0;
}

static int chatroom_svc_gc_buildmenu(WPARAM wParam, LPARAM lParam)
{
	static struct gc_item usermenu_items[] = {
		{ "Alert Beep", 1, MENU_ITEM, FALSE },
		{ "", 0, MENU_POPUPSEPARATOR, FALSE },
		{ "&Add User", 2, MENU_ITEM, FALSE }
	};
	static struct gc_item chansettings_items[] = {
		{ "Channel &settings", 1, MENU_ITEM, FALSE }
	};

	GCMENUITEMS * gcmi = (GCMENUITEMS *)lParam;

	if(gcmi) {
		if(!lstrcmpi(gcmi->pszModule, VQCHAT_PROTO)) {
			char * dst = util_loc2utf(gcmi->pszUID);
			
			switch(gcmi->Type) {
			case MENU_ON_NICKLIST:
				gcmi->nItems = 3;
				gcmi->Item = usermenu_items;

				/* disable 'Add User' if already added */
				gcmi->Item[2].bDisabled =
					user_is_my_nickname(dst)
					|| contacts_find_contact(dst)!=NULL;
				break;

			case MENU_ON_LOG:
				gcmi->nItems = 1;
				gcmi->Item = chansettings_items;
				break;
			}
			free(dst);
		}
	}
	
	return 0;
}

static __inline void chatroom_setup_event(
	GCDEST * pDest, GCEVENT * pEvent, const char * channel, int event_type)
{
	pDest->pszModule = VQCHAT_PROTO;
	pDest->pszID = (char *)channel;
	pDest->iType = event_type;

	memset(pEvent, 0, sizeof(GCEVENT));
	pEvent->cbSize = sizeof(GCEVENT);
	pEvent->pDest = pDest;
	pEvent->time = time(NULL);
}
static __inline char * chatroom_escape_message(const char * message)
{
	int msg_len = strlen(message);
	char * escaped = malloc(msg_len * 2 + 1);	/* to be safe */
	char * p_escaped = escaped;

	do {
		if(*message == '%')
			*(p_escaped++) = '%';
		
		*(p_escaped++) = *message;
	} while(*(message++));

	return escaped;
}

static void chatroom_flush_channel_hash()
{
	if(s_channelHash)
		hashtable_destroy(s_channelHash, 1);

	s_channelHash = create_hashtable(
		32, hashtable_strhashfunc, hashtable_strequalfunc,
		chatroom_free_channelhash_value);
}

/* exported routines
 */
void chatroom_init()/*{{{*/
{
	s_fHaveChat = FALSE;
	s_channelHash = NULL;
	s_hwndChannelSettingsDlg = NULL;
	s_chatroom_connected_join_timer = 0;
}/*}}}*/

void chatroom_uninit()/*{{{*/
{
	if(s_hwndChannelSettingsDlg) {
		SendMessage(s_hwndChannelSettingsDlg, WM_CLOSE, 0, 0);
		s_hwndChannelSettingsDlg = NULL;
	}

	if(s_channelHash) {
		hashtable_destroy(s_channelHash, 1);
		s_channelHash = NULL;
	}

	if(s_fHaveChat) {
		/* unhook all the hooks */
		UnhookEvent(s_hGcUserEvent);
		UnhookEvent(s_hGcMenuBuild);
	}

	if(s_chatroom_connected_join_timer) {
		KillTimer(NULL, s_chatroom_connected_join_timer);
		s_chatroom_connected_join_timer = 0;
	}
}/*}}}*/

void chatroom_hook_modules_loaded()/*{{{*/
{
	/* check if we have m3x's Chat module in miranda */
	s_fHaveChat = ServiceExists(MS_GC_REGISTER);

	if(s_fHaveChat) {
		/* register with the Chat module */
		GCREGISTER gcr;
		gcr.cbSize = sizeof(gcr);
		gcr.dwFlags = GC_CHANMGR;
		gcr.pszModule = VQCHAT_PROTO;
		gcr.pszModuleDispName = VQCHAT_PROTO_NAME;
		gcr.iMaxText = VQP_MAX_PACKET_SIZE; /* approximation */
		gcr.nColors = 0;
		gcr.pColors = NULL;	/* hope this works */
		CallService(MS_GC_REGISTER, 0, (LPARAM)&gcr);

		/* hook to hooks */
		s_hGcUserEvent = HookEvent(ME_GC_EVENT, chatroom_svc_gc_event);
		s_hGcMenuBuild = HookEvent(ME_GC_BUILDMENU,
						chatroom_svc_gc_buildmenu);
	} else {
		/* ask the user to get the Chat module */
		if(IDYES == MessageBox(NULL,
			"The " VQCHAT_PROTO_NAME " depends on another plugin"
			" \"Chat\".\nDo you want to download it from the"
			" Miranda IM web site now?",
			"Information",
			MB_YESNO|MB_ICONINFORMATION)
			)
		{
			CallService(MS_UTILS_OPENURL, 1,
				(LPARAM)"http://www.miranda-im.org/download/"
					"details.php?action=viewfile&id=1309");
		}
			
	}
}/*}}}*/

/* chatroom_module_installed:
 *	returns TRUE if the "Chat" module is installed
 */
int chatroom_module_installed()/*{{{*/
{
	return s_fHaveChat;
}/*}}}*/

static int chatroom_connected_join_enum(const char * channel, void * data)/*{{{*/
{
	chatroom_channel_join(channel, 1);
	return 1; /* keep enumerating */
}/*}}}*/

static void CALLBACK chatroom_connected_join_timer_cb(/*{{{*/
	HWND hWnd, UINT nMsg, UINT_PTR nEvent, DWORD dwTime)
{
	/* join all of user's channels */
	chanlist_enum(*user_p_chanlist(), chatroom_connected_join_enum, NULL);

	/* this is a single-shot timer */
	KillTimer(0, s_chatroom_connected_join_timer);
	s_chatroom_connected_join_timer = 0;
}/*}}}*/

void chatroom_connected(vqp_link_t link)/*{{{*/
{
	char * r_src;
	
	s_vqpLink = link;
	
	/* create/flush channel status entry hash */
	chatroom_flush_channel_hash();

	/* join all of user's channels after a second
	 */
	s_chatroom_connected_join_timer
		= SetTimer(NULL, 0, 500, chatroom_connected_join_timer_cb);

	/* join the Main channel (not-really, just for the effect)
	 * (in case the user doesn't have it on the his chanlist)
	 */
	chatroom_send_net_join(VQCHAT_MAIN_CHANNEL);

	/* set active status update */
	r_src = util_utf2vqp(user_codepage(), user_nickname());
	msgloop_send(vqp_msg_active_change(s_vqpLink, r_src, VQCHAT_UNDEF_ACTIVE), 0);
	free(r_src);
}/*}}}*/

static int chatroom_disconnected_part_enum(const char * channel, void * data)
{
	chatroom_channel_part(channel, 1);
	return 1; /* keep enumerating */
}

void chatroom_disconnected()
{
	/* part the #Main channel, - this will leave the network */
	chatroom_send_net_part(VQCHAT_MAIN_CHANNEL);

	/* leave all the channels in user's chanlist */
	chanlist_enum(*user_p_chanlist(), chatroom_disconnected_part_enum, NULL);

	s_vqpLink = NULL;
}

/* chatroom_is_valid_channel:
 *	validates that channel name is valid
 * returns:
 *	0 if invalid,
 *	non-0 if valid
 */
int chatroom_is_valid_channel(const char * channel)
{
	ASSERT_RETURNVALIFFAIL(VALIDPTR(channel), 0);

	/* channel name must be of non-0 length
	 * and channel name must not contain a '#' char
	 */
	return strlen(channel)!=0 && strchr(channel, '#')==NULL;
}

/* chatroom_channel_join:
 *	  Join the specified channel, and add it to the user's chanlist
 *	(if not there already).
 *	  The connected_event param is set to non-0 only when joining the
 *	channels after connecting to the network.
 */
static int chatroom_channel_join_userlist_enum_fn(
	const char * user_name, void * channel)
{
	/* join the channel, if the user is there */
	if(chanlist_contains(userlist_user_chanlist(user_name), (const char *)channel)) {
		GCDEST gcd;
		GCEVENT gce;
		char * l_user_name = util_utf2loc(user_name);

		chatroom_setup_event(&gcd, &gce, (const char *)channel, GC_EVENT_JOIN);
		gce.pszStatus = CHATROOM_NORMAL_STATUS;
		gce.bIsMe = FALSE;
		gce.bAddToLog = FALSE;
		gce.pszNick = l_user_name;
		gce.pszUID = l_user_name;
		CallService(MS_GC_EVENT, 0, (LPARAM)&gce);

		free(l_user_name);
	}

	return 1; /* keep enumerating */
}

/* chatroom_channel_join:
 *	joins the specified channel and updates channel list
 *	(if @connecting_to_net != 0)
 * params:
 *	@connecting_to_net - non-0 if the chatroom_channel_join() was called
 *		due to connecting-to-the-net event
 */
void chatroom_channel_join(
	const char * channel, int connecting_to_net)
{
	char * chanlist;
	struct chatroom_status_entry * entry;
	
	ASSERT_RETURNIFFAIL(!user_offline());

	/* validate channel name */
	if(!chatroom_is_valid_channel(channel))
		return;

	/* add channel entry to hashtable
	 */
	entry = (struct chatroom_status_entry*)hashtable_search(s_channelHash, (void*)channel);
	if(entry == NULL) {
		entry = malloc(sizeof(struct chatroom_status_entry));
		entry->topic = strdup("");
		hashtable_insert(s_channelHash, strdup(channel), entry);
	}

	/* update user's channel list
	 */
	chanlist = *user_p_chanlist();
	if(!connecting_to_net) {
		/* don't do anything if we've joined the channel already */
		if(chanlist_contains(chanlist, channel))
			return;

		/* add this channel to chanlist and store it in db */
		user_set_chanlist(*user_p_chanlist() = chanlist_add(chanlist, channel), TRUE);
	}

	/* send join msg to the network:
	 *	dont do this for the MAIN channel, as it is considered
	 *	"special"
	 */
	if(strcmp(channel, VQCHAT_MAIN_CHANNEL))
		chatroom_send_net_join(channel);

	/* do stuff with Chat module */
	if(s_fHaveChat) {
		GCWINDOW gcw;
		char * channel_name = chatroom_make_name(channel),
			* l_channel_name = util_utf2loc(channel_name),
			* l_current_topic = util_utf2loc(entry->topic);
		free(channel_name);

		/* create window */
		gcw.cbSize = sizeof(gcw);
		gcw.iType = GCW_CHATROOM;
		gcw.pszModule = VQCHAT_PROTO;
		gcw.pszName = l_channel_name;
		gcw.pszID = channel;
		gcw.pszStatusbarText = NULL;
		gcw.bDisableNickList = FALSE;
		gcw.dwItemData = 0;

		if(!CallService(MS_GC_NEWCHAT, 0, (LPARAM)&gcw)) {
			/* setup channel's window */
			GCDEST gcd;
			GCEVENT gce;
			char * l_user_nickname = util_utf2loc(user_nickname());

			/* register the "Normal" status */
			chatroom_setup_event(&gcd, &gce, channel, GC_EVENT_ADDGROUP);
			gce.pszStatus = CHATROOM_NORMAL_STATUS;
			CallService(MS_GC_EVENT, 0, (LPARAM)&gce);

			/* fill in the nick list */
			userlist_enum(chatroom_channel_join_userlist_enum_fn, (void*) channel);

			/* add me to the list */
			chatroom_setup_event(&gcd, &gce, channel, GC_EVENT_JOIN);
			gce.pszNick = l_user_nickname;
			gce.pszUID = l_user_nickname;
			gce.pszStatus = CHATROOM_NORMAL_STATUS;
			gce.bIsMe = TRUE;
			CallService(MS_GC_EVENT, 0, (LPARAM)&gce);

			/* set current topic */
			chatroom_setup_event(&gcd, &gce, channel, GC_EVENT_TOPIC);
			gce.pszText = l_current_topic;
			gce.bAddToLog = FALSE;
			CallService(MS_GC_EVENT, 0, (LPARAM)&gce);

			chatroom_setup_event(&gcd, &gce, channel, GC_EVENT_SETSBTEXT);
			/* this is a workaround for a bug in Chat implementation
			 * where it will segfault if strlen(pszText) == "" */
			gce.pszText = strlen(l_current_topic) ? l_current_topic: " ";
			CallService(MS_GC_EVENT, 0, (LPARAM)&gce);

			/* init done */
			chatroom_setup_event(&gcd, &gce, channel, GC_EVENT_CONTROL);
			CallService(MS_GC_EVENT, (WPARAM)WINDOW_INITDONE, (LPARAM)&gce);
			CallService(MS_GC_EVENT, (WPARAM)WINDOW_ONLINE, (LPARAM)&gce);

			free(l_user_nickname);
		}

		/* free channel name '#chan' we've alloced */
		free(l_channel_name);
		free(l_current_topic);
	}
}

void chatroom_channel_part(const char * channel, int disconnected_event)
{
	char * chanlist;
	ASSERT_RETURNIFFAIL(!user_offline());

	chanlist = *user_p_chanlist();
	if(!disconnected_event) {
		/* check if we've parted this channel already */
		if(!chanlist_contains(chanlist, channel))
			return;

		/* remove channel from chanlist and store it in db */
		user_set_chanlist(*user_p_chanlist() = chanlist_remove(chanlist, channel), TRUE);
	}

	/* remove channel entry from hashtable */
	hashtable_remove(s_channelHash, (void*)channel, 1);

	/* send part message to the network, except for the VQCHAT_MAIN_CHANNEL */
	if(strcmp(channel, VQCHAT_MAIN_CHANNEL))
		chatroom_send_net_part(channel);

	/* do stuff with the chat module */
	if(s_fHaveChat) {
		GCDEST gcd;
		GCEVENT gce;
		
		/* close chat window */
		chatroom_setup_event(&gcd, &gce, channel, GC_EVENT_CONTROL);
		CallService(MS_GC_EVENT, WINDOW_TERMINATE, (LPARAM)&gce);
	}
}

void chatroom_channel_topic_change(
	const char * channel, const char * new_topic,
	int notify_network, int add_to_log)
{
	struct chatroom_status_entry * entry;
	int topic_changed;

	ASSERT_RETURNIFFAIL(VALIDPTR(channel) && VALIDPTR(new_topic));

	/* get current channel topic (if there is one)
	 * or alloc new one to store channel (as this might not be open already
	 * and we will need this topic if we join the channel)
	 */
	entry = (struct chatroom_status_entry*) hashtable_search(s_channelHash, (void*)channel);
	if(entry == NULL) {
		topic_changed = 1;
		
		entry = malloc(sizeof(struct chatroom_status_entry));
		entry->topic = strdup(new_topic);
		hashtable_insert(s_channelHash, strdup(channel), entry);
	} else {
		/* we have previous topic here: check if the new one is different */
		topic_changed = strcmp(entry->topic, new_topic);

		/* store the new topic for the channel */
		if(topic_changed) {
			free(entry->topic);
			entry->topic = strdup(new_topic);
		}
	}

	/* the topic has changed and we have this channel's topic
	 * on the list: change the topic 
	 */
	if(s_fHaveChat && topic_changed
			&& chanlist_contains(*user_p_chanlist(), channel)) {
		GCDEST gcd;
		GCEVENT gce;
		char * l_new_topic = util_utf2loc(new_topic);

		/* update chatroom topic */
		chatroom_setup_event(&gcd, &gce, channel, GC_EVENT_TOPIC);
		gce.pszText = l_new_topic;
		gce.bAddToLog = add_to_log;
		CallService(MS_GC_EVENT, 0, (LPARAM)&gce);

		/* set status bar to topic text */
		chatroom_setup_event(&gcd, &gce, channel, GC_EVENT_SETSBTEXT);
		/* this is a workaround for a bug in Chat implementation
		 * where it will segfault if strlen(pszText) == "" */
		gce.pszText = strlen(l_new_topic) ? l_new_topic: " ";
		CallService(MS_GC_EVENT, 0, (LPARAM)&gce);

		/* emit topic change message to the network */
		if(notify_network) {
			char * r_channel = util_utf2vqp(user_codepage(), channel),
				* r_new_topic = util_utf2vqp(user_codepage(), new_topic);

			msgloop_send(
				vqp_msg_channel_topic_change(s_vqpLink, r_channel, r_new_topic),
				0);
			free(r_channel);
			free(r_new_topic);
		}

		free(l_new_topic);
	}
}

/* chatroom_channel_show_settings_dlg:
 *	Shows channel settings dialog for specified channel.
 *	Focuses on current channel settings dialog, if one already active.
 */
void chatroom_channel_show_settings_dlg(const char * channel)
{
	ASSERT_RETURNIFFAIL(VALIDPTR(channel)
			&& chanlist_contains(*user_p_chanlist(), channel));

	/* show channel settings dialog
	 */
	if(!s_hwndChannelSettingsDlg) {
		/* create and init the dialog */
		s_hwndChannelSettingsDlg = CreateDialog(
			g_hDllInstance, MAKEINTRESOURCE(IDD_CS),
			NULL, chatroom_channel_settings_wndproc);

		SendMessage(s_hwndChannelSettingsDlg, VQCHAT_WM_INITCS, 0, (LPARAM)channel);
	}

	/* activate window */
	SetActiveWindow(s_hwndChannelSettingsDlg);
}

const char * chatroom_channel_get_topic(const char * channel)
{
	struct chatroom_status_entry * entry;

	ASSERT_RETURNVALIFFAIL(VALIDPTR(channel), "");

	entry = (struct chatroom_status_entry*)hashtable_search(s_channelHash, (void*)channel);
	return (entry && entry->topic) ? entry->topic: "";
}

static int chatroom_user_name_change_enum_fn(
	const char * channel, void * enum_fn_data)
{
	GCDEST gcd;
	GCEVENT gce;
	const char * user_name = ((const char **)enum_fn_data)[0],
	      * new_user_name = ((const char **)enum_fn_data)[1];
	
	char	* l_user_name = util_utf2loc(user_name),
		* l_new_user_name = util_utf2loc(new_user_name);
	
	ASSERT_RETURNVALIFFAIL(VALIDPTR(channel) && VALIDPTR(enum_fn_data), 1);
	ASSERT_RETURNVALIFFAIL(VALIDPTR(user_name) && VALIDPTR(new_user_name), 1);

	/* update contact's nickname */
	chatroom_setup_event(&gcd, &gce, channel, GC_EVENT_NICK);
	gce.pszUID = l_user_name;
	gce.pszNick = l_user_name;
	gce.pszText = l_new_user_name;
	gce.bAddToLog = TRUE;
	gce.bIsMe = TRUE;
	CallService(MS_GC_EVENT, 0, (LPARAM)&gce);

	/* update contact's UID */
	chatroom_setup_event(&gcd, &gce, channel, GC_EVENT_CHID);
	gce.pszUID = l_user_name;
	gce.pszText = l_new_user_name;
	gce.pszStatus = CHATROOM_NORMAL_STATUS;
	gce.bIsMe = TRUE;
	CallService(MS_GC_EVENT, 0, (LPARAM)&gce);

	free(l_user_name);
	free(l_new_user_name);

	return 1;
}

void chatroom_user_name_change(const char * user_name, const char * new_user_name)
{
	const char * enum_fn_data[2];
	ASSERT_RETURNIFFAIL(VALIDPTR(user_name) && VALIDPTR(new_user_name));

	enum_fn_data[0] = user_name;
	enum_fn_data[1] = new_user_name;
	
	chanlist_enum(
		*user_p_chanlist(),
		chatroom_user_name_change_enum_fn,
		enum_fn_data);
}

static int chatroom_user_status_change_enum_fn(
	const char * channel, void * notice_text)
{
	GCDEST gcd;
	GCEVENT gce;
	char * l_notice_text = util_utf2loc(notice_text);
	
	chatroom_setup_event(&gcd, &gce, channel, GC_EVENT_INFORMATION);
	gce.pszText = (const char *)l_notice_text;
	gce.bAddToLog = TRUE;
	CallService(MS_GC_EVENT, 0, (LPARAM)&gce);

	free(l_notice_text);

	return 1;
}

void chatroom_user_status_change(int new_status)
{
	char * notice_text;

	notice_text = malloc(sizeof(user_nickname()) + 64);
	ASSERT_RETURNIFFAIL(VALIDPTR(notice_text));

	sprintf(notice_text, "%s changed status to \"%s\"",
		user_nickname(), user_status_name(new_status));
	
	chanlist_enum(
		*user_p_chanlist(),
		chatroom_user_status_change_enum_fn,
		(void*)notice_text);

	free(notice_text);
}

void chatroom_channel_user_join(
	const char * channel, const char * user_name,
	int explicit_join)
{
	ASSERT_RETURNIFFAIL(VALIDPTR(channel) && VALIDPTR(user_name));

	if(s_fHaveChat) {
		GCDEST gcd;
		GCEVENT gce;
		char * l_user_name = util_utf2loc(user_name);
		
		chatroom_setup_event(&gcd, &gce, channel, GC_EVENT_JOIN);
		gce.pszNick = l_user_name;
		gce.pszUID = l_user_name;
		gce.pszStatus = CHATROOM_NORMAL_STATUS;
		gce.bIsMe = FALSE;
		gce.bAddToLog = explicit_join;

		CallService(MS_GC_EVENT, 0, (LPARAM)&gce);

		free(l_user_name);
	}
}

void chatroom_channel_user_part(
	const char * channel, const char * user_name,
	int explicit_part)
{
	ASSERT_RETURNIFFAIL(VALIDPTR(channel) && VALIDPTR(user_name));

	if(s_fHaveChat) {
		GCDEST gcd;
		GCEVENT gce;
		char * l_nickname = util_utf2loc(user_name);
		
		chatroom_setup_event(&gcd, &gce, channel, GC_EVENT_PART);
		gce.pszUID = l_nickname;
		gce.pszNick = l_nickname;
		gce.pszText = explicit_part ? NULL: Translate("ping timeout");
		gce.bAddToLog = TRUE;
		CallService(MS_GC_EVENT, 0, (LPARAM)&gce);

		free(l_nickname);
	}
}

static int chatroom_channel_user_name_change_enum_event_fn(
	const char * channel, void * fn_data)
{
	const char * user_name = ((const char **)fn_data)[0],
	      * new_user_name = ((const char **)fn_data)[1];
	int add_to_log = (int)((const char **)fn_data)[2];

	/* send notice text to this channel, only
	 * if the user is there too
	 */
	if(!chanlist_contains(userlist_user_chanlist(user_name), channel)) {
		GCDEST gcd;
		GCEVENT gce;
		char * l_user_name = util_utf2loc(user_name),
			* l_new_user_name = util_utf2loc(new_user_name);

		/* change nickname */
		chatroom_setup_event(&gcd, &gce, channel, GC_EVENT_NICK);
		gce.pszUID = l_user_name;
		gce.pszText = l_new_user_name;
		gce.pszNick = l_new_user_name;
		gce.bAddToLog = add_to_log;
		CallService(MS_GC_EVENT, 0, (LPARAM)&gce);

		/* change user UID */
		chatroom_setup_event(&gcd, &gce, channel, GC_EVENT_CHID);
		gce.pszUID = l_user_name;
		gce.pszText = l_new_user_name;
		gce.pszStatus = CHATROOM_NORMAL_STATUS;
		CallService(MS_GC_EVENT, 0, (LPARAM)&gce);

		free(l_user_name);
		free(l_new_user_name);
	}

	return 1; /* keep enumerating */
}

void chatroom_channel_user_name_change(
	const char * user_name, const char * new_user_name,
	int add_to_log)
{
	ASSERT_RETURNIFFAIL(VALIDPTR(user_name) && VALIDPTR(new_user_name));

	if(s_fHaveChat) {
		const char * enum_fn_data[3];

		enum_fn_data[0] = user_name;
		enum_fn_data[1] = new_user_name;
		enum_fn_data[2] = (const char *) add_to_log;
		chanlist_enum(
			*user_p_chanlist(),
			chatroom_channel_user_name_change_enum_event_fn,
			(void*)enum_fn_data);
	}
}

static int chatroom_channel_user_status_change_enum_event_fn(
	const char * channel, void * enum_fn_data)
{
	const char * user_name = ((const char **)enum_fn_data)[0],
	      * notice_text = ((const char **)enum_fn_data)[1];
	int add_to_log = (int)((const char **)enum_fn_data)[2];

	ASSERT_RETURNVALIFFAIL(VALIDPTR(notice_text) && VALIDPTR(user_name), 1);
	ASSERT_RETURNVALIFFAIL(VALIDPTR(channel) && VALIDPTR(notice_text), 1);

	/* send notice text to this channel, only
	 * if the user is there too
	 */
	if(chanlist_contains(userlist_user_chanlist(user_name), channel)) {
		GCDEST gcd;
		GCEVENT gce;
		char * l_notice_text = util_utf2loc(notice_text);

		chatroom_setup_event(&gcd, &gce, channel, GC_EVENT_INFORMATION);
		gce.pszText = (const char *)l_notice_text;
		gce.bAddToLog = add_to_log;
		CallService(MS_GC_EVENT, 0, (LPARAM)&gce);

		free(l_notice_text);
	}

	return 1; /* keep enumerating */
}

/* chatroom_channel_user_status_change:
 *	notifies user status change to all of registered channel
 */
void chatroom_channel_user_status_change(
	const char * user_name, int new_status, const char * new_status_text,
	int add_to_log)
{
	ASSERT_RETURNIFFAIL(VALIDPTR(user_name));

	if(s_fHaveChat) {
		const char * enum_fn_data[3];
		char * notice_text;
		int status_len = strlen(new_status_text);

		/* make notice text */
		notice_text = malloc(64 + strlen(user_name) + status_len);
		sprintf(notice_text,
			status_len
				? "%s changed status to \"%s\": %s"
				: "%s changed status to \"%s\"",
			user_name,
			user_status_name(new_status),
			new_status_text
		);

		/* send notice text through all the channels */
		enum_fn_data[0] = user_name;
		enum_fn_data[1] = notice_text;
		enum_fn_data[2] = (const char *)add_to_log;
		chanlist_enum(
			*user_p_chanlist(),
			chatroom_channel_user_status_change_enum_event_fn,
			(void*)enum_fn_data);

		/* free notice text */
		free(notice_text);
	}
}

void chatroom_channel_user_text(
	const char * channel, const char * user_name,
	const char * text, int action_text)
{
	ASSERT_RETURNIFFAIL(VALIDPTR(channel) && VALIDPTR(user_name));
	ASSERT_RETURNIFFAIL(VALIDPTR(text));
	
	if(s_fHaveChat && chanlist_contains(*user_p_chanlist(), channel)) {
		GCDEST gcd;
		GCEVENT gce;
		char * escaped_text = chatroom_escape_message(text),
			* l_user_name = util_utf2loc(user_name),
			* l_escaped_text = util_utf2loc(escaped_text);

		chatroom_setup_event(
			&gcd, &gce, channel,
			action_text ? GC_EVENT_ACTION: GC_EVENT_MESSAGE);
		gce.pszUID = l_user_name;
		gce.pszNick = l_user_name;
		gce.pszText = l_escaped_text;
		gce.bAddToLog = TRUE;
		CallService(MS_GC_EVENT, 0, (LPARAM)&gce);

		free(l_user_name);
		free(l_escaped_text);
		free(escaped_text);
	}
}

