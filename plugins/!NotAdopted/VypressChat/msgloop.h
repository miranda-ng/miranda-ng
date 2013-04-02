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
 * $Id: msgloop.h,v 1.10 2005/03/08 17:42:26 bobas Exp $
 */


#ifndef __MSGLOOP_H
#define __MSGLOOP_H

#include "miranda.h"
#include "libvqproto/vqproto.h"

void msgloop_start(vqp_link_t link, PAPCFUNC lpfMsgHandlerApc);
void msgloop_stop();
void msgloop_send(vqp_msg_t msg, int never_wait);
void msgloop_send_to(vqp_msg_t msg, int never_wait, vqp_addr_t addr);

#endif /* #ifndef __MSGLOOP_H */

