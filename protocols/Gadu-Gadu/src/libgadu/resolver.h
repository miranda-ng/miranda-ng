/* coding: UTF-8 */
/* $Id$ */

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

#ifdef _WIN32
#include "win32.h"
#else
#include <arpa/inet.h>
#endif /* _WIN32 */

int gg_gethostbyname_real(const char *hostname, struct in_addr *result, int pthread);

#endif /* LIBGADU_RESOLVER_H */
