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
 * $Id: message.h,v 1.12 2005/03/08 16:53:22 bobas Exp $
 */

#ifndef __MESSAGE_H
#define __MESSAGE_H

#include "link.h"

/* message defaults */
#define VQP_COMPAT_ACTIVE	VQP_ACTIVE_ACTIVE
#define VQP_COMPAT_GENDER	VQP_GENDER_MALE
#define VQP_COMPAT_CODEPAGE	VQP_CODEPAGE_LOCALE
#define VQP_COMPAT_SWVERSION	0
#define VQP_COMPAT_PREF_COLOR	0
#define VQP_COMPAT_PACKET_SIG	""
#define VQP_COMPAT_SWNAME_QC	"QuickChat 1.5 (or compatible)"
#define VQP_COMPAT_SWNAME_VC	"VypressChat 1.5 (or compatible)"
#define VQP_COMPAT_NETGROUP	""
#ifdef _WIN32
# define VQP_COMPAT_PLATFORM	"Windows"
#else
# define VQP_COMPAT_PLATFORM	"Unix"
#endif

/* message structs */

struct vqp_message_struct {
	struct vqp_link_struct * link;

	char sig[VQP_LINK_SIG_LEN + 1];	/* packet signature */
	size_t content_len;		/* message length */
	char * content;			/* message contents */
	vqp_addr_t src_addr;		/* packet source address */
	vqp_addr_t dst_addr;		/* packet destination address (can be not specified) */
};
#define P_VQP_MESSAGE_STRUCT(p) ((struct vqp_message_struct *)p)

#endif	/* __MESSAGE_H */

