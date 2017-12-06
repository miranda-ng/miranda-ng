/* coding: UTF-8 */
/* $Id$ */

/*
 *  (C) Copyright 2009 Jakub Zawadzki <darkjames@darkjames.ath.cx>
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

#ifndef LIBGADU_INTERNAL_H
#define LIBGADU_INTERNAL_H

#include "libgadu.h"

struct gg_dcc7_relay {
	uint32_t addr;
	uint16_t port;
	uint8_t family;
};

typedef struct gg_dcc7_relay gg_dcc7_relay_t;

char *gg_cp_to_utf8(const char *b);
char *gg_utf8_to_cp(const char *b);
int gg_pubdir50_handle_reply_sess(struct gg_session *sess, struct gg_event *e, const char *packet, int length);
void gg_debug_dump_session(struct gg_session *sess, const void *buf, unsigned int buf_length, const char *format, ...);

int gg_resolve(int *fd, int *pid, const char *hostname);
int gg_resolve_pthread(int *fd, void **resolver, const char *hostname);
void gg_resolve_pthread_cleanup(void *resolver, int kill);

#ifdef GG_CONFIG_HAVE_UINT64_T
uint64_t gg_fix64(uint64_t x);
#endif

#endif /* LIBGADU_INTERNAL_H */
