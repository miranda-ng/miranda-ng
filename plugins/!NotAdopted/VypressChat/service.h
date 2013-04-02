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
 * $Id: service.h,v 1.8 2005/03/08 16:53:20 bobas Exp $
 */


#ifndef __SERVICE_H
#define __SERVICE_H

void service_register_services();
void service_hook_all();
void service_uninit();

void service_connected(vqp_link_t link);
void service_disconnected();

void service_join_channel_merge_chanlist(const char * chanlist);

#endif	/* #ifndef __SERVICE_H */

