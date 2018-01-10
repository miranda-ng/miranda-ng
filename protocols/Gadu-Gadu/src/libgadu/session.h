/*
 *  (C) Copyright 2008-2010 Wojtek Kaniewski <wojtekka@irc.pl>
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

#ifndef LIBGADU_SESSION_H
#define LIBGADU_SESSION_H

#ifdef GG_CONFIG_HAVE_GNUTLS
#  include <gnutls/gnutls.h>
#endif

#define GG_SESSION_CHECK(gs, result) \
	do { \
		if ((gs) == NULL) { \
			errno = EINVAL; \
			return (result); \
		} \
	} while (0)

#define GG_SESSION_CHECK_CONNECTED(gs, result) \
	do { \
		GG_SESSION_CHECK(gs, result); \
		\
		if (!GG_SESSION_IS_CONNECTED(gs)) { \
			errno = ENOTCONN; \
			return (result); \
		} \
	} while (0)

#define GG_SESSION_IS_PROTOCOL_7_7(gs) ((gs)->protocol_version >= 0x2a)
#define GG_SESSION_IS_PROTOCOL_8_0(gs) ((gs)->protocol_version >= 0x2d)

#define GG_SESSION_IS_IDLE(gs) ((gs)->state == GG_STATE_IDLE)
#define GG_SESSION_IS_CONNECTING(gs) ((gs)->state != GG_STATE_IDLE && (gs)->state != GG_STATE_CONNECTED)
#define GG_SESSION_IS_CONNECTED(gs) ((gs)->state == GG_STATE_CONNECTED)

#ifdef GG_CONFIG_HAVE_GNUTLS

typedef struct {
	gnutls_session_t session;
	gnutls_certificate_credentials_t xcred;
} gg_session_gnutls_t;

#define GG_SESSION_GNUTLS(gs) ((gg_session_gnutls_t*) (gs)->ssl)->session

#endif /* GG_CONFIG_HAVE_GNUTLS */

#ifdef GG_CONFIG_HAVE_OPENSSL

#define GG_SESSION_OPENSSL(gs) ((SSL*) (gs)->ssl)

#endif /* GG_CONFIG_HAVE_OPENSSL */

int gg_session_handle_packet(struct gg_session *gs, uint32_t type, const char *ptr, size_t len, struct gg_event *ge);

#endif /* LIBGADU_SESSION_H */
