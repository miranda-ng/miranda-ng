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
 * $Id: chatroom.h,v 1.13 2005/03/13 02:31:09 bobas Exp $
 */


#ifndef __CHATROOM_H
#define __CHATROOM_H

void chatroom_init();
void chatroom_uninit();
void chatroom_hook_modules_loaded();
int chatroom_module_installed();

void chatroom_connected(vqp_link_t link);
void chatroom_disconnected();

int chatroom_is_valid_channel(const char * channel);

void chatroom_user_name_change(
	const char * user_name, const char * new_user_name);
void chatroom_user_status_change(int new_status);

void chatroom_channel_join(const char * channel, int connected_event);
void chatroom_channel_part(const char * channel, int disconnected_event);
void chatroom_channel_topic_change(
	const char * channel, const char * new_topic, int notify_network,
	int add_to_log);
void chatroom_channel_show_settings_dlg(const char * channel);

const char * chatroom_channel_get_topic(const char * channel);

void chatroom_channel_user_join(const char * channel, const char * user_name, int explicit_join);
void chatroom_channel_user_part(const char * channel, const char * user_name, int explicit_part);
void chatroom_channel_user_name_change(
	const char * user_name, const char * new_user_name,
	int add_to_log);
void chatroom_channel_user_status_change(
	const char * user_name, int new_status, const char * new_status_text,
	int add_to_log);

void chatroom_channel_user_text(
	const char * channel, const char * user_name,
	const char * text, int action_text);

#endif /* #ifndef __CHATROOM_H */

