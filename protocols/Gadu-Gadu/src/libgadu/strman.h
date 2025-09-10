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

/**
 * \file strman.h
 *
 * \brief Makra zapewniające kompatybilność API do obsługi operacji na stringach na różnych systemach
 */

#ifndef LIBGADU_STRMAN_H
#define LIBGADU_STRMAN_H

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#ifndef _MSC_VER
#  include <strings.h>
#else
#  define snprintf(str, size, format, ...) _snprintf_s(str, size, _TRUNCATE, format, __VA_ARGS__)
#  define vsnprintf(str, size, format, ap) vsnprintf_s(str, size, _TRUNCATE, format, ap)
#  define strdup _strdup
#  define strcasecmp _stricmp
#  define strncasecmp _strnicmp
#endif

#endif /* LIBGADU_STRMAN_H */
