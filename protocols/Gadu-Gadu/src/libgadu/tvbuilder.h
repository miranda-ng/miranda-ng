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

#ifndef LIBGADU_TVBUILDER_H
#define LIBGADU_TVBUILDER_H

#include "libgadu.h"

typedef struct gg_tvbuilder gg_tvbuilder_t;

gg_tvbuilder_t * gg_tvbuilder_new(struct gg_session *gs, struct gg_event *ge);
void gg_tvbuilder_free(gg_tvbuilder_t *tvb);
void gg_tvbuilder_fail(gg_tvbuilder_t *tvb, enum gg_failure_t failure);
int gg_tvbuilder_send(gg_tvbuilder_t *tvb, int type);

int gg_tvbuilder_is_valid(const gg_tvbuilder_t *tvb);

size_t gg_tvbuilder_get_size(const gg_tvbuilder_t *tvb);
void gg_tvbuilder_expected_size(gg_tvbuilder_t *tvb, size_t length);
void gg_tvbuilder_strip(gg_tvbuilder_t *tvb, size_t length);

void gg_tvbuilder_write_uint8(gg_tvbuilder_t *tvb, uint8_t value);
void gg_tvbuilder_write_uint32(gg_tvbuilder_t *tvb, uint32_t value);
void gg_tvbuilder_write_uint64(gg_tvbuilder_t *tvb, uint64_t value);

void gg_tvbuilder_write_packed_uint(gg_tvbuilder_t *tvb, uint64_t value);

void gg_tvbuilder_write_buff(gg_tvbuilder_t *tvb, const char *buffer, size_t length);
void gg_tvbuilder_write_str(gg_tvbuilder_t *tvb, const char *buffer, ssize_t length);

void gg_tvbuilder_write_uin(gg_tvbuilder_t *tvb, uin_t uin);

#endif /* LIBGADU_TVBUILDER_H */
