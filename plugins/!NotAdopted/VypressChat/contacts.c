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
 * $Id: contacts.c,v 1.26 2005/03/17 11:02:43 bobas Exp $
 */

#include "miranda.h"

#include "main.h"
#include "user.h"
#include "userlist.h"
#include "msgloop.h"
#include "contacts.h"
#include "util.h"

/* static data
 */
static vqp_link_t s_vqpLink;

/* static routines
 */

static __inline const char * contact_prop_name_by_enum(
	enum contact_property_enum property)
{
	const char * name = NULL;
	switch(property) {
	case CONTACT_COMPUTER:	name = "Computer"; break;
	case CONTACT_USER:	name = "ComputerUser"; break;
	case CONTACT_WORKGROUP:	name = "Workgroup"; break;
	case CONTACT_PLATFORM:	name = "Platform"; break;
	case CONTACT_SOFTWARE:	name = "Software"; break;
	}
	return name;
}

/* contacts_add_queued_message:
 *	adds a message to contacts message queue while
 *	the contact is offline. contacts_set_contact_status should
 *	send these messages when contact gets online.
 */
static int contacts_add_queued_message(
	HANDLE hContact, const char * text, WPARAM flags)
{
	DBVARIANT dbv = {0, };
	char * new_data;
	size_t new_data_sz;
	DBCONTACTWRITESETTING cws;

	if(!db_get(hContact, VQCHAT_PROTO, "QueuedMsgs", &dbv)) {
		size_t text_len = strlen(text) + 1;

		if(dbv.type!=DBVT_BLOB && (dbv.cpbVal < (sizeof(WPARAM) + 1)))
			return 1;
		
		new_data_sz = dbv.cpbVal + sizeof(flags) + text_len;
		new_data = malloc(new_data_sz);

		memcpy(new_data, dbv.pbVal, dbv.cpbVal);
		*((WPARAM *)(new_data + dbv.cpbVal)) = flags;
		memcpy(new_data + dbv.cpbVal + sizeof(flags), text, text_len);
	} else {
		size_t text_len = strlen(text) + 1;
		new_data_sz = sizeof(flags) + text_len;
		new_data = malloc(new_data_sz);

		*((WPARAM *)new_data) = flags;
		memcpy(new_data + sizeof(flags), text, text_len);
	}

	cws.szModule = VQCHAT_PROTO;
	cws.szSetting = "QueuedMsgs";
	cws.value.type = DBVT_BLOB;
	cws.value.cpbVal = new_data_sz;
	cws.value.pbVal = new_data;
	CallService(MS_DB_CONTACT_WRITESETTING, (WPARAM)hContact, (LPARAM)&cws);

	free(new_data);
	db_free(&dbv);

	return 0;
}

/* contacts_fetch_queued_message
 *	returns queued message (or NULL if no more)
 */
static char * contacts_fetch_queued_message(
	HANDLE hContact, WPARAM * p_wparam)
{
	DBVARIANT dbv;
	size_t msg_sz;
	char * text;

	if(db_get(hContact, VQCHAT_PROTO, "QueuedMsgs", &dbv))
		return NULL;

	if(dbv.type != DBVT_BLOB || (dbv.cpbVal < (sizeof(WPARAM) + 1))) {
		db_unset(hContact, VQCHAT_PROTO, "QueuedMsgs");
		return NULL;
	}

	/* XXX: check that we have '\0' terminator at the end */
	msg_sz = sizeof(WPARAM) + strlen(dbv.pbVal + sizeof(WPARAM)) + 1;
	if(p_wparam)
		*p_wparam = *(WPARAM *)dbv.pbVal;
	text = strdup(dbv.pbVal + sizeof(WPARAM));

	if(msg_sz == dbv.cpbVal) {
		/* unset the setting if there are no more msgs
		 */
		db_unset(hContact, VQCHAT_PROTO, "QueuedMsgs");
	} else {
		/* skip past this message and rewrite the blob
		 */
		DBCONTACTWRITESETTING cws;

		cws.szModule = VQCHAT_PROTO;
		cws.szSetting = "QueuedMsgs";
		cws.value.type = DBVT_BLOB;
		cws.value.cpbVal = dbv.cpbVal - msg_sz;
		cws.value.pbVal = dbv.pbVal + msg_sz;
		CallService(MS_DB_CONTACT_WRITESETTING, (WPARAM)hContact, (LPARAM)&cws);
	}

	db_free(&dbv);
	return text;
}

/* exported routines
 */
void contacts_connected(vqp_link_t link)
{
	s_vqpLink = link;
}

void contacts_disconnected()
{
	s_vqpLink = NULL;
}

void contacts_hook_modules_loaded()
{
	contacts_set_all_offline();
}

void contacts_set_all_offline()
{
	HANDLE hContact;

	hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while(hContact) {
		char * proto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
		if(proto && !strcmp(proto, VQCHAT_PROTO)) {
			/* dont touch chat rooms */
			if(db_byte_get(hContact, VQCHAT_PROTO, "ChatRoom", 0) == 0)
				contacts_set_contact_status(hContact, ID_STATUS_OFFLINE);
		}

		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
	}
}

HANDLE contacts_add_contact(const char * nickname, int permanent)
{
	HANDLE hContact;

	hContact = contacts_find_contact(nickname);
	if(hContact) {
		/* show it on the list, if hidden */
		db_unset(hContact, "CList", "Hidden");
		if(permanent)
			db_unset(hContact, "CList", "NotOnList");
		
		contacts_set_contact_status(hContact, userlist_user_status(nickname));
		return hContact;
	}

	/* add contact to db */
	hContact = (HANDLE)CallService(MS_DB_CONTACT_ADD, 0, 0);
	if(hContact) {
		/* add contact to VQCHAT_PROTO contact list */
		CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hContact, (LPARAM)VQCHAT_PROTO);
		
		/* init contact settings */
		db_unset(hContact, "CList", "Hidden");
		if(permanent) {
			db_unset(hContact, "CList", "NotOnList");
		} else	{
			db_byte_set(hContact, "CList", "NotOnList", 1);
		}
		
		contacts_set_contact_nickname(hContact, nickname);

		db_byte_set(hContact, VQCHAT_PROTO, "PreferMsg",
			db_byte_get(NULL, VQCHAT_PROTO, "ContactsPreferMsg", 0));
	}

	return hContact;
}

/* contacts_find_contact:
 *	finds contact by user nickname
 *	the returned contact is ensured to be valid
 */
HANDLE contacts_find_contact(const char * nickname)
{
	HANDLE hContact;
	
	/* walk all the contacts and find ours */
	hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	while(hContact) {
		if(contacts_is_user_contact(hContact)) {
			char * contact_nickname = contacts_get_nickname(hContact);

			if(!strcmp(contact_nickname, nickname)) {
				free(contact_nickname);
				break;
			}

			free(contact_nickname);
		}
		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
	}

	return hContact;
}

void contacts_set_contact_status(HANDLE hContact, int new_status)
{
	WORD prevStatus;

	/* dont touch hidden contacts */
	if(db_byte_get(hContact, "CList", "Hidden", 0) != 0)
		return;
				
	/* set contact status, if prev status is not the same */
	prevStatus = db_word_get(hContact, VQCHAT_PROTO, "Status", ID_STATUS_OFFLINE);
	if(prevStatus != new_status) {
		db_word_set(hContact, VQCHAT_PROTO, "Status", new_status);

		if(prevStatus == ID_STATUS_OFFLINE) {
			/* send queued messages, if any */
			char * text;
			WPARAM wparam;

			while((text = contacts_fetch_queued_message(hContact, &wparam)) != NULL) {
				contacts_send_contact_message(hContact, text, wparam, 1);
				free(text);
			}
		}
	}
}

/* contacts_set_contact_nickname:
 *	updates contact's nickname, also updates contacts' status
 *	based on userlist's user data
 */
void contacts_set_contact_nickname(HANDLE hContact, const char * new_nickname)
{
	char * loc_new_nickname;
	ASSERT_RETURNIFFAIL(VALIDPTR(new_nickname));

	/* XXX: check for duplicates */

	/* set db name */
	loc_new_nickname = util_utf2loc(new_nickname);
	db_string_set(hContact, VQCHAT_PROTO, "Nick", loc_new_nickname);
	free(loc_new_nickname);

	/* update contact status */
	contacts_set_contact_status(hContact, userlist_user_status(new_nickname));
}

void contacts_set_contact_addr(HANDLE hContact, vqp_addr_t new_addr)
{
	/* dont touch hidden contacts */
	if(db_byte_get(hContact, "CList", "Hidden", 0) != 0)
		return;

	if(new_addr.conn == VQP_PROTOCOL_CONN_UDP) {
		if(db_dword_get(hContact, VQCHAT_PROTO, "IP", 0) != new_addr.node.ip)
			db_dword_set(hContact, VQCHAT_PROTO, "IP", new_addr.node.ip);
	} else {
		db_unset(hContact, VQCHAT_PROTO, "IP");
	}
}

void contacts_set_contact_about(HANDLE hContact, const char * about_text)
{
	char * loc_about_text;

	ASSERT_RETURNIFFAIL(VALIDPTR(about_text));
	
	/* dont touch hidden contacts */
	if(db_byte_get(hContact, "CList", "Hidden", 0) != 0)
		return;

	/* store about text in contact */
	loc_about_text = util_utf2loc(about_text);
	db_string_set(hContact, VQCHAT_PROTO, "About", loc_about_text);
	free(loc_about_text);
}

void contacts_set_contact_gender(HANDLE hContact, enum vqp_gender gender)
{
	ASSERT_RETURNIFFAIL(hContact!=NULL);
	db_byte_set(hContact, VQCHAT_PROTO, "Gender", gender == VQP_GENDER_MALE ? 'M': 'F');
}

void contacts_set_contact_property(
	HANDLE hContact, enum contact_property_enum property,
	const char * string)
{
	const char * prop_name;
	ASSERT_RETURNIFFAIL(hContact!=NULL && VALIDPTR(string));

	prop_name = contact_prop_name_by_enum(property);
	ASSERT_RETURNIFFAIL(VALIDPTR(prop_name));

	db_string_set(hContact, VQCHAT_PROTO, prop_name, string);
}

char * contacts_get_contact_property(
	HANDLE hContact, enum contact_property_enum property)
{
	DBVARIANT dbv;
	const char * prop_name;
	char * value;

	prop_name = contact_prop_name_by_enum(property);
	ASSERT_RETURNVALIFFAIL(VALIDPTR(prop_name), strdup("unknown"));

	value = NULL;
	if(!db_get(hContact, VQCHAT_PROTO, prop_name, &dbv)) {
		if(dbv.type == DBVT_ASCIIZ) {
			value = strdup(dbv.pszVal);
		}
		db_free(&dbv);
	}
	if(value == NULL)
		value = strdup("(unknown)");

	return value;
}

/* contacts_get_contact_info:
 *	queries user info from contact
 *	(see PSS_GETINFO)
 */
static void CALLBACK
contacts_get_contact_info_minimal_apc(ULONG_PTR cb_data)
{
	HANDLE hContact = (HANDLE)(((void **)cb_data)[0]);
	int nReplies = (int)(((void **)cb_data)[1]);

	/* just send the ack */
	ProtoBroadcastAck(VQCHAT_PROTO, hContact,
		ACKTYPE_GETINFO, ACKRESULT_SUCCESS, (HANDLE)nReplies, (LPARAM)0);

	/* free cb data */
	free((void *) cb_data);
}

int contacts_get_contact_info(HANDLE hContact, WPARAM flags)
{
	int nReplies = (flags & SGIF_MINIMAL) ? 1: 2;
	void ** cb_data;

	/* we can get info on user contacts only */
	if(!contacts_is_user_contact(hContact))
		return 1;

	/* we know minimal info already */
	cb_data = (void **) malloc(sizeof(void*) * 2);
	cb_data[0] = (void *)hContact;
	cb_data[1] = (void *)nReplies;
	QueueUserAPC(contacts_get_contact_info_minimal_apc, g_hMainThread, (ULONG_PTR)cb_data);

	if(!(flags & SGIF_MINIMAL)) {
		/* we need not only minimal info: send info request msg:
		 * see msghandler.c QCS_MSG_INFO_REPLY handler rountine
		 */
		char * r_nickname, * dst, * r_dst;
		enum vqp_codepage cp;

		dst = contacts_get_nickname(hContact);
		cp = userlist_user_codepage(dst);

		r_nickname = util_utf2vqp(cp, user_nickname());
		r_dst = util_utf2vqp(cp, dst);
		free(dst);
		
		msgloop_send_to(
			vqp_msg_info_req(s_vqpLink, r_nickname, r_dst),
			0, userlist_user_addr(dst));

		free(r_nickname);
		free(r_dst);
	}

	return 0; /* success */
}

/* contacts_get_contact_status:
 *	returns contact's current status
 */
int contacts_get_contact_status(HANDLE hContact)
{
	int status;
	char * utf_nickname;
	ASSERT_RETURNVALIFFAIL(contacts_is_user_contact(hContact), ID_STATUS_OFFLINE);

	utf_nickname = contacts_get_nickname(hContact);
	if(utf_nickname) {
		status = userlist_user_status(utf_nickname);
		free(utf_nickname);
	} else {
		status = ID_STATUS_OFFLINE;
	}

	return status;
}

/* contacts_get_nickname:
 *	return utf nickname of the contact
 */
char * contacts_get_nickname(HANDLE hContact)
{
	char * nickname;

	if(contacts_is_user_contact(hContact)) {
		DBVARIANT dbv;
		
		db_get(hContact, VQCHAT_PROTO, "Nick", &dbv);
		nickname = util_loc2utf(dbv.pszVal);
		db_free(&dbv);
	} else {
		nickname = strdup("(null)");
	}

	return nickname;
}

/* contacts_input_contact_message:
 *	  Writes an unread input message to contact's log.
 *
 *	  The message will get routed through protocol plugins
 *	and (hopefully) will be afterwards received by
 *	service.c:service_contact_recv_message(), which will add it to our the database
 */
void contacts_input_contact_message(HANDLE hContact, const char * message_text)
{
	CCSDATA ccs;
	PROTORECVEVENT pre;

	/* make miranda-style ansi-unicode mixed string
	 */
	char * loc_text = util_utf2loc(message_text);
	wchar_t * uni_text = util_utf2uni(message_text);
	int loc_text_len = strlen(loc_text) + 1;
	char * m_text = malloc(loc_text_len + loc_text_len * sizeof(wchar_t));

	memcpy(m_text, loc_text, loc_text_len);
	free(loc_text);
	memcpy(m_text + loc_text_len, uni_text, loc_text_len * sizeof(wchar_t));
	free(uni_text);

	/* send message event through protocol plugins
	 */
	pre.flags = PREF_UNICODE;
	pre.timestamp = (DWORD)time(NULL);
	pre.szMessage = m_text;
	pre.lParam = 0;
	ccs.szProtoService = PSR_MESSAGE;
	ccs.hContact = hContact;
	ccs.wParam = 0;
	ccs.lParam = (LPARAM) &pre;
			
	CallService(MS_PROTO_CHAINRECV, 0, (LPARAM)&ccs);

	/* free ansi/ucs-2 text */
	free(m_text);
}

/* contacts_send_contact_message:
 *	sends a message to a contact
 *	for details look miranda's m_protosvc.h:PSS_MESSAGE
 *
 * returns:
 *	(HANDLE)hprocess
 */
static void CALLBACK
contacts_send_contact_message_result_apc(ULONG_PTR data)
{
	HANDLE hContact = (HANDLE)(((void**)data)[0]);
	int result = (int)(((void**)data)[1]);
	HANDLE hProcess = (HANDLE)(((void **)data)[2]);

	ProtoBroadcastAck(VQCHAT_PROTO, hContact, ACKTYPE_MESSAGE, result, hProcess, (LPARAM)0);

	/* free cb data */
	free((void*)data);
}
	
int contacts_send_contact_message(
	HANDLE hContact, const char * message_text, WPARAM flags, int is_queued)
{
	static DWORD msg_process_handle = 0;

	int i;
	char * contact_nick, * r_nickname, * r_contact;
	enum vqp_codepage contact_cp;
	DWORD process;

	ASSERT_RETURNVALIFFAIL(VALIDPTR(message_text), 0);

	/* cannot send message when offline
	 * or when user has no nickname
	 */
	contact_nick = contacts_get_nickname(hContact);
	if(!contact_nick) {
		/* send failure message ack */
		void ** cb_data = malloc(sizeof(void*) * 3);
		cb_data[0] = (void*)hContact;
		cb_data[1] = (void*)ACKRESULT_FAILED;
		cb_data[2] = (void*)1;	/* process handle */
		QueueUserAPC(
			contacts_send_contact_message_result_apc,
			g_hMainThread, (ULONG_PTR)cb_data);

		return (int)cb_data[2];
	}

	/* write as queued message if we're not online
	 * or the contact is not online
	 */
	if(user_offline() || !userlist_user_exists(contact_nick)) {
		process = msg_process_handle ++;

		contacts_add_queued_message(hContact, message_text, flags);
		
		/* we've sent this message successfully (not really) */
		void ** cb_data = malloc(sizeof(void*) * 3);
		cb_data[0] = (void*)hContact;
		cb_data[1] = (void*)(int)ACKRESULT_SUCCESS;
		cb_data[2] = (void*)process;
		QueueUserAPC(
			contacts_send_contact_message_result_apc,
			g_hMainThread, (ULONG_PTR)cb_data);

		free(contact_nick);
		return process;
	}

	/* ok, the user is online and we can send the message
	 */
	contact_cp = userlist_user_codepage(contact_nick);
	r_contact = util_utf2vqp(contact_cp, contact_nick);
	r_nickname = util_utf2vqp(contact_cp, user_nickname());

	/* vypress chat 2.0 has abandoned private chats, -
	 * use private messages; also check the "PreferMsg" option
	 */
	if((userlist_user_swversion(contact_nick) >= VQP_MAKE_SWVERSION(2, 0))
		|| (db_byte_get(hContact, VQCHAT_PROTO, "PreferMsg", 0)
			&& userlist_user_can_receive_msg(contact_nick))
		) {
		char * r_msg_text = util_utf2vqp(contact_cp, message_text);

		msgloop_send_to(
			vqp_msg_message(s_vqpLink, r_nickname, r_contact, r_msg_text, 0),
			0, userlist_user_addr(contact_nick));
		free(r_msg_text);
	} else {
		char ** msg_lines;
		if(!userlist_user_is_chat_open(contact_nick)) {
			/* mark chat as opened */
			userlist_user_set_chat_open(contact_nick, 1);
		
			/* send "Open private chat" message */
			msgloop_send_to(
				vqp_msg_private_open(s_vqpLink, r_nickname, r_contact, user_gender()),
				0, userlist_user_addr(contact_nick));
		}

		/* send message text, after spliting it into multiple lines
		 * and skipping the empty ones
		 */
		msg_lines = util_split_multiline(message_text, 0);
		for(i = 0; msg_lines[i]; i++) {
			char * r_text = util_utf2vqp(contact_cp, msg_lines[i]);
			
			msgloop_send_to(
				vqp_msg_private_text(s_vqpLink, r_nickname, r_contact, r_text, 0),
				0, userlist_user_addr(contact_nick));
			free(r_text);
		}
		util_free_str_list(msg_lines);
	}

	if(!is_queued) {
		/* add APC to send protocol ack, to notify that we've sent the message
		 * (if this message was queued, there's no sense to send the ack, it was sent already)
		 */
		process = msg_process_handle ++;

		void ** cb_data = malloc(sizeof(void*) * 3);
		cb_data[0] = (void*)hContact;
		cb_data[1] = (void*)(int)ACKRESULT_SUCCESS;
		cb_data[2] = (void*)process;
		QueueUserAPC(
			contacts_send_contact_message_result_apc,
			g_hMainThread, (ULONG_PTR)cb_data);
	} else {
		process = 0;
	}

	/* free strings */
	free(contact_nick);
	free(r_contact);
	free(r_nickname);
	
	return process;	/* success */
}

/* contacts_send_beep:
 *	sends a beep to the user
 */
void contacts_send_beep(HANDLE hContact)
{
	char * contact_nick;
	ASSERT_RETURNIFFAIL(hContact!=NULL);

	contact_nick = contacts_get_nickname(hContact);
	if(contact_nick) {
		enum vqp_codepage contact_cp = userlist_user_codepage(contact_nick);
		char * r_nickname = util_utf2vqp(contact_cp, user_nickname()),
			* r_contact = util_utf2vqp(contact_cp, contact_nick);

		/* send the beep message */
		msgloop_send_to(
			vqp_msg_beep_signal(s_vqpLink, r_nickname, r_contact),
			0, userlist_user_addr(contact_nick));
		free(r_nickname);
		free(r_contact);
	}
}

/* contacts_is_user_contact:
 *	returns non-0 if the contact is VQCHAT_PROTO user contact
 */
int contacts_is_user_contact(HANDLE hContact)
{
	char * proto;
	DBVARIANT dbv;

	ASSERT_RETURNVALIFFAIL(hContact!=NULL, 0);

	/* check contact protocol */
	proto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	if(!proto || strcmp(proto, VQCHAT_PROTO))
		return 0;

	/* check that the contact has Nick set */
	if(db_get(hContact, VQCHAT_PROTO, "Nick", &dbv)) return 0;
	if(dbv.type != DBVT_ASCIIZ) return 0;
	db_free(&dbv);
	
	/* check if this is a chatroom */
	if(db_byte_get(hContact, VQCHAT_PROTO, "ChatRoom", 0))
		return 0;

	return 1;
}

int contacts_is_chatroom_contact(HANDLE hContact)
{
	char * proto;

	ASSERT_RETURNVALIFFAIL(hContact!=NULL, 0);
	
	/* check contact protocol */
	proto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);
	if(!proto || strcmp(proto, VQCHAT_PROTO))
		return 0;	/* not VQCHAT_PROTO protocol contact */

	/* check that the contact has "ChatRoom" byte set */
	return db_byte_get(hContact, VQCHAT_PROTO, "ChatRoom", 0)!=0;
}

