/*
 *  (C) Copyright 2008-2009 Jakub Zawadzki <darkjames@darkjames.ath.cx>
 *                          Wojtek Kaniewski <wojtekka@irc.pl>
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

#ifndef LIBGADU_ENCODING_H
#define LIBGADU_ENCODING_H

#include "libgadu.h"

char *gg_encoding_convert(const char *src, gg_encoding_t src_encoding, gg_encoding_t dst_encoding, int src_length, int dst_length);

#endif /* LIBGADU_SESSION_H */
