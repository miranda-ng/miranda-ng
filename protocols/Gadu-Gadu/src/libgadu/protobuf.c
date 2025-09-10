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

/**
 * \file protobuf.c
 *
 * \brief Funkcje pomocnicze do obsługi formatu protocol buffers
 */

#include "internal.h"

#include "protobuf.h"

#define GG_PROTOBUFF_UIN_MAXLEN 15
struct _gg_protobuf_uin_buff
{
	char data[GG_PROTOBUFF_UIN_MAXLEN + 1 + 2];
};

void gg_protobuf_expected(struct gg_session *gs, const char *field_name,
	uint32_t value, uint32_t expected)
{
	if (value == expected) {
		return;
	}
	gg_debug_session(gs, GG_DEBUG_WARNING, "// gg_packet: field %s was "
		"expected to be %#x, but its value was %#x\n",
		field_name, expected, value);
}

int gg_protobuf_valid_chknull(struct gg_session *gs, const char *msg_name,
	int isNull)
{
	if (isNull) {
		gg_debug_session(gs, GG_DEBUG_ERROR, "// gg_protobuf: couldn't "
			"unpack %s message\n", msg_name);
	}
	return !isNull;
}

int gg_protobuf_valid_chkunknown(struct gg_session *gs, const char *msg_name,
	ProtobufCMessage *base)
{
	if (base->n_unknown_fields > 0) {
		gg_debug_session(gs, GG_DEBUG_WARNING, "// gg_protobuf: message"
		" %s had %d unknown field(s)\n",
		msg_name, base->n_unknown_fields);
	}
	return 1;
}

int gg_protobuf_send_ex(struct gg_session *gs, struct gg_event *ge, int type,
	void *msg, gg_protobuf_size_cb_t size_cb,
	gg_protobuf_pack_cb_t pack_cb)
{
	void *buffer;
	size_t len;
	int succ = 1;
	enum gg_failure_t failure;

	len = size_cb(msg);
	buffer = malloc(len);
	if (buffer == NULL) {
		gg_debug_session(gs, GG_DEBUG_ERROR, "// gg_protobuf_send: out "
			"of memory - tried to allocate %" GG_SIZE_FMT
			" bytes for %#x packet\n", len, type);
		succ = 0;
		failure = GG_FAILURE_INTERNAL;
	} else {
		pack_cb(msg, buffer);
		succ = (-1 != gg_send_packet(gs, type, buffer, len, NULL));
		free(buffer);
		buffer = NULL;
		if (!succ) {
			failure = GG_FAILURE_WRITING;
			gg_debug_session(gs, GG_DEBUG_ERROR,
				"// gg_protobuf_send: sending packet %#x "
				"failed. (errno=%d, %s)\n", type, errno,
				strerror(errno));
		}
	}

	if (!succ) {
		gg_connection_failure(gs, ge, failure);
	}

	return succ;
}

void gg_protobuf_set_uin(ProtobufCBinaryData *dst, uin_t uin, gg_protobuf_uin_buff_t *buff)
{
	char *uin_str;
	int uin_len;
	static gg_protobuf_uin_buff_t static_buffer;

	if (buff == NULL) {
		buff = &static_buffer;
	}

	uin_str = buff->data + 2;
	uin_len = snprintf(uin_str, GG_PROTOBUFF_UIN_MAXLEN + 1, "%u", uin);

	buff->data[0] = 0x01; /* magic: 0x00 lub 0x01 */
	buff->data[1] = uin_len;

	dst->len = uin_len + 2;
	dst->data = (uint8_t*)&buff->data;
}

uin_t gg_protobuf_get_uin(ProtobufCBinaryData uin_data)
{
	uint8_t magic;
	size_t uin_len;
	const char *uin_str;
	uin_t uin;

	magic = (uin_data.len > 0) ? uin_data.data[0] : 0;
	uin_len = (uin_data.len > 1) ? uin_data.data[1] : 0;

	if (uin_data.len != uin_len + 2 || uin_len > 10) {
		gg_debug(GG_DEBUG_ERROR, "// gg_protobuf_get_uin: "
			"invalid length\n");
		return 0;
	}
	if (magic != 0) {
		gg_debug(GG_DEBUG_WARNING, "// gg_protobuf_get_uin: "
			"unexpected magic value=%#x\n", magic);
	}

	uin_str = (char*)(uin_data.data + 2);
	uin = gg_str_to_uin(uin_str, uin_len);

	if (uin == 0) {
		gg_debug(GG_DEBUG_ERROR, "// gg_protobuf_get_uin: "
			"invalid uin\n");
	}
	return uin;
}
