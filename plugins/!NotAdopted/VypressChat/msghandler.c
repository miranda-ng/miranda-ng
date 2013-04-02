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
 * $Id: msghandler.c,v 1.41 2005/03/22 16:12:29 bobas Exp $
 */

#include <time.h>

#include "miranda.h"
#include <lm.h>

#include "libvqproto/vqproto.h"

#include "main.h"
#include "msghandler.h"
#include "user.h"
#include "userlist.h"
#include "contacts.h"
#include "chatroom.h"
#include "chanlist.h"
#include "util.h"
#include "service.h"
#include "skin.h"

/* static data
 */
static struct vqp_parse_func_struct s_parseFuncs;

/* static routines
 */

static void msghandler_refresh_req(
	const vqp_msg_t msg, void * user_data,
	const char * r_src, enum vqp_codepage src_codepage)
{
	char * src = util_vqp2utf(src_codepage, r_src);

	if(!user_is_my_nickname(src)) {
		char * r_nickname = util_utf2vqp(src_codepage, user_nickname());

		/* send refresh ack, without delaying */
		msgloop_send_to(
			vqp_msg_refresh_ack(
				vqp_msg_link(msg),
				r_nickname, r_src, user_vqp_status(), VQP_ACTIVE_ACTIVE,
				user_gender(), VQCHAT_VQP_SWVERSION, user_p_uuid(),
				user_codepage(), 0),
			1, vqp_msg_src_addr(msg)
		);
		free(r_nickname);
	}
	free(src);
}

static void msghandler_refresh_ack(
	const vqp_msg_t msg, void * user_data,
	const char * r_src, const char * r_dst,
	enum vqp_status src_status, enum vqp_active src_active, enum vqp_gender src_gender,
	unsigned int src_swversion, const vqp_uuid_t * src_uuid,
	enum vqp_codepage src_codepage, unsigned int src_pref_color)
{
	char * src = util_vqp2utf(src_codepage, r_src),
		* dst = util_vqp2utf(src_codepage, r_dst);

	if(!user_is_my_nickname(src) && user_is_my_nickname(dst)) {
		userlist_add(
			src, src_status, src_gender, src_uuid, src_codepage, src_swversion,
			vqp_msg_src_addr(msg), 0);
	}
	free(dst);
	free(src);
}

#ifdef VYPRESSCHAT
static void msghandler_ping(
	const vqp_msg_t msg, void * user_data,
	const char * r_src, const char * r_dst, const char * r_timestamp)
{
	enum vqp_codepage cp = userlist_user_codepage_vqp(r_src);
	char * dst = util_vqp2utf(cp, r_dst),
		* src = util_vqp2utf(cp, r_src);

	if(!user_is_my_nickname(src) && user_is_my_nickname(dst)) {
		/* send pong, without waiting */
		msgloop_send_to(
			vqp_msg_pong(vqp_msg_link(msg), r_dst, r_src, r_timestamp),
			1, vqp_msg_src_addr(msg));
	}

	free(src);
	free(dst);
}

static void msghandler_pong(
	const vqp_msg_t msg, void * user_data,
	const char * r_src, const char * r_dst, const char * r_timestamp)
{
	/* we don't use or need ping/pong mechanism,
	 * at least for now
	 */
}

static void msghandler_flood_notification(
	const vqp_msg_t msg, void * user_data,
	const char * r_src, const char * r_dst, const char * r_secs_blocked)
{
	enum vqp_codepage cp = userlist_user_codepage_vqp(r_src);
	char * src = util_vqp2utf(cp, r_src),
		* dst = util_vqp2utf(cp, r_dst);

	if(!user_is_my_nickname(src) && user_is_my_nickname(dst)) {
		char * secs_blocked = util_vqp2utf(cp, r_secs_blocked);
		
		HANDLE hContact = contacts_find_contact(src);
		if(hContact) {
			int len = strlen(secs_blocked);
			char * fmt = malloc(len + 64);
			sprintf(fmt, "*** YOU'RE BLOCKED FOR %s SECONDS FOR FLOOD ***",
				secs_blocked);
			
			contacts_input_contact_message(hContact, fmt);
			free(fmt);
		}
		free(secs_blocked);
	}
	free(src);
	free(dst);
}
#endif

static void msghandler_nick_change(
	vqp_msg_t msg, void * user_data,
	const char * r_src, const char * r_src_new_nick, enum vqp_gender src_gender)
{
	enum vqp_codepage cp = userlist_user_codepage_vqp(r_src);
	char * src = util_vqp2utf(cp, r_src),
		* src_new_nick = util_vqp2utf(cp, r_src_new_nick);

	if(!user_is_my_nickname(src)) {
		userlist_name_change(src, src_new_nick);
	}

	free(src);
	free(src_new_nick);
}

static void msghandler_status_change(
	vqp_msg_t msg, void * user_data,
	const char * r_src, enum vqp_status src_status,
	enum vqp_gender src_gender, const char * r_src_autoanswer)
{
	enum vqp_codepage cp = userlist_user_codepage_vqp(r_src);
	char * src = util_vqp2utf(cp, r_src),
		* src_autoanswer = util_vqp2utf(cp, r_src_autoanswer);


	if(!user_is_my_nickname(src))
		userlist_status_change(src, src_status, src_gender, src_autoanswer, 1);

	free(src);
	free(src_autoanswer);
}

static void msghandler_active_change(
	vqp_msg_t msg, void * user_data,
	const char * r_src, enum vqp_active src_is_active)
{
	/* XXX: we'll ignore 'active change' for now */
}

static void msghandler_channel_join(
	vqp_msg_t msg, void * user_data,
	const char * r_channel, const char * r_src,
	enum vqp_status src_status, enum vqp_gender src_gender,
	unsigned int src_swversion, const vqp_uuid_t * src_uuid,
	enum vqp_codepage src_codepage, unsigned int src_pref_color)
{
	char * channel = util_vqp2utf(src_codepage, r_channel),
		* src = util_vqp2utf(src_codepage, r_src);

	if(!user_is_my_nickname(src)) {
		const char * topic;
		char * my_chanlist;
		
		/* add user to userlist and update it's channel list */
		userlist_add(
			src, src_status, src_gender, src_uuid,
			src_codepage, src_swversion, vqp_msg_src_addr(msg),
			1);
		userlist_user_channel_join(src, channel, 1);

		/* reply with refresh ack (if the channel is #Main), who-here ack,
		 *	and topic ack if we're in this channel 
		 */
		my_chanlist = chanlist_copy(*user_p_chanlist());
		my_chanlist = chanlist_add(my_chanlist, VQP_MAIN_CHANNEL);
			
		if(chanlist_contains(my_chanlist, channel)) {
			char * r_nickname = util_utf2vqp(src_codepage, user_nickname());
			
			if(!strcmp(channel, VQP_MAIN_CHANNEL)) {
				/* reply with refresh ack (without waiting) */
				msgloop_send_to(
					vqp_msg_refresh_ack(
						vqp_msg_link(msg),
						r_nickname, r_src, user_vqp_status(),
						VQP_ACTIVE_ACTIVE, user_gender(),
						VQCHAT_VQP_SWVERSION, user_p_uuid(),
						user_codepage(), 0),
					1, vqp_msg_src_addr(msg)
				);
			} else {
				/* on channels not #Main, reply with who-here ack */
				msgloop_send_to(
					vqp_msg_channel_whohere_ack(
						vqp_msg_link(msg),
						r_channel, r_nickname, r_src, VQCHAT_UNDEF_ACTIVE),
					0, vqp_msg_src_addr(msg)
				);
			}
			
			/* reply the user with channel's topic */
			topic = chatroom_channel_get_topic(channel);
			if(topic) {
				char * r_topic_text = util_utf2vqp(src_codepage, topic);

				msgloop_send_to(
					vqp_msg_channel_current_topic(
						vqp_msg_link(msg), r_src, r_channel, r_topic_text),
					0, vqp_msg_src_addr(msg)
				);
				free(r_topic_text);
			}

			/* free strings */
			free(r_nickname);
		}

		chanlist_free(my_chanlist);
	}

	free(channel);
	free(src);
}
static void msghandler_channel_leave(
	const vqp_msg_t msg, void * user_data,
	const char * r_channel, const char * r_src, enum vqp_gender src_gender)
{
	enum vqp_codepage cp = userlist_user_codepage_vqp(r_src);
	char * channel = util_vqp2utf(cp, r_channel),
		* src = util_vqp2utf(cp, r_src);

	if(!user_is_my_nickname(src)) {
		if(!strcmp(channel, VQCHAT_MAIN_CHANNEL)) {
			/* user has left the network */
			userlist_remove(src, 1);
		} else {
			/* user has left the specified channel */
			userlist_user_channel_part(src, channel, 1);
		}
	}

	free(channel);
	free(src);
}

static void msghandler_channel_text(
	vqp_msg_t msg, void * user_data,
	const char * r_channel, const char * r_src,
	const char * r_text, int is_action_text)
{
	enum vqp_codepage cp = userlist_user_codepage_vqp(r_src);
	char * src = util_vqp2utf(cp, r_src);

	if(!user_is_my_nickname(src)) {
		char * channel = util_vqp2utf(cp, r_channel),
			* text = util_vqp2utf(cp, r_text);

		userlist_user_channel_text(
			src, channel, text, vqp_msg_src_addr(msg), is_action_text);
		free(channel);
		free(text);
	}
	free(src);
}

#ifdef QUICKCHAT
static int msghandler_channel_topic_change_chan_enum_fn(
	const char * channel, void * fn_data)
{
	const char * topic_text = ((const char **)fn_data)[0],
		* notify_user = ((const char **)fn_data)[1];
	
	ASSERT_RETURNVALIFFAIL(VALIDPTR(channel) && VALIDPTR(topic_text), 0);

	chatroom_channel_topic_change(channel, (const char *)topic_text, 0, (int)notify_user);
	return 1;
}
#endif

static void msghandler_channel_topic_change(
	vqp_msg_t msg, void * user_data,
	const char * r_channel, const char * r_topic_text)
{
	char * channel = util_vqp2utf(user_codepage(), r_channel),
		* topic_text = util_vqp2utf(user_codepage(), r_topic_text);

#ifdef VYPRESSCHAT
	chatroom_channel_topic_change(channel, topic_text, 0, 1);
#endif
	
#ifdef QUICKCHAT
	/* there's a single topic for all the channels in quickChat:
	 * enumerate all of them and set their topics
	 */
	const char * fn_data[2];
	fn_data[0] = topic_text;
	fn_data[1] = (const char *)1;
	chanlist_enum(
		*user_p_chanlist(),
		msghandler_channel_topic_change_chan_enum_fn,
		(void *) fn_data);
#endif

	free(channel);
	free(topic_text);
}

static void msghandler_channel_current_topic(
	vqp_msg_t msg, void * user_data,
	const char * r_dst, const char * r_channel, const char * r_topic_text)
{
	enum vqp_codepage cp = user_codepage();
	char * dst = util_vqp2utf(cp, r_dst);

	if(user_is_my_nickname(dst)) {
		char * channel = util_vqp2utf(cp, r_channel),
			* topic_text = util_vqp2utf(cp, r_topic_text);

#ifdef VYPRESSCHAT
		chatroom_channel_topic_change(channel, topic_text, 0, 0);
#endif

#ifdef QUICKCHAT
		/* there's a single topic for all the channels in quickChat:
		 * enumerate all of them and set their topics
		 */
		const char * fn_data[2];
		fn_data[0] = topic_text;
		fn_data[1] = (const char *)0;
		chanlist_enum(
			*user_p_chanlist(),
			msghandler_channel_topic_change_chan_enum_fn,
			(void *) fn_data);
#endif
		free(channel);
		free(topic_text);
	}
	free(dst);
}
static void msghandler_channel_whohere_req(
	vqp_msg_t msg, void * user_data,
	const char * r_channel, const char * r_src)
{
	enum vqp_codepage cp = userlist_user_codepage_vqp(r_src);
	char * src = util_vqp2utf(cp, r_src);

	if(!user_is_my_nickname(src)) {
		char * channel = util_vqp2utf(cp, r_channel);

		if(chanlist_contains(*user_p_chanlist(), channel)) {
			char * r_user_nickname = util_utf2vqp(cp, user_nickname());
			
			/* reply, that we're on this channel */
			msgloop_send_to(
				vqp_msg_channel_whohere_ack(
					vqp_msg_link(msg), r_channel, r_user_nickname, r_src,
					VQCHAT_UNDEF_ACTIVE),
				0, vqp_msg_src_addr(msg)
			);
			free(r_user_nickname);
		}
		free(channel);
	}
	free(src);
}

static void msghandler_channel_whohere_ack(
	vqp_msg_t msg, void * user_data,
	const char * r_channel, const char * r_src, const char * r_dst,
	enum vqp_active src_is_active)
{
	enum vqp_codepage cp = userlist_user_codepage_vqp(r_src);
	char * src = util_vqp2utf(cp, r_src),
		* dst = util_vqp2utf(cp, r_dst);

	if(!user_is_my_nickname(src) && user_is_my_nickname(dst)) {
		char * channel = util_vqp2utf(cp, r_channel);

		/* add channel to user's chanlist */
		userlist_user_channel_join(src, channel, 0);
		free(channel);
	}
	free(src);
	free(dst);
}

static void msghandler_channel_list_req(
	vqp_msg_t msg, void * user_data,
	const char * r_src)
{
	enum vqp_codepage cp = userlist_user_codepage_vqp(r_src);
	char * src = util_vqp2utf(cp, r_src);

	if(!user_is_my_nickname(src)) {
		/* make sure our chanlist contains the #Main channel */
		char * chanlist, * vqp_chanlist, * r_chanlist;

		chanlist = chanlist_copy(*user_p_chanlist());
		chanlist = chanlist_add(chanlist, VQCHAT_MAIN_CHANNEL);
		vqp_chanlist = chanlist_make_vqp_chanlist(chanlist);
		free(chanlist);
		r_chanlist = util_utf2vqp(cp, vqp_chanlist);
		free(vqp_chanlist);

		/* reply with ack */
		msgloop_send(vqp_msg_channel_list_ack(vqp_msg_link(msg), r_src, r_chanlist), 0);
		free(r_chanlist);
	}
	free(src);
}

static void msghandler_channel_list_ack(
	vqp_msg_t msg, void * user_data,
	const char * r_dst, const char * r_channel_list)
{
	enum vqp_codepage cp = user_codepage();
	char * dst = util_vqp2utf(cp, r_dst),
		* channel_list = util_vqp2utf(cp, r_channel_list);

	if(user_is_my_nickname(dst)) {
		char * chanlist = chanlist_parse_vqp_chanlist(channel_list);
		service_join_channel_merge_chanlist(chanlist);
		free(chanlist);
	}
	free(dst);
	free(channel_list);
}

static void msghandler_message(
	vqp_msg_t msg, void * user_data,
	const char * r_src, const char * r_dst, const char * r_text,
	int is_multiaddress_msg)
{
	enum vqp_codepage cp = userlist_user_codepage_vqp(r_src);
	char * src = util_vqp2utf(cp, r_src),
		* dst = util_vqp2utf(cp, r_dst);

	if(!user_is_my_nickname(src) && user_is_my_nickname(dst)) {
		char * text = util_vqp2utf(cp, r_text);
		HANDLE hContact;

		hContact = contacts_find_contact(src);
		if(!hContact)
			hContact = contacts_add_contact(src, 0);
		
		if(hContact) {
			char * r_autoanswer = util_utf2vqp(cp, user_awaymsg());

			/* write message event to contact window/history */
			contacts_input_contact_message(hContact, text);

			/* send message ack */
			msgloop_send_to(
				vqp_msg_message_ack(
					vqp_msg_link(msg), dst, src,
					user_vqp_status(), user_gender(),
					r_autoanswer, vqp_msg_signature(msg)),
				0, vqp_msg_src_addr(msg)
			);
			free(r_autoanswer);
		}
		
		free(text);
	}
	free(src);
	free(dst);
}

static void msghandler_message_ack(
	vqp_msg_t msg, void * user_data,
	const char * r_src, const char * r_dst,
	enum vqp_status src_status, enum vqp_gender src_gender,
	const char * r_src_autoanswer, const char * orig_packetsig)
{
	/* we don't send any messages for now */
}

static void msghandler_info_req(
	vqp_msg_t msg, void * user_data,
	const char * r_src, const char * r_dst)
{
	enum vqp_codepage cp = userlist_user_codepage_vqp(r_src);
	char * src = util_vqp2utf(cp, r_src),
		* dst = util_vqp2utf(cp, r_dst);

	if(!user_is_my_nickname(r_src) && user_is_my_nickname(r_dst)) {
		wchar_t w_computer_name[MAX_COMPUTERNAME_LENGTH + 1],
			w_user_name[UNLEN + 1];
		char * computer_name, * r_computer_name;
		char * user_name, * r_user_name;
		char * netgroup_name, * r_netgroup_name;
		char * chanlist, * vqp_chanlist, * r_chanlist;
		char * r_autoanswer;
		DWORD computer_name_len, user_name_len;
/*		LPWKSTA_INFO_100 wksta_info100; */
		
		/* get computer name */
		computer_name_len = MAX_COMPUTERNAME_LENGTH;
		w_computer_name[0] = '\0';	/* in case the call fails */
		GetComputerNameW(w_computer_name, &computer_name_len);
		computer_name = util_uni2utf(w_computer_name);
		r_computer_name = util_utf2vqp(cp, computer_name);
		free(computer_name);

		/* get user name */
		user_name_len = UNLEN;
		w_user_name[0] = '\0';		/* in case the call fails */
		GetUserNameW(w_user_name, &user_name_len);
		user_name = util_uni2utf(w_user_name);
		r_user_name = util_utf2vqp(cp, user_name);
		free(user_name);
		
		/* get netgroup name */
/*
		if(NetWkstaGetInfo(NULL, 100, (LPBYTE *) &wksta_info100) == NERR_Success) {
			r_netgroup_name = util_uni2utf(wksta_info100->wki100_langroup);
			NetApiBufferFree(wksta_info100);
		} else {
			r_netgroup_name = strdup("");
		}
*/
		r_netgroup_name = strdup("");

		/* make vqp chanlist */
		chanlist = chanlist_copy(*user_p_chanlist());
		chanlist = chanlist_add(chanlist, VQP_MAIN_CHANNEL);
		vqp_chanlist = chanlist_make_vqp_chanlist(chanlist);
		free(chanlist);
		r_chanlist = util_utf2vqp(cp, vqp_chanlist);
		free(vqp_chanlist);

		/* make autoanswer */
		r_autoanswer = util_utf2vqp(cp, user_awaymsg());

		/* return info request reply */
		/* XXX: add real ip/port address string */
		msgloop_send_to(
			vqp_msg_info_ack(
				vqp_msg_link(msg), r_dst, r_src,
				r_computer_name, r_user_name,
				"0.0.0.0:0", r_chanlist, r_autoanswer,
				r_netgroup_name, VQCHAT_VQP_SWPLATFORM, VQCHAT_VQP_SWNAME
			),
			0, vqp_msg_src_addr(msg)
		);

		free(r_autoanswer);
		free(r_chanlist);
		free(r_user_name);
		free(r_computer_name);
		free(r_netgroup_name);
	}
	free(src);
	free(dst);
}

static void msghandler_info_ack(
	vqp_msg_t msg, void * user_data,
	const char * r_src, const char * r_dst,
	const char * r_src_computer_name, const char * r_src_user_name,
	const char * r_src_node_address, const char * r_src_channel_list,
	const char * r_src_autoanswer, const char * r_src_netgroup,
	const char * r_src_platform, const char * r_src_software)
{
	enum vqp_codepage cp = userlist_user_codepage_vqp(r_src);
	char * src = util_vqp2utf(cp, r_src),
		* dst = util_vqp2utf(cp, r_dst);

	if(!user_is_my_nickname(r_src) && user_is_my_nickname(r_dst)) {
		char * computer_name = util_vqp2utf(cp, r_src_computer_name),
			* user_name = util_vqp2utf(cp, r_src_user_name),
			* awaymsg = util_vqp2utf(cp, r_src_autoanswer),
			* netgroup = util_vqp2utf(cp, r_src_netgroup),
			* platform = util_vqp2utf(cp, r_src_platform),
			* software = util_vqp2utf(cp, r_src_software),
			* vqp_chanlist = util_vqp2utf(cp, r_src_channel_list);

		HANDLE hContact = contacts_find_contact(src);
		if(hContact) {
			char * chanlist;

			/* store contact info
			 */
			contacts_set_contact_addr(hContact, vqp_msg_src_addr(msg));
			contacts_set_contact_about(hContact, awaymsg);
			contacts_set_contact_property(hContact, CONTACT_COMPUTER, computer_name);
			contacts_set_contact_property(hContact, CONTACT_USER, user_name);
			contacts_set_contact_property(hContact, CONTACT_WORKGROUP, netgroup);
			contacts_set_contact_property(hContact, CONTACT_PLATFORM, platform);
			contacts_set_contact_property(hContact, CONTACT_SOFTWARE, software);
			contacts_set_contact_gender(hContact, userlist_user_gender(src));

			/* update user's channel list */
			chanlist = chanlist_parse_vqp_chanlist(vqp_chanlist);
			userlist_user_chanlist_update(src, chanlist);
			chanlist_free(chanlist);

			/* confirm we've received the info correctly */
			ProtoBroadcastAck(
				VQCHAT_PROTO, hContact,
				ACKTYPE_GETINFO, ACKRESULT_SUCCESS, (HANDLE)2, (LPARAM)1);
		}

		free(vqp_chanlist);
		free(computer_name);
		free(user_name);
		free(awaymsg);
		free(netgroup);
		free(platform);
		free(software);
	}
	
	free(src);
	free(dst);
}

static void msghandler_beep_signal(/*{{{*/
	vqp_msg_t msg, void * user_data,
	const char * r_src, const char * r_dst)
{
	enum vqp_codepage cp = userlist_user_codepage_vqp(r_src);
	char * src = util_vqp2utf(cp, r_src),
		* dst = util_vqp2utf(cp, r_dst);

	if(!user_is_my_nickname(src) && user_is_my_nickname(dst)) {
		HANDLE hContact = contacts_find_contact(src);
		if(hContact) {
			/* play beep sound and emit msg, if configured */
			SkinPlaySound(SKIN_SOUND_ALERT_BEEP);
			if(db_byte_get(NULL, VQCHAT_PROTO, "MsgOnAlertBeep", 0))
				contacts_input_contact_message(hContact, "*** ALERT BEEP ***");
			
			/* send beep ack */
			msgloop_send_to(
				vqp_msg_beep_ack(vqp_msg_link(msg), r_dst, r_src, user_gender()),
				0, vqp_msg_src_addr(msg));
		}
	}
	free(src);
	free(dst);
}

static void msghandler_beep_ack(
	vqp_msg_t msg, void * user_data,
	const char * r_src, const char * r_dst, enum vqp_gender src_gender)
{
	/* we don't make any use of this ack, at least for now */
}

static void msghandler_private_open(
	vqp_msg_t msg, void * user_data,
	const char * r_src, const char * r_dst, enum vqp_gender src_gender)
{
	enum vqp_codepage cp = userlist_user_codepage_vqp(r_src);
	char * src = util_vqp2utf(cp, r_src),
		* dst = util_vqp2utf(cp, r_dst);

	if(!user_is_my_nickname(src) && user_is_my_nickname(dst)) {
		/* open chat */
		if(!userlist_user_is_chat_open(src)) {
			/* send chat open reply */
			msgloop_send_to(
				vqp_msg_private_open(
					vqp_msg_link(msg), r_dst, r_src, user_gender()),
				0, vqp_msg_src_addr(msg));
			
			/* mark the chat as open */
			userlist_user_set_chat_open(src, 1);
		}
	}
	free(src);
	free(dst);
}

static void msghandler_private_close(
	vqp_msg_t msg, void * user_data,
	const char * r_src, const char * r_dst, enum vqp_gender src_gender)
{
	enum vqp_codepage cp = userlist_user_codepage_vqp(r_src);
	char * src = util_vqp2utf(cp, r_src),
		* dst = util_vqp2utf(cp, r_dst);

	if(!user_is_my_nickname(src) && user_is_my_nickname(dst)) {
		userlist_user_set_chat_open(src, 0);
	}
	free(src);
	free(dst);
}

static void msghandler_private_text(
	vqp_msg_t msg, void * user_data,
	const char * r_src, const char * r_dst, const char * r_text,
	int is_action_text)
{
	enum vqp_codepage cp = userlist_user_codepage_vqp(r_src);
	char * src = util_vqp2utf(cp, r_src),
		* dst = util_vqp2utf(cp, r_dst);

	if(!user_is_my_nickname(src) && user_is_my_nickname(dst)) {
		HANDLE hContact = contacts_find_contact(src);
		if(!hContact)
			hContact = contacts_add_contact(src, 0);

		if(hContact) {
			char * text = util_vqp2utf(cp, r_text);

			/* check that we have chat open with the user */
			if(!userlist_user_is_chat_open(src)) {
				msgloop_send_to(
					vqp_msg_private_open(
						vqp_msg_link(msg), r_dst, r_src, user_gender()),
					0, vqp_msg_src_addr(msg));
			
				/* mark the chat as open */
				userlist_user_set_chat_open(src, 1);
			}

			if(is_action_text) {
				/* make the '<nickname> <text>' - style action text */
				int text_len = strlen(text);
				int src_len = strlen(src);
				char * action_text = malloc(src_len + text_len + 10);

				memcpy(action_text, src, src_len);
				action_text[src_len] = ' ';
				memcpy(action_text + src_len + 1, text, text_len + 1);
				
				contacts_input_contact_message(hContact, action_text);
				free(action_text);
			} else {
				contacts_input_contact_message(hContact, text);
			}
			free(text);
		}
	}
	free(src);
	free(dst);
}

/* exported routines
 */

/* msghandler_init:
 *	setups msghandler module
 */
void msghandler_init()
{
	memset(&s_parseFuncs, 0, sizeof(s_parseFuncs));
	s_parseFuncs.struct_size = sizeof(s_parseFuncs);

	/* add handler funcs */
	s_parseFuncs.func_refresh_req = msghandler_refresh_req;/*{{{*/
	s_parseFuncs.func_refresh_ack = msghandler_refresh_ack;

#ifdef VYPRESSCHAT
	s_parseFuncs.func_ping = msghandler_ping;
	s_parseFuncs.func_pong = msghandler_pong;
	s_parseFuncs.func_flood_notification = msghandler_flood_notification;
#endif
/*	s_parseFuncs.func_remote_exec = msghandler_remote_exec;
	s_parseFuncs.func_remote_exec_ack = msghandler_remote_exec_ack;
*/
	s_parseFuncs.func_nick_change = msghandler_nick_change;
	s_parseFuncs.func_status_change = msghandler_status_change;
	s_parseFuncs.func_active_change = msghandler_active_change;
	s_parseFuncs.func_channel_join = msghandler_channel_join;
	s_parseFuncs.func_channel_leave = msghandler_channel_leave;
	s_parseFuncs.func_channel_text = msghandler_channel_text;
	s_parseFuncs.func_channel_topic_change = msghandler_channel_topic_change;
	s_parseFuncs.func_channel_current_topic = msghandler_channel_current_topic;
	s_parseFuncs.func_channel_whohere_req = msghandler_channel_whohere_req;
	s_parseFuncs.func_channel_whohere_ack = msghandler_channel_whohere_ack;
	s_parseFuncs.func_channel_list_req = msghandler_channel_list_req;
	s_parseFuncs.func_channel_list_ack = msghandler_channel_list_ack;
	s_parseFuncs.func_message = msghandler_message;
	s_parseFuncs.func_message_ack = msghandler_message_ack;
	s_parseFuncs.func_info_req = msghandler_info_req;
	s_parseFuncs.func_info_ack = msghandler_info_ack;
	s_parseFuncs.func_beep_signal = msghandler_beep_signal;
	s_parseFuncs.func_beep_ack = msghandler_beep_ack;
	s_parseFuncs.func_private_open = msghandler_private_open;
	s_parseFuncs.func_private_close = msghandler_private_close;
	s_parseFuncs.func_private_text = msghandler_private_text;/*}}}*/
}

/* msghandler_uninit:
 *	cleansup user status module
 */
void msghandler_uninit()
{
	/* nothing here */
}

/* msghandler_apc:
 *	invoked from msgloop thread with QueueUserAPC,
 *	the param is vqp_msg_t, which gets free'd after use
 */
void CALLBACK msghandler_apc(ULONG_PTR vqmsg)
{
	vqp_msg_parse(&s_parseFuncs, (vqp_msg_t) vqmsg, 0);
	vqp_msg_free((vqp_msg_t)vqmsg);
}

