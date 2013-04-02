/*
 * libvqproto: Vypress/QChat protocol interface library
 * (c) Saulius Menkevicius 2005
 *
 *   This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: link.h,v 1.11 2005/03/08 17:21:36 bobas Exp $
 */

#ifndef __LINK_H
#define __LINK_H

/* header stuff */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
/* Win32 system */
# include <windows.h>
# include <winsock2.h>
# include <ws2tcpip.h>
# include <wsipx.h>
#else
/* UNIX system */
# include <sys/types.h>
# include <netinet/in.h>
# include <sys/socket.h>
# include <sys/time.h>
# include <unistd.h>
# include <netdb.h>
#endif

/* windows compatibility hacks
 */
#ifdef WIN32
#define ENETUNREACH	ENOENT
#define ENOMSG		ENOENT
#else
/* unix system */
#define closesocket	close
#endif

/* link structure
 */
#define VQP_LINK_SEEN_SIGS_NUM	20
#define VQP_LINK_SIG_LEN	9

struct vqp_link_struct {
	/* link info */
	enum vqp_protocol_type protocol;
	enum vqp_protocol_options options;
	enum vqp_protocol_connection connection;
	unsigned short port;

	/* connection-specific data */
	union vqp_link_conndata_union {
		struct vqp_link_conndata_udp_struct {
			unsigned long local_address;
			unsigned long * p_broadcast_addresses;
			unsigned long multicast_address;
		} udp;

		struct vqp_link_conndata_ipx_struct {
			unsigned char netnum[4];
			unsigned char nodenum[6];
		} ipx;
	} conndata;
	

	/* link sockets */
	int rx_socket, tx_socket;

	/* keeps track packet sigs already seen */
	unsigned int seen_sig_inc;
	char seen_sigs[VQP_LINK_SEEN_SIGS_NUM][VQP_LINK_SIG_LEN];
};
#define P_VQP_LINK_STRUCT(p) ((struct vqp_link_struct *) p)

/* global data */

#ifdef MEMWATCH
# include "../contrib/memwatch.h"
# define vqp_mmi_malloc(s) malloc(s)
# define vqp_mmi_free(p) free(p)
#else
extern void * (* vqp_mmi_malloc)(size_t);
extern void (* vqp_mmi_free)(void *);
#endif

#endif	/* __LINK_H */

