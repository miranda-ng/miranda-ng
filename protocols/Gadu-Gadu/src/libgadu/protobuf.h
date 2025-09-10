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

#include "internal.h"

#ifndef LIBGADU_PROTOBUF_H
#define LIBGADU_PROTOBUF_H

#include <stdlib.h>
#include <errno.h>
#include <string.h>

#ifdef GG_CONFIG_HAVE_PROTOBUF_C
#include <protobuf-c/protobuf-c.h>
#else
#include "protobuf-c.h"
#endif

#include "fileio.h"

typedef size_t (*gg_protobuf_size_cb_t)(const void *message);
typedef size_t (*gg_protobuf_pack_cb_t)(const void *message, uint8_t *out);

typedef struct _gg_protobuf_uin_buff gg_protobuf_uin_buff_t;

/* Ostatni warunek (msg != NULL) jest tylko po to, żeby uciszyć analizę
 * statyczną (zawiera się w pierwszym). */
#define GG_PROTOBUF_VALID(gs, name, msg) \
	(gg_protobuf_valid_chknull(gs, name, msg == NULL) && \
	gg_protobuf_valid_chkunknown(gs, name, &msg->base) && \
	msg != NULL)

#define GG_PROTOBUF_SEND(gs, ge, packet_type, msg_type, msg) \
	gg_protobuf_send_ex(gs, ge, packet_type, &msg, \
		(gg_protobuf_size_cb_t) msg_type ## __get_packed_size, \
		(gg_protobuf_pack_cb_t) msg_type ## __pack)

void gg_protobuf_expected(struct gg_session *gs, const char *field_name,
	uint32_t value, uint32_t expected);

int gg_protobuf_valid_chknull(struct gg_session *gs, const char *msg_name,
	int isNull);
int gg_protobuf_valid_chkunknown(struct gg_session *gs, const char *msg_name,
	ProtobufCMessage *base);

int gg_protobuf_send_ex(struct gg_session *gs, struct gg_event *ge, int type,
	void *msg, gg_protobuf_size_cb_t size_cb,
	gg_protobuf_pack_cb_t pack_cb);

void gg_protobuf_set_uin(ProtobufCBinaryData *dst, uin_t uin, gg_protobuf_uin_buff_t *buff);
uin_t gg_protobuf_get_uin(ProtobufCBinaryData uin_data);

#endif /* LIBGADU_PROTOBUF_H */
