/* coding: UTF-8 */
/* $Id: dcc7.c,v 1.2 2007-07-20 23:00:49 wojtekka Exp $ */

/*
 *  (C) Copyright 2001-2010 Wojtek Kaniewski <wojtekka@irc.pl>
 *                          Tomasz Chiliński <chilek@chilan.com>
 *                          Adam Wysocki <gophi@ekg.chmurka.net>
 *                          Bartłomiej Zimoń <uzi18@o2.pl>
 *  
 *  Thanks to Jakub Zawadzki <darkjames@darkjames.ath.cx>
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
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110,
 *  USA.
 */

/**
 * \file dcc7.c
 *
 * \brief Obsługa połączeń bezpośrednich od wersji Gadu-Gadu 7.x
 */

#ifndef _WIN64
#define _USE_32BIT_TIME_T
#endif

#include <sys/types.h>
#include <sys/stat.h>
#ifdef _WIN32
#include "win32.h"
#else
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#ifdef sun
#  include <sys/filio.h>
#endif
#endif /* _WIN32 */
#include <time.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef _WIN32
#include <unistd.h>
#endif

#include "compat.h"
#include "libgadu.h"
#include "protocol.h"
#include "resolver.h"
#include "internal.h"

/**
 * \internal Dodaje połączenie bezpośrednie do sesji.
 *
 * \param sess Struktura sesji
 * \param dcc Struktura połączenia
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 */
static int gg_dcc7_session_add(struct gg_session *sess, struct gg_dcc7 *dcc)
{
	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_dcc7_session_add(%p, %p)\n", sess, dcc);

	if (!sess || !dcc || dcc->next) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_dcc7_session_add() invalid parameters\n");
		errno = EINVAL;
		return -1;
	}

	dcc->next = sess->dcc7_list;
	sess->dcc7_list = dcc;

	return 0;
}

/**
 * \internal Usuwa połączenie bezpośrednie z sesji.
 *
 * \param sess Struktura sesji
 * \param dcc Struktura połączenia
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 */
static int gg_dcc7_session_remove(struct gg_session *sess, struct gg_dcc7 *dcc)
{
	struct gg_dcc7 *tmp;

	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_dcc7_session_remove(%p, %p)\n", sess, dcc);

	if (sess == NULL || dcc == NULL) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_dcc7_session_remove() invalid parameters\n");
		errno = EINVAL;
		return -1;
	}

	if (sess->dcc7_list == dcc) {
		sess->dcc7_list = dcc->next;
		dcc->next = NULL;
		return 0;
	}

	for (tmp = sess->dcc7_list; tmp != NULL; tmp = tmp->next) {
		if (tmp->next == dcc) {
			tmp->next = dcc->next;
			dcc->next = NULL;
			return 0;
		}
	}

	errno = ENOENT;
	return -1;
}

/**
 * \internal Zwraca strukturę połączenia o danym identyfikatorze.
 *
 * \param sess Struktura sesji
 * \param id Identyfikator połączenia
 * \param uin Numer nadawcy lub odbiorcy
 *
 * \return Struktura połączenia lub \c NULL jeśli nie znaleziono
 */
static struct gg_dcc7 *gg_dcc7_session_find(struct gg_session *sess, gg_dcc7_id_t id, uin_t uin)
{
	struct gg_dcc7 *tmp;
	int empty;

	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_dcc7_session_find(%p, ..., %d)\n", sess, (int) uin);

	empty = !memcmp(&id, "\0\0\0\0\0\0\0\0", 8);

	for (tmp = sess->dcc7_list; tmp; tmp = tmp->next) {
		if (empty) {
			if (tmp->peer_uin == uin && tmp->state != GG_STATE_WAITING_FOR_ACCEPT)
				return tmp;
		} else {
			if (!memcmp(&tmp->cid, &id, sizeof(id)))
				return tmp;
		}
	}

	return NULL;
}

/**
 * \internal Rozpoczyna proces pobierania adresu
 *
 * \param dcc Struktura połączenia
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 */
static int gg_dcc7_get_relay_addr(struct gg_dcc7 *dcc)
{
	gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_FUNCTION, "** gg_dcc7_get_relay_addr(%p)\n", dcc);

	if (dcc == NULL || dcc->sess == NULL) {
		gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_get_relay_addr() invalid parameters\n");
		errno = EINVAL;
		return -1;
	}

	if (dcc->sess->resolver_start(&dcc->fd, &dcc->resolver, GG_RELAY_HOST) == -1) {
		gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_get_relay_addr() resolving failed (errno=%d, %s)\n", errno, strerror(errno));
		return -1;
	}

	dcc->state = GG_STATE_RESOLVING_RELAY;
	dcc->check = GG_CHECK_READ;
	dcc->timeout = GG_DEFAULT_TIMEOUT;

	return 0;
}

/**
 * \internal Nawiązuje połączenie bezpośrednie
 *
 * \param dcc Struktura połączenia
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 */
static int gg_dcc7_connect(struct gg_dcc7 *dcc)
{
	gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_FUNCTION, "** gg_dcc7_connect(%p)\n", dcc);

	if (dcc == NULL) {
		gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_connect() invalid parameters\n");
		errno = EINVAL;
		return -1;
	}

	if ((dcc->fd = gg_connect(&dcc->remote_addr, dcc->remote_port, 1)) == -1) {
		gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_connect() connection failed\n");
		return -1;
	}

	dcc->state = GG_STATE_CONNECTING;
	dcc->check = GG_CHECK_WRITE;
	dcc->timeout = GG_DCC7_TIMEOUT_CONNECT;
	dcc->soft_timeout = 1;

	return 0;
}

/**
 * \internal Tworzy gniazdo nasłuchujące dla połączenia bezpośredniego
 *
 * \param dcc Struktura połączenia
 * \param port Preferowany port (jeśli równy 0 lub -1, próbuje się domyślnego)
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 */
static int gg_dcc7_listen(struct gg_dcc7 *dcc, uint16_t port)
{
	struct sockaddr_in sin;
	SOCKET fd;

	gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_FUNCTION, "** gg_dcc7_listen(%p, %d)\n", dcc, port);

	if (!dcc) {
		gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_listen() invalid parameters\n");
		errno = EINVAL;
		return -1;
	}

	if ((fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_listen() can't create socket (%s)\n", strerror(errno));
		return -1;
	}

	// XXX losować porty?
	
	if (!port)
		port = GG_DEFAULT_DCC_PORT;

	while (1) {
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = INADDR_ANY;
		sin.sin_port = htons(port);

		gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_listen() trying port %d\n", port);

		if (!bind(fd, (struct sockaddr*) &sin, sizeof(sin)))
			break;

		if (port++ == 65535) {
			gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_listen() no free port found\n");
			gg_sock_close(fd);
			errno = ENOENT;
			return -1;
		}
	}

	if (listen(fd, 1)) {
		int errsv = errno;
		gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_listen() unable to listen (%s)\n", strerror(errno));
		gg_sock_close(fd);
		errno = errsv;
		return -1;
	}

	dcc->fd = fd;
	dcc->local_port = port;
	
	dcc->state = GG_STATE_LISTENING;
	dcc->check = GG_CHECK_READ;
	dcc->timeout = GG_DCC7_TIMEOUT_FILE_ACK;

	return 0;
}

/**
 * \internal Tworzy gniazdo nasłuchujące i wysyła jego parametry
 *
 * \param dcc Struktura połączenia
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 */
static int gg_dcc7_listen_and_send_info(struct gg_dcc7 *dcc)
{
	struct gg_dcc7_info pkt;
	uint16_t external_port;
	uint16_t local_port;
	uint32_t count;

	gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_FUNCTION, "** gg_dcc7_listen_and_send_info(%p)\n", dcc);
	if (dcc == NULL)
		return -1;

	if (!dcc->sess->client_port)
		local_port = dcc->sess->external_port;
	else
		local_port = dcc->sess->client_port;

	if (gg_dcc7_listen(dcc, local_port) == -1)
		return -1;
	
	if (!dcc->sess->external_port || dcc->local_port != local_port)
		external_port = dcc->local_port;
	else
		external_port = dcc->sess->external_port;

	if (!dcc->sess->external_addr || dcc->local_port != local_port)
		dcc->local_addr = dcc->sess->client_addr;
	else
		dcc->local_addr = dcc->sess->external_addr;

	gg_debug_session((dcc)->sess, GG_DEBUG_MISC, "// dcc7_listen_and_send_info() sending IP address %s and port %d\n", inet_ntoa(*((struct in_addr*) &dcc->local_addr)), external_port);

	memset(&pkt, 0, sizeof(pkt));
	pkt.uin = gg_fix32(dcc->peer_uin);
	pkt.type = GG_DCC7_TYPE_P2P;
	pkt.id = dcc->cid;
	snprintf((char*) pkt.info, sizeof(pkt.info), "%s %d", inet_ntoa(*((struct in_addr*) &dcc->local_addr)), external_port);
	// TODO: implement hash count
	// we MUST fill hash to recive from server request for server connection
	//snprintf((char*) pkt.hash, sizeof(pkt.hash), "0");
	count = dcc->local_addr + external_port * rand();
	mir_snprintf(pkt.hash, sizeof(pkt.hash), "%d", count);

	return gg_send_packet(dcc->sess, GG_DCC7_INFO, &pkt, sizeof(pkt), NULL);
}

/**
 * \internal Odwraca połączenie po nieudanym connect()
 *
 * \param dcc Struktura połączenia
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 */
static int gg_dcc7_reverse_connect(struct gg_dcc7 *dcc)
{
	gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_FUNCTION, "** gg_dcc7_reverse_connect(%p)\n", dcc);

	if (dcc->reverse) {
		gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_reverse_connect() already reverse connection\n");
		return -1;
	}

	gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_reverse_connect() timeout, trying reverse connection\n");
	gg_sock_close(dcc->fd);
	dcc->fd = -1;
	dcc->reverse = 1;

	return gg_dcc7_listen_and_send_info(dcc);
}

/**
 * \internal Wysyła do serwera żądanie nadania identyfikatora sesji
 *
 * \param sess Struktura sesji
 * \param type Rodzaj połączenia (\c GG_DCC7_TYPE_FILE lub \c GG_DCC7_TYPE_VOICE)
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 */
static int gg_dcc7_request_id(struct gg_session *sess, uint32_t type)
{
	struct gg_dcc7_id_request pkt;

	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_dcc7_request_id(%p, %d)\n", sess, type);

	if (!sess) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_dcc7_request_id() invalid parameters\n");
		errno = EFAULT;
		return -1;
	}

	if (sess->state != GG_STATE_CONNECTED) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_dcc7_request_id() not connected\n");
		errno = ENOTCONN;
		return -1;
	}

	if (type != GG_DCC7_TYPE_VOICE && type != GG_DCC7_TYPE_FILE) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_dcc7_request_id() invalid transfer type (%d)\n", type);
		errno = EINVAL;
		return -1;
	}
	
	memset(&pkt, 0, sizeof(pkt));
	pkt.type = gg_fix32(type);

	return gg_send_packet(sess, GG_DCC7_ID_REQUEST, &pkt, sizeof(pkt), NULL);
}

/**
 * \internal Rozpoczyna wysyłanie pliku.
 *
 * Funkcja jest wykorzystywana przez \c gg_dcc7_send_file() oraz
 * \c gg_dcc_send_file_fd().
 *
 * \param sess Struktura sesji
 * \param rcpt Numer odbiorcy
 * \param fd Deskryptor pliku
 * \param size Rozmiar pliku
 * \param filename1250 Nazwa pliku w kodowaniu CP-1250
 * \param hash Skrót SHA-1 pliku
 * \param seek Flaga mówiąca, czy można używać lseek()
 *
 * \return Struktura \c gg_dcc7 lub \c NULL w przypadku błędu
 *
 * \ingroup dcc7
 */
static struct gg_dcc7 *gg_dcc7_send_file_common(struct gg_session *sess, uin_t rcpt, int fd, size_t size, const char *filename1250, const char *hash, int seek)
{
	struct gg_dcc7 *dcc = NULL;

	if (!sess || !rcpt || !filename1250 || !hash || fd == -1) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_dcc7_send_file_common() invalid parameters\n");
		errno = EINVAL;
		goto fail;
	}

	if (!(dcc = (gg_dcc7*)malloc(sizeof(struct gg_dcc7)))) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_dcc7_send_file_common() not enough memory\n");
		goto fail;
	}

	if (gg_dcc7_request_id(sess, GG_DCC7_TYPE_FILE) == -1)
		goto fail;

	memset(dcc, 0, sizeof(struct gg_dcc7));
	dcc->type = GG_SESSION_DCC7_SEND;
	dcc->dcc_type = GG_DCC7_TYPE_FILE;
	dcc->state = GG_STATE_REQUESTING_ID;
	dcc->timeout = GG_DEFAULT_TIMEOUT;
	dcc->sess = sess;
	dcc->fd = -1;
	dcc->uin = sess->uin;
	dcc->peer_uin = rcpt;
	dcc->file_fd = fd;
	dcc->size = (unsigned int)size;
	dcc->seek = seek;

	strncpy((char*) dcc->filename, filename1250, GG_DCC7_FILENAME_LEN - 1);
	dcc->filename[GG_DCC7_FILENAME_LEN] = 0;

	memcpy(dcc->hash, hash, GG_DCC7_HASH_LEN);

	if (gg_dcc7_session_add(sess, dcc) == -1)
		goto fail;

	return dcc;

fail:
	free(dcc);
	return NULL;
}

/**
 * Rozpoczyna wysyłanie pliku o danej nazwie.
 *
 * \param sess Struktura sesji
 * \param rcpt Numer odbiorcy
 * \param filename Nazwa pliku w lokalnym systemie plików
 * \param filename1250 Nazwa pliku w kodowaniu CP-1250
 * \param hash Skrót SHA-1 pliku (lub \c NULL jeśli ma być wyznaczony)
 *
 * \return Struktura \c gg_dcc7 lub \c NULL w przypadku błędu
 *
 * \ingroup dcc7
 */
struct gg_dcc7 *gg_dcc7_send_file(struct gg_session *sess, uin_t rcpt, const char *filename, const char *filename1250, const char *hash)
{
	struct gg_dcc7 *dcc = NULL;
	const char *tmp;
	char hash_buf[GG_DCC7_HASH_LEN];
	struct stat st;
	int fd = -1;

	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_dcc7_send_file(%p, %d, \"%s\", %p)\n", sess, rcpt, filename, hash);

	if (!sess || !rcpt || !filename) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_dcc7_send_file() invalid parameters\n");
		errno = EINVAL;
		goto fail;
	}

	if (!filename1250)
		filename1250 = filename;

	if (stat(filename, &st) == -1) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_dcc7_send_file() stat() failed (%s)\n", strerror(errno));
		goto fail;
	}

	if ((st.st_mode & S_IFDIR)) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_dcc7_send_file() that's a directory\n");
		errno = EINVAL;
		goto fail;
	}

	if ((fd = open(filename, O_RDONLY | O_BINARY)) == -1) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_dcc7_send_file() open() failed (%s)\n", strerror(errno));
		goto fail;
	}

	if (!hash) {
		if (gg_file_hash_sha1(fd, (uint8_t*) hash_buf) == -1)
			goto fail;

		hash = hash_buf;
	}

#ifdef _WIN32
	if ((tmp = strrchr(filename1250, '\\')))
#else
	if ((tmp = strrchr(filename1250, '/')))
#endif
		filename1250 = tmp + 1;

	if (!(dcc = gg_dcc7_send_file_common(sess, rcpt, fd, st.st_size, filename1250, hash, 1)))
		goto fail;

	return dcc;

fail:
	if (fd != -1) {
		int errsv = errno;
		close(fd);
		errno = errsv;
	}

	free(dcc);
	return NULL;
}

/**
 * \internal Rozpoczyna wysyłanie pliku o danym deskryptorze.
 *
 * \note Wysyłanie pliku nie będzie działać poprawnie, jeśli deskryptor
 * źródłowy jest w trybie nieblokującym i w pewnym momencie zabraknie danych.
 *
 * \param sess Struktura sesji
 * \param rcpt Numer odbiorcy
 * \param fd Deskryptor pliku
 * \param size Rozmiar pliku
 * \param filename1250 Nazwa pliku w kodowaniu CP-1250
 * \param hash Skrót SHA-1 pliku
 *
 * \return Struktura \c gg_dcc7 lub \c NULL w przypadku błędu
 *
 * \ingroup dcc7
 */
struct gg_dcc7 *gg_dcc7_send_file_fd(struct gg_session *sess, uin_t rcpt, int fd, size_t size, const char *filename1250, const char *hash)
{
	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_dcc7_send_file_fd(%p, %d, %d, %u, \"%s\", %p)\n", sess, rcpt, fd, size, filename1250, hash);

	return gg_dcc7_send_file_common(sess, rcpt, fd, size, filename1250, hash, 0);
}


/**
 * Potwierdza chęć odebrania pliku.
 *
 * \param dcc Struktura połączenia
 * \param offset Początkowy offset przy wznawianiu przesyłania pliku
 *
 * \note Biblioteka nie zmienia położenia w odbieranych plikach. Jeśli offset
 * początkowy jest różny od zera, należy ustawić go funkcją \c lseek() lub
 * podobną.
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 *
 * \ingroup dcc7
 */
int gg_dcc7_accept(struct gg_dcc7 *dcc, unsigned int offset)
{
	struct gg_dcc7_accept pkt;

	gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_FUNCTION, "** gg_dcc7_accept(%p, %d)\n", dcc, offset);

	if (!dcc || !dcc->sess) {
		gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_accept() invalid parameters\n");
		errno = EFAULT;
		return -1;
	}

	memset(&pkt, 0, sizeof(pkt));
	pkt.uin = gg_fix32(dcc->peer_uin);
	pkt.id = dcc->cid;
	pkt.offset = gg_fix32(offset);

	if (gg_send_packet(dcc->sess, GG_DCC7_ACCEPT, &pkt, sizeof(pkt), NULL) == -1)
		return -1;

	dcc->offset = offset;

	return gg_dcc7_listen_and_send_info(dcc);
}

/**
 * Odrzuca próbę przesłania pliku.
 *
 * \param dcc Struktura połączenia
 * \param reason Powód odrzucenia
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 *
 * \ingroup dcc7
 */
int gg_dcc7_reject(struct gg_dcc7 *dcc, int reason)
{
	struct gg_dcc7_reject pkt;

	gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_FUNCTION, "** gg_dcc7_reject(%p, %d)\n", dcc, reason);

	if (!dcc || !dcc->sess) {
		gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_reject() invalid parameters\n");
		errno = EFAULT;
		return -1;
	}

	memset(&pkt, 0, sizeof(pkt));
	pkt.uin = gg_fix32(dcc->peer_uin);
	pkt.id = dcc->cid;
	pkt.reason = gg_fix32(reason);

	return gg_send_packet(dcc->sess, GG_DCC7_REJECT, &pkt, sizeof(pkt), NULL);
}

/**
 * Przerwanie żądania przesłania pliku.
 *
 * \param dcc Struktura połączenia
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 *
 * \ingroup dcc7
 */
int gg_dcc7_abort(struct gg_dcc7 *dcc)
{
	struct gg_dcc7_abort pkt;

	gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_FUNCTION, "** gg_dcc7_abort(%p)\n", dcc);

	if (!dcc || !dcc->sess) {
		gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_abort() invalid parameters\n");
		errno = EFAULT;
		return -1;
	}

	memset(&pkt, 0, sizeof(pkt));
	pkt.id = dcc->cid;
	pkt.uin_from = gg_fix32(dcc->uin);
	pkt.uin_to = gg_fix32(dcc->peer_uin);

	return gg_send_packet(dcc->sess, GG_DCC7_ABORT, &pkt, sizeof(pkt), NULL);
}

/**
 * \internal Obsługuje pakiet identyfikatora połączenia bezpośredniego.
 *
 * \param sess Struktura sesji
 * \param e Struktura zdarzenia
 * \param payload Treść pakietu
 * \param len Długość pakietu
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 */
int gg_dcc7_handle_id(struct gg_session *sess, struct gg_event *e, void *payload, int len)
{
	struct gg_dcc7_id_reply *p = (gg_dcc7_id_reply*)payload;
	struct gg_dcc7 *tmp;

	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_dcc7_handle_id(%p, %p, %p, %d)\n", sess, e, payload, len);

	for (tmp = sess->dcc7_list; tmp; tmp = tmp->next) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// checking dcc %p, state %d, type %d\n", tmp, tmp->state, tmp->dcc_type);

		if (tmp->state != GG_STATE_REQUESTING_ID || tmp->dcc_type != (int)gg_fix32(p->type))
			continue;
		
		tmp->cid = p->id;

		switch (tmp->dcc_type) {
			case GG_DCC7_TYPE_FILE:
			{
				struct gg_dcc7_new s;

				memset(&s, 0, sizeof(s));
				s.id = tmp->cid;
				s.type = gg_fix32(GG_DCC7_TYPE_FILE);
				s.uin_from = gg_fix32(tmp->uin);
				s.uin_to = gg_fix32(tmp->peer_uin);
				s.size = gg_fix32(tmp->size);

				strncpy((char*) s.filename, (char*) tmp->filename, GG_DCC7_FILENAME_LEN);

				tmp->state = GG_STATE_WAITING_FOR_ACCEPT;
				tmp->timeout = GG_DCC7_TIMEOUT_FILE_ACK;

				return gg_send_packet(sess, GG_DCC7_NEW, &s, sizeof(s), NULL);
			}
		}
	}

	return 0;
}

/**
 * \internal Obsługuje pakiet akceptacji połączenia bezpośredniego.
 *
 * \param sess Struktura sesji
 * \param e Struktura zdarzenia
 * \param payload Treść pakietu
 * \param len Długość pakietu
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 */
int gg_dcc7_handle_accept(struct gg_session *sess, struct gg_event *e, void *payload, int len)
{
	struct gg_dcc7_accept *p = (struct gg_dcc7_accept*)payload;
	struct gg_dcc7 *dcc;

	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_dcc7_handle_accept(%p, %p, %p, %d)\n", sess, e, payload, len);

	if (!(dcc = gg_dcc7_session_find(sess, p->id, gg_fix32(p->uin)))) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_dcc7_handle_accept() unknown dcc session\n");
		// XXX wysłać reject?
		e->type = GG_EVENT_DCC7_ERROR;
		e->event.dcc7_error = GG_ERROR_DCC7_HANDSHAKE;
		e->event.dcc7_error_ex.dcc7 = dcc;
		return 0;
	}

	if (dcc->state != GG_STATE_WAITING_FOR_ACCEPT) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_dcc7_handle_accept() invalid state\n");
		e->type = GG_EVENT_DCC7_ERROR;
		e->event.dcc7_error = GG_ERROR_DCC7_HANDSHAKE;
		e->event.dcc7_error_ex.dcc7 = dcc;
		return 0;
	}
	
	// XXX czy dla odwrotnego połączenia powinniśmy wywołać już zdarzenie GG_DCC7_ACCEPT?
	
	dcc->offset = gg_fix32(p->offset);
	dcc->state = GG_STATE_WAITING_FOR_INFO;

	return 0;
}

/**
 * \internal Obsługuje pakiet informacji o połączeniu bezpośrednim.
 *
 * \param sess Struktura sesji
 * \param e Struktura zdarzenia
 * \param payload Treść pakietu
 * \param len Długość pakietu
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 */
int gg_dcc7_handle_info(struct gg_session *sess, struct gg_event *e, void *payload, int len)
{
	struct gg_dcc7_info *p = (gg_dcc7_info*)payload;
	struct gg_dcc7 *dcc;
	char *tmp;

	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_dcc7_handle_info(%p, %p, %p, %d)\n", sess, e, payload, len);
	gg_debug_session(sess, GG_DEBUG_FUNCTION, "// gg_dcc7_handle_info() received address: %s, hash: %s\n", p->info, p->hash);

	if (!(dcc = gg_dcc7_session_find(sess, p->id, gg_fix32(p->uin)))) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_dcc7_handle_info() unknown dcc session\n");
		return 0;
	}
	
	if (dcc->state == GG_STATE_CONNECTED) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_dcc7_handle_info() state is already connected\n");
		return 0;
	}

	switch (p->type)
	{
	case GG_DCC7_TYPE_P2P:
		if ((dcc->remote_addr = inet_addr(p->info)) == INADDR_NONE) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_dcc7_handle_info() invalid IP address\n");
			e->type = GG_EVENT_DCC7_ERROR;
			e->event.dcc7_error = GG_ERROR_DCC7_HANDSHAKE;
			e->event.dcc7_error_ex.dcc7 = dcc;
			return 0;
		}

		if (!(tmp = strchr(p->info, ' ')) || !(dcc->remote_port = atoi(tmp + 1))) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_dcc7_handle_info() invalid IP port\n");
			e->type = GG_EVENT_DCC7_ERROR;
			e->event.dcc7_error = GG_ERROR_DCC7_HANDSHAKE;
			e->event.dcc7_error_ex.dcc7 = dcc;
			return 0;
		}

		if (dcc->state == GG_STATE_WAITING_FOR_INFO) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_dcc7_handle_info() wainting for info so send one\n");
			gg_dcc7_listen_and_send_info(dcc);
			return 0;
		}

		break;

	case GG_DCC7_TYPE_SERVER:
		if (!(tmp = strstr(p->info, "GG"))) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_dcc7_handle_info() unknown info packet\n");
			e->type = GG_EVENT_DCC7_ERROR;
			e->event.dcc7_error = GG_ERROR_DCC7_HANDSHAKE;
			e->event.dcc7_error_ex.dcc7 = dcc;
			return 0;
		}

#if defined(GG_CONFIG_HAVE_UINT64_T) && defined(GG_CONFIG_HAVE_STRTOULL)
		{
			uint64_t cid;

			cid = strtoull(tmp + 2, NULL, 0);

			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_dcc7_handle_info() info.str=%s, info.id=%llu, sess.id=%llu\n", tmp + 2, cid, *((unsigned long long*) &dcc->cid));

			cid = gg_fix64(cid);

			if (memcmp(&dcc->cid, &cid, sizeof(cid)) != 0) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_dcc7_handle_info() invalid session id\n");
				e->type = GG_EVENT_DCC7_ERROR;
				e->event.dcc7_error = GG_ERROR_DCC7_HANDSHAKE;
				e->event.dcc7_error_ex.dcc7 = dcc;
				return 0;
			}
		}
#endif

		if (gg_dcc7_get_relay_addr(dcc) == -1) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_dcc7_handle_info() unable to retrieve relay address\n");
			e->type = GG_EVENT_DCC7_ERROR;
			e->event.dcc7_error = GG_ERROR_DCC7_RELAY;
			e->event.dcc7_error_ex.dcc7 = dcc;
			return 0;
		}

		// XXX wysyłać dopiero jeśli uda się połączyć z serwerem?

		gg_send_packet(dcc->sess, GG_DCC7_INFO, payload, len, NULL);

		break;

	default:
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_dcc7_handle_info() unhandled transfer type (%d)\n", p->type);
		e->type = GG_EVENT_DCC7_ERROR;
		e->event.dcc7_error = GG_ERROR_DCC7_HANDSHAKE;
		e->event.dcc7_error_ex.dcc7 = dcc;
		return 0;
	}

	// jeśli nadal czekamy na połączenie przychodzące, a druga strona nie
	// daje rady i oferuje namiary na siebie, bierzemy co dają.

// 	if (dcc->state != GG_STATE_WAITING_FOR_INFO && (dcc->state != GG_STATE_LISTENING || dcc->reverse)) {
// 		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_dcc7_handle_info() invalid state\n");
// 		e->type = GG_EVENT_DCC7_ERROR;
// 		e->event.dcc7_error = GG_ERROR_DCC7_HANDSHAKE;
//		e->event.dcc7_error_ex.dcc7 = dcc;
// 		return 0;
// 	}

	if (dcc->state == GG_STATE_LISTENING) {
		gg_sock_close(dcc->fd);
		dcc->fd = -1;
		dcc->reverse = 1;
	}
	
	if (dcc->type == GG_SESSION_DCC7_SEND) {
		e->type = GG_EVENT_DCC7_ACCEPT;
		e->event.dcc7_accept.dcc7 = dcc;
		e->event.dcc7_accept.type = gg_fix32(p->type);
		e->event.dcc7_accept.remote_ip = dcc->remote_addr;
		e->event.dcc7_accept.remote_port = dcc->remote_port;
	} else {
		e->type = GG_EVENT_DCC7_PENDING;
		e->event.dcc7_pending.dcc7 = dcc;
	}

	if (dcc->state == GG_STATE_RESOLVING_RELAY)
		return 0;

	if (gg_dcc7_connect(dcc) == -1) {
		if (gg_dcc7_reverse_connect(dcc) == -1) {
			e->type = GG_EVENT_DCC7_ERROR;
			e->event.dcc7_error = GG_ERROR_DCC7_NET;
			e->event.dcc7_error_ex.dcc7 = dcc;
			return 0;
		}
	}

	return 0;
}

/**
 * \internal Obsługuje pakiet odrzucenia połączenia bezpośredniego.
 *
 * \param sess Struktura sesji
 * \param e Struktura zdarzenia
 * \param payload Treść pakietu
 * \param len Długość pakietu
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 */
int gg_dcc7_handle_reject(struct gg_session *sess, struct gg_event *e, void *payload, int len)
{
	struct gg_dcc7_reject *p = (struct gg_dcc7_reject*)payload;
	struct gg_dcc7 *dcc;

	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_dcc7_handle_reject(%p, %p, %p, %d)\n", sess, e, payload, len);

	if (!(dcc = gg_dcc7_session_find(sess, p->id, gg_fix32(p->uin)))) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_dcc7_handle_reject() unknown dcc session\n");
		return 0;
	}
	
	if (dcc->state != GG_STATE_WAITING_FOR_ACCEPT) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_dcc7_handle_reject() invalid state\n");
		e->type = GG_EVENT_DCC7_ERROR;
		e->event.dcc7_error = GG_ERROR_DCC7_HANDSHAKE;
		e->event.dcc7_error_ex.dcc7 = dcc;
		return 0;
	}

	e->type = GG_EVENT_DCC7_REJECT;
	e->event.dcc7_reject.dcc7 = dcc;
	e->event.dcc7_reject.reason = gg_fix32(p->reason);

	// XXX ustawić state na rejected?

	return 0;
}

/**
 * \internal Obsługuje pakiet przerwania żądania połączenia bezpośredniego.
 *
 * \param sess Struktura sesji
 * \param e Struktura zdarzenia
 * \param payload Treść pakietu
 * \param len Długość pakietu
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 */
int gg_dcc7_handle_abort(struct gg_session *sess, struct gg_event *e, void *payload, int len)
{
	struct gg_dcc7_aborted *p = (gg_dcc7_aborted*)payload;
	struct gg_dcc7 *dcc;

	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_dcc7_handle_abort(%p, %p, %p, %d)\n", sess, e, payload, len);

	if (!(dcc = gg_dcc7_session_find(sess, p->id, gg_fix32(0)))) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_dcc7_handle_abort() unknown dcc session\n");
		return 0;
	}

	gg_debug_session(sess, GG_DEBUG_MISC, "// gg_dcc7_handle_abort() state %d\n", dcc->state);

	if (dcc->state != GG_STATE_IDLE) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_dcc7_handle_abort() invalid state\n");
		e->type = GG_EVENT_DCC7_ERROR;
		e->event.dcc7_error = GG_ERROR_DCC7_HANDSHAKE;
		e->event.dcc7_error_ex.dcc7 = dcc;
		return 0;
	}

	e->type = GG_EVENT_DCC7_REJECT;
	e->event.dcc7_reject.dcc7 = dcc;
	e->event.dcc7_reject.reason = gg_fix32(GG_DCC7_REJECT_USER);

	// XXX ustawić state na rejected?

	return 0;
}

/**
 * \internal Obsługuje pakiet nowego połączenia bezpośredniego.
 *
 * \param sess Struktura sesji
 * \param e Struktura zdarzenia
 * \param payload Treść pakietu
 * \param len Długość pakietu
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 */
int gg_dcc7_handle_new(struct gg_session *sess, struct gg_event *e, void *payload, int len)
{
	struct gg_dcc7_new *p = (gg_dcc7_new*)payload;
	struct gg_dcc7 *dcc;

	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_dcc7_handle_new(%p, %p, %p, %d)\n", sess, e, payload, len);

	switch (gg_fix32(p->type)) {
		case GG_DCC7_TYPE_FILE:
			if (!(dcc = (gg_dcc7*)malloc(sizeof(struct gg_dcc7)))) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_dcc7_handle_new() not enough memory\n");
				return -1;
			}
			
			memset(dcc, 0, sizeof(struct gg_dcc7));
			dcc->type = GG_SESSION_DCC7_GET;
			dcc->dcc_type = GG_DCC7_TYPE_FILE;
			dcc->fd = -1;
			dcc->file_fd = -1;
			dcc->uin = sess->uin;
			dcc->peer_uin = gg_fix32(p->uin_from);
			dcc->cid = p->id;
			dcc->sess = sess;

			if (gg_dcc7_session_add(sess, dcc) == -1) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_dcc7_handle_new() unable to add to session\n");
				gg_dcc7_free(dcc);
				return -1;
			}

			dcc->size = gg_fix32(p->size);
			strncpy((char*) dcc->filename, (char*) p->filename, GG_DCC7_FILENAME_LEN - 1);
			dcc->filename[GG_DCC7_FILENAME_LEN] = 0;
			memcpy(dcc->hash, p->hash, GG_DCC7_HASH_LEN);

			e->type = GG_EVENT_DCC7_NEW;
			e->event.dcc7_new = dcc;

			break;

		case GG_DCC7_TYPE_VOICE:
			if (!(dcc = (gg_dcc7*)malloc(sizeof(struct gg_dcc7)))) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_dcc7_handle_packet() not enough memory\n");
				return -1;
			}
			
			memset(dcc, 0, sizeof(struct gg_dcc7));

			dcc->type = GG_SESSION_DCC7_VOICE;
			dcc->dcc_type = GG_DCC7_TYPE_VOICE;
			dcc->fd = -1;
			dcc->file_fd = -1;
			dcc->uin = sess->uin;
			dcc->peer_uin = gg_fix32(p->uin_from);
			dcc->cid = p->id;
			dcc->sess = sess;

			if (gg_dcc7_session_add(sess, dcc) == -1) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_dcc7_handle_new() unable to add to session\n");
				gg_dcc7_free(dcc);
				return -1;
			}

			e->type = GG_EVENT_DCC7_NEW;
			e->event.dcc7_new = dcc;

			break;

		default:
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_dcc7_handle_new() unknown dcc type (%d) from %ld\n", gg_fix32(p->type), gg_fix32(p->uin_from));

			break;
	}

	return 0;
}

/**
 * \internal Ustawia odpowiednie stany wewnętrzne w zależności od rodzaju
 * połączenia.
 * 
 * \param dcc Struktura połączenia
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu.
 */
static int gg_dcc7_postauth_fixup(struct gg_dcc7 *dcc)
{
	gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_FUNCTION, "** gg_dcc7_postauth_fixup(%p)\n", dcc);

	if (!dcc) {
		gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_postauth_fixup() invalid parameters\n");
		errno = EINVAL;
		return -1;
	}

	switch (dcc->type) {
		case GG_SESSION_DCC7_GET:
			dcc->state = GG_STATE_GETTING_FILE;
			dcc->check = GG_CHECK_READ;
			return 0;

		case GG_SESSION_DCC7_SEND:
			dcc->state = GG_STATE_SENDING_FILE;
			dcc->check = GG_CHECK_WRITE;
			return 0;

		case GG_SESSION_DCC7_VOICE:
			dcc->state = GG_STATE_READING_VOICE_DATA;
			dcc->check = GG_CHECK_READ;
			return 0;
	}

	errno = EINVAL;

	return -1;
}

/**
 * Funkcja wywoływana po zaobserwowaniu zmian na deskryptorze połączenia.
 *
 * Funkcja zwraca strukturę zdarzenia \c gg_event. Jeśli rodzaj zdarzenia
 * to \c GG_EVENT_NONE, nie wydarzyło się jeszcze nic wartego odnotowania.
 * Strukturę zdarzenia należy zwolnić funkcja \c gg_event_free().
 *
 * \param dcc Struktura połączenia
 *
 * \return Struktura zdarzenia lub \c NULL jeśli wystąpił błąd
 *
 * \ingroup dcc7
 */
struct gg_event *gg_dcc7_watch_fd(struct gg_dcc7 *dcc)
{
	struct gg_event *e;

	gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_FUNCTION, "** gg_dcc7_watch_fd(%p)\n", dcc);

	if (!dcc || (dcc->type != GG_SESSION_DCC7_SEND && dcc->type != GG_SESSION_DCC7_GET && dcc->type != GG_SESSION_DCC7_VOICE)) {
		gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_watch_fd() invalid parameters\n");
		errno = EINVAL;
		return NULL;
	}

	if (!(e = (gg_event*)malloc(sizeof(struct gg_event)))) {
		gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_watch_fd() not enough memory\n");
		return NULL;
	}

	memset(e, 0, sizeof(struct gg_event));
	e->type = GG_EVENT_NONE;

	switch (dcc->state) {
		case GG_STATE_LISTENING:
		{
			struct sockaddr_in sin;
			SOCKET fd;
			int one = 1;
			int sin_len = sizeof(sin);

			gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_watch_fd() GG_STATE_LISTENING\n");

			if ((fd = accept(dcc->fd, (struct sockaddr*) &sin, &sin_len)) == -1) {
				gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_watch_fd() accept() failed (%s)\n", strerror(errno));
				return e;
			}

			gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_watch_fd() connection from %s:%d\n", inet_ntoa(sin.sin_addr), htons(sin.sin_port));

#ifdef FIONBIO
			if (ioctl(fd, FIONBIO, &one) == -1) {
#else
			if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1) {
#endif
				gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_watch_fd() can't set nonblocking (%s)\n", strerror(errno));
				gg_sock_close(fd);
				e->type = GG_EVENT_DCC7_ERROR;
				e->event.dcc7_error = GG_ERROR_DCC7_HANDSHAKE;
				e->event.dcc7_error_ex.dcc7 = dcc;
				return e;
			}

			gg_sock_close(dcc->fd);
			dcc->fd = fd;

			dcc->state = GG_STATE_READING_ID;
			dcc->check = GG_CHECK_READ;
			dcc->timeout = GG_DEFAULT_TIMEOUT;
			dcc->incoming = 1;

			dcc->remote_port = ntohs(sin.sin_port);
			dcc->remote_addr = sin.sin_addr.s_addr;

			e->type = GG_EVENT_DCC7_CONNECTED;
			e->event.dcc7_connected.dcc7 = dcc;

			return e;
		}

		case GG_STATE_CONNECTING:
		{
			int res = 0, error = 0;
			int error_size = sizeof(error);

			gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_watch_fd() GG_STATE_CONNECTING\n");

			dcc->soft_timeout = 0;

			if (dcc->timeout == 0)
				error = ETIMEDOUT;

			if (error || (res = gg_getsockopt(dcc->fd, SOL_SOCKET, SO_ERROR, &error, &error_size)) == -1 || error != 0) {
				gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_watch_fd() connection failed (%s)\n", (res == -1) ? strerror(errno) : strerror(error));

				if (dcc->relay) {
					for (dcc->relay_index++; dcc->relay_index < dcc->relay_count; dcc->relay_index++) {
						dcc->remote_addr = dcc->relay_list[dcc->relay_index].addr;
						dcc->remote_port = dcc->relay_list[dcc->relay_index].port;

						if (gg_dcc7_connect(dcc) == 0)
							break;
					}

					if (dcc->relay_index >= dcc->relay_count) {
						gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_watch_fd() no relay available");
						e->type = GG_EVENT_DCC7_ERROR;
						e->event.dcc7_error = GG_ERROR_DCC7_RELAY;
						e->event.dcc7_error_ex.dcc7 = dcc;
						return e;
					}
				} else {
					if (gg_dcc7_reverse_connect(dcc) != -1) {
						e->type = GG_EVENT_DCC7_PENDING;
						e->event.dcc7_pending.dcc7 = dcc;
					} else {
						e->type = GG_EVENT_DCC7_ERROR;
						e->event.dcc7_error = GG_ERROR_DCC7_NET;
						e->event.dcc7_error_ex.dcc7 = dcc;
					}

					return e;
				}
			}

			gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_watch_fd() connected, sending id\n");

			dcc->state = GG_STATE_SENDING_ID;
			dcc->check = GG_CHECK_WRITE;
			dcc->timeout = GG_DEFAULT_TIMEOUT;
			dcc->incoming = 0;

			return e;
		}

		case GG_STATE_READING_ID:
		{
			int res;

			gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_watch_fd() GG_STATE_READING_ID\n");

			if (!dcc->relay) {
				struct gg_dcc7_welcome_p2p welcome, welcome_ok;
				welcome_ok.id = dcc->cid;

				if ((res = gg_sock_read(dcc->fd, &welcome, sizeof(welcome))) != sizeof(welcome)) {
					gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_watch_fd() read() failed (%d, %s)\n", res, strerror(errno));
					e->type = GG_EVENT_DCC7_ERROR;
					e->event.dcc7_error = GG_ERROR_DCC7_HANDSHAKE;
					e->event.dcc7_error_ex.dcc7 = dcc;
					return e;
				}

				if (memcmp(&welcome, &welcome_ok, sizeof(welcome))) {
					gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_watch_fd() invalid id\n");
					e->type = GG_EVENT_DCC7_ERROR;
					e->event.dcc7_error = GG_ERROR_DCC7_HANDSHAKE;
					e->event.dcc7_error_ex.dcc7 = dcc;
					return e;
				}
			} else {
				struct gg_dcc7_welcome_server welcome, welcome_ok;
				welcome_ok.magic = GG_DCC7_WELCOME_SERVER;
				welcome_ok.id = dcc->cid;

				if ((res = gg_sock_read(dcc->fd, &welcome, sizeof(welcome))) != sizeof(welcome)) {
					gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_watch_fd() read() failed (%d, %s)\n", res, strerror(errno));
					e->type = GG_EVENT_DCC7_ERROR;
					e->event.dcc7_error = GG_ERROR_DCC7_HANDSHAKE;
					e->event.dcc7_error_ex.dcc7 = dcc;
					return e;
				}

				if (memcmp(&welcome, &welcome_ok, sizeof(welcome)) != 0) {
					gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_watch_fd() invalid id\n");
					e->type = GG_EVENT_DCC7_ERROR;
					e->event.dcc7_error = GG_ERROR_DCC7_HANDSHAKE;
					e->event.dcc7_error_ex.dcc7 = dcc;
					return e;
				}
			}

			if (dcc->incoming) {
				dcc->state = GG_STATE_SENDING_ID;
				dcc->check = GG_CHECK_WRITE;
				dcc->timeout = GG_DEFAULT_TIMEOUT;
			} else {
				gg_dcc7_postauth_fixup(dcc);
				dcc->timeout = GG_DEFAULT_TIMEOUT;
			}

			return e;
		}

		case GG_STATE_SENDING_ID:
		{
			int res;

			gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_watch_fd() GG_SENDING_ID\n");

			if (!dcc->relay) {
				struct gg_dcc7_welcome_p2p welcome;

				welcome.id = dcc->cid;

				if ((res = gg_sock_write(dcc->fd, &welcome, sizeof(welcome))) != sizeof(welcome)) {
					gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_watch_fd() write() failed (%d, %s)", res, strerror(errno));
					e->type = GG_EVENT_DCC7_ERROR;
					e->event.dcc7_error = GG_ERROR_DCC7_HANDSHAKE;
					e->event.dcc7_error_ex.dcc7 = dcc;
					return e;
				}
			} else {
				struct gg_dcc7_welcome_server welcome;

				welcome.magic = gg_fix32(GG_DCC7_WELCOME_SERVER);
				welcome.id = dcc->cid;

				if ((res = gg_sock_write(dcc->fd, &welcome, sizeof(welcome))) != sizeof(welcome)) {
					gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_watch_fd() write() failed (%d, %s)\n", res, strerror(errno));
					e->type = GG_EVENT_DCC7_ERROR;
					e->event.dcc7_error = GG_ERROR_DCC7_HANDSHAKE;
					e->event.dcc7_error_ex.dcc7 = dcc;
					return e;
				}
			}

			if (dcc->incoming) {
				gg_dcc7_postauth_fixup(dcc);
				dcc->timeout = GG_DEFAULT_TIMEOUT;
			} else {
				dcc->state = GG_STATE_READING_ID;
				dcc->check = GG_CHECK_READ;
				dcc->timeout = GG_DEFAULT_TIMEOUT;
			}

			return e;
		}

		case GG_STATE_SENDING_FILE:
		{
			char buf[1024];
			int chunk, res;

			gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_watch_fd() GG_STATE_SENDING_FILE (offset=%d, size=%d)\n", dcc->offset, dcc->size);

			if (dcc->offset >= dcc->size) {
				gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_watch_fd() offset >= size, finished\n");
				e->type = GG_EVENT_DCC7_DONE;
				e->event.dcc7_done.dcc7 = dcc;
				return e;
			}

			if (dcc->seek && lseek(dcc->file_fd, dcc->offset, SEEK_SET) == (off_t) -1) {
				gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_watch_fd() lseek() failed (%s)\n", strerror(errno));
				e->type = GG_EVENT_DCC7_ERROR;
				e->event.dcc7_error = GG_ERROR_DCC7_FILE;
				e->event.dcc7_error_ex.dcc7 = dcc;
				return e;
			}

			if ((chunk = dcc->size - dcc->offset) > sizeof(buf))
				chunk = sizeof(buf);

			if ((res = read(dcc->file_fd, buf, chunk)) < 1) {
				gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_watch_fd() read() failed (res=%d, %s)\n", res, strerror(errno));
				e->type = GG_EVENT_DCC7_ERROR;
				e->event.dcc7_error = (res == -1) ? GG_ERROR_DCC7_FILE : GG_ERROR_DCC7_EOF;
				e->event.dcc7_error_ex.dcc7 = dcc;
				return e;
			}

			if ((res = gg_sock_write(dcc->fd, buf, res)) == -1) {
				gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_watch_fd() write() failed (%s)\n", strerror(errno));
				e->type = GG_EVENT_DCC7_ERROR;
				e->event.dcc7_error = GG_ERROR_DCC7_NET;
				e->event.dcc7_error_ex.dcc7 = dcc;
				return e;
			}

			dcc->offset += res;

			if (dcc->offset >= dcc->size) {
				gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_watch_fd() finished\n");
				e->type = GG_EVENT_DCC7_DONE;
				e->event.dcc7_done.dcc7 = dcc;
				return e;
			}

			dcc->state = GG_STATE_SENDING_FILE;
			dcc->check = GG_CHECK_WRITE;
			dcc->timeout = GG_DCC7_TIMEOUT_SEND;

			return e;
		}

		case GG_STATE_GETTING_FILE:
		{
			char buf[1024];
			int res, wres;

			gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_watch_fd() GG_STATE_GETTING_FILE (offset=%d, size=%d)\n", dcc->offset, dcc->size);

			if (dcc->offset >= dcc->size) {
				gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_watch_fd() finished\n");
				e->type = GG_EVENT_DCC7_DONE;
				e->event.dcc7_done.dcc7 = dcc;
				return e;
			}

			if ((res = gg_sock_read(dcc->fd, buf, sizeof(buf))) < 1) {
				gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_watch_fd() read() failed (fd=%d, res=%d, %s)\n", dcc->fd, res, strerror(errno));
				e->type = GG_EVENT_DCC7_ERROR;
				e->event.dcc7_error = (res == -1) ? GG_ERROR_DCC7_NET : GG_ERROR_DCC7_EOF;
				e->event.dcc7_error_ex.dcc7 = dcc;
				return e;
			}

			// XXX zapisywać do skutku?

			if ((wres = write(dcc->file_fd, buf, res)) < res) {
				gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_watch_fd() write() failed (fd=%d, res=%d, %s)\n", dcc->file_fd, wres, strerror(errno));
				e->type = GG_EVENT_DCC7_ERROR;
				e->event.dcc7_error = GG_ERROR_DCC7_FILE;
				e->event.dcc7_error_ex.dcc7 = dcc;
				return e;
			}

			dcc->offset += res;

			if (dcc->offset >= dcc->size) {
				gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_watch_fd() finished\n");
				e->type = GG_EVENT_DCC7_DONE;
				e->event.dcc7_done.dcc7 = dcc;
				return e;
			}

			dcc->state = GG_STATE_GETTING_FILE;
			dcc->check = GG_CHECK_READ;
			dcc->timeout = GG_DCC7_TIMEOUT_GET;

			return e;
		}

		case GG_STATE_RESOLVING_RELAY:
		{
			struct in_addr addr;

			gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_watch_fd() GG_STATE_RESOLVING_RELAY\n");

			if (gg_sock_read(dcc->fd, &addr, sizeof(addr)) < sizeof(addr) || addr.s_addr == INADDR_NONE) {
				int errno_save = errno;

				gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_watch_fd() resolving failed\n");
				gg_sock_close(dcc->fd);
				dcc->fd = -1;
				dcc->sess->resolver_cleanup(&dcc->resolver, 0);
				errno = errno_save;
				e->type = GG_EVENT_DCC7_ERROR;
				e->event.dcc7_error = GG_ERROR_DCC7_RELAY;
				e->event.dcc7_error_ex.dcc7 = dcc;
				return e;
			}

			gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_watch_fd() resolved, connecting to %s:%d\n", inet_ntoa(addr), GG_RELAY_PORT);

			if ((dcc->fd = gg_connect(&addr, GG_RELAY_PORT, 1)) == -1) {
				gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_watch_fd() connection failed (errno=%d, %s), critical\n", errno, strerror(errno));
				e->type = GG_EVENT_DCC7_ERROR;
				e->event.dcc7_error = GG_ERROR_DCC7_RELAY;
				e->event.dcc7_error_ex.dcc7 = dcc;
				return e;
			}
			
			dcc->state = GG_STATE_CONNECTING_RELAY;
			dcc->check = GG_CHECK_WRITE;
			dcc->timeout = GG_DEFAULT_TIMEOUT;

			return e;
		}

		case GG_STATE_CONNECTING_RELAY:
		{
			int res;
			int res_size = sizeof(res);
			struct gg_dcc7_relay_req pkt;

			gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_watch_fd() GG_STATE_CONNECTING_RELAY\n");
			
			if (gg_getsockopt(dcc->fd, SOL_SOCKET, SO_ERROR, &res, &res_size) != 0 || res != 0) {
				gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_watch_fd() connection failed (errno=%d, %s)\n", res, strerror(res));
				e->type = GG_EVENT_DCC7_ERROR;
				e->event.dcc7_error = GG_ERROR_DCC7_RELAY;
				e->event.dcc7_error_ex.dcc7 = dcc;
				return e;
			}

			memset(&pkt, 0, sizeof(pkt));
			pkt.magic = gg_fix32(GG_DCC7_RELAY_REQUEST);
			pkt.len = gg_fix32(sizeof(pkt));
			pkt.id = dcc->cid;
			pkt.type = gg_fix16(GG_DCC7_RELAY_TYPE_SERVER);
			pkt.dunno1 = gg_fix16(GG_DCC7_RELAY_DUNNO1);

			gg_debug_dump_session((dcc) ? (dcc)->sess : NULL, &pkt, sizeof(pkt), "// gg_dcc7_watch_fd() send pkt(0x%.2x)\n", gg_fix32(pkt.magic));

			if ((res = gg_sock_write(dcc->fd, &pkt, sizeof(pkt))) != sizeof(pkt)) {
				gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_watch_fd() sending failed\n");
				e->type = GG_EVENT_DCC7_ERROR;
				e->event.dcc7_error = GG_ERROR_DCC7_RELAY;
				e->event.dcc7_error_ex.dcc7 = dcc;
				return e;
			}

			dcc->state = GG_STATE_READING_RELAY;
			dcc->check = GG_CHECK_READ;
			dcc->timeout = GG_DEFAULT_TIMEOUT;

			return e;
		}

		case GG_STATE_READING_RELAY:
		{
			char buf[256];
			struct gg_dcc7_relay_reply *pkt;
			struct gg_dcc7_relay_reply_server srv;
			int res;
			int i;

			gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_watch_fd() GG_STATE_READING_RELAY\n");

			if ((res = gg_sock_read(dcc->fd, buf, sizeof(buf))) < sizeof(*pkt)) {
				if (res == 0)
					errno = ECONNRESET;
				gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_watch_fd() read() failed (%d, %s)\n", res, strerror(errno));
				e->type = GG_EVENT_DCC7_ERROR;
				e->event.dcc7_error = GG_ERROR_DCC7_RELAY;
				e->event.dcc7_error_ex.dcc7 = dcc;
				return e;
			}

			pkt = (struct gg_dcc7_relay_reply*) buf;

			if (gg_fix32(pkt->magic) != GG_DCC7_RELAY_REPLY || gg_fix32(pkt->rcount) < 1 || gg_fix32(pkt->rcount > 256) || gg_fix32(pkt->len) < sizeof(*pkt) + gg_fix32(pkt->rcount) * sizeof(srv)) {
				gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_wathc_fd() invalid reply\n");
				errno = EINVAL;
				e->type = GG_EVENT_DCC7_ERROR;
				e->event.dcc7_error = GG_ERROR_DCC7_RELAY;
				e->event.dcc7_error_ex.dcc7 = dcc;
				return e;
			}

		        gg_debug_dump_session((dcc) ? (dcc)->sess : NULL, buf, res, "// gg_dcc7_get_relay() read pkt(0x%.2x)\n", gg_fix32(pkt->magic));

			free(dcc->relay_list);

			dcc->relay_index = 0;
			dcc->relay_count = gg_fix32(pkt->rcount);
			dcc->relay_list = (gg_dcc7_relay*)malloc(dcc->relay_count * sizeof(gg_dcc7_relay_t));

			if (dcc->relay_list == NULL) {
				gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_watch_fd() not enough memory");
				dcc->relay_count = 0;
				free(e);
				return NULL;
			}

			for (i = 0; i < dcc->relay_count; i++) {
				struct in_addr addr;

				memcpy(&srv, buf + sizeof(*pkt) + i * sizeof(srv), sizeof(srv));
				dcc->relay_list[i].addr = srv.addr;
				dcc->relay_list[i].port = gg_fix16(srv.port);
				dcc->relay_list[i].family = srv.family;

				addr.s_addr = srv.addr;
				gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "//    %s %d %d\n", inet_ntoa(addr), gg_fix16(srv.port), srv.family);
			}
			
			dcc->relay = 1;

			for (; dcc->relay_index < dcc->relay_count; dcc->relay_index++) {
				dcc->remote_addr = dcc->relay_list[dcc->relay_index].addr;
				dcc->remote_port = dcc->relay_list[dcc->relay_index].port;

				if (gg_dcc7_connect(dcc) == 0)
					break;
			}

			if (dcc->relay_index >= dcc->relay_count) {
				gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_watch_fd() no relay available");
				e->type = GG_EVENT_DCC7_ERROR;
				e->event.dcc7_error = GG_ERROR_DCC7_RELAY;
				e->event.dcc7_error_ex.dcc7 = dcc;
				return e;
			}

			return e;
		}

		default:
		{
			gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_MISC, "// gg_dcc7_watch_fd() GG_STATE_???\n");
			e->type = GG_EVENT_DCC7_ERROR;
			e->event.dcc7_error = GG_ERROR_DCC7_HANDSHAKE;
			e->event.dcc7_error_ex.dcc7 = dcc;

			return e;
		}
	}

	return e;
}

/**
 * Zwalnia zasoby używane przez połączenie bezpośrednie.
 *
 * \param dcc Struktura połączenia
 *
 * \ingroup dcc7
 */
void gg_dcc7_free(struct gg_dcc7 *dcc)
{
	gg_debug_session((dcc) ? (dcc)->sess : NULL, GG_DEBUG_FUNCTION, "** gg_dcc7_free(%p)\n", dcc);

	if (!dcc)
		return;

	if (dcc->fd != -1)
		gg_sock_close(dcc->fd);

	if (dcc->file_fd != -1)
		close(dcc->file_fd);

	if (dcc->sess)
		gg_dcc7_session_remove(dcc->sess, dcc);

	free(dcc->relay_list);

	free(dcc);
}

