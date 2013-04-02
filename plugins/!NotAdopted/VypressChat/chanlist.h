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
 * $Id: chanlist.h,v 1.5 2005/02/26 20:23:17 bobas Exp $
 */


#ifndef __CHANLIST_H
#define __CHANLIST_H

#define chanlist_free(chanlist) if(chanlist) free(chanlist);
int chanlist_is_valid(const char * chanlist, int no_empty_channels);
#define chanlist_copy(chanlist) ((chanlist) ? strdup(chanlist): NULL)

char * chanlist_add(char * chanlist, const char * channel);
char * chanlist_remove(char * chanlist, const char * channel);
char * chanlist_shift(char ** p_chanlist);
char * chanlist_merge(char * chanlist_dst, const char * chanlist_src);
int chanlist_contains(const char * chanlist, const char * channel);

typedef int (*chanlist_enum_fn)(const char * channel, void * enum_data);
void chanlist_enum(const char * chanlist, chanlist_enum_fn enum_fn, void * enum_data);

char * chanlist_make_vqp_chanlist(const char * chanlist);
char * chanlist_parse_vqp_chanlist(const char * vqp_chanlist);

#endif

