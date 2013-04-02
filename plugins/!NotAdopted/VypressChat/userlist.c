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
 * $Id: userlist.c,v 1.26 2005/03/22 19:38:13 bobas Exp $
 */

#include <stdlib.h>
#include <string.h>

#include "contrib/hashtable.h"
#include "contrib/strhashfunc.h"

#include "miranda.h"
#include "main.h"
#include "user.h"
#include "userlist.h"
#include "contacts.h"
#include "msgloop.h"
#include "chatroom.h"
#include "chanlist.h"
#include "util.h"

/* struct descriptions
 */
struct userlist_entry {
	enum vqp_status vqp_status;
	enum vqp_codepage codepage;
	enum vqp_gender gender;
	unsigned int swversion;
	vqp_uuid_t uuid;
	vqp_addr_t addr;

	int alive;		/* non-0, if replied to last refresh req */
	int chat_open;
	char * chanlist;
};

struct userlist_remove_unreplied_info_struct {
	char ** dead_keys;
	int dead_key_count;
};

/* forward references
 */
static void userlist_entry_status_change(
	const char * user_name, struct userlist_entry * entry,
	enum vqp_status new_user_status, enum vqp_gender new_user_gender,
	const char * new_status_text, int add_to_log);

/* static data
 */
static struct hashtable * s_userlistHash = NULL;
static UINT_PTR s_userlistTimer;
static vqp_link_t s_vqpLink;

/* static routines
 */
#define userlist_entry_by_name(user_name) \
	((struct userlist_entry*)hashtable_search(s_userlistHash, (void*)(user_name)))

static struct userlist_entry *
userlist_entry_by_name_vqp(const char * vqp_user_name)
{
	struct userlist_entry * entry;
	char * user_name;

#ifdef VYPRESSCHAT
	/* by default we'll try search supposing vqp_user_name is in utf encoding */
	entry = userlist_entry_by_name(vqp_user_name);
	if(!entry) {
		/* next - we'll thinking the vqp_user_name is in locale encoding */
		entry = userlist_entry_by_name(user_name = util_loc2utf(vqp_user_name));
		free(user_name);
	}
#else
	/* by default we'll try the locale encoding:
	 * qChat clients don't know a thing about utf8
	 */
	entry = userlist_entry_by_name(user_name = util_loc2utf(vqp_user_name));
	free(user_name);
#endif

	return entry;
}

static void
s_userlistHash_free_entry(void * value)
{
	struct userlist_entry * entry = (struct userlist_entry *)value;

	chanlist_free(entry->chanlist);
	free(entry);
}

/* userlist_remove_unreplied_cb:
 *	this is used to remove 'dead' users from user list
 *	and should be invoked periodically
 *
 *	userlist_add/userlist_status_change are used to notify
 *	that a user is alive during the refresh period
 */
static int
userlist_remove_unreplied_enum_fn(
	void * key, void * value, void * user_data)
{
	struct userlist_entry * entry = (struct userlist_entry*)value;
	struct userlist_remove_unreplied_info_struct * info
		= (struct userlist_remove_unreplied_info_struct*)user_data;
	
	if(!entry->alive) {
		/* add user to dead user list */
		info->dead_keys[info->dead_key_count++] = strdup((char*)key);
	} else {
		/* reset the user as dead */
		entry->alive = 0;
	}

	return 1;	/* continue enumeration */
}

static void CALLBACK
userlist_remove_unreplied_cb(
	HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	struct userlist_remove_unreplied_info_struct info;
	int i;
	char * r_nickname;
	
	ASSERT_RETURNIFFAIL(VALIDPTR(s_userlistHash));

	/* perform enumeration to find the unreplied ones */
	info.dead_keys = malloc(
			sizeof(void*) * hashtable_count(s_userlistHash));
	info.dead_key_count = 0;
	hashtable_enumerate(
		s_userlistHash,
		userlist_remove_unreplied_enum_fn, (void*)&info);

	/* quietly remove those dead */
	for(i = 0; i < info.dead_key_count; i++) {
		userlist_remove(info.dead_keys[i], 0);
		free(info.dead_keys[i]);
	}

	/* free leftovers */
	free(info.dead_keys);

	/* send another user refresh list request to the network
	 */
	r_nickname = util_utf2vqp(user_codepage(), user_nickname());
	msgloop_send(vqp_msg_refresh_req(s_vqpLink, r_nickname, user_codepage()), 0);
	free(r_nickname);
}

/* exported routines
 */
void userlist_init()
{
	s_userlistTimer = 0;

	s_userlistHash = NULL;
	userlist_flush();
}

void userlist_uninit()
{
	hashtable_destroy(s_userlistHash, 1);
	s_userlistHash = NULL;

	if(s_userlistTimer) {
		KillTimer(NULL, s_userlistTimer);
		s_userlistTimer = 0;
	}
}

void userlist_flush()
{
	/* a rude way to cleanup the list */
	if(s_userlistHash) {
		hashtable_destroy(s_userlistHash, 1);
		s_userlistHash = NULL;
	}

	s_userlistHash = create_hashtable(
		256, hashtable_strhashfunc, hashtable_strequalfunc,
		s_userlistHash_free_entry);
}

void userlist_connected(vqp_link_t vqpLink)
{
	DWORD timeout = 1000 * db_byte_get(NULL, VQCHAT_PROTO,
				"UserlistTimeout", VQCHAT_DEF_REFRESH_TIMEOUT);
	s_vqpLink = vqpLink;

	userlist_flush();

	/* start userlist refresh timers */
	ASSERT_RETURNIFFAIL(!s_userlistTimer);
	s_userlistTimer = SetTimer(NULL, 0, timeout, userlist_remove_unreplied_cb);
}

void userlist_disconnected()
{
	ASSERT_RETURNIFFAIL(s_userlistTimer);

	userlist_flush();

	s_vqpLink = NULL;

	KillTimer(NULL, s_userlistTimer);
	s_userlistTimer = 0;
}

/* userlist_add:
 *	adds user to userlist, updates it's info if the user already exists
 */
void userlist_add(
	const char * name,
	enum vqp_status vqp_status, enum vqp_gender gender, const vqp_uuid_t * p_uuid,
	enum vqp_codepage codepage, unsigned int swversion, vqp_addr_t addr,
	int add_to_log)
{
	struct userlist_entry * entry;
	HANDLE hContact;

	ASSERT_RETURNIFFAIL(VALIDPTR(name) && VALIDPTR(s_userlistHash));

	entry = userlist_entry_by_name(name);
	if(entry) {
		entry->alive = 1;

		userlist_entry_status_change(
			name, entry, vqp_status, gender, "", add_to_log);
		return;
	}

	/* setup new user list entry struct */
	entry = malloc(sizeof(struct userlist_entry));
	entry->vqp_status = vqp_status;
	entry->codepage = codepage;
	entry->gender = gender;
	entry->swversion = swversion;
	entry->uuid = *p_uuid;
	entry->addr = addr;
	entry->alive = 1;
	entry->chat_open = 0;
	entry->chanlist = NULL;

	/* insert into the hash table */
	hashtable_insert(s_userlistHash, strdup(name), entry);

	/* notify DB that the user has become visible (if he is on list)
	 * and set user ip
	 */
	hContact = contacts_find_contact(name);
	if(hContact) {
		contacts_set_contact_status(hContact, user_status_by_vqp_status(vqp_status));
		contacts_set_contact_addr(hContact, addr);
	}

	/* every user of QuickChat/VypressChat networks is a member
	 * of #Main channel */
	userlist_user_channel_join(name, VQCHAT_MAIN_CHANNEL, add_to_log);
}

static int userlist_remove_channel_part_enum(
	const char * channel, void * enum_data)
{
	const char * user = ((const char **)enum_data)[0],
	      * explicit_part = ((const char **)enum_data)[1];

	ASSERT_RETURNVALIFFAIL(VALIDPTR(channel) && VALIDPTR(enum_data), 0);

	userlist_user_channel_part(user, channel, (int)explicit_part);
	return 1;
}

/* userlist_remove:
 *	removes specified user from userlist
 * params:
 *	@explicit_part - non-0 if the user left explicitly (he notified us of that)
 */
void userlist_remove(const char * user_name, int explicit_remove)
{
	struct userlist_entry * entry;

	ASSERT_RETURNIFFAIL(VALIDPTR(user_name) && VALIDPTR(s_userlistHash));

	entry = userlist_entry_by_name(user_name);
	if(entry) {
		const char * enum_cb_data[2];
		
		/* notify DB that the user has become offline
		 * (if the user is in db list)
		 */
		HANDLE hContact = contacts_find_contact(user_name);
		if(hContact)
			contacts_set_contact_status(hContact, ID_STATUS_OFFLINE);

		/* update user's chatroom:
		 * leave every channel the user is in */
		enum_cb_data[0] = user_name;
		enum_cb_data[1] = (const char *)explicit_remove;
		chanlist_enum(
			entry->chanlist,
			userlist_remove_channel_part_enum, (void*)enum_cb_data);
		
		/* remove user entry from the hash and free it */
		hashtable_remove(s_userlistHash, (void*)user_name, 1);
	}
}

static int
userlist_enum_proxy_fn(void * key, void * value, void * user_data)
{
	void ** proxy_data = (void **) user_data;

	return ((userlist_enum_fn)proxy_data[0])(
				(const char*)key, (void*)proxy_data[1]);
}

void userlist_enum(userlist_enum_fn enum_fn, void * enum_fn_data)
{
	void * proxy_data[2];
	
	ASSERT_RETURNIFFAIL(enum_fn);
	
	proxy_data[0] = (void*)enum_fn;
	proxy_data[1] = (void*)enum_fn_data;

	hashtable_enumerate(s_userlistHash,
			userlist_enum_proxy_fn, (void*)proxy_data);
}

void userlist_name_change(const char * user_name, const char * new_user_name)
{
	struct userlist_entry * entry;

	ASSERT_RETURNIFFAIL(VALIDPTR(user_name) && VALIDPTR(new_user_name));
	ASSERT_RETURNIFFAIL(VALIDPTR(s_userlistHash));
	ASSERT_RETURNIFFAIL(strcmp(user_name, new_user_name));

	/* bail out if we have the new user name in the userlist already */
	if(userlist_entry_by_name(new_user_name) )
		return;

	entry = userlist_entry_by_name(user_name);
	if(entry) {
		HANDLE hContact;

		/* remove entry from hash table and reinsert with different nickname */
		hashtable_remove(s_userlistHash, (void*)user_name, 0);
		hashtable_insert(s_userlistHash, strdup(new_user_name), entry);

		entry->alive = 1;

		/* update contact nickname and status
		 */
		hContact = contacts_find_contact(user_name);
		if(hContact) {
			if(db_byte_get(hContact, VQCHAT_PROTO, "LockNick", 0)) {
				contacts_set_contact_status(hContact, ID_STATUS_OFFLINE);
			} else {
				HANDLE hAnotherContact = contacts_find_contact(new_user_name);
				if(hAnotherContact) {
					/* there is another contact with destination name:
					 * set hContact to offline, and get hAnotherContact alive
					 */
					contacts_set_contact_status(hContact, ID_STATUS_OFFLINE);

					contacts_set_contact_status(
						hAnotherContact,
						user_status_by_vqp_status(entry->vqp_status)
					);
					contacts_set_contact_addr(hContact, entry->addr);
				} else {
					/* change nickname for the contact */
					contacts_set_contact_nickname(hContact, new_user_name);
				}
			}
		} else {
			/* the specified contact does not exist,
			 * try to find the contact with `new_user_name' nickname
			 */
			hContact = contacts_find_contact(new_user_name);
			if(hContact) {
				contacts_set_contact_status(
					hContact,
					user_status_by_vqp_status(entry->vqp_status)
				);
				contacts_set_contact_addr(hContact, entry->addr);
			}
		}

		/* update chatrooms */
		chatroom_channel_user_name_change(user_name, new_user_name, 1);
	}
}

/* userlist_entry_status_change:
 *	updates user's status
 */
static void
userlist_entry_status_change(
	const char * user_name, struct userlist_entry * entry,
	enum vqp_status new_user_status, enum vqp_gender new_user_gender,
	const char * mode_text, int add_to_log)
{
	entry->alive = 1;

	if(entry->vqp_status != new_user_status) {
		HANDLE hContact;
		int new_status = user_status_by_vqp_status(new_user_status);
			
		entry->vqp_status = new_user_status;
	
		/* notify db of user mode change */
		hContact = contacts_find_contact(user_name);
		if(hContact) {
			contacts_set_contact_status(hContact, new_status);
			contacts_set_contact_gender(hContact, new_user_gender);
		}

		/* update chatrooms */
		chatroom_channel_user_status_change(user_name, new_status, mode_text, add_to_log);
	}

}

void userlist_status_change(
	const char * user_name,
	enum vqp_status new_user_status, enum vqp_gender new_user_gender,
	const char * status_text, int add_to_log)
{
	struct userlist_entry * entry;

	ASSERT_RETURNIFFAIL(VALIDPTR(user_name) && VALIDPTR(s_userlistHash));

	entry = userlist_entry_by_name(user_name);
	if(entry) {
		userlist_entry_status_change(
			user_name, entry, new_user_status, new_user_gender,
			status_text, add_to_log);
	}
}

void userlist_user_channel_join(
	const char * user_name, const char * channel,
	int add_to_log)
{
	struct userlist_entry * entry;
	ASSERT_RETURNIFFAIL(VALIDPTR(user_name) && VALIDPTR(s_userlistHash));

	entry = userlist_entry_by_name(user_name);
	if(entry && !chanlist_contains(entry->chanlist, channel)) {
		/* add channel to user's channel list */
		entry->chanlist = chanlist_add(entry->chanlist, channel);

		/* notify chatrooms */
		chatroom_channel_user_join(channel, user_name, add_to_log);
	}
}

/* userlist_user_channel_part:
 *	this will make the user leave the specified channel
 * parameters:
 *	@explicit_part - specifies whether the user left explicitly, or because
 *	of refresh timeout
 */
void userlist_user_channel_part(
	const char * user_name, const char * channel,
	int explicit_part)
{
	struct userlist_entry * entry;

	ASSERT_RETURNIFFAIL(VALIDPTR(user_name) && VALIDPTR(s_userlistHash));
	ASSERT_RETURNIFFAIL(VALIDPTR(channel));

	entry = userlist_entry_by_name(user_name);
	if(entry && chanlist_contains(entry->chanlist, channel)) {
		/* remove channel from user's channel list */
		entry->chanlist = chanlist_remove(entry->chanlist, channel);

		/* notify chatrooms */
		chatroom_channel_user_part(channel, user_name, explicit_part);
	}
}

/* userlist_user_channel_text:
 *	writes text to specified channel,
 *	and registers user and joins the channel, if not already
 */
void userlist_user_channel_text(
	const char * user_name, const char * channel, const char * text,
	vqp_addr_t src_addr, int is_action_text)
{
	struct userlist_entry * entry;

	entry = userlist_entry_by_name(user_name);
	if(!entry) {
		vqp_uuid_t nil_uuid;
		vqp_uuid_create_nil(&nil_uuid);

		/* add the user to our user list */
		userlist_add(
			user_name, VQP_STATUS_AVAILABLE, VQP_GENDER_MALE, &nil_uuid,
			VQCHAT_VQP_COMPAT_CODEPAGE, VQCHAT_UNDEF_SWVERSION, src_addr,
			0);
	}

	entry = userlist_entry_by_name(user_name);
	if(entry) {
		/* join the channel first and
		 * write the text to the channel
		 */
		userlist_user_channel_join(user_name, channel, 0);
		chatroom_channel_user_text(channel, user_name, text, is_action_text);
	}
}

static int userlist_user_chanlist_update_join_enum_fn(
	const char * channel, void * user_name)
{
	userlist_user_channel_join((const char *)user_name, channel, 0);
	return 1;
}

static int userlist_user_chanlist_update_part_enum_fn(
	const char * channel, void * updated_chanlist)
{
	const char ** enum_fn_data = (const char **)updated_chanlist;

	if(!chanlist_contains(enum_fn_data[0], channel))
		userlist_user_channel_part(enum_fn_data[1], channel, 0);

	return 1;
}

void userlist_user_chanlist_update(
	const char * user_name,
	const char * updated_chanlist)
{
	struct userlist_entry * entry;

	ASSERT_RETURNIFFAIL(VALIDPTR(user_name) && VALIDPTR(s_userlistHash));
	entry = userlist_entry_by_name(user_name);
	if(entry) {
		const char * enum_fn_data[2];
		char * new_chanlist = chanlist_copy(updated_chanlist);

		/* join new previously-missing channels */
		chanlist_enum(
			new_chanlist,
			userlist_user_chanlist_update_join_enum_fn, (void*)user_name);

		/* leave the channels that the user is off */
		enum_fn_data[0] = new_chanlist;
		enum_fn_data[1] = user_name;
		chanlist_enum(
			entry->chanlist,
			userlist_user_chanlist_update_part_enum_fn, (void*)enum_fn_data);

		/* free chanlists */
		chanlist_free(new_chanlist);
	}
}

int userlist_user_exists(const char * user_name)
{
	ASSERT_RETURNVALIFFAIL(
		VALIDPTR(s_userlistHash) && VALIDPTR(user_name), 0);
	return userlist_entry_by_name(user_name) != NULL;
}

/* userlist_user_status:
 *	returns miranda's status for user, or ID_STATUS_OFFLINE, if not found
 */
int userlist_user_status(const char * user_name)
{
	struct userlist_entry * entry;

	ASSERT_RETURNVALIFFAIL(VALIDPTR(user_name) && VALIDPTR(s_userlistHash), ID_STATUS_OFFLINE);

	entry = userlist_entry_by_name(user_name);
	return entry ? user_status_by_vqp_status(entry->vqp_status): ID_STATUS_OFFLINE;
}

/* userlist_user_addr:
 *	returns user's address for user, or 0, if not found
 */
vqp_addr_t
userlist_user_addr(const char * user_name)
{
	struct userlist_entry * entry;
	vqp_addr_t addr;

	entry = userlist_entry_by_name(user_name);
	if(entry)
		return entry->addr;

	memset(&addr, 0, sizeof(addr));
	return addr;
}

/* userlist_user_chanlist:
 *	returns users' chanlist
 */
const char *
userlist_user_chanlist(const char * user_name)
{
	struct userlist_entry * entry;
	ASSERT_RETURNVALIFFAIL(VALIDPTR(user_name) && VALIDPTR(s_userlistHash), 0);

	entry = userlist_entry_by_name(user_name);
	return entry ? entry->chanlist: NULL;

}

/* userlist_user_gender:
 *	returns user's gender
 */
enum vqp_gender userlist_user_gender(const char * user_name)
{
	struct userlist_entry * entry;
	ASSERT_RETURNVALIFFAIL(VALIDPTR(user_name) && VALIDPTR(s_userlistHash), 0);
	entry = userlist_entry_by_name(user_name);
	return entry ? entry->gender: VQP_GENDER_MALE;
}

/* userlist_user_swversion:
 *	returns' user's software version
 */
unsigned int userlist_user_swversion(const char * user_name)
{
	struct userlist_entry * entry;
	ASSERT_RETURNVALIFFAIL(VALIDPTR(user_name) && VALIDPTR(s_userlistHash), 0);
	entry = userlist_entry_by_name(user_name);
	return entry ? entry->swversion: VQCHAT_UNDEF_SWVERSION;
}

/* userlist_user_is_chat_open:
 *	returns if we have a private chat opened with the user
 */
int userlist_user_is_chat_open(const char * user_name)
{
	struct userlist_entry * entry;
	ASSERT_RETURNVALIFFAIL(VALIDPTR(user_name) && VALIDPTR(s_userlistHash), 0);
	entry = userlist_entry_by_name(user_name);
	return entry ? entry->chat_open: 0;
}

/* userlist_user_set_chat_open:
 *	sets wherether the chat is open with the user
 */
void userlist_user_set_chat_open(const char * user_name, int set_chat_open)
{
	struct userlist_entry * entry;
	ASSERT_RETURNIFFAIL(VALIDPTR(user_name) && VALIDPTR(s_userlistHash));

	entry = userlist_entry_by_name(user_name);
	if(entry)
		entry->chat_open = set_chat_open;
}

/* userlist_user_codepage:
 *	returns codepage for specified user
 *	(and guesses it, if the user was not found)
 */
enum vqp_codepage
userlist_user_codepage(const char * user_name)
{
	struct userlist_entry * entry;
	ASSERT_RETURNVALIFFAIL(VALIDPTR(user_name), VQP_CODEPAGE_LOCALE);

	entry = userlist_entry_by_name(user_name);

	return entry ? entry->codepage: VQCHAT_VQP_COMPAT_CODEPAGE;
}

/* userlist_user_codepage_vqp:
 *	tries to get or guess codepage for specified user name
 *	(on vypresschat it tries UTF8 encoding first, on qChat - locale)
 */
enum vqp_codepage
userlist_user_codepage_vqp(const char * vqp_user_name)
{
	struct userlist_entry * entry;
	ASSERT_RETURNVALIFFAIL(VALIDPTR(vqp_user_name), VQCHAT_VQP_COMPAT_CODEPAGE);

	entry = userlist_entry_by_name_vqp(vqp_user_name);
	return entry ? entry->codepage: VQCHAT_VQP_COMPAT_CODEPAGE;
}

void userlist_user_set_codepage(const char * user_name, enum vqp_codepage codepage)
{
	struct userlist_entry * entry;
	ASSERT_RETURNIFFAIL(VALIDPTR(user_name));

	entry = userlist_entry_by_name(user_name);
	if(entry)
		entry->codepage = codepage;
}

