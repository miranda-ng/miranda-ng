/*
 *  (C) Copyright 2008 Wojtek Kaniewski <wojtekka@irc.pl>
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

#ifndef LIBGADU_RESOLVER_H
#define LIBGADU_RESOLVER_H

#include "network.h"

int gg_gethostbyname_real(const char *hostname, struct in_addr **result, unsigned int *count, int pthread);
int gg_resolver_recv(int fd, void *buf, size_t len);
void gg_resolver_cleaner(void *data);

#endif /* LIBGADU_RESOLVER_H */
