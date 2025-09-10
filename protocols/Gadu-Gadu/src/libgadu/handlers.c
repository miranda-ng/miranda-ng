/*
 *  (C) Copyright 2001-2011 Wojtek Kaniewski <wojtekka@irc.pl>
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
 * \file handlers.c
 *
 * \brief Funkcje obsługi przychodzących pakietów
 */

#include "internal.h"

#include <ctype.h>

#include "fileio.h"
#include "network.h"
#include "strman.h"
#include "resolver.h"
#include "session.h"
#include "protocol.h"
#include "encoding.h"
#include "message.h"
#include "deflate.h"
#include "tvbuff.h"
#include "protobuf.h"
#include "packets.pb-c.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Ograniczenie długości listy kontaktów
 * z pakietów GG_USERLIST_REPLY do 10MB. */
#define GG_USERLIST_REPLY_MAX_LENGTH 10485760

/**
 * \internal Struktura opisująca funkcję obsługi pakietu.
 */
typedef struct {
	/* Typ pakietu */
	uint32_t type;
	/* Stan w którym pakiet jest obsługiwany */
	enum gg_state_t state;
	/* Minimalny rozmiar danych pakietu */
	size_t min_length;
	/* Funkcja obsługująca pakiet. Patrz gg_session_handle_packet(). */
	int (*handler)(struct gg_session *, uint32_t, const char *, size_t, struct gg_event *);
} gg_packet_handler_t;

static int gg_ack_110(struct gg_session *gs, GG110Ack__Type type, uint32_t seq, struct gg_event *ge)
{
	GG110Ack msg = GG110_ACK__INIT;

	msg.type = type;
	msg.seq = seq;

	if (!GG_PROTOBUF_SEND(gs, ge, GG_ACK110, gg110_ack, msg))
		return -1;
	return 0;
}

static void gg_sync_time(struct gg_session *gs, time_t server_time)
{
	time_t local_time = time(NULL);
	int time_diff = server_time - local_time;

	if (gs->private_data->time_diff == time_diff)
		return;

	gs->private_data->time_diff = time_diff;
	gg_debug_session(gs, GG_DEBUG_MISC | GG_DEBUG_VERBOSE,
		"// time synchronized (diff = %d)\n", time_diff);
}

static int gg_session_handle_welcome_110(struct gg_session *gs, uint32_t seed,
	struct gg_event *ge)
{
	GG105Login msg = GG105_LOGIN__INIT;
	char client_str[1000];
	uint8_t hash[64];
	const char *client_name = GG11_VERSION;
	const char *client_version = GG_DEFAULT_CLIENT_VERSION_110;
	const char *client_target = GG11_TARGET;
	uint8_t dummy4[4] = {0, 0, 0, 0};

	if (gs->hash_type != GG_LOGIN_HASH_SHA1) {
		gg_debug_session(gs, GG_DEBUG_ERROR, "// Unsupported hash type "
			"for this protocol version\n");
		gg_connection_failure(gs, ge, GG_FAILURE_INTERNAL);
		return -1;
	}

	if (gg_login_hash_sha1_2(gs->password, seed, hash) == -1) {
		gg_debug_session(gs, GG_DEBUG_ERROR, "// gg_watch_fd() "
			"gg_login_hash_sha1_2() failed, "
			"probably out of memory\n");
		gg_connection_failure(gs, ge, GG_FAILURE_INTERNAL);
		return -1;
	}

	if (gs->client_version != NULL && !isdigit(gs->client_version[0])) {
		client_name = "";
		client_target = "";
	}
	if (gs->client_version != NULL)
		client_version = gs->client_version;
	snprintf(client_str, sizeof(client_str), "%s%s%s",
		client_name, client_version, client_target);
	client_str[sizeof(client_str) - 1] = '\0';

	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd() "
		"sending GG_LOGIN105 packet\n");

	msg.lang = GG8_LANG;
	gg_protobuf_set_uin(&msg.uin, gs->uin, NULL);
	msg.hash.len = 20;
	msg.hash.data = hash;
	msg.client = client_str;

	/* flagi gg8 są różne od tych dla gg11 */
	msg.initial_status = gs->initial_status ?
		(gs->initial_status & 0xFF) : GG_STATUS_AVAIL;

	if (gs->initial_descr != NULL) {
		msg.initial_descr = gs->initial_descr;
	}

	/* GG11.0
	msg.supported_features = "avatar,StatusComments,gg_account_sdp,"
		"edisc,bot,fanpage,pubdir,botCaps"; */
	/* GG11.2 */
	msg.supported_features = "avatar,StatusComments,ggaccount,edisc,"
		"music_shared,bot,fanpage,pubdir,botCaps,gifts,Gift";

	msg.dummy4.len = sizeof(dummy4);
	msg.dummy4.data = dummy4;

	msg.has_dummy7 = 1;
	msg.has_dummy8 = 1;
	msg.has_dummy10 = 1;

	if (!GG_PROTOBUF_SEND(gs, ge, GG_LOGIN105, gg105_login, msg))
		return -1;

	gs->state = GG_STATE_READING_REPLY;
	gs->check = GG_CHECK_READ;
	return 0;
}

static int gg_session_handle_login110_ok(struct gg_session *gs, uint32_t type,
	const char *ptr, size_t len, struct gg_event *ge)
{
	GG110LoginOK *msg = gg110_login_ok__unpack(NULL, len, (uint8_t*)ptr);

	if (!GG_PROTOBUF_VALID(gs, "GG110LoginOK", msg))
		return -1;

	gg_protobuf_expected(gs, "GG110LoginOK.dummy1", msg->dummy1, 1);
	gg_sync_time(gs, msg->server_time);

	gg_debug_session(gs, GG_DEBUG_MISC, "// login110_ok: "
		"uin=%u, dummyhash=%s\n", msg->uin, msg->dummyhash);

	gg110_login_ok__free_unpacked(msg, NULL);

	ge->type = GG_EVENT_CONN_SUCCESS;
	gs->state = GG_STATE_CONNECTED;
	gs->check = GG_CHECK_READ;
	gs->timeout = -1;
	gs->status = (gs->initial_status) ? gs->initial_status : GG_STATUS_AVAIL;
#if 0
	free(gs->status_descr);
	gs->status_descr = gs->initial_descr;
#else
	free(gs->initial_descr);
#endif
	gs->initial_descr = NULL;

	return 0;
}

/**
 * \internal Obsługuje pakiet GG_WELCOME.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_welcome(struct gg_session *gs, uint32_t type,
	const char *ptr, size_t len, struct gg_event *ge)
{
	const struct gg_welcome *w;
	int ret;
	uint8_t hash_buf[64];
	uint32_t local_ip;
	struct sockaddr_in sin;
	socklen_t sin_len = sizeof(sin);
	uint32_t seed;

	struct gg_login80 l80;
	const char *client_name, *version, *descr;
	uint32_t client_name_len, version_len, descr_len;

	if (len < sizeof(struct gg_welcome)) {
		ge->type = GG_EVENT_CONN_FAILED;
		ge->event.failure = GG_FAILURE_INVALID;
		gs->state = GG_STATE_IDLE;
		gg_close(gs);
		return 0;
	}

	w = (const struct gg_welcome*) ptr;
	seed = gg_fix32(w->key);

	if (gs->protocol_version >= GG_PROTOCOL_VERSION_110)
		return gg_session_handle_welcome_110(gs, seed, ge);

	memset(hash_buf, 0, sizeof(hash_buf));

	switch (gs->hash_type) {
		case GG_LOGIN_HASH_GG32:
		{
			uint32_t hash;

			hash = gg_fix32(gg_login_hash((unsigned char*) gs->password, seed));
			gg_debug_session(gs, GG_DEBUG_DUMP, "// gg_watch_fd() "
				"challenge %.4x --> GG32 hash %.8x\n",
				seed, hash);
			memcpy(hash_buf, &hash, sizeof(hash));

			break;
		}

		case GG_LOGIN_HASH_SHA1:
		{
#ifndef GG_DEBUG_DISABLE
			char tmp[41];
			int i;
#endif

			if (gg_login_hash_sha1_2(gs->password, seed, hash_buf) == -1) {
				gg_debug_session(gs, GG_DEBUG_MISC,
					"// gg_watch_fd() gg_login_hash_sha1_2()"
					" failed, probably out of memory\n");
				gg_close(gs);
				ge->type = GG_EVENT_CONN_FAILED;
				ge->event.failure = GG_FAILURE_INTERNAL;
				gs->state = GG_STATE_IDLE;
				return -1;
			}

#ifndef GG_DEBUG_DISABLE
			for (i = 0; i < 40; i += 2)
				snprintf(tmp + i, sizeof(tmp) - i, "%02x", hash_buf[i / 2]);

			gg_debug_session(gs, GG_DEBUG_DUMP, "// gg_watch_fd() "
				"challenge %.4x --> SHA1 hash: %s\n",
				seed, tmp);
#endif

			break;
		}

		default:
			break;
	}

#if 0
	if (gs->password != NULL && (gs->flags & (1 << GG_SESSION_FLAG_CLEAR_PASSWORD))) {
		memset(gs->password, 0, strlen(gs->password));
		free(gs->password);
		gs->password = NULL;
	}
#endif

	if (!getsockname(gs->fd, (struct sockaddr*) &sin, &sin_len)) {
		gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd() "
			"detected address to %s\n", inet_ntoa(sin.sin_addr));
		local_ip = sin.sin_addr.s_addr;
	} else {
		gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd() unable to detect address\n");
		local_ip = 0;
	}

	if (gs->external_addr == 0)
		gs->external_addr = local_ip;

	memset(&l80, 0, sizeof(l80));
	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd() sending GG_LOGIN80 packet\n");
	l80.uin = gg_fix32(gs->uin);
	memcpy(l80.language, GG8_LANG, sizeof(l80.language));
	l80.hash_type = gs->hash_type;
	memcpy(l80.hash, hash_buf, sizeof(l80.hash));
	l80.status = gg_fix32(gs->initial_status ? gs->initial_status : GG_STATUS_AVAIL);
	l80.flags = gg_fix32(gs->status_flags);
	l80.features = gg_fix32(gs->protocol_features);
	l80.image_size = gs->image_size;
	l80.dunno2 = 0x64;

	if (gs->client_version != NULL && !isdigit(gs->client_version[0])) {
		client_name = "";
		client_name_len = 0;
	} else {
		client_name = GG8_VERSION;
		client_name_len = strlen(GG8_VERSION);
	}

	version = (gs->client_version != NULL) ? gs->client_version : GG_DEFAULT_CLIENT_VERSION_100;
	version_len = gg_fix32(client_name_len + strlen(version));

	descr = (gs->initial_descr != NULL) ? gs->initial_descr : "";
	descr_len = (gs->initial_descr != NULL) ? gg_fix32(strlen(gs->initial_descr)) : 0;

	ret = gg_send_packet(gs,
			GG_LOGIN80,
			&l80, sizeof(l80),
			&version_len, sizeof(version_len),
			client_name, client_name_len,
			version, strlen(version),
			&descr_len, sizeof(descr_len),
			descr, strlen(descr),
			NULL);

	if (ret == -1) {
		gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd() "
			"sending packet failed. (errno=%d, %s)\n",
			errno, strerror(errno));
		gg_close(gs);
		ge->type = GG_EVENT_CONN_FAILED;
		ge->event.failure = GG_FAILURE_WRITING;
		gs->state = GG_STATE_IDLE;
		return -1;
	}

	gs->state = GG_STATE_READING_REPLY;
	gs->check = GG_CHECK_READ;

	return 0;
}

/**
 * \internal Obsługuje pakiet GG_LOGIN_OK.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_login_ok(struct gg_session *gs, uint32_t type,
	const char *ptr, size_t len, struct gg_event *ge)
{
	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd() login succeeded\n");
	ge->type = GG_EVENT_CONN_SUCCESS;
	gs->state = GG_STATE_CONNECTED;
	gs->check = GG_CHECK_READ;
	gs->timeout = -1;
	gs->status = (gs->initial_status) ? gs->initial_status : GG_STATUS_AVAIL;
#if 0
	free(gs->status_descr);
	gs->status_descr = gs->initial_descr;
#else
	free(gs->initial_descr);
#endif
	gs->initial_descr = NULL;

	return 0;
}

/**
 * \internal Obsługuje pakiet GG_LOGIN_FAILED.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_login_failed(struct gg_session *gs, uint32_t type,
	const char *ptr, size_t len, struct gg_event *ge)
{
	if (type != GG_DISCONNECTING)
		gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd() login failed\n");
	else
		gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd() too many incorrect password attempts\n");
	ge->type = GG_EVENT_CONN_FAILED;
	ge->event.failure = (type != GG_DISCONNECTING) ? GG_FAILURE_PASSWORD : GG_FAILURE_INTRUDER;
	gs->state = GG_STATE_IDLE;
	gg_close(gs);
	errno = EACCES;

	return 0;
}

/**
 * \internal Obsługuje pakiet GG_SEND_MSG_ACK.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_send_msg_ack(struct gg_session *gs, uint32_t type,
	const char *ptr, size_t len, struct gg_event *ge)
{
	struct gg_session_private *p = gs->private_data;
	const struct gg_send_msg_ack *s = (const struct gg_send_msg_ack*) ptr;

	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received a message ack\n");

	ge->type = GG_EVENT_ACK;
	ge->event.ack.status = gg_fix32(s->status);
	ge->event.ack.recipient = gg_fix32(s->recipient);
	ge->event.ack.seq = gg_fix32(s->seq);

	if (ge->event.ack.seq == 0 && p->imgout_waiting_ack > 0)
		p->imgout_waiting_ack--;
	gg_image_sendout(gs);

	return 0;
}

/**
 * \internal Obsługuje pakiet GG_SEND_MSG_ACK110.
 */
static int gg_session_handle_send_msg_ack_110(struct gg_session *gs,
	uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	struct gg_session_private *p = gs->private_data;
	GG110MessageAck *msg = gg110_message_ack__unpack(NULL, len, (uint8_t*)ptr);
	size_t i;

	if (!GG_PROTOBUF_VALID(gs, "GG110MessageAck", msg))
		return -1;

	if (msg->dummy1 == 0x4000) {
		/* zaobserwowane w EKG rev2856, po wywołaniu check_conn, czyli
		 * gg_image_request(sess, uin, 0, time(NULL));
		 */
		gg_debug_session(gs, GG_DEBUG_MISC | GG_DEBUG_WARNING,
			"// gg_session_handle_send_msg_ack_110() magic dummy1 "
			"value 0x4000\n");
	} else if (msg->dummy1 != 0) {
		gg_debug_session(gs, GG_DEBUG_MISC | GG_DEBUG_WARNING,
			"// gg_session_handle_send_msg_ack_110() unknown dummy1 "
			"value: %x\n", msg->dummy1);
	}

	gg_debug_session(gs, GG_DEBUG_VERBOSE,
		"// gg_session_handle_send_msg_ack_110() "
		"%s=%016" PRIx64 " %s=%016" PRIx64 "\n",
		msg->has_msg_id ? "msg_id" : "0", msg->msg_id,
		msg->has_conv_id ? "conv_id" : "0", msg->conv_id);

	for (i = 0; i < msg->n_links; i++) {
		GG110MessageAckLink *link = msg->links[i];
		if (!GG_PROTOBUF_VALID(gs, "GG110MessageAckLink", link))
			continue;
		gg_debug_session(gs, GG_DEBUG_MISC,
			"// gg_session_handle_send_msg_ack_110() "
			"got link (id=%" PRIx64 ") \"%s\"\n", link->id, link->url);
	}

	ge->type = GG_EVENT_ACK110;
	ge->event.ack110.msg_type = msg->msg_type;
	ge->event.ack110.seq = msg->seq;
	ge->event.ack110.time = msg->time;

	gg_compat_message_ack(gs, msg->seq);

	gg110_message_ack__free_unpacked(msg, NULL);

	if (msg->seq == 0 && p->imgout_waiting_ack > 0)
		p->imgout_waiting_ack--;
	gg_image_sendout(gs);

	return 0;
}

/**
 * \internal Obsługuje pakiet GG_PONG.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_pong(struct gg_session *gs, uint32_t type,
	const char *ptr, size_t len, struct gg_event *ge)
{
	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received a pong\n");

	ge->type = GG_EVENT_PONG;

	gs->last_pong = time(NULL);

	return 0;
}

/**
 * \internal Obsługuje pakiet GG_DISCONNECTING.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_disconnecting(struct gg_session *gs, uint32_t type,
	const char *ptr, size_t len, struct gg_event *ge)
{
	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received disconnection warning\n");

	ge->type = GG_EVENT_DISCONNECT;

	return 0;
}

/**
 * \internal Obsługuje pakiet GG_DISCONNECT_ACK.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_disconnect_ack(struct gg_session *gs,
	uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received logoff acknowledge\n");

	ge->type = GG_EVENT_DISCONNECT_ACK;

	return 0;
}

/**
 * \internal Obsługuje pakiety GG_XML_EVENT i GG_XML_ACTION.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_xml_event(struct gg_session *gs, uint32_t type,
	const char *ptr, size_t len, struct gg_event *ge)
{
	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received XML event\n");

	ge->type = GG_EVENT_XML_EVENT;
	ge->event.xml_event.data = malloc(len + 1);

	if (ge->event.xml_event.data == NULL) {
		gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() out of memory\n");
		return -1;
	}

	memcpy(ge->event.xml_event.data, ptr, len);
	ge->event.xml_event.data[len] = 0;

	return 0;
}

static int gg_session_handle_event_110(struct gg_session *gs, uint32_t type,
	const char *ptr, size_t len, struct gg_event *ge)
{
	GG110Event *msg = gg110_event__unpack(NULL, len, (uint8_t*)ptr);
	int succ = 1;

	if (!GG_PROTOBUF_VALID(gs, "GG110Event", msg))
		return -1;

	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_session_handle_event_110: "
		"received GG11 event (type=%d, id=%" PRIx64 ")\n", msg->type, msg->id);

	if (msg->type == GG110_EVENT__TYPE__XML) {
		ge->type = GG_EVENT_XML_EVENT;
		ge->event.xml_event.data = strdup(msg->data);
		succ = succ && (ge->event.xml_event.data != NULL);
	} else if (msg->type == GG110_EVENT__TYPE__JSON) {
		ge->type = GG_EVENT_JSON_EVENT;
		ge->event.json_event.data = strdup(msg->data);
		succ = succ && (ge->event.json_event.data != NULL);
		ge->event.json_event.type = strdup(msg->subtype);
		succ = succ && (ge->event.json_event.type != NULL);
	} else {
		gg_debug_session(gs, GG_DEBUG_WARNING,
			"// gg_session_handle_event_110: "
			"unsupported GG11 event type: %d\n", msg->type);
		succ = 0;
	}

	if (gg_ack_110(gs, GG110_ACK__TYPE__MPA, msg->seq, ge) != 0) {
		succ = 0;
	}

	gg110_event__free_unpacked(msg, NULL);

	return succ ? 0 : -1;
}

/**
 * \internal Obsługuje pakiet GG_PUBDIR50_REPLY.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_pubdir50_reply(struct gg_session *gs,
	uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received pubdir/search reply\n");

	return gg_pubdir50_handle_reply_sess(gs, ge, ptr, len);
}

/**
 * \internal Obsługuje pakiet GG_USERLIST_REPLY.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_userlist_reply(struct gg_session *gs,
	uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	char reply_type;

	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received userlist reply\n");

	reply_type = ptr[0];

	/* jeśli odpowiedź na eksport, wywołaj zdarzenie tylko
	 * gdy otrzymano wszystkie odpowiedzi */
	if (reply_type == GG_USERLIST_PUT_REPLY || reply_type == GG_USERLIST_PUT_MORE_REPLY) {
		if (--gs->userlist_blocks)
			return 0;

		reply_type = GG_USERLIST_PUT_REPLY;
	}

	if (len > 1) {
		unsigned int reply_len = (gs->userlist_reply != NULL) ? strlen(gs->userlist_reply) : 0;
		char *tmp;

		gg_debug_session(gs, GG_DEBUG_MISC, "userlist_reply=%p, len=%"
			GG_SIZE_FMT "\n", gs->userlist_reply, len);

		if (reply_len + len > GG_USERLIST_REPLY_MAX_LENGTH) {
			gg_debug_session(gs, GG_DEBUG_MISC,
				"// gg_session_handle_userlist_reply() "
				"too many userlist replies\n");
			return -1;
		}

		tmp = realloc(gs->userlist_reply, reply_len + len);

		if (tmp == NULL) {
			gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() out of memory\n");
			return -1;
		}

		gs->userlist_reply = tmp;
		memcpy(gs->userlist_reply + reply_len, ptr + 1, len - 1);
		gs->userlist_reply[reply_len + len - 1] = 0;
	}

	if (reply_type == GG_USERLIST_GET_MORE_REPLY)
		return 0;

	ge->type = GG_EVENT_USERLIST;
	ge->event.userlist.type = reply_type;
	ge->event.userlist.reply = gs->userlist_reply;

	gs->userlist_reply = NULL;

	return 0;
}

/**
 * \internal Obsługuje pakiet GG_DCC7_ID_REPLY.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_dcc7_id_reply(struct gg_session *gs, uint32_t type,
	const char *ptr, size_t len, struct gg_event *ge)
{
	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received dcc7 id packet\n");

	return gg_dcc7_handle_id(gs, ge, ptr, len);
}

/**
 * \internal Obsługuje pakiet GG_DCC7_ACCEPT.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_dcc7_accept(struct gg_session *gs, uint32_t type,
	const char *ptr, size_t len, struct gg_event *ge)
{
	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received dcc7 accept\n");

	return gg_dcc7_handle_accept(gs, ge, ptr, len);
}

/**
 * \internal Obsługuje pakiet GG_DCC7_NEW.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_dcc7_new(struct gg_session *gs, uint32_t type,
	const char *ptr, size_t len, struct gg_event *ge)
{
	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received dcc7 request\n");

	return gg_dcc7_handle_new(gs, ge, ptr, len);
}

/**
 * \internal Obsługuje pakiet GG_DCC7_REJECT.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_dcc7_reject(struct gg_session *gs, uint32_t type,
	const char *ptr, size_t len, struct gg_event *ge)
{
	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received dcc7 reject\n");

	return gg_dcc7_handle_reject(gs, ge, ptr, len);
}

/**
 * \internal Obsługuje pakiet GG_DCC7_INFO.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_dcc7_info(struct gg_session *gs, uint32_t type,
	const char *ptr, size_t len, struct gg_event *ge)
{
	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received dcc7 info\n");

	return gg_dcc7_handle_info(gs, ge, ptr, len);
}

/**
 * \internal Analizuje przychodzący pakiet z obrazkiem.
 *
 * \param e Struktura zdarzenia
 * \param p Bufor z danymi
 * \param len Długość bufora
 * \param sess Struktura sesji
 * \param sender Numer nadawcy
 * \param type Typ pakietu (NIE typ GG_MSG_OPTION_IMAGE_*)
 */
static void gg_image_queue_parse(struct gg_event *e, const char *p,
	unsigned int len, struct gg_session *sess, uin_t sender,
	uint32_t type)
{
	const struct gg_msg_image_reply *i = (const void*) p;
	struct gg_image_queue *q, *qq;

	gg_debug_session(sess, GG_DEBUG_VERBOSE,
		"// gg_image_queue_parse(%p, %p, %d, %p, %u, %d)\n",
		e, p, len, sess, sender, type);

	if (!p || !sess || !e) {
		errno = EFAULT;
		return;
	}

	if (i->flag == GG_MSG_OPTION_IMAGE_REQUEST) {
		e->type = GG_EVENT_IMAGE_REQUEST;
		e->event.image_request.sender = sender;
		e->event.image_reply.size = i->size;
		e->event.image_request.crc32 = i->crc32;
		return;
	}

	/* znajdź dany obrazek w kolejce danej sesji */

	for (qq = sess->images, q = NULL; qq; qq = qq->next) {
		if (sender == qq->sender && i->size == qq->size && i->crc32 == qq->crc32) {
			q = qq;
			break;
		}
	}

	if (!q) {
		gg_debug_session(sess, GG_DEBUG_WARNING,
			"// gg_image_queue_parse() unknown image from %d, "
			"size=%d, crc32=%.8x\n", sender, i->size, i->crc32);
		return;
	}

	if (q->packet_type == 0)
		q->packet_type = type;
	if (q->packet_type != type)
		return;

	if (i->flag == GG_MSG_OPTION_IMAGE_REPLY) {
		q->done = 0;

		len -= sizeof(struct gg_msg_image_reply);
		p += sizeof(struct gg_msg_image_reply);

		if (memchr(p, 0, len) == NULL) {
			gg_debug_session(sess, GG_DEBUG_ERROR,
				"// gg_image_queue_parse() malformed packet "
				"from %d, unlimited filename\n", sender);
			return;
		}

		if (!(q->filename = strdup(p))) {
			gg_debug_session(sess, GG_DEBUG_ERROR, "// gg_image_queue_parse() out of memory\n");
			return;
		}

		len -= strlen(p) + 1;
		p += strlen(p) + 1;
	} else if (i->flag == GG_MSG_OPTION_IMAGE_REPLY_MORE) {
		len -= sizeof(struct gg_msg_image_reply);
		p += sizeof(struct gg_msg_image_reply);
	} else {
		gg_debug_session(sess, GG_DEBUG_WARNING, "// gg_image_queue_parse() unexpected flag\n");
		return;
	}

	if (q->done + len > q->size) {
		gg_debug_session(sess, GG_DEBUG_ERROR, "// gg_image_queue_parse() got too much\n");
		len = q->size - q->done;
	}

	memcpy(q->image + q->done, p, len);
	q->done += len;

	gg_debug_session(sess, GG_DEBUG_VERBOSE,
		"// gg_image_queue_parse() got image part (done: %d of %d)\n",
		q->done, q->size);

	/* jeśli skończono odbierać obrazek, wygeneruj zdarzenie */

	if (q->done >= q->size) {
		gg_debug_session(sess, GG_DEBUG_VERBOSE,
			"// gg_image_queue_parse() image ready\n");

		e->type = GG_EVENT_IMAGE_REPLY;
		e->event.image_reply.sender = sender;
		e->event.image_reply.size = q->size;
		e->event.image_reply.crc32 = q->crc32;
		e->event.image_reply.filename = q->filename;
		e->event.image_reply.image = q->image;

		gg_image_queue_remove(sess, q, 0);

		free(q);
	}
}

/**
 * \internal Analizuje informacje rozszerzone wiadomości.
 *
 * \param sess Struktura sesji.
 * \param e Struktura zdarzenia.
 * \param sender Numer nadawcy.
 * \param p Wskaźnik na dane rozszerzone.
 * \param packet_end Wskaźnik na koniec pakietu.
 * \param packet_type Typ pakietu, w którym otrzymaliśmy wiadomość.
 *
 * \return 0 jeśli się powiodło, -1 jeśli wiadomość obsłużono i wynik ma
 * zostać przekazany aplikacji, -2 jeśli wystąpił błąd ogólny, -3 jeśli
 * wiadomość jest niepoprawna.
 */
static int gg_handle_recv_msg_options(struct gg_session *sess,
	struct gg_event *e, uin_t sender, const char *p, const char *packet_end,
	uint32_t packet_type)
{
	while (p < packet_end) {
		switch (*p) {
			case GG_MSG_OPTION_CONFERENCE:
			{
				const struct gg_msg_recipients *m = (const void*) p;
				uint32_t i, count;

				p += sizeof(*m);

				if (p > packet_end) {
					gg_debug_session(sess, GG_DEBUG_MISC,
						"// gg_handle_recv_msg_options()"
						" packet out of bounds (1)\n");
					goto malformed;
				}

				count = gg_fix32(m->count);

				if (p + count * sizeof(uin_t) > packet_end ||
					p + count * sizeof(uin_t) < p ||
					count > 0xffff)
				{
					gg_debug_session(sess, GG_DEBUG_MISC,
						"// gg_handle_recv_msg_options()"
						" packet out of bounds (1.5)\n");
					goto malformed;
				}

				if (e->event.msg.recipients != NULL) {
					gg_debug_session(sess, GG_DEBUG_MISC,
						"// gg_handle_recv_msg_options()"
						" e->event.msg.recipients already exist\n");
					goto malformed;
				}

				e->event.msg.recipients = malloc(count * sizeof(uin_t));

				if (e->event.msg.recipients == NULL) {
					gg_debug_session(sess, GG_DEBUG_MISC,
						"// gg_handle_recv_msg_options()"
						" not enough memory for recipients data\n");
					goto fail;
				}

				memcpy(e->event.msg.recipients, p, count * sizeof(uin_t));
				p += count * sizeof(uin_t);

				for (i = 0; i < count; i++)
					e->event.msg.recipients[i] = gg_fix32(e->event.msg.recipients[i]);

				e->event.msg.recipients_count = count;

				break;
			}

			case GG_MSG_OPTION_ATTRIBUTES:
			{
				uint16_t len;
				char *buf;

				if (p + 3 > packet_end) {
					gg_debug_session(sess, GG_DEBUG_MISC,
						"// gg_handle_recv_msg_options()"
						" packet out of bounds (2)\n");
					goto malformed;
				}

				memcpy(&len, p + 1, sizeof(uint16_t));
				len = gg_fix16(len);

				if (e->event.msg.formats != NULL) {
					gg_debug_session(sess, GG_DEBUG_MISC,
						"// gg_handle_recv_msg_options()"
						" e->event.msg.formats already exist\n");
					goto malformed;
				}

				buf = malloc(len);

				if (buf == NULL) {
					gg_debug_session(sess, GG_DEBUG_MISC,
						"// gg_handle_recv_msg_options()"
						" not enough memory for richtext data\n");
					goto fail;
				}

				p += 3;

				if (p + len > packet_end) {
					gg_debug_session(sess, GG_DEBUG_MISC,
						"// gg_handle_recv_msg_options()"
						" packet out of bounds (3)\n");
					free(buf);
					goto malformed;
				}

				memcpy(buf, p, len);

				e->event.msg.formats = buf;
				e->event.msg.formats_length = len;

				p += len;

				break;
			}

			case GG_MSG_OPTION_IMAGE_REQUEST:
			{
				const struct gg_msg_image_request *i = (const void*) p;

				if (p + sizeof(*i) > packet_end) {
					gg_debug_session(sess, GG_DEBUG_MISC,
						"// gg_handle_recv_msg() "
						"packet out of bounds (3)\n");
					goto malformed;
				}

				if (e->event.msg.formats != NULL || e->event.msg.recipients != NULL) {
					gg_debug_session(sess, GG_DEBUG_MISC,
						"// gg_handle_recv_msg_options()"
						" mixed options (1)\n");
					goto malformed;
				}

				e->event.image_request.sender = sender;
				e->event.image_request.size = gg_fix32(i->size);
				e->event.image_request.crc32 = gg_fix32(i->crc32);

				e->type = GG_EVENT_IMAGE_REQUEST;

				goto handled;
			}

			case GG_MSG_OPTION_IMAGE_REPLY:
			case GG_MSG_OPTION_IMAGE_REPLY_MORE:
			{
				struct gg_msg_image_reply *rep = (void*) p;

				if (e->event.msg.formats != NULL || e->event.msg.recipients != NULL) {
					gg_debug_session(sess, GG_DEBUG_MISC,
						"// gg_handle_recv_msg_options() "
						"mixed options (2)\n");
					goto malformed;
				}

				if (p + sizeof(struct gg_msg_image_reply) == packet_end) {

					/* pusta odpowiedź - klient po drugiej stronie nie ma żądanego obrazka */

					e->type = GG_EVENT_IMAGE_REPLY;
					e->event.image_reply.sender = sender;
					e->event.image_reply.size = 0;
					e->event.image_reply.crc32 = gg_fix32(rep->crc32);
					e->event.image_reply.filename = NULL;
					e->event.image_reply.image = NULL;
					goto handled;

				} else if (p + sizeof(struct gg_msg_image_reply) + 1 > packet_end) {

					gg_debug_session(sess, GG_DEBUG_MISC,
						"// gg_handle_recv_msg() "
						"packet out of bounds (4)\n");
					goto malformed;
				}

				rep->size = gg_fix32(rep->size);
				rep->crc32 = gg_fix32(rep->crc32);
				gg_image_queue_parse(e, p, (unsigned int)(packet_end - p), sess, sender, packet_type);

				goto handled;
			}

			default:
			{
				gg_debug_session(sess, GG_DEBUG_MISC,
					"// gg_handle_recv_msg() "
					"unknown payload 0x%.2x\n", *p);
				p = packet_end;
			}
		}
	}

	return 0;

handled:
	return -1;

fail:
	return -2;

malformed:
	return -3;
}

/**
 * \internal Wysyła potwierdzenie odebrania wiadomości.
 *
 * \param gs Struktura sesji
 * \param seq Numer sekwencyjny odebranej wiadomości
 *
 * \return 0 jeśli się powiodło, -1 jeśli wystąpił błąd
 */
static int gg_session_send_msg_ack(struct gg_session *gs, uint32_t seq)
{
	struct gg_recv_msg_ack pkt;

	gg_debug_session(gs, GG_DEBUG_FUNCTION, "** gg_session_send_msg_ack(%p);\n", gs);

	if ((gs->protocol_features & GG_FEATURE_MSG_ACK) == 0)
		return 0;

	/* Kiedyś zdawało nam się, że mamy wysyłać liczbę odebranych
	 * wiadomości, ale okazało się, że numer sekwencyjny. */
	gs->recv_msg_count++;

	pkt.seq = gg_fix32(seq);

	return gg_send_packet(gs, GG_RECV_MSG_ACK, &pkt, sizeof(pkt), NULL);
}

/**
 * \internal Obsługuje pakiet GG_RECV_MSG.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_recv_msg(struct gg_session *sess, uint32_t type,
	const char *packet, size_t length, struct gg_event *e)
{
	const struct gg_recv_msg *r = (const struct gg_recv_msg*) packet;
	const char *payload = packet + sizeof(struct gg_recv_msg);
	const char *payload_end = packet + length;
	size_t len;

	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_handle_recv_msg(%p, %"
		GG_SIZE_FMT ", %p);\n", packet, length, e);

	if (sess == NULL)
		goto fail;

	if ((r->seq == 0) && (r->msgclass == 0)) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_recv_msg() oops, silently ignoring the bait\n");
		goto malformed;
	}

	/* jednobajtowa wiadomość o treści \x02 to żądanie połączenia DCC */
	if (*payload == GG_MSG_CALLBACK && payload == payload_end - 1) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_recv_msg() received ctcp packet\n");
		length = 1;
	} else {
		const char *options;

		options = memchr(payload, 0, (size_t) (payload_end - payload));

		if (options == NULL) {
			gg_debug_session(sess, GG_DEBUG_MISC,
				"// gg_handle_recv_msg() malformed packet, "
				"message out of bounds (0)\n");
			goto malformed;
		}

		length = (size_t) (options - payload);

		switch (gg_handle_recv_msg_options(sess, e, gg_fix32(r->sender), options + 1, payload_end, type)) {
			case -1:	/* handled */
				gg_session_send_msg_ack(sess, gg_fix32(r->seq));
				return 0;

			case -2:	/* failed */
				goto fail;

			case -3:	/* malformed */
				goto malformed;
		}
	}

	e->type = GG_EVENT_MSG;
	e->event.msg.msgclass = gg_fix32(r->msgclass);
	e->event.msg.sender = gg_fix32(r->sender);
	e->event.msg.time = gg_fix32(r->time);
	e->event.msg.seq = gg_fix32(r->seq);

	e->event.msg.message = (unsigned char*)gg_encoding_convert(payload,
		GG_ENCODING_CP1250, sess->encoding, length, -1);
	if (e->event.msg.message == NULL) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_session_handle_recv_msg() out of memory\n");
		goto fail;
	}

	len = gg_message_text_to_html(NULL, (char*)e->event.msg.message,
		sess->encoding, e->event.msg.formats,
		e->event.msg.formats_length);
	e->event.msg.xhtml_message = malloc(len + 1);

	if (e->event.msg.xhtml_message == NULL) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_session_handle_recv_msg() out of memory\n");
		goto fail;
	}

	gg_message_text_to_html(e->event.msg.xhtml_message,
		(char*)e->event.msg.message, sess->encoding,
		e->event.msg.formats, e->event.msg.formats_length);

	gg_session_send_msg_ack(sess, gg_fix32(r->seq));
	return 0;

fail:
	free(e->event.msg.message);
	free(e->event.msg.xhtml_message);
	free(e->event.msg.recipients);
	free(e->event.msg.formats);
	return -1;

malformed:
	e->type = GG_EVENT_NONE;
	free(e->event.msg.message);
	free(e->event.msg.xhtml_message);
	free(e->event.msg.recipients);
	free(e->event.msg.formats);
	gg_session_send_msg_ack(sess, gg_fix32(r->seq));
	return 0;
}

/**
 * \internal Obsługuje pakiet GG_RECV_MSG80.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_recv_msg_80(struct gg_session *sess, uint32_t type,
	const char *packet, size_t length, struct gg_event *e)
{
	const struct gg_recv_msg80 *r = (const struct gg_recv_msg80*) packet;
	uint32_t offset_plain;
	uint32_t offset_attr;

	gg_debug_session(sess, GG_DEBUG_FUNCTION,
		"** gg_handle_recv_msg80(%p, %" GG_SIZE_FMT ", %p);\n",
		packet, length, e);

	if (sess == NULL)
		goto fail;

	if (r->seq == 0 && r->msgclass == 0) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_handle_recv_msg80() oops, silently ignoring the bait\n");
		goto malformed;
	}

	offset_plain = gg_fix32(r->offset_plain);
	offset_attr = gg_fix32(r->offset_attr);

	if (offset_plain < sizeof(struct gg_recv_msg80) || offset_plain >= length) {
		gg_debug_session(sess, GG_DEBUG_MISC,
			"// gg_handle_recv_msg80() malformed packet, "
			"message out of bounds (0)\n");
		goto malformed;
	}

	if (offset_attr < sizeof(struct gg_recv_msg80) || offset_attr > length) {
		gg_debug_session(sess, GG_DEBUG_MISC,
			"// gg_handle_recv_msg80() malformed packet, "
			"attr out of bounds (1)\n");
		offset_attr = 0; /* nie parsuj attr. */
	}

	/* Normalna sytuacja, więc nie podpada pod powyższy warunek. */
	if (offset_attr == length)
		offset_attr = 0;

	if (memchr(packet + offset_plain, 0, length - offset_plain) == NULL) {
		gg_debug_session(sess, GG_DEBUG_MISC,
			"// gg_handle_recv_msg80() malformed packet, "
			"message out of bounds (2)\n");
		goto malformed;
	}

	if (offset_plain > sizeof(struct gg_recv_msg80) && memchr(packet +
		sizeof(struct gg_recv_msg80), 0, offset_plain -
		sizeof(struct gg_recv_msg80)) == NULL)
	{
		gg_debug_session(sess, GG_DEBUG_MISC,
			"// gg_handle_recv_msg80() malformed packet, "
			"message out of bounds (3)\n");
		goto malformed;
	}

	e->type = (type != GG_RECV_OWN_MSG) ? GG_EVENT_MSG : GG_EVENT_MULTILOGON_MSG;
	e->event.msg.msgclass = gg_fix32(r->msgclass);
	e->event.msg.sender = gg_fix32(r->sender);
	e->event.msg.time = gg_fix32(r->time);
	e->event.msg.seq = gg_fix32(r->seq);

	if (offset_attr != 0) {
		switch (gg_handle_recv_msg_options(sess, e, gg_fix32(r->sender),
			packet + offset_attr, packet + length, type))
		{
			case -1:	/* handled */
				gg_session_send_msg_ack(sess, gg_fix32(r->seq));
				return 0;

			case -2:	/* failed */
				goto fail;

			case -3:	/* malformed */
				goto malformed;
		}
	}

	if (sess->encoding == GG_ENCODING_CP1250) {
		e->event.msg.message = (unsigned char*) strdup(packet + offset_plain);

		if (e->event.msg.message == NULL) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_session_handle_recv_msg_80() out of memory\n");
			goto fail;
		}
	} else {
		if (offset_plain > sizeof(struct gg_recv_msg80)) {
			size_t len, fmt_len;

			len = gg_message_html_to_text(NULL, NULL, &fmt_len,
				packet + sizeof(struct gg_recv_msg80),
				GG_ENCODING_UTF8);
			e->event.msg.message = malloc(len + 1);

			if (e->event.msg.message == NULL) {
				gg_debug_session(sess, GG_DEBUG_MISC,
					"// gg_session_handle_recv_msg_80() "
					"out of memory\n");
				goto fail;
			}

			free(e->event.msg.formats);
			e->event.msg.formats_length = fmt_len;
			e->event.msg.formats = malloc(fmt_len);

			if (e->event.msg.formats == NULL) {
				gg_debug_session(sess, GG_DEBUG_MISC,
					"// gg_session_handle_recv_msg_80() "
					"out of memory\n");
				goto fail;
			}

			gg_message_html_to_text((char*)e->event.msg.message,
				e->event.msg.formats, NULL,
				packet + sizeof(struct gg_recv_msg80),
				GG_ENCODING_UTF8);
		} else {
			e->event.msg.message = (unsigned char*)gg_encoding_convert(
				packet + offset_plain, GG_ENCODING_CP1250,
				sess->encoding, -1, -1);

			if (e->event.msg.message == NULL) {
				gg_debug_session(sess, GG_DEBUG_MISC,
					"// gg_session_handle_recv_msg_80() "
					"out of memory\n");
				goto fail;
			}
		}
	}

	if (offset_plain > sizeof(struct gg_recv_msg80)) {
		e->event.msg.xhtml_message = gg_encoding_convert(
			packet + sizeof(struct gg_recv_msg80), GG_ENCODING_UTF8,
			sess->encoding, -1, -1);

		if (e->event.msg.xhtml_message == NULL) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_session_handle_recv_msg_80() out of memory\n");
			goto fail;
		}
	} else {
		size_t len;

		len = gg_message_text_to_html(NULL,
			(char*)e->event.msg.message, sess->encoding,
			e->event.msg.formats, e->event.msg.formats_length);
		e->event.msg.xhtml_message = malloc(len + 1);

		if (e->event.msg.xhtml_message == NULL) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_session_handle_recv_msg_80() out of memory\n");
			goto fail;
		}

		gg_message_text_to_html(e->event.msg.xhtml_message,
			(char*)e->event.msg.message, sess->encoding,
			e->event.msg.formats, e->event.msg.formats_length);
	}

	gg_session_send_msg_ack(sess, gg_fix32(r->seq));
	return 0;

fail:
	free(e->event.msg.message);
	free(e->event.msg.xhtml_message);
	free(e->event.msg.recipients);
	free(e->event.msg.formats);
	return -1;

malformed:
	e->type = GG_EVENT_NONE;
	free(e->event.msg.message);
	free(e->event.msg.xhtml_message);
	free(e->event.msg.recipients);
	free(e->event.msg.formats);
	gg_session_send_msg_ack(sess, gg_fix32(r->seq));
	return 0;
}

static int gg_session_handle_recv_msg_110(struct gg_session *gs, uint32_t type,
	const char *ptr, size_t len, struct gg_event *ge)
{
	GG110RecvMessage *msg = gg110_recv_message__unpack(NULL, len, (uint8_t*)ptr);
	uint8_t ack_type;
	uin_t sender = 0;
	uint32_t seq;
	int succ = 1;
	struct gg_event_msg *ev = &ge->event.msg;

	gg_debug_session(gs, GG_DEBUG_FUNCTION,
		"** gg_session_handle_recv_msg_110(%p, %" GG_SIZE_FMT
		", %p);\n", ptr, len, ge);

	if (!GG_PROTOBUF_VALID(gs, "GG110RecvMessage", msg))
		return -1;

	seq = msg->seq;
	if (type == GG_CHAT_RECV_MSG || type == GG_CHAT_RECV_OWN_MSG)
		ack_type = GG110_ACK__TYPE__CHAT;
	else
		ack_type = GG110_ACK__TYPE__MSG;

	if (msg->has_msg_id || msg->has_conv_id) {
		msg->msg_id = msg->has_msg_id ? msg->msg_id : 0;
		msg->conv_id = msg->has_conv_id ? msg->conv_id : 0;
		gg_debug_session(gs, GG_DEBUG_VERBOSE,
			"// gg_session_handle_recv_msg_110() "
			"msg_id=%016" PRIx64 " conv_id=%016" PRIx64 "\n",
			msg->msg_id, msg->conv_id);
	}

	if (msg->has_sender)
		sender = gg_protobuf_get_uin(msg->sender);
	else if (type == GG_CHAT_RECV_OWN_MSG)
		sender = gs->uin;

	if (msg->has_data && msg->msg_plain[0] == '\0') {
		if (msg->data.len < sizeof(struct gg_msg_image_reply)) {
			gg_debug_session(gs, GG_DEBUG_ERROR,
				"// gg_session_handle_recv_msg_110() "
				"packet too small (%" GG_SIZE_FMT " < %"
				GG_SIZE_FMT ")\n", msg->data.len,
				sizeof(struct gg_msg_image_reply));
		} else {
			gg_image_queue_parse(ge, (char *)msg->data.data,
				msg->data.len, gs, sender, type);
		}
		gg110_recv_message__free_unpacked(msg, NULL);
		return gg_ack_110(gs, GG110_ACK__TYPE__MSG, seq, ge);
	}

	if (type == GG_RECV_OWN_MSG110 || type == GG_CHAT_RECV_OWN_MSG)
		ge->type = GG_EVENT_MULTILOGON_MSG;
	else
		ge->type = GG_EVENT_MSG;
	ev->msgclass = GG_CLASS_CHAT;
	ev->seq = seq;
	ev->sender = sender;
	ev->flags = msg->flags;
	ev->seq = seq;
	ev->time = msg->time;

	if (labs(msg->time - gg_server_time(gs)) > 2)
		ev->msgclass |= GG_CLASS_QUEUED;

	ev->message = NULL;
	if (msg->msg_plain[0] != '\0') {
		ev->message = (unsigned char*)gg_encoding_convert(
			msg->msg_plain, GG_ENCODING_UTF8, gs->encoding, -1, -1);
		succ = succ && (ev->message != NULL);
	}
	ev->xhtml_message = NULL;
	if (msg->msg_xhtml != NULL) {
		ev->xhtml_message = gg_encoding_convert(
			msg->msg_xhtml, GG_ENCODING_UTF8, gs->encoding, -1, -1);
		succ = succ && (ev->xhtml_message != NULL);
	}

	/* wiadomości wysłane z mobilnego gg nie posiadają wersji xhtml */
	if (ev->message == NULL && ev->xhtml_message == NULL) {
		ev->message = (unsigned char*)strdup("");
		succ = succ && (ev->message != NULL);
	} else if (ev->message == NULL) {
		ev->message = (unsigned char*)gg_message_html_to_text_110(
			ev->xhtml_message);
		succ = succ && (ev->message != NULL);
	} else if (ev->xhtml_message == NULL) {
		ev->xhtml_message = gg_message_text_to_html_110(
			(char*)ev->message, -1);
		succ = succ && (ev->xhtml_message != NULL);
	}

	/* otrzymywane tylko od gg <= 10.5 */
	ev->formats = NULL;
	ev->formats_length = 0;
	if (msg->has_data && succ) {
		ev->formats_length = msg->data.len;
		ev->formats = malloc(msg->data.len);
		if (ev->formats == NULL)
			succ = 0;
		else
			memcpy(ev->formats, msg->data.data, msg->data.len);
	}

	if (msg->has_chat_id && succ) {
		gg_chat_list_t *chat;

		ev->chat_id = msg->chat_id;

		chat = gg_chat_find(gs, msg->chat_id);
		if (chat) {
			size_t rcpt_size = chat->participants_count *
				sizeof(uin_t);
			ev->recipients = malloc(rcpt_size);
			ev->recipients_count = chat->participants_count;
			if (ev->recipients == NULL)
				succ = 0;
			else {
				memcpy(ev->recipients, chat->participants,
					rcpt_size);
			}
		}
	}

	gg110_recv_message__free_unpacked(msg, NULL);

	if (gg_ack_110(gs, ack_type, seq, ge) != 0)
		succ = 0;

	if (succ)
		return 0;
	else {
		free(ev->message);
		free(ev->xhtml_message);
		free(ev->formats);
		free(ev->recipients);
		return -1;
	}
}

/**
 * \internal Obsługuje pakiet GG_STATUS.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_status(struct gg_session *gs, uint32_t type,
	const char *ptr, size_t len, struct gg_event *ge)
{
	const struct gg_status *s = (const void*) ptr;

	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received a status change\n");

	ge->type = GG_EVENT_STATUS;
	ge->event.status.uin = gg_fix32(s->uin);
	ge->event.status.status = gg_fix32(s->status);
	ge->event.status.descr = NULL;

	if (len > sizeof(*s)) {
		ge->event.status.descr = gg_encoding_convert(ptr + sizeof(*s),
			GG_ENCODING_CP1250, gs->encoding, len - sizeof(*s), -1);

		if (ge->event.status.descr == NULL) {
			gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() out of memory\n");
			return -1;
		}
	}

	return 0;
}

/**
 * \internal Obsługuje pakiety GG_STATUS60, GG_STATUS77 i GG_STATUS80BETA.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_status_60_77_80beta(struct gg_session *gs,
	uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	const struct gg_status60 *s60 = (const void*) ptr;
	const struct gg_status77 *s77 = (const void*) ptr;
	size_t struct_len;
	uint32_t uin;

	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received a status change\n");

	ge->type = GG_EVENT_STATUS60;
	ge->event.status60.descr = NULL;
	ge->event.status60.time = 0;

	if (type == GG_STATUS60) {
		uin = gg_fix32(s60->uin);
		ge->event.status60.status = s60->status;
		ge->event.status60.remote_ip = s60->remote_ip;
		ge->event.status60.remote_port = gg_fix16(s60->remote_port);
		ge->event.status60.version = s60->version;
		ge->event.status60.image_size = s60->image_size;
		struct_len = sizeof(*s60);
	} else {
		uin = gg_fix32(s77->uin);
		ge->event.status60.status = s77->status;
		ge->event.status60.remote_ip = s77->remote_ip;
		ge->event.status60.remote_port = gg_fix16(s77->remote_port);
		ge->event.status60.version = s77->version;
		ge->event.status60.image_size = s77->image_size;
		struct_len = sizeof(*s77);
	}

	ge->event.status60.uin = uin & 0x00ffffff;

	if (uin & 0x40000000)
		ge->event.status60.version |= GG_HAS_AUDIO_MASK;
	if (uin & 0x20000000)
		ge->event.status60.version |= GG_HAS_AUDIO7_MASK;
	if (uin & 0x08000000)
		ge->event.status60.version |= GG_ERA_OMNIX_MASK;

	if (len > struct_len) {
		size_t descr_len;

		descr_len = len - struct_len;

		ge->event.status60.descr = gg_encoding_convert(ptr + struct_len,
			(type == GG_STATUS80BETA) ? GG_ENCODING_UTF8 : GG_ENCODING_CP1250,
			gs->encoding, descr_len, -1);

		if (ge->event.status60.descr == NULL) {
			gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() out of memory\n");
			return -1;
		}

		if (descr_len > 4 && ptr[len - 5] == 0) {
			uint32_t t;
			memcpy(&t, ptr + len - 4, sizeof(uint32_t));
			ge->event.status60.time = gg_fix32(t);
		}
	}

	return 0;
}

/**
 * \internal Obsługuje pakiet GG_NOTIFY_REPLY.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_notify_reply(struct gg_session *gs, uint32_t type,
	const char *ptr, size_t len, struct gg_event *ge)
{
	const struct gg_notify_reply *n = (const void*) ptr;
	char *descr;

	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received a notify reply\n");

	if (gg_fix32(n->status) == GG_STATUS_BUSY_DESCR ||
		gg_fix32(n->status) == GG_STATUS_NOT_AVAIL_DESCR ||
		gg_fix32(n->status) == GG_STATUS_AVAIL_DESCR)
	{
		size_t descr_len;

		ge->type = GG_EVENT_NOTIFY_DESCR;

		if (!(ge->event.notify_descr.notify = (void*) malloc(sizeof(*n) * 2))) {
			gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() out of memory\n");
			return -1;
		}
		ge->event.notify_descr.notify[1].uin = 0;
		memcpy(ge->event.notify_descr.notify, ptr, sizeof(*n));
		ge->event.notify_descr.notify[0].uin = gg_fix32(ge->event.notify_descr.notify[0].uin);
		ge->event.notify_descr.notify[0].status = gg_fix32(ge->event.notify_descr.notify[0].status);
		ge->event.notify_descr.notify[0].remote_port = gg_fix16(ge->event.notify_descr.notify[0].remote_port);
		ge->event.notify_descr.notify[0].version = gg_fix32(ge->event.notify_descr.notify[0].version);

		descr_len = len - sizeof(*n);

		descr = gg_encoding_convert(ptr + sizeof(*n), GG_ENCODING_CP1250, gs->encoding, descr_len, -1);

		if (descr == NULL) {
			gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() out of memory\n");
			return -1;
		}

		ge->event.notify_descr.descr = descr;

	} else {
		unsigned int i, count;

		ge->type = GG_EVENT_NOTIFY;

		if (!(ge->event.notify = (void*) malloc(len + 2 * sizeof(*n)))) {
			gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() out of memory\n");
			return -1;
		}

		memcpy(ge->event.notify, ptr, len);
		count = len / sizeof(*n);
		ge->event.notify[count].uin = 0;

		for (i = 0; i < count; i++) {
			ge->event.notify[i].uin = gg_fix32(ge->event.notify[i].uin);
			ge->event.notify[i].status = gg_fix32(ge->event.notify[i].status);
			ge->event.notify[i].remote_port = gg_fix16(ge->event.notify[i].remote_port);
			ge->event.notify[i].version = gg_fix32(ge->event.notify[i].version);
		}
	}

	return 0;
}

/**
 * \internal Obsługuje pakiet GG_STATUS80.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_status_80(struct gg_session *gs, uint32_t type,
	const char *ptr, size_t len, struct gg_event *ge)
{
	const struct gg_notify_reply80 *n = (const void*) ptr;
	size_t descr_len;

	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received a status change\n");

	ge->type = GG_EVENT_STATUS60;
	ge->event.status60.uin = gg_fix32(n->uin);
	ge->event.status60.status = gg_fix32(n->status);
	ge->event.status60.remote_ip = n->remote_ip;
	ge->event.status60.remote_port = gg_fix16(n->remote_port);
	ge->event.status60.version = 0;
	ge->event.status60.image_size = n->image_size;
	ge->event.status60.descr = NULL;
	ge->event.status60.time = 0;

	descr_len = gg_fix32(n->descr_len);

	if (descr_len != 0 && sizeof(struct gg_notify_reply80) + descr_len <= len) {
		ge->event.status60.descr = gg_encoding_convert(
			(const char*) n + sizeof(struct gg_notify_reply80),
			GG_ENCODING_UTF8, gs->encoding, descr_len, -1);

		if (ge->event.status60.descr == NULL) {
			gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() out of memory\n");
			return -1;
		}

		/* XXX czas */
	}

	return 0;
}

/**
 * \internal Obsługuje pakiet GG_NOTIFY_REPLY80.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_notify_reply_80(struct gg_session *gs,
	uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	const struct gg_notify_reply80 *n = (const void*) ptr;
	unsigned int length = len, i = 0;

	/* TODO: najpierw przeanalizować strukturę i określić
	 * liczbę rekordów, żeby obyć się bez realloc()
	 */

	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received a notify reply\n");

	ge->type = GG_EVENT_NOTIFY60;
	ge->event.notify60 = malloc(sizeof(*ge->event.notify60));

	if (!ge->event.notify60) {
		gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() out of memory\n");
		return -1;
	}

	ge->event.notify60[0].uin = 0;

	while (length >= sizeof(struct gg_notify_reply80)) {
		uin_t uin = gg_fix32(n->uin);
		int descr_len;
		void *tmp;

		ge->event.notify60[i].uin = uin;
		ge->event.notify60[i].status = gg_fix32(n->status);
		ge->event.notify60[i].remote_ip = n->remote_ip;
		ge->event.notify60[i].remote_port = gg_fix16(n->remote_port);
		ge->event.notify60[i].version = 0;
		ge->event.notify60[i].image_size = n->image_size;
		ge->event.notify60[i].descr = NULL;
		ge->event.notify60[i].time = 0;

		descr_len = gg_fix32(n->descr_len);

		if (descr_len != 0) {
			if (sizeof(struct gg_notify_reply80) + descr_len <= length) {
				ge->event.notify60[i].descr = gg_encoding_convert(
					(const char*) n + sizeof(struct gg_notify_reply80),
					GG_ENCODING_UTF8, gs->encoding, descr_len, -1);

				if (ge->event.notify60[i].descr == NULL) {
					gg_debug_session(gs, GG_DEBUG_MISC,
						"// gg_watch_fd_connected() "
						"out of memory\n");
					return -1;
				}

				/* XXX czas */

				length -= sizeof(struct gg_notify_reply80) + descr_len;
				n = (const void*) ((const char*) n + sizeof(struct gg_notify_reply80) + descr_len);
			} else {
				length = 0;
			}

		} else {
			length -= sizeof(struct gg_notify_reply80);
			n = (const void*) ((const char*) n + sizeof(struct gg_notify_reply80));
		}

		if (!(tmp = realloc(ge->event.notify60, (i + 2) * sizeof(*ge->event.notify60)))) {
			gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() out of memory\n");
			free(ge->event.notify60);
			return -1;
		}

		ge->event.notify60 = tmp;
		ge->event.notify60[++i].uin = 0;
	}

	return 0;
}

/**
 * \internal Obsługuje pakiety GG_NOTIFY_REPLY77 i GG_NOTIFY_REPLY80BETA.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_notify_reply_77_80beta(struct gg_session *gs,
	uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	const struct gg_notify_reply77 *n = (const void*) ptr;
	unsigned int length = len, i = 0;

	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received a notify reply\n");

	ge->type = GG_EVENT_NOTIFY60;
	ge->event.notify60 = malloc(sizeof(*ge->event.notify60));

	if (ge->event.notify60 == NULL) {
		gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() out of memory\n");
		return -1;
	}

	ge->event.notify60[0].uin = 0;

	while (length >= sizeof(struct gg_notify_reply77)) {
		uin_t uin = gg_fix32(n->uin);
		void *tmp;

		ge->event.notify60[i].uin = uin & 0x00ffffff;
		ge->event.notify60[i].status = n->status;
		ge->event.notify60[i].remote_ip = n->remote_ip;
		ge->event.notify60[i].remote_port = gg_fix16(n->remote_port);
		ge->event.notify60[i].version = n->version;
		ge->event.notify60[i].image_size = n->image_size;
		ge->event.notify60[i].descr = NULL;
		ge->event.notify60[i].time = 0;

		if (uin & 0x40000000)
			ge->event.notify60[i].version |= GG_HAS_AUDIO_MASK;
		if (uin & 0x20000000)
			ge->event.notify60[i].version |= GG_HAS_AUDIO7_MASK;
		if (uin & 0x08000000)
			ge->event.notify60[i].version |= GG_ERA_OMNIX_MASK;

		if (GG_S_D(n->status)) {
			unsigned char descr_len = *((const char*) n + sizeof(struct gg_notify_reply77));

			if (sizeof(struct gg_notify_reply77) + descr_len <= length) {
				ge->event.notify60[i].descr = gg_encoding_convert(
					(const char*) n + sizeof(struct gg_notify_reply77) + 1,
					(type == GG_NOTIFY_REPLY80BETA) ? GG_ENCODING_UTF8 : GG_ENCODING_CP1250,
					gs->encoding, descr_len, -1);

				if (ge->event.notify60[i].descr == NULL) {
					gg_debug_session(gs, GG_DEBUG_MISC,
						"// gg_watch_fd_connected() "
						"out of memory\n");
					return -1;
				}

				/* XXX czas */

				length -= sizeof(struct gg_notify_reply77) + descr_len + 1;
				n = (const void*) ((const char*) n + sizeof(struct gg_notify_reply77) + descr_len + 1);
			} else {
				length = 0;
			}

		} else {
			length -= sizeof(struct gg_notify_reply77);
			n = (const void*) ((const char*) n + sizeof(struct gg_notify_reply77));
		}

		if (!(tmp = realloc(ge->event.notify60, (i + 2) * sizeof(*ge->event.notify60)))) {
			gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() out of memory\n");
			free(ge->event.notify60);
			return -1;
		}

		ge->event.notify60 = tmp;
		ge->event.notify60[++i].uin = 0;
	}

	return 0;
}

/**
 * \internal Obsługuje pakiet GG_NOTIFY_REPLY60.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_notify_reply_60(struct gg_session *gs,
	uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	const struct gg_notify_reply60 *n = (const void*) ptr;
	unsigned int length = len, i = 0;

	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received a notify reply\n");

	ge->type = GG_EVENT_NOTIFY60;
	ge->event.notify60 = malloc(sizeof(*ge->event.notify60));

	if (ge->event.notify60 == NULL) {
		gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() out of memory\n");
		return -1;
	}

	ge->event.notify60[0].uin = 0;

	while (length >= sizeof(struct gg_notify_reply60)) {
		uin_t uin = gg_fix32(n->uin);
		void *tmp;

		ge->event.notify60[i].uin = uin & 0x00ffffff;
		ge->event.notify60[i].status = n->status;
		ge->event.notify60[i].remote_ip = n->remote_ip;
		ge->event.notify60[i].remote_port = gg_fix16(n->remote_port);
		ge->event.notify60[i].version = n->version;
		ge->event.notify60[i].image_size = n->image_size;
		ge->event.notify60[i].descr = NULL;
		ge->event.notify60[i].time = 0;

		if (uin & 0x40000000)
			ge->event.notify60[i].version |= GG_HAS_AUDIO_MASK;
		if (uin & 0x08000000)
			ge->event.notify60[i].version |= GG_ERA_OMNIX_MASK;

		if (GG_S_D(n->status)) {
			unsigned char descr_len = *((const char*) n + sizeof(struct gg_notify_reply60));

			if (sizeof(struct gg_notify_reply60) + descr_len <= length) {
				char *descr;

				descr = gg_encoding_convert((const char*) n +
					sizeof(struct gg_notify_reply60) + 1,
					GG_ENCODING_CP1250, gs->encoding,
					descr_len, -1);

				if (descr == NULL) {
					gg_debug_session(gs, GG_DEBUG_MISC,
						"// gg_watch_fd_connected() "
						"out of memory\n");
					return -1;
				}

				ge->event.notify60[i].descr = descr;

				/* XXX czas */

				length -= sizeof(struct gg_notify_reply60) + descr_len + 1;
				n = (const void*) ((const char*) n + sizeof(struct gg_notify_reply60) + descr_len + 1);
			} else {
				length = 0;
			}

		} else {
			length -= sizeof(struct gg_notify_reply60);
			n = (const void*) ((const char*) n + sizeof(struct gg_notify_reply60));
		}

		if (!(tmp = realloc(ge->event.notify60, (i + 2) * sizeof(*ge->event.notify60)))) {
			gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() out of memory\n");
			free(ge->event.notify60);
			return -1;
		}

		ge->event.notify60 = tmp;
		ge->event.notify60[++i].uin = 0;
	}

	return 0;
}

/**
 * \internal Obsługuje pakiet GG_USER_DATA.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_user_data(struct gg_session *gs, uint32_t type,
	const char *ptr, size_t len, struct gg_event *ge)
{
	struct gg_user_data d;
	const char *p = (const char*) ptr;
	const char *packet_end = (const char*) ptr + len;
	struct gg_event_user_data_user *users;
	unsigned int i, j;
	int res = 0;

	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received user data\n");

	ge->event.user_data.user_count = 0;
	ge->event.user_data.users = NULL;

	if (ptr + sizeof(d) > packet_end)
		goto malformed;

	memcpy(&d, p, sizeof(d));
	p += sizeof(d);

	d.type = gg_fix32(d.type);
	d.user_count = gg_fix32(d.user_count);

	if (d.user_count > 0xffff) {
		gg_debug_session(gs, GG_DEBUG_MISC, "// gg_session_handle_user_data() malformed packet (1)\n");
		goto malformed;
	}

	if (d.user_count > 0) {
		users = calloc(d.user_count, sizeof(struct gg_event_user_data_user));

		if (users == NULL) {
			gg_debug_session(gs, GG_DEBUG_MISC,
				"// gg_session_handle_user_data() out of memory"
				" (%d*%" GG_SIZE_FMT ")\n", d.user_count,
				sizeof(struct gg_event_user_data_user));
			goto fail;
		}
	} else {
		users = NULL;
	}

	ge->type = GG_EVENT_USER_DATA;
	ge->event.user_data.type = d.type;
	ge->event.user_data.user_count = d.user_count;
	ge->event.user_data.users = users;

	gg_debug_session(gs, GG_DEBUG_DUMP, "type=%d, count=%d\n", d.type, d.user_count);

	for (i = 0; i < d.user_count; i++) {
		struct gg_user_data_user u;
		struct gg_event_user_data_attr *attrs;

		if (p + sizeof(u) > packet_end) {
			gg_debug_session(gs, GG_DEBUG_MISC, "// gg_session_handle_user_data() malformed packet (2)\n");
			goto malformed;
		}

		memcpy(&u, p, sizeof(u));
		p += sizeof(u);

		u.uin = gg_fix32(u.uin);
		u.attr_count = gg_fix32(u.attr_count);

		if (u.attr_count > 0xffff) {
			gg_debug_session(gs, GG_DEBUG_MISC, "// gg_session_handle_user_data() malformed packet (2)\n");
			goto malformed;
		}

		if (u.attr_count > 0) {
			attrs = calloc(u.attr_count, sizeof(struct gg_event_user_data_attr));

			if (attrs == NULL) {
				gg_debug_session(gs, GG_DEBUG_MISC,
					"// gg_session_handle_user_data() "
					"out of memory (%d*%" GG_SIZE_FMT
					")\n", u.attr_count,
					sizeof(struct gg_event_user_data_attr));
				goto fail;
			}
		} else {
			attrs = NULL;
		}

		users[i].uin = u.uin;
		users[i].attr_count = u.attr_count;
		users[i].attrs = attrs;

		gg_debug_session(gs, GG_DEBUG_DUMP, "    uin=%d, count=%d\n", u.uin, u.attr_count);

		for (j = 0; j < u.attr_count; j++) {
			uint32_t key_size;
			uint32_t attr_type;
			uint32_t value_size;
			char *key;
			char *value;

			if (p + sizeof(key_size) > packet_end) {
				gg_debug_session(gs, GG_DEBUG_MISC,
					"// gg_session_handle_user_data()"
					"malformed packet (3)\n");
				goto malformed;
			}

			memcpy(&key_size, p, sizeof(key_size));
			p += sizeof(key_size);

			key_size = gg_fix32(key_size);

			if (key_size > 0xffff || p + key_size > packet_end) {
				gg_debug_session(gs, GG_DEBUG_MISC,
					"// gg_session_handle_user_data() "
					"malformed packet (3)\n");
				goto malformed;
			}

			key = malloc(key_size + 1);

			if (key == NULL) {
				gg_debug_session(gs, GG_DEBUG_MISC,
					"// gg_session_handle_user_data() "
					"out of memory (%d)\n", key_size + 1);
				goto fail;
			}

			memcpy(key, p, key_size);
			p += key_size;

			key[key_size] = 0;

			attrs[j].key = key;

			if (p + sizeof(attr_type) + sizeof(value_size) > packet_end) {
				gg_debug_session(gs, GG_DEBUG_MISC,
					"// gg_session_handle_user_data() "
					"malformed packet (4)\n");
				goto malformed;
			}

			memcpy(&attr_type, p, sizeof(attr_type));
			p += sizeof(attr_type);
			memcpy(&value_size, p, sizeof(value_size));
			p += sizeof(value_size);

			attrs[j].type = gg_fix32(attr_type);
			value_size = gg_fix32(value_size);

			if (value_size > 0xffff || p + value_size > packet_end) {
				gg_debug_session(gs, GG_DEBUG_MISC,
					"// gg_session_handle_user_data() "
					"malformed packet (5)\n");
				goto malformed;
			}

			value = malloc(value_size + 1);

			if (value == NULL) {
				gg_debug_session(gs, GG_DEBUG_MISC,
					"// gg_session_handle_user_data() "
					"out of memory (%d)\n", value_size + 1);
				goto fail;
			}

			memcpy(value, p, value_size);
			p += value_size;

			value[value_size] = 0;

			attrs[j].value = value;

			gg_debug_session(gs, GG_DEBUG_DUMP, "	key=\"%s\", "
				"type=%d, value=\"%s\"\n", key, attr_type, value);
		}
	}

	return 0;

fail:
	res = -1;

malformed:
	ge->type = GG_EVENT_NONE;

	for (i = 0; i < ge->event.user_data.user_count; i++) {
		for (j = 0; j < ge->event.user_data.users[i].attr_count; j++) {
			free(ge->event.user_data.users[i].attrs[j].key);
			free(ge->event.user_data.users[i].attrs[j].value);
		}

		free(ge->event.user_data.users[i].attrs);
	}

	free(ge->event.user_data.users);

	return res;
}

/**
 * \internal Obsługuje pakiet GG_TYPING_NOTIFICATION.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_typing_notification(struct gg_session *gs,
	uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	const struct gg_typing_notification *n = (const void*) ptr;
	uin_t uin;

	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received typing notification\n");

	memcpy(&uin, &n->uin, sizeof(uin_t));

	ge->type = GG_EVENT_TYPING_NOTIFICATION;
	ge->event.typing_notification.uin = gg_fix32(uin);
	ge->event.typing_notification.length = gg_fix16(n->length);

	return 0;
}

/**
 * \internal Obsługuje pakiet GG_MULTILOGON_INFO.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_multilogon_info(struct gg_session *gs,
	uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	const char *packet_end = (const char*) ptr + len;
	const struct gg_multilogon_info *info = (const struct gg_multilogon_info*) ptr;
	const char *p = (const char*) ptr + sizeof(*info);
	struct gg_multilogon_session *sessions = NULL;
	size_t count;
	size_t i;
	int res = 0;

	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received multilogon info\n");

	count = gg_fix32(info->count);

	if (count > 0xffff) {
		gg_debug_session(gs, GG_DEBUG_MISC, "// gg_handle_multilogon_info() malformed packet (1)\n");
		goto malformed;
	}

	sessions = calloc(count, sizeof(struct gg_multilogon_session));

	if (sessions == NULL) {
		gg_debug_session(gs, GG_DEBUG_MISC, "// "
			"gg_handle_multilogon_info() out of memory (%"
			GG_SIZE_FMT "*%" GG_SIZE_FMT ")\n",
			count, sizeof(struct gg_multilogon_session));
		return -1;
	}

	ge->type = GG_EVENT_MULTILOGON_INFO;
	ge->event.multilogon_info.count = count;
	ge->event.multilogon_info.sessions = sessions;

	for (i = 0; i < count; i++) {
		struct gg_multilogon_info_item item;
		size_t name_size;

		if (p + sizeof(item) > packet_end) {
			gg_debug_session(gs, GG_DEBUG_MISC, "// gg_handle_multilogon_info() malformed packet (2)\n");
			goto malformed;
		}

		memcpy(&item, p, sizeof(item));

		sessions[i].id = item.conn_id;
		sessions[i].remote_addr = item.addr;
		sessions[i].status_flags = gg_fix32(item.flags);
		sessions[i].protocol_features = gg_fix32(item.features);
		sessions[i].logon_time = gg_fix32(item.logon_time);

		p += sizeof(item);

		name_size = gg_fix32(item.name_size);

		if (name_size > 0xffff || p + name_size > packet_end) {
			gg_debug_session(gs, GG_DEBUG_MISC, "// gg_handle_multilogon_info() malformed packet (3)\n");
			goto malformed;
		}

		sessions[i].name = malloc(name_size + 1);

		if (sessions[i].name == NULL) {
			gg_debug_session(gs, GG_DEBUG_MISC,
				"// gg_handle_multilogon_info() out of "
				"memory (%" GG_SIZE_FMT ")\n", name_size);
			goto fail;
		}

		memcpy(sessions[i].name, p, name_size);
		sessions[i].name[name_size] = 0;

		p += name_size;
	}

	return 0;

fail:
	res = -1;

malformed:
	ge->type = GG_EVENT_NONE;

	for (i = 0; (int) i < ge->event.multilogon_info.count; i++)
		free(ge->event.multilogon_info.sessions[i].name);

	free(ge->event.multilogon_info.sessions);

	return res;
}

/**
 * \internal Obsługuje pakiet GG_USERLIST100_VERSION.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_userlist_100_version(struct gg_session *gs,
	uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	const struct gg_userlist100_version *version = (const struct gg_userlist100_version*) ptr;

	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received userlist 100 version\n");

	ge->type = GG_EVENT_USERLIST100_VERSION;
	ge->event.userlist100_version.version = gg_fix32(version->version);

	return 0;
}

/**
 * \internal Obsługuje pakiet GG_USERLIST100_REPLY.
 *
 * Patrz gg_packet_handler_t
 */
static int gg_session_handle_userlist_100_reply(struct gg_session *gs,
	uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	const struct gg_userlist100_reply *reply = (const struct gg_userlist100_reply*) ptr;
	char *data = NULL;

	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() received userlist 100 reply\n");

	if (len > sizeof(*reply)) {
		data = gg_inflate((const unsigned char*) ptr + sizeof(*reply), len - sizeof(*reply));

		if (data == NULL) {
			gg_debug_session(gs, GG_DEBUG_MISC, "// gg_handle_userlist_100_reply() gg_inflate() failed\n");
			return -1;
		}
	}

	ge->type = GG_EVENT_USERLIST100_REPLY;
	ge->event.userlist100_reply.type = reply->type;
	ge->event.userlist100_reply.version = gg_fix32(reply->version);
	ge->event.userlist100_reply.format_type = reply->format_type;
	ge->event.userlist100_reply.reply = data;

	return 0;
}

static int gg_session_handle_imtoken(struct gg_session *gs, uint32_t type,
	const char *ptr, size_t len, struct gg_event *ge)
{
	GG110Imtoken *msg = gg110_imtoken__unpack(NULL, len, (uint8_t*)ptr);
	char *imtoken = NULL;
	int succ = 1;

	if (!GG_PROTOBUF_VALID(gs, "GG110Imtoken", msg))
		return -1;

	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() "
		"received imtoken\n");

	if (msg->imtoken[0] != '\0') {
		imtoken = strdup(msg->imtoken);
		succ = succ && (imtoken != NULL);
	}

	gg110_imtoken__free_unpacked(msg, NULL);

	ge->type = GG_EVENT_IMTOKEN;
	ge->event.imtoken.imtoken = imtoken;

	return succ ? 0 : -1;
}

static int gg_session_handle_pong_110(struct gg_session *gs, uint32_t type,
	const char *ptr, size_t len, struct gg_event *ge)
{
	GG110Pong *msg = gg110_pong__unpack(NULL, len, (uint8_t*)ptr);

	if (!GG_PROTOBUF_VALID(gs, "GG110Pong", msg))
		return -1;

	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_watch_fd_connected() "
		"received pong110\n");

	ge->type = GG_EVENT_PONG110;
	ge->event.pong110.time = msg->server_time;

	gg_sync_time(gs, msg->server_time);

	gg110_pong__free_unpacked(msg, NULL);

	return 0;
}

static int gg_session_handle_chat_info(struct gg_session *gs, uint32_t type,
	const char *ptr, size_t len, struct gg_event *ge)
{
	gg_tvbuff_t *tvb;
	uint32_t i;

	uint64_t id;
	uint32_t version;
	uint32_t map_size;
	uint32_t participants_count;
	uin_t *participants = NULL;

	tvb = gg_tvbuff_new(ptr, len);

	id = gg_tvbuff_read_uint64(tvb);
	gg_tvbuff_expected_uint32(tvb, 0); /* unknown */
	version = gg_tvbuff_read_uint32(tvb);

	map_size = gg_tvbuff_read_uint32(tvb);
	for (i = 0; i < map_size && gg_tvbuff_is_valid(tvb); i++) {
		uint32_t key_length;
		uint32_t value_length;

		/* \todo Obsługa opisu (tytułu, tematu) pokoju.
		 * Jeżeli klucz to "title", to w wartości mamy opis pokoju.
		 * Dodatkowo, w momencie zmiany opisu pokoju dostajemy pakiet 0x54.
		 */
		key_length = gg_tvbuff_read_uint32(tvb);
		gg_tvbuff_skip(tvb, key_length);

		value_length = gg_tvbuff_read_uint32(tvb);
		gg_tvbuff_skip(tvb, value_length);

		gg_tvbuff_expected_uint32(tvb, 2); /* unknown */
	}

	participants_count = gg_tvbuff_read_uint32(tvb);
	if (id == 0 && participants_count > 0) {
		gg_debug_session(gs, GG_DEBUG_MISC | GG_DEBUG_WARNING,
			"// gg_session_handle_chat_info() terminating packet "
			"shouldn't contain participants\n");
		participants_count = 0;
	}

	if (participants_count > 0) {
		participants = malloc(sizeof(uin_t) * participants_count);
		if (participants == NULL) {
			gg_tvbuff_close(tvb);
			return -1;
		}
	}

	for (i = 0; i < participants_count && gg_tvbuff_is_valid(tvb); i++) {
		participants[i] = gg_tvbuff_read_uint32(tvb);
		gg_tvbuff_read_uint32(tvb); /* 0x1e lub 0x18 */
	}

	if (!gg_tvbuff_close(tvb)) {
		free(participants);
		return -1;
	}

	if (id == 0) {
		ge->type = GG_EVENT_CHAT_INFO_GOT_ALL;
		return 0;
	}

	if (0 != gg_chat_update(gs, id, version, participants,
		participants_count))
	{
		free(participants);
		return -1;
	}

	ge->type = GG_EVENT_CHAT_INFO;
	ge->event.chat_info.id = id;
	ge->event.chat_info.version = version;
	ge->event.chat_info.participants_count = participants_count;
	ge->event.chat_info.participants = participants;

	return 0;
}

static int gg_session_handle_chat_info_update(struct gg_session *gs,
	uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	GG110ChatInfoUpdate *msg = gg110_chat_info_update__unpack(NULL, len, (uint8_t*)ptr);
	gg_chat_list_t *chat;
	uin_t participant;

	if (!GG_PROTOBUF_VALID(gs, "GG110ChatInfoUpdate", msg))
		return -1;

	gg_debug_session(gs, GG_DEBUG_VERBOSE,
		"// gg_session_handle_chat_info_update() "
		"msg_id=%016" PRIx64 " conv_id=%016" PRIx64 "\n",
		msg->msg_id, msg->conv_id);

	ge->type = GG_EVENT_CHAT_INFO_UPDATE;
	ge->event.chat_info_update.id = msg->chat_id;
	ge->event.chat_info_update.type = msg->update_type;
	ge->event.chat_info_update.participant = participant = gg_protobuf_get_uin(msg->participant);
	ge->event.chat_info_update.inviter = gg_protobuf_get_uin(msg->inviter);
	ge->event.chat_info_update.version = msg->version;
	ge->event.chat_info_update.time = msg->time;

	chat = gg_chat_find(gs, msg->chat_id);
	if (!chat) {
		gg110_chat_info_update__free_unpacked(msg, NULL);
		return 0;
	}

	chat->version = msg->version;
	if (msg->update_type == GG_CHAT_INFO_UPDATE_ENTERED) {
		uin_t *old_part = chat->participants;
		chat->participants = realloc(chat->participants,
			sizeof(uin_t) * (chat->participants_count + 1));
		if (chat->participants == NULL) {
			chat->participants = old_part;
			gg_debug_session(gs, GG_DEBUG_ERROR,
				"// gg_session_handle_chat_info_update() "
				"out of memory (count=%u)\n",
				chat->participants_count);
			gg110_chat_info_update__free_unpacked(msg, NULL);
			return -1;
		}
		chat->participants_count++;
		chat->participants[chat->participants_count - 1] = participant;
	} else if (msg->update_type == GG_CHAT_INFO_UPDATE_EXITED) {
		uint32_t idx;
		for (idx = 0; idx < chat->participants_count; idx++)
			if (chat->participants[idx] == participant)
				break;
		if (idx < chat->participants_count) {
			chat->participants_count--;
			if (chat->participants_count == 0) {
				free(chat->participants);
				chat->participants = NULL;
			} else {
				chat->participants[idx] =
					chat->participants[chat->participants_count];
				chat->participants = realloc(chat->participants,
					sizeof(uin_t)*chat->participants_count);
			}
		}
	}

	gg110_chat_info_update__free_unpacked(msg, NULL);
	return 0;
}

static int gg_session_handle_chat_created(struct gg_session *gs, uint32_t type,
	const char *ptr, size_t len, struct gg_event *ge)
{
	const struct gg_chat_created *p = (const struct gg_chat_created *)ptr;

	if (0 != gg_chat_update(gs, gg_fix64(p->id), 0, &gs->uin, 1))
		return -1;

	ge->type = GG_EVENT_CHAT_CREATED;
	ge->event.chat_created.id = gg_fix64(p->id);
	ge->event.chat_created.seq = gg_fix32(p->seq);
	return 0;
}

static int gg_session_handle_chat_invite_ack(struct gg_session *gs,
	uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	const struct gg_chat_invite_ack *p =
		(const struct gg_chat_invite_ack *)ptr;

	ge->type = GG_EVENT_CHAT_INVITE_ACK;
	ge->event.chat_invite_ack.id = gg_fix64(p->id);
	ge->event.chat_invite_ack.seq = gg_fix32(p->seq);

	return 0;
}

static int gg_session_handle_chat_left(struct gg_session *gs, uint32_t type,
	const char *ptr, size_t len, struct gg_event *ge)
{
	const struct gg_chat_left *p = (const struct gg_chat_left *)ptr;

	ge->type = GG_EVENT_CHAT_INFO_UPDATE;
	ge->event.chat_info_update.id = gg_fix64(p->id);
	ge->event.chat_info_update.type = GG_CHAT_INFO_UPDATE_EXITED;
	/* Właściwie, to nie wiadomo, czy to jest "osoba wychodząca", czy
	 * "osoba wyrzucająca nas" z konferencji. */
	ge->event.chat_info_update.participant = gg_fix32(p->uin);
	ge->event.chat_info_update.inviter = gg_fix32(p->uin);
	ge->event.chat_info_update.version = 0;
	ge->event.chat_info_update.time = time(NULL);

	return 0;
}

static int gg_session_handle_options(struct gg_session *gs, uint32_t type,
	const char *ptr, size_t len, struct gg_event *ge)
{
	GG110Options *msg = gg110_options__unpack(NULL, len, (uint8_t*)ptr);
	size_t i;

	if (!GG_PROTOBUF_VALID(gs, "GG110Options", msg))
		return -1;

	gg_protobuf_expected(gs, "GG110Options.dummy1", msg->dummy1, 0);

	for (i = 0; i < msg->n_options; i++) {
		ProtobufKVP *kvp = msg->options[i];
		if (!GG_PROTOBUF_VALID(gs, "ProtobufKVP", kvp))
			continue;
		gg_debug_session(gs, GG_DEBUG_MISC,
			"// gg_session_handle_options[%s] = \"%s\"\n",
			kvp->key, kvp->value);
	}

	gg110_options__free_unpacked(msg, NULL);

	return 0;
}

static int gg_session_handle_access_info(struct gg_session *gs, uint32_t type,
	const char *ptr, size_t len, struct gg_event *ge)
{
	GG110AccessInfo *msg = gg110_access_info__unpack(NULL, len, (uint8_t*)ptr);

	if (!GG_PROTOBUF_VALID(gs, "GG110AccessInfo", msg))
		return -1;

	gg_debug_session(gs, GG_DEBUG_MISC,
		"// gg_session_handle_access_info: dummy[%02x, %02x], "
		"last[message=%u, file_transfer=%u, conference_ch=%u]\n",
		msg->dummy1, msg->dummy2, msg->last_message,
		msg->last_file_transfer, msg->last_conference_ch);

	gg110_access_info__free_unpacked(msg, NULL);

	return 0;
}

/* ten pakiet jest odbierany tylko, jeżeli przy logowaniu użyliśmy identyfikatora typu 0x01 */
static int gg_session_handle_uin_info(struct gg_session *gs, uint32_t type,
	const char *ptr, size_t len, struct gg_event *ge)
{
	gg_tvbuff_t *tvb;
	char *uin1 = NULL, *uin2 = NULL;

	tvb = gg_tvbuff_new(ptr, len);

	gg_tvbuff_expected_uint32(tvb, 1); /* unknown */
	gg_tvbuff_expected_uint32(tvb, 2); /* unknown */

	/* podstawowy identyfikator (numer GG) */
	gg_tvbuff_expected_uint8(tvb, 0);
	gg_tvbuff_read_str_dup(tvb, &uin1);

	/* identyfikator użyty przy logowaniu (numer GG lub email) */
	gg_tvbuff_expected_uint8(tvb, 1);
	gg_tvbuff_read_str_dup(tvb, &uin2);

	if (!gg_tvbuff_close(tvb)) {
		free(uin1);
		free(uin2);
		return -1;
	}

	gg_debug_session(gs, GG_DEBUG_MISC, "// gg_session_handle_uin_info: "
		"uin1=\"%s\", uin2=\"%s\"\n", uin1, uin2);

	free(uin1);
	free(uin2);

	return 0;
}

static int gg_session_handle_transfer_info(struct gg_session *gs, uint32_t type,
	const char *ptr, size_t len, struct gg_event *ge)
{
	GG112TransferInfo *msg = gg112_transfer_info__unpack(NULL, len, (uint8_t*)ptr);
	int succ = 1;
	size_t i;
	uin_t peer = 0, sender = 0;

	if (!GG_PROTOBUF_VALID(gs, "GG112TransferInfo", msg))
		return -1;

	/* see packets.proto */
	if (msg->dummy1 != 5 && msg->dummy1 != 6) {
		gg_debug_session(gs, GG_DEBUG_MISC | GG_DEBUG_WARNING,
			"// gg_session_handle_transfer_info: "
			"unknown dummy1 value: %d\n", msg->dummy1);
	}

	if (GG_PROTOBUF_VALID(gs, "GG112TransferInfoUin", msg->peer)) {
		gg_protobuf_expected(gs, "GG112TransferInfoUin.dummy1",
			msg->peer->dummy1, 1);
		peer = gg_protobuf_get_uin(msg->peer->uin);
	}
	if (GG_PROTOBUF_VALID(gs, "GG112TransferInfoUin", msg->sender)) {
		gg_protobuf_expected(gs, "GG112TransferInfoUin.dummy1",
			msg->sender->dummy1, 1);
		sender = gg_protobuf_get_uin(msg->sender->uin);
	}

	gg_debug_session(gs, GG_DEBUG_MISC,
		"// gg_session_handle_transfer_info: dummy1=%#x, time=%u, "
			"sender=%u, peer=%u, msg_id=%#016" PRIx64 ", "
			"conv_id=%#016" PRIx64 "\n",
		msg->dummy1, msg->time, sender, peer, msg->msg_id,
		msg->conv_id);

	for (i = 0; i < msg->n_data; i++) {
		ProtobufKVP *kvp = msg->data[i];
		if (!GG_PROTOBUF_VALID(gs, "ProtobufKVP", kvp))
			continue;
		gg_debug_session(gs, GG_DEBUG_MISC,
			"// gg_session_handle_transfer_info[%s] = \"%s\"\n",
			kvp->key, kvp->value);
	}

	if (msg->file && GG_PROTOBUF_VALID(gs, "GG112TransferInfoFile", msg->file)) {
		GG112TransferInfoFile *file = msg->file;
		gg_debug_session(gs, GG_DEBUG_MISC,
			"// gg_session_handle_transfer_info file: "
			"type=\"%s\", content_type=\"%s\", filename=\"%s\", "
			"filesize=%u, msg_id=%#016" PRIx64 " url=\"%s\"\n",
			file->type, file->content_type, file->filename,
			file->filesize, file->msg_id, file->url);
	}

	succ = (gg_ack_110(gs, GG110_ACK__TYPE__TRANSFER_INFO,
		msg->seq, ge) == 0);

	gg112_transfer_info__free_unpacked(msg, NULL);

	return succ ? 0 : -1;
}

static int gg_session_handle_magic_notification(struct gg_session *gs, uint32_t type,
	const char *ptr, size_t len, struct gg_event *ge)
{
	GG110MagicNotification *msg = gg110_magic_notification__unpack(NULL, len, (uint8_t*)ptr);
	int succ = 1;

	if (!GG_PROTOBUF_VALID(gs, "GG110MagicNotification", msg))
		return -1;

	gg_debug_session(gs, GG_DEBUG_MISC,
		"// gg_session_handle_magic_notification \n");

	gg_protobuf_expected(gs, "GG110MagicNotification.dummy1", msg->dummy1, 2);
	gg_protobuf_expected(gs, "GG110MagicNotification.dummy2", msg->dummy2, 1);
	gg_protobuf_expected(gs, "GG110MagicNotification.dummy3", msg->dummy3, 1);

	succ = (gg_ack_110(gs, GG110_ACK__TYPE__MAGIC_NOTIFICATION, msg->seq, ge) == 0);

	gg110_magic_notification__free_unpacked(msg, NULL);

	return succ ? 0 : -1;
}

/**
 * \internal Tablica obsługiwanych pakietów
 */
static const gg_packet_handler_t handlers[] =
{
	/* style:maxlinelength:start-ignore */
	{ GG_WELCOME, GG_STATE_READING_KEY, 0, gg_session_handle_welcome },
	{ GG_LOGIN_OK, GG_STATE_READING_REPLY, 0, gg_session_handle_login_ok },
	{ GG_LOGIN80_OK, GG_STATE_READING_REPLY, 0, gg_session_handle_login_ok },
	{ GG_LOGIN110_OK, GG_STATE_READING_REPLY, 0, gg_session_handle_login110_ok },
	{ GG_NEED_EMAIL, GG_STATE_READING_REPLY, 0, gg_session_handle_login_ok },
	{ GG_LOGIN_FAILED, GG_STATE_READING_REPLY, 0, gg_session_handle_login_failed },
	{ GG_LOGIN80_FAILED, GG_STATE_READING_REPLY, 0, gg_session_handle_login_failed },
	{ GG_SEND_MSG_ACK, GG_STATE_CONNECTED, sizeof(struct gg_send_msg_ack), gg_session_handle_send_msg_ack },
	{ GG_SEND_MSG_ACK110, GG_STATE_CONNECTED, 0, gg_session_handle_send_msg_ack_110 },
	{ GG_PONG, GG_STATE_CONNECTED, 0, gg_session_handle_pong },
	{ GG_DISCONNECTING, GG_STATE_CONNECTED, 0, gg_session_handle_disconnecting },
	{ GG_DISCONNECT_ACK, GG_STATE_DISCONNECTING, 0, gg_session_handle_disconnect_ack },
	{ GG_XML_EVENT, GG_STATE_CONNECTED, 0, gg_session_handle_xml_event },
	{ GG_EVENT110, GG_STATE_CONNECTED, 0, gg_session_handle_event_110 },
	{ GG_PUBDIR50_REPLY, GG_STATE_CONNECTED, 0, gg_session_handle_pubdir50_reply },
	{ GG_USERLIST_REPLY, GG_STATE_CONNECTED, sizeof(char), gg_session_handle_userlist_reply },
	{ GG_DCC7_ID_REPLY, GG_STATE_CONNECTED, sizeof(struct gg_dcc7_id_reply), gg_session_handle_dcc7_id_reply },
	{ GG_DCC7_ACCEPT, GG_STATE_CONNECTED, sizeof(struct gg_dcc7_accept), gg_session_handle_dcc7_accept },
	{ GG_DCC7_NEW, GG_STATE_CONNECTED, sizeof(struct gg_dcc7_new), gg_session_handle_dcc7_new },
	{ GG_DCC7_REJECT, GG_STATE_CONNECTED, sizeof(struct gg_dcc7_reject), gg_session_handle_dcc7_reject },
	{ GG_DCC7_INFO, GG_STATE_CONNECTED, sizeof(struct gg_dcc7_info), gg_session_handle_dcc7_info },
	{ GG_RECV_MSG, GG_STATE_CONNECTED, sizeof(struct gg_recv_msg), gg_session_handle_recv_msg },
	{ GG_RECV_MSG80, GG_STATE_CONNECTED, sizeof(struct gg_recv_msg80), gg_session_handle_recv_msg_80 },
	{ GG_RECV_MSG110, GG_STATE_CONNECTED, 0, gg_session_handle_recv_msg_110 },
	{ GG_RECV_OWN_MSG110, GG_STATE_CONNECTED, 0, gg_session_handle_recv_msg_110 },
	{ GG_STATUS, GG_STATE_CONNECTED, sizeof(struct gg_status), gg_session_handle_status },
	{ GG_STATUS60, GG_STATE_CONNECTED, sizeof(struct gg_status60), gg_session_handle_status_60_77_80beta },
	{ GG_STATUS77, GG_STATE_CONNECTED, sizeof(struct gg_status77), gg_session_handle_status_60_77_80beta },
	{ GG_STATUS80BETA, GG_STATE_CONNECTED, sizeof(struct gg_status77), gg_session_handle_status_60_77_80beta },
	{ GG_STATUS80, GG_STATE_CONNECTED, sizeof(struct gg_notify_reply80), gg_session_handle_status_80 },
	{ GG_NOTIFY_REPLY, GG_STATE_CONNECTED, sizeof(struct gg_notify_reply), gg_session_handle_notify_reply },
	{ GG_NOTIFY_REPLY60, GG_STATE_CONNECTED, sizeof(struct gg_notify_reply60), gg_session_handle_notify_reply_60 },
	{ GG_NOTIFY_REPLY77, GG_STATE_CONNECTED, sizeof(struct gg_notify_reply77), gg_session_handle_notify_reply_77_80beta },
	{ GG_NOTIFY_REPLY80BETA, GG_STATE_CONNECTED, sizeof(struct gg_notify_reply77), gg_session_handle_notify_reply_77_80beta },
	{ GG_NOTIFY_REPLY80, GG_STATE_CONNECTED, sizeof(struct gg_notify_reply80), gg_session_handle_notify_reply_80 },
	{ GG_USER_DATA, GG_STATE_CONNECTED, sizeof(struct gg_user_data), gg_session_handle_user_data },
	{ GG_TYPING_NOTIFICATION, GG_STATE_CONNECTED, sizeof(struct gg_typing_notification), gg_session_handle_typing_notification },
	{ GG_MULTILOGON_INFO, GG_STATE_CONNECTED, sizeof(struct gg_multilogon_info), gg_session_handle_multilogon_info },
	{ GG_XML_ACTION, GG_STATE_CONNECTED, 0, gg_session_handle_xml_event },
	{ GG_RECV_OWN_MSG, GG_STATE_CONNECTED, sizeof(struct gg_recv_msg80), gg_session_handle_recv_msg_80 },
	{ GG_USERLIST100_VERSION, GG_STATE_CONNECTED, sizeof(struct gg_userlist100_version), gg_session_handle_userlist_100_version },
	{ GG_USERLIST100_REPLY, GG_STATE_CONNECTED, sizeof(struct gg_userlist100_reply), gg_session_handle_userlist_100_reply },
	{ GG_IMTOKEN, GG_STATE_CONNECTED, 0, gg_session_handle_imtoken },
	{ GG_PONG110, GG_STATE_CONNECTED, 0, gg_session_handle_pong_110 },
	{ GG_CHAT_INFO, GG_STATE_CONNECTED, 0, gg_session_handle_chat_info },
	{ GG_CHAT_INFO_UPDATE, GG_STATE_CONNECTED, 0, gg_session_handle_chat_info_update },
	{ GG_CHAT_CREATED, GG_STATE_CONNECTED, sizeof(struct gg_chat_created), gg_session_handle_chat_created },
	{ GG_CHAT_INVITE_ACK, GG_STATE_CONNECTED, sizeof(struct gg_chat_invite_ack), gg_session_handle_chat_invite_ack },
	{ GG_CHAT_RECV_MSG, GG_STATE_CONNECTED, 0, gg_session_handle_recv_msg_110 },
	{ GG_CHAT_RECV_OWN_MSG, GG_STATE_CONNECTED, 0, gg_session_handle_recv_msg_110 },
	{ GG_CHAT_LEFT, GG_STATE_CONNECTED, sizeof(struct gg_chat_left), gg_session_handle_chat_left },
	{ GG_OPTIONS, GG_STATE_CONNECTED, 0, gg_session_handle_options },
	{ GG_ACCESS_INFO, GG_STATE_CONNECTED, 0, gg_session_handle_access_info },
	{ GG_UIN_INFO, GG_STATE_CONNECTED, 0, gg_session_handle_uin_info },
	{ GG_TRANSFER_INFO, GG_STATE_CONNECTED, 0, gg_session_handle_transfer_info },
	{ GG_MAGIC_NOTIFICATION, GG_STATE_CONNECTED, 0, gg_session_handle_magic_notification }
	/* style:maxlinelength:end-ignore */
};

/**
 * \internal Obsługuje przychodzący pakiet danych.
 *
 * \param gs Struktura sesji
 * \param type Typ pakietu
 * \param ptr Wskaźnik do bufora pakietu
 * \param len Długość bufora pakietu
 * \param[out] ge Struktura zdarzenia
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 */
int gg_session_handle_packet(struct gg_session *gs, uint32_t type, const char *ptr, size_t len, struct gg_event *ge)
{
	unsigned int i;

	gg_debug_session(gs, GG_DEBUG_FUNCTION,
		"// gg_session_handle_packet(%d, %p, %" GG_SIZE_FMT ")\n",
		type, ptr, len);

	gs->last_event = time(NULL);

#if 0
	if ((gs->flags & (1 << GG_SESSION_FLAG_RAW_PACKET)) != 0) {
		char *tmp;

		tmp = malloc(len);

		if (tmp == NULL) {
			gg_debug_session(gs, GG_DEBUG_ERROR,
				"// gg_session_handle_packet() out of memory "
				"(%d bytes)\n", len);
			return -1;
		}

		memcpy(tmp, ptr, len);

		ge->type = GG_EVENT_RAW_PACKET;
		ge->event.raw_packet.type = type;
		ge->event.raw_packet.length = len;
		ge->event.raw_packet.data = tmp;

		return 0;
	}
#endif

	for (i = 0; i < sizeof(handlers) / sizeof(handlers[0]); i++) {
		if (handlers[i].type != 0 && handlers[i].type != type)
			continue;

		if (handlers[i].state != 0 && handlers[i].state != (enum gg_state_t) gs->state) {
			gg_debug_session(gs, GG_DEBUG_WARNING,
				"// gg_session_handle_packet() packet 0x%02x "
				"unexpected in state %d\n", type, gs->state);
			continue;
		}

		if (len < handlers[i].min_length) {
			gg_debug_session(gs, GG_DEBUG_ERROR,
				"// gg_session_handle_packet() packet 0x%02x "
				"too short (%" GG_SIZE_FMT " bytes)\n",
				type, len);
			continue;
		}

		return (*handlers[i].handler)(gs, type, ptr, len, ge);
	}

	gg_debug_session(gs, GG_DEBUG_WARNING, "// gg_session_handle_packet() "
		"unhandled packet 0x%02x, len %" GG_SIZE_FMT ", state %d\n",
		type, len, gs->state);

	return 0;
}
