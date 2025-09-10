/*
 *  (C) Copyright 2012 Tomek Wasilczyk <www.wasilczyk.pl>
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

#ifndef LIBGADU_TVBUFF_H
#define LIBGADU_TVBUFF_H

#include "libgadu.h"

typedef struct gg_tvbuff gg_tvbuff_t;

gg_tvbuff_t * gg_tvbuff_new(const char *buffer, size_t length);
int gg_tvbuff_close(gg_tvbuff_t *tvb);

int gg_tvbuff_is_valid(const gg_tvbuff_t *tvb);

size_t gg_tvbuff_get_remaining(const gg_tvbuff_t *tvb);

int gg_tvbuff_have_remaining(gg_tvbuff_t *tvb, size_t length);

void gg_tvbuff_skip(gg_tvbuff_t *tvb, size_t amount);
void gg_tvbuff_rewind(gg_tvbuff_t *tvb, size_t amount);

int gg_tvbuff_match(gg_tvbuff_t *tvb, uint8_t value);

uint8_t gg_tvbuff_read_uint8(gg_tvbuff_t *tvb);
uint32_t gg_tvbuff_read_uint32(gg_tvbuff_t *tvb);
uint64_t gg_tvbuff_read_uint64(gg_tvbuff_t *tvb);

uint64_t gg_tvbuff_read_packed_uint(gg_tvbuff_t *tvb);

const char * gg_tvbuff_read_buff(gg_tvbuff_t *tvb, size_t length);
void gg_tvbuff_read_buff_cpy(gg_tvbuff_t *tvb, char *buffer, size_t length);
const char * gg_tvbuff_read_str(gg_tvbuff_t *tvb, size_t *length);
void gg_tvbuff_read_str_dup(gg_tvbuff_t *tvb, char **dst);

uin_t gg_tvbuff_read_uin(gg_tvbuff_t *tvb);

void gg_tvbuff_expected_uint8(gg_tvbuff_t *tvb, uint8_t value);
void gg_tvbuff_expected_uint32(gg_tvbuff_t *tvb, uint32_t value);
void gg_tvbuff_expected_eob(const gg_tvbuff_t *tvb);

#endif /* LIBGADU_TVBUFF_H */
