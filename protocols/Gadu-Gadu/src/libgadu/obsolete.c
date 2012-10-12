/* coding: UTF-8 */
/* $Id: obsolete.c 854 2009-10-12 21:06:28Z wojtekka $ */

/*
 *  (C) Copyright 2001-2003 Wojtek Kaniewski <wojtekka@irc.pl>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License Version
 *  2.1 as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307,
 *  USA.
 */

/**
 * \file obsolete.c
 *
 * \brief Nieaktualne funkcje
 *
 * Plik zawiera definicje funkcji, które są już nieaktualne ze względu
 * na zmiany w protokole. Programy konsolidowane ze starszych wersjami
 * bibliotek powinny nadal mieć możliwość działania, mimo ograniczonej
 * funkcjonalności.
 */

/** \cond obsolete */

#include <errno.h>

#include "libgadu.h"
#include "internal.h"

struct gg_http *gg_userlist_get(uin_t uin, const char *passwd, int async)
{
	gg_debug(GG_DEBUG_MISC, "// gg_userlist_get() is obsolete. use gg_userlist_request() instead!\n");
	errno = EINVAL;
	return NULL;
}

int gg_userlist_get_watch_fd(struct gg_http *h)
{
	errno = EINVAL;
	return -1;
}

void gg_userlist_get_free(struct gg_http *h)
{

}

struct gg_http *gg_userlist_put(uin_t uin, const char *password, const char *contacts, int async)
{
	gg_debug(GG_DEBUG_MISC, "// gg_userlist_put() is obsolete. use gg_userlist_request() instead!\n");
	errno = EINVAL;
	return NULL;
}

int gg_userlist_put_watch_fd(struct gg_http *h)
{
	errno = EINVAL;
	return -1;
}

void gg_userlist_put_free(struct gg_http *h)
{

}

struct gg_http *gg_userlist_remove(uin_t uin, const char *passwd, int async)
{
	gg_debug(GG_DEBUG_MISC, "// gg_userlist_remove() is obsolete. use gg_userlist_request() instead!\n");
	errno = EINVAL;
	return NULL;
}

int gg_userlist_remove_watch_fd(struct gg_http *h)
{
	errno = EINVAL;
	return -1;
}

void gg_userlist_remove_free(struct gg_http *h)
{

}

struct gg_http *gg_search(const struct gg_search_request *r, int async)
{
	gg_debug(GG_DEBUG_MISC, "// gg_search() is obsolete. use gg_search50() instead!\n");
	errno = EINVAL;
	return NULL;
}

int gg_search_watch_fd(struct gg_http *h)
{
	errno = EINVAL;
	return -1;
}

void gg_search_free(struct gg_http *h)
{

}

const struct gg_search_request *gg_search_request_mode_0(char *nickname, char *first_name, char *last_name, char *city, int gender, int min_birth, int max_birth, int active, int start)
{
	return NULL;
}

const struct gg_search_request *gg_search_request_mode_1(char *email, int active, int start)
{
	return NULL;
}

const struct gg_search_request *gg_search_request_mode_2(char *phone, int active, int start)
{
	return NULL;
}

const struct gg_search_request *gg_search_request_mode_3(uin_t uin, int active, int start)
{
	return NULL;
}

void gg_search_request_free(struct gg_search_request *r)
{

}

struct gg_http *gg_register(const char *email, const char *password, int async)
{
	gg_debug(GG_DEBUG_MISC, "// gg_register() is obsolete. use gg_register3() instead!\n");
	errno = EINVAL;
	return NULL;
}

struct gg_http *gg_register2(const char *email, const char *password, const char *qa, int async)
{
	gg_debug(GG_DEBUG_MISC, "// gg_register2() is obsolete. use gg_register3() instead!\n");
	errno = EINVAL;
	return NULL;
}

struct gg_http *gg_unregister(uin_t uin, const char *password, const char *email, int async)
{
	gg_debug(GG_DEBUG_MISC, "// gg_unregister() is obsolete. use gg_unregister3() instead!\n");
	errno = EINVAL;
	return NULL;
}

struct gg_http *gg_unregister2(uin_t uin, const char *password, const char *qa, int async)
{
	gg_debug(GG_DEBUG_MISC, "// gg_unregister2() is obsolete. use gg_unregister3() instead!\n");
	errno = EINVAL;
	return NULL;
}


struct gg_http *gg_change_passwd(uin_t uin, const char *passwd, const char *newpasswd, const char *newemail, int async)
{
	gg_debug(GG_DEBUG_MISC, "// gg_change_passwd() is obsolete. use gg_change_passwd4() instead!\n");
	errno = EINVAL;
	return NULL;
}

struct gg_http *gg_change_passwd2(uin_t uin, const char *passwd, const char *newpasswd, const char *email, const char *newemail, int async)
{
	gg_debug(GG_DEBUG_MISC, "// gg_change_passwd2() is obsolete. use gg_change_passwd4() instead!\n");
	errno = EINVAL;
	return NULL;
}

struct gg_http *gg_change_passwd3(uin_t uin, const char *passwd, const char *newpasswd, const char *qa, int async)
{
	gg_debug(GG_DEBUG_MISC, "// gg_change_passwd3() is obsolete. use gg_change_passwd4() instead!\n");
	errno = EINVAL;
	return NULL;
}

struct gg_http *gg_remind_passwd(uin_t uin, int async)
{
	gg_debug(GG_DEBUG_MISC, "// gg_remind_passwd() is obsolete. use gg_remind_passwd3() instead!\n");
	errno = EINVAL;
	return NULL;
}

struct gg_http *gg_remind_passwd2(uin_t uin, const char *tokenid, const char *tokenval, int async)
{
	gg_debug(GG_DEBUG_MISC, "// gg_remind_passwd2() is obsolete. use gg_remind_passwd3() instead!\n");
	errno = EINVAL;
	return NULL;
}

struct gg_http *gg_change_info(uin_t uin, const char *passwd, const struct gg_change_info_request *request, int async)
{
	gg_debug(GG_DEBUG_MISC, "// gg_change_info() is obsolete. use gg_pubdir50() instead\n");
	errno = EINVAL;
	return NULL;
}

struct gg_change_info_request *gg_change_info_request_new(const char *first_name, const char *last_name, const char *nickname, const char *email, int born, int gender, const char *city)
{
	return NULL;
}

void gg_change_info_request_free(struct gg_change_info_request *r)
{

}

int gg_resolve(int *fd, int *pid, const char *hostname)
{
	return -1;
}

void gg_resolve_pthread_cleanup(void *arg, int kill)
{

}

int gg_resolve_pthread(int *fd, void **resolver, const char *hostname)
{
	return -1;
}

int gg_pubdir50_handle_reply(struct gg_event *e, const char *packet, int length)
{
	return -1;
}

/** \endcond */
