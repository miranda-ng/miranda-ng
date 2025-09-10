/*
 *  (C) Copyright 2001-2010 Wojtek Kaniewski <wojtekka@irc.pl>
 *                          Robert J. Woźny <speedy@ziew.org>
 *                          Arkadiusz Miśkiewicz <arekm@pld-linux.org>
 *                          Tomasz Chiliński <chilek@chilan.com>
 *                          Adam Wysocki <gophi@ekg.chmurka.net>
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
 * \file endian.c
 *
 * \brief Konwersja między różnymi kolejnościami bajtów
 */

#include "internal.h"

/**
 * \internal Zamienia kolejność bajtów w 64-bitowym słowie.
 *
 * Ze względu na little-endianowość protokołu Gadu-Gadu, na maszynach
 * big-endianowych odwraca kolejność bajtów w słowie.
 *
 * \param x Liczba do zamiany
 *
 * \return Liczba z odpowiednią kolejnością bajtów
 *
 * \ingroup helper
 */
uint64_t gg_fix64(uint64_t x)
{
#ifndef GG_CONFIG_BIGENDIAN
	return x;
#else
	return (uint64_t)
		(((x & (uint64_t) 0x00000000000000ffULL) << 56) |
		((x & (uint64_t) 0x000000000000ff00ULL) << 40) |
		((x & (uint64_t) 0x0000000000ff0000ULL) << 24) |
		((x & (uint64_t) 0x00000000ff000000ULL) << 8) |
		((x & (uint64_t) 0x000000ff00000000ULL) >> 8) |
		((x & (uint64_t) 0x0000ff0000000000ULL) >> 24) |
		((x & (uint64_t) 0x00ff000000000000ULL) >> 40) |
		((x & (uint64_t) 0xff00000000000000ULL) >> 56));
#endif
}

/**
 * \internal Zamienia kolejność bajtów w 32-bitowym słowie.
 *
 * Ze względu na little-endianowość protokołu Gadu-Gadu, na maszynach
 * big-endianowych odwraca kolejność bajtów w słowie.
 *
 * \param x Liczba do zamiany
 *
 * \return Liczba z odpowiednią kolejnością bajtów
 *
 * \ingroup helper
 */
uint32_t gg_fix32(uint32_t x)
{
#ifndef GG_CONFIG_BIGENDIAN
	return x;
#else
	return (uint32_t)
		(((x & (uint32_t) 0x000000ffU) << 24) |
		((x & (uint32_t) 0x0000ff00U) << 8) |
		((x & (uint32_t) 0x00ff0000U) >> 8) |
		((x & (uint32_t) 0xff000000U) >> 24));
#endif
}

/**
 * \internal Zamienia kolejność bajtów w 16-bitowym słowie.
 *
 * Ze względu na little-endianowość protokołu Gadu-Gadu, na maszynach
 * big-endianowych zamienia kolejność bajtów w słowie.
 *
 * \param x Liczba do zamiany
 *
 * \return Liczba z odpowiednią kolejnością bajtów
 *
 * \ingroup helper
 */
uint16_t gg_fix16(uint16_t x)
{
#ifndef GG_CONFIG_BIGENDIAN
	return x;
#else
	return (uint16_t)
		(((x & (uint16_t) 0x00ffU) << 8) |
		((x & (uint16_t) 0xff00U) >> 8));
#endif
}
