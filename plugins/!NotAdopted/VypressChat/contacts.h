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
 * $Id: contacts.h,v 1.13 2005/03/15 02:03:47 bobas Exp $
 */

#ifndef __CONTACTS_H
#define __CONTACTS_H

#include <time.h>

void contacts_connected(vqp_link_t link);
void contacts_disconnected();
void contacts_hook_modules_loaded();

void contacts_set_all_offline();

HANDLE contacts_add_contact(const char * nickname, int permanent);
HANDLE contacts_find_contact(const char * nickname);

void contacts_set_contact_status(HANDLE hContact, int new_status);
void contacts_set_contact_nickname(HANDLE hContact, const char * new_nickname);
void contacts_set_contact_addr(HANDLE hContact, vqp_addr_t addr);
void contacts_set_contact_about(HANDLE hContact, const char * about_text);
void contacts_set_contact_gender(HANDLE hContact, enum vqp_gender gender);

enum contact_property_enum {
	CONTACT_COMPUTER, CONTACT_USER, CONTACT_WORKGROUP,
	CONTACT_PLATFORM, CONTACT_SOFTWARE
};
void contacts_set_contact_property(
	HANDLE hContact, enum contact_property_enum property, const char * string);
char * contacts_get_contact_property(HANDLE hContact, enum contact_property_enum property);

int contacts_get_contact_info(HANDLE hContact, WPARAM flags);
int contacts_get_contact_status(HANDLE hContact);
char * contacts_get_nickname(HANDLE hContact);
void contacts_input_contact_message(HANDLE hContact, const char * message_text);
int contacts_send_contact_message(
	HANDLE hContact, const char * message_text, WPARAM flags, int is_queued);
void contacts_send_beep(HANDLE hContact);

int contacts_is_user_contact(HANDLE hContact);
int contacts_is_chatroom_contact(HANDLE hContact);

#endif
