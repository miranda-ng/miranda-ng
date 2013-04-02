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
 * $Id: user.h,v 1.10 2005/03/09 22:56:21 bobas Exp $
 */

#ifndef __USER_H
#define __USER_H

#include "libvqproto/vqproto.h"

void user_init();
void user_uninit();

void user_hook_modules_loaded();

enum vqp_status user_vqp_status_by_status(int status);
int user_status_by_vqp_status(enum vqp_status user_status);
const char * user_status_name(int status);

/* user settings
 */

/* nickname */
int user_set_nickname(const char * new_nickname, int store_in_db);
const char * user_nickname();
int user_is_my_nickname(const char *);

/* status */
int user_set_status(int);
int user_status();
#define user_offline() (user_status()==ID_STATUS_OFFLINE)
#define user_vqp_status() user_vqp_status_by_status(user_status())

/* gender */
enum vqp_gender user_gender();
void user_set_gender(enum vqp_gender new_gender, int store_in_db);

/* guid */
const vqp_uuid_t * user_p_uuid();
void user_set_uuid(const vqp_uuid_t * p_new_guid, int store_in_db);

/* codepage */
enum vqp_codepage user_codepage();
void user_set_codepage(enum vqp_codepage codepage, int store_in_db);

/* awaymsg */
void user_set_awaymsg(const char * msg);
const char * user_awaymsg();

/* chanlist */
void user_set_chanlist(const char * new_chanlist, int store_in_db);
char ** user_p_chanlist();

#endif /* #ifndef __USER_H */
