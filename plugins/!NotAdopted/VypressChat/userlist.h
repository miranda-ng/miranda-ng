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
 * $Id: userlist.h,v 1.16 2005/03/17 11:02:43 bobas Exp $
 */

#ifndef __USERLIST_H
#define __USERLIST_h

#include "libvqproto/vqproto.h"

void userlist_init();
void userlist_uninit();

void userlist_flush();

void userlist_connected(vqp_link_t vqpLink);
void userlist_disconnected();

void userlist_add(
	const char * name, enum vqp_status vqp_status, enum vqp_gender gender,
	const vqp_uuid_t * p_uuid, enum vqp_codepage codepage, unsigned int swversion,
	vqp_addr_t addr, int add_to_log);
void userlist_remove(const char * user_name, int add_to_log);
typedef int (*userlist_enum_fn)(const char * user_name, void * enum_fn_data);
void userlist_enum(userlist_enum_fn enum_fn, void * enum_fn_data);

void userlist_name_change(const char * user_name, const char * new_user_name);
void userlist_status_change(
	const char * user_name,
	enum vqp_status new_user_status, enum vqp_gender new_user_gender,
	const char * status_text, int add_to_log);

void userlist_user_channel_join(const char * user_name, const char * channel, int explicit_join);
void userlist_user_channel_part(const char * user_name, const char * channel, int explicit_part);
void userlist_user_channel_text(
	const char * user_name, const char * channel, const char * text,
	vqp_addr_t addr, int is_action_text);
void userlist_user_chanlist_update(const char * user_name, const char * chanlist);

int userlist_user_exists(const char * user_name);
int userlist_user_status(const char * user_name);
#define userlist_user_can_receive_msg(user_name) (userlist_user_status(user_name)!=ID_STATUS_NA)

vqp_addr_t userlist_user_addr(const char * user_name);
const char * userlist_user_chanlist(const char * user_name);
enum vqp_gender userlist_user_gender(const char * user_name);
unsigned int userlist_user_swversion(const char * user_name);

void userlist_user_set_chat_open(const char * user_name, int chat_open);
int userlist_user_is_chat_open(const char * user_name);

enum vqp_codepage userlist_user_codepage(const char * user_name);
enum vqp_codepage userlist_user_codepage_vqp(const char * vqp_user_name);
void userlist_user_set_codepage(const char * user_name, enum vqp_codepage codepage);

#endif	/* #ifndef __USERLIST_H */

