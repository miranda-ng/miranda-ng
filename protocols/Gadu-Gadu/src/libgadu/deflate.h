/* $Id$ */

/*
 *  (C) Copyright 2011 Bartosz Brachaczek <b.brachaczek@gmail.com>
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

#ifndef LIBGADU_DEFLATE_H
#define LIBGADU_DEFLATE_H

#include "libgadu.h"

unsigned char *gg_deflate(const char *in, size_t *out_lenp);
char *gg_inflate(const unsigned char *in, size_t length);

#endif /* LIBGADU_DEFLATE_H */
