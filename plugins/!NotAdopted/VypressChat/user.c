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
 * $Id: user.c,v 1.24 2005/04/11 18:49:44 bobas Exp $
 */

#include "miranda.h"

#include "main.h"
#include "msgloop.h"
#include "msghandler.h"
#include "user.h"
#include "userlist.h"
#include "contacts.h"
#include "chatroom.h"
#include "options.h"
#include "chanlist.h"
#include "service.h"
#include "util.h"

/* static data
 */
static char *	s_userNickname;
static char *	s_awayMsg;
static int	s_userStatus;
static char *	s_userChanlist;
static enum vqp_gender s_userGender;
static vqp_uuid_t s_userUuid;
static enum vqp_codepage s_userCodepage;

static vqp_link_t s_vqpLink;

/* static routines
 */

/* user_closest_supported_status:
 *	we do not support every possible user status mode
 *	in this plugin, thus must select the closest status
 *
 * XXX: do the actual approximation, instead of defaulting to ID_STATUS_ONLINE
 */
static int
user_closest_supported_status(int status)
{
	switch(status) {
	case ID_STATUS_OFFLINE:
	case ID_STATUS_AWAY:
	case ID_STATUS_DND:
	case ID_STATUS_NA:
	case ID_STATUS_ONLINE:
		break;

	default:
		status = ID_STATUS_ONLINE;
		break;
	}
	return status;
}

/* user_validate_settings:
 *	checks if we have valid settings to connect to network,
 *	and shows configuration dialog, if wanted
 */
static int
user_validate_settings(int show_config_dlg)
{
	ASSERT_RETURNVALIFFAIL(VALIDPTR(s_userNickname), FALSE);
	if(!strlen(s_userNickname)) {
		if(show_config_dlg)
			options_show_user_options();

		return FALSE;
	}

#ifdef VYPRESSCHAT
	/* we must ensure to have a non-null UUID in order to connect */
	if(vqp_uuid_is_nil(&s_userUuid)) {
		if(show_config_dlg)
			options_show_user_options();

		return FALSE;
	}
#endif

	return TRUE;
}

static vqp_link_t user_open_link()
{
	DWORD * list;
	unsigned long * broadcast_masks;
	size_t list_sz, i;
	vqp_link_t link;

	/* setup broadcast mask list
	 */
	list = db_dword_list(NULL, VQCHAT_PROTO, "BroadcastMasks", &list_sz);
	broadcast_masks = malloc(sizeof(unsigned long) * (list_sz + 1));
	for(i = 0; i < list_sz; i++)
		broadcast_masks[i] = list[i];
	broadcast_masks[i] = 0UL;

	free(list);
			
	/* connect to the network
	 */
	link = vqp_link_open(
		VQCHAT_VQP_PROTO,
		db_dword_get(NULL, VQCHAT_PROTO, "ProtoOpt", VQCHAT_VQP_DEF_PROTO_OPT),
		(db_byte_get(NULL, VQCHAT_PROTO, "ProtoConn", 0) == 0)
				? VQP_PROTOCOL_CONN_UDP
				: VQP_PROTOCOL_CONN_IPX,
		0, broadcast_masks,
		db_dword_get(NULL, VQCHAT_PROTO, "Multicast", VQCHAT_VQP_DEF_MULTICAST),
		db_word_get(NULL, VQCHAT_PROTO, "Port", VQCHAT_VQP_DEF_PORT),
		NULL
	);
	
	free(broadcast_masks);

	return link;
}

/* exported routines
 */
void user_init()
{
	s_userNickname = strdup("");
	s_userStatus = ID_STATUS_OFFLINE;
	s_awayMsg = strdup("");
	s_userChanlist = NULL;
	vqp_uuid_create_nil(&s_userUuid);
}

void user_uninit()
{
	free(s_userNickname);
	s_userNickname = NULL;
	
	free(s_awayMsg);
	s_awayMsg = NULL;

	chanlist_free(s_userChanlist);
	s_userChanlist = NULL;
	
	s_userStatus = ID_STATUS_OFFLINE;
}

enum vqp_status
user_vqp_status_by_status(int status)
{
	switch(status) {
	case ID_STATUS_AWAY:	return VQP_STATUS_AWAY;
	case ID_STATUS_DND:	return VQP_STATUS_DND;
	case ID_STATUS_NA:	return VQP_STATUS_NOTAVAILABLE;
	case ID_STATUS_ONLINE:	return VQP_STATUS_AVAILABLE;
	}
	return VQP_STATUS_AVAILABLE;
}

int user_status_by_vqp_status(enum vqp_status user_status)
{
	switch(user_status) {
	case VQP_STATUS_AVAILABLE:	return ID_STATUS_ONLINE;
	case VQP_STATUS_AWAY:		return ID_STATUS_AWAY;
	case VQP_STATUS_DND:		return ID_STATUS_DND;
	case VQP_STATUS_NOTAVAILABLE:	return ID_STATUS_NA;
	default: break;
	}
	return ID_STATUS_ONLINE;
}

const char *
user_status_name(int status)
{
	switch(status) {
	case ID_STATUS_AWAY:	return "Away";
	case ID_STATUS_DND:	return "Do Not Disturb";
	case ID_STATUS_NA:	return "Not Available";
	case ID_STATUS_ONLINE:	return "Online";
	}
	return "(unknown)";
}

/* user_hook_modules_loaded:
 *	invoked from ME_SYSTEM_MODULESLOADED hook after all of miranda's modules
 *	have loaded
 */
void user_hook_modules_loaded()
{
	user_validate_settings(TRUE);
}

/* user_set_nickname:
 *	sets user nickname
 *
 * returns:
 *	non-0 on failure (e.g. !strlen(new_nickname))
 *	0 on success
 */
int user_set_nickname(const char * new_nickname, int store_in_db)
{
	ASSERT_RETURNVALIFFAIL(VALIDPTR(new_nickname), 1);
	ASSERT_RETURNVALIFFAIL(VALIDPTR(s_userNickname), 1);

	/* check if we have a valid nickname */
	if(strlen(new_nickname) == 0)
		return 1;
	
	/* update my settings in DB */
	if(store_in_db)
		db_string_set(NULL, VQCHAT_PROTO, "Nick", new_nickname);

	/* check that the new nickname is not the same */
	if(!strcmp(new_nickname, s_userNickname))
		return 0;

	/* send message to network */
	if(!user_offline()) {
		/* send nickname change message */
		char * r_nickname = util_utf2vqp(user_codepage(), user_nickname()),
			* r_new_nickname = util_utf2vqp(user_codepage(), new_nickname);

		msgloop_send(
			vqp_msg_nick_change(s_vqpLink, r_nickname, r_new_nickname, user_gender()),
			0);
		free(r_nickname);
		free(r_new_nickname);

		/* update chatrooms */
		chatroom_user_name_change(s_userNickname, new_nickname);
	}

	/* change nickname */
	free(s_userNickname);
	s_userNickname = strdup(new_nickname);

	return 0;	/* success */
}

const char * user_nickname()
{
	ASSERT_RETURNVALIFFAIL(VALIDPTR(s_userNickname), "no-nickname");
	return s_userNickname;
}

int user_is_my_nickname(const char * nickname)
{
	ASSERT_RETURNVALIFFAIL(VALIDPTR(nickname), FALSE);
	ASSERT_RETURNVALIFFAIL(VALIDPTR(s_userNickname), FALSE);

	return !strcmp(s_userNickname, nickname);
}

/* user_set_status:
 *	sets specified user status
 *	(if previous status was ID_STATUS_OFFLINE, then it tries to connect,
 *	et vice-versa)
 *
 * returns:
 *	zero,	: status change completed (thus caller needs to send ProtoAck)
 *	non-zero: no status change took place
 */
int user_set_status(int new_status)
{
	new_status = user_closest_supported_status(new_status);

	if(new_status==s_userStatus)
		return 1;	/* no need to change status */

	/* check if we need to disconnect */
	if(new_status==ID_STATUS_OFFLINE) {
		/* stop message loop */
		msgloop_stop();

		/* notify chatroom module that we've diconnected
		 * (this will make the user leave all the channels)
		 */
		chatroom_disconnected();

		/* cleanup user list */
		userlist_disconnected();

		/* close link */
		vqp_link_close(s_vqpLink);

		/* set all contacts offline */
		contacts_set_all_offline();

		/* notify modules */
		contacts_disconnected();
		service_disconnected();

		/* set status */
		s_userStatus = ID_STATUS_OFFLINE;
		return 0;	/* status change succeeded */
	}

	/* check if we need to connect */
	if(s_userStatus==ID_STATUS_OFFLINE) {
		/* check if we have valid settings to connect to network */
		if(!user_validate_settings(FALSE)) {
			/* stay in offline mode */
			ProtoBroadcastAck(
				VQCHAT_PROTO, NULL, ACKTYPE_LOGIN,
				ACKRESULT_SUCCESS, (HANDLE)s_userStatus, s_userStatus);
	
			/* deny status change */
			ProtoBroadcastAck(
				VQCHAT_PROTO, NULL, ACKTYPE_LOGIN,
				ACKRESULT_FAILED, NULL, (LPARAM)LOGINERR_BADUSERID);
	
			return 1;	/* mode change failure */
		}

		s_vqpLink = user_open_link();
		if(!s_vqpLink) {
			/* stay in offline mode */
			ProtoBroadcastAck(
				VQCHAT_PROTO, NULL, ACKTYPE_LOGIN,
				ACKRESULT_SUCCESS, (HANDLE)s_userStatus, s_userStatus);

			/* send an ack to notify that there was a network failure */
			ProtoBroadcastAck(
				VQCHAT_PROTO, NULL, ACKTYPE_LOGIN,
				ACKRESULT_FAILED, NULL, (LPARAM)LOGINERR_NONETWORK);
					
			return 1;	/* mode change failure */
		}

		/* set new status */
		s_userStatus = new_status;

		/* start message loop so we can send and receive
		 * messages asynchronuously, in a separate thread
		 */
		msgloop_start(s_vqpLink, msghandler_apc);


		/* notify chatroom module, that we've connected
		 * (this will init user channel list and join the #Main) */
		chatroom_connected(s_vqpLink);

		/* init user list
		 * (this should go after chatroom_connected(), as userlist_connected()
		 * will send a refresh req)
		 */
		userlist_connected(s_vqpLink);
	
		/* notify modules */
		service_connected(s_vqpLink);
		contacts_connected(s_vqpLink);
	} else {
		char * r_nickname, * r_awaymsg;
		/* simply change our status
		 */

		/* send status change msg */
		r_nickname = util_utf2vqp(user_codepage(), user_nickname());
		r_awaymsg = util_utf2vqp(user_codepage(), user_awaymsg());

		msgloop_send(
			vqp_msg_status_change(
				s_vqpLink, r_nickname, user_vqp_status_by_status(new_status),
				user_gender(), r_awaymsg),
			0
		);
		free(r_nickname);
		free(r_awaymsg);

		/* set status */
		s_userStatus = new_status;

		/* notify chatrooms */
		chatroom_user_status_change(new_status);
	}

	return 0;	/* status change succeeded */
}

/* user_status:
 *	returns user's status in miranda's ID_STATUS_* ints
 */
int user_status()
{
	return s_userStatus;
}

enum vqp_gender user_gender()
{
	return s_userGender;
}

void user_set_gender(enum vqp_gender new_gender, int store_in_db)
{
	if(s_userGender != new_gender) {
		s_userGender = new_gender;
		
		if(store_in_db) {
			db_byte_set(
				NULL, VQCHAT_PROTO, "Gender",
				s_userGender == VQP_GENDER_MALE ? 'M': 'F');
		}
	}
}

/* user_p_uuid:
 *	returns user's active uuid (universally unique identifier)
 */
const vqp_uuid_t * user_p_uuid()
{
	return &s_userUuid;
}

/* user_set_uuid:
 *	updates user's active uuid, store_in_db should be set to TRUE,
 *	if it should be saved to dababase settings
 */
void user_set_uuid(const vqp_uuid_t * p_new_uuid, int store_in_db)
{
	ASSERT_RETURNIFFAIL(VALIDPTR(p_new_uuid));

	if(!vqp_uuid_is_equal(p_new_uuid, &s_userUuid)) {
		s_userUuid = *p_new_uuid;
		
		if(store_in_db) {
			char * uuid_str = vqp_uuid_to_string(&s_userUuid);
			db_string_set(NULL, VQCHAT_PROTO, "Uuid", uuid_str);

			free(uuid_str);
		}
	}
}

enum vqp_codepage user_codepage()
{
	return s_userCodepage;
}

void user_set_codepage(enum vqp_codepage codepage, int store_in_db)
{
	if(codepage != s_userCodepage) {
		s_userCodepage = codepage;

		if(store_in_db) {
			db_byte_set(
				NULL, VQCHAT_PROTO, "Codepage",
				codepage==VQP_CODEPAGE_LOCALE ? 0: 1);
		}
	}
}

void user_set_awaymsg(const char * msg_text)
{
	ASSERT_RETURNIFFAIL(VALIDPTR(msg_text) && VALIDPTR(s_awayMsg));

	free(s_awayMsg);
	s_awayMsg = strdup(msg_text);
}

const char * user_awaymsg()
{
	return s_awayMsg;	
}

/* user_set_chanlist:
 *	stores specified chanlist as my chanlist
 *
 *	(chanlist can be == *user_p_chanlist())
 */
void user_set_chanlist(const char * chanlist, int store_in_db)
{
	if(chanlist != s_userChanlist) {
		/* copy the specified chanlist */
		chanlist_free(s_userChanlist);
		s_userChanlist = chanlist_copy(chanlist);
		
	}

	if(store_in_db) {
		/* in case of empty chanlist it will store "" string,
		 * otherwise -- the s_userChanlist string
		 */
		db_string_set(NULL, VQCHAT_PROTO, "Chanlist",
			s_userChanlist ? s_userChanlist: "");
	}
}

/* user_p_chanlist:
 *	returns pointer to user's chanlist (which can be modified)
 */
char ** user_p_chanlist()
{
	return &s_userChanlist;
}

