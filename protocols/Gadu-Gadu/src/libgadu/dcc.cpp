/* coding: UTF-8 */
/* $Id: dcc.c 12145 2010-07-07 23:49:04Z dezred $ */

/*
 *  (C) Copyright 2001-2008 Wojtek Kaniewski <wojtekka@irc.pl>
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
 * \file dcc.c
 *
 * \brief Obsługa połączeń bezpośrednich do wersji Gadu-Gadu 6.x
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

#ifdef _WIN32
#undef small
#endif

#ifndef GG_DEBUG_DISABLE

/**
 * \internal Przekazuje zawartość pakietu do odpluskwiania.
 *
 * \param prefix Prefiks informacji
 * \param fd Deskryptor gniazda
 * \param buf Bufor z danumi
 * \param size Rozmiar bufora z danymi
 */
static void gg_dcc_debug_data(const char *prefix, SOCKET fd, const void *buf, unsigned int size)
{
	unsigned int i;

	gg_debug(GG_DEBUG_MISC, "++ gg_dcc %s (fd=%d,len=%d)", prefix, fd, size);

	for (i = 0; i < size; i++)
		gg_debug(GG_DEBUG_MISC, " %.2x", ((unsigned char*) buf)[i]);

	gg_debug(GG_DEBUG_MISC, "\n");
}
#else
#define gg_dcc_debug_data(a,b,c,d) do { } while (0)
#endif

/**
 * Wysyła żądanie zwrotnego połączenia bezpośredniego.
 *
 * Funkcję wykorzystuje się, jeśli nie ma możliwości połączenia się z odbiorcą
 * pliku lub rozmowy głosowej. Po otrzymaniu żądania druga strona spróbuje
 * nawiązać zwrotne połączenie bezpośrednie z nadawcą.
 * gg_dcc_request()
 *
 * \param sess Struktura sesji
 * \param uin Numer odbiorcy
 *
 * \return Patrz \c gg_send_message_ctcp()
 *
 * \ingroup dcc6
 */
int gg_dcc_request(struct gg_session *sess, uin_t uin)
{
	return gg_send_message_ctcp(sess, GG_CLASS_CTCP, uin, (unsigned char*) "\002", 1);
}

/**
 * \internal Zamienia znacznik czasu w postaci uniksowej na format API WIN32.
 *
 * \note Funkcja działa jedynie gdy kompilator obsługuje typ danych
 * \c long \c long.
 *
 * \param ut Czas w postaci uniksowej
 * \param ft Czas w postaci API WIN32
 */
static void gg_dcc_fill_filetime(time_t ut, uint32_t *ft)
{
#ifdef GG_CONFIG_HAVE_LONG_LONG
	unsigned long long tmp;

	tmp = ut;
	tmp += 11644473600LL;
	tmp *= 10000000LL;

#ifndef GG_CONFIG_BIGENDIAN
	ft[0] = (uint32_t) tmp;
	ft[1] = (uint32_t) (tmp >> 32);
#else
	ft[0] = gg_fix32((uint32_t) (tmp >> 32));
	ft[1] = gg_fix32((uint32_t) tmp);
#endif

#endif
}

/**
 * Wypełnia pola struktury \c gg_dcc niezbędne do wysłania pliku.
 *
 * \note Większą funkcjonalność zapewnia funkcja \c gg_dcc_fill_file_info2().
 *
 * \param d Struktura połączenia
 * \param filename Nazwa pliku
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 *
 * \ingroup dcc6
 */
int gg_dcc_fill_file_info(struct gg_dcc *d, const char *filename)
{
	return gg_dcc_fill_file_info2(d, filename, filename);
}

/**
 * Wypełnia pola struktury \c gg_dcc niezbędne do wysłania pliku.
 *
 * \param d Struktura połączenia
 * \param filename Nazwa pliku zapisywana w strukturze
 * \param local_filename Nazwa pliku w lokalnym systemie plików
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 *
 * \ingroup dcc6
 */
int gg_dcc_fill_file_info2(struct gg_dcc *d, const char *filename, const char *local_filename)
{
	struct stat st;
	const char *name, *ext, *p;
	unsigned char *q;
	int i, j;

	gg_debug(GG_DEBUG_FUNCTION, "** gg_dcc_fill_file_info2(%p, \"%s\", \"%s\");\n", d, filename, local_filename);

	if (!d || d->type != GG_SESSION_DCC_SEND) {
		gg_debug(GG_DEBUG_MISC, "// gg_dcc_fill_file_info2() invalid arguments\n");
		errno = EINVAL;
		return -1;
	}

	if (stat(local_filename, &st) == -1) {
		gg_debug(GG_DEBUG_MISC, "// gg_dcc_fill_file_info2() stat() failed (%s)\n", strerror(errno));
		return -1;
	}

	if ((st.st_mode & S_IFDIR)) {
		gg_debug(GG_DEBUG_MISC, "// gg_dcc_fill_file_info2() that's a directory\n");
		errno = EINVAL;
		return -1;
	}

	if ((d->file_fd = open(local_filename, O_RDONLY | O_BINARY)) == -1) {
		gg_debug(GG_DEBUG_MISC, "// gg_dcc_fill_file_info2() open() failed (%s)\n", strerror(errno));
		return -1;
	}

	memset(&d->file_info, 0, sizeof(d->file_info));

	if (!(st.st_mode & S_IWUSR))
		d->file_info.mode |= gg_fix32(GG_DCC_FILEATTR_READONLY);

	gg_dcc_fill_filetime(st.st_atime, d->file_info.atime);
	gg_dcc_fill_filetime(st.st_mtime, d->file_info.mtime);
	gg_dcc_fill_filetime(st.st_ctime, d->file_info.ctime);

	d->file_info.size = gg_fix32(st.st_size);
	d->file_info.mode = gg_fix32(0x20);	/* FILE_ATTRIBUTE_ARCHIVE */

#ifdef _WIN32
	if (!(name = strrchr(filename, '\\')))
#else
	if (!(name = strrchr(filename, '/')))
#endif
		name = filename;
	else
		name++;

	if (!(ext = strrchr(name, '.')))
		ext = name + strlen(name);

	for (i = 0, p = name; i < 8 && p < ext; i++, p++)
		d->file_info.short_filename[i] = toupper(name[i]);

	if (i == 8 && p < ext) {
		d->file_info.short_filename[6] = '~';
		d->file_info.short_filename[7] = '1';
	}

	if (strlen(ext) > 0) {
		for (j = 0; *ext && j < 4; j++, p++)
			d->file_info.short_filename[i + j] = toupper(ext[j]);
	}

	for (q = d->file_info.short_filename; *q; q++) {
		if (*q == 185) {
			*q = 165;
		} else if (*q == 230) {
			*q = 198;
		} else if (*q == 234) {
			*q = 202;
		} else if (*q == 179) {
			*q = 163;
		} else if (*q == 241) {
			*q = 209;
		} else if (*q == 243) {
			*q = 211;
		} else if (*q == 156) {
			*q = 140;
		} else if (*q == 159) {
			*q = 143;
		} else if (*q == 191) {
			*q = 175;
		}
	}

	gg_debug(GG_DEBUG_MISC, "// gg_dcc_fill_file_info2() short name \"%s\", dos name \"%s\"\n", name, d->file_info.short_filename);
	strncpy((char*) d->file_info.filename, name, sizeof(d->file_info.filename) - 1);

	return 0;
}

/**
 * \internal Rozpoczyna połączenie bezpośrednie z danym klientem.
 *
 * \param ip Adres IP odbiorcy
 * \param port Port odbiorcy
 * \param my_uin Własny numer
 * \param peer_uin Numer odbiorcy
 * \param type Rodzaj połączenia (\c GG_SESSION_DCC_SEND lub \c GG_SESSION_DCC_GET)
 *
 * \return Struktura \c gg_dcc lub \c NULL w przypadku błędu
 */
static struct gg_dcc *gg_dcc_transfer(uint32_t ip, uint16_t port, uin_t my_uin, uin_t peer_uin, int type)
{
	struct gg_dcc *d = NULL;
	struct in_addr addr;

	addr.s_addr = ip;

	gg_debug(GG_DEBUG_FUNCTION, "** gg_dcc_transfer(%s, %d, %ld, %ld, %s);\n", inet_ntoa(addr), port, my_uin, peer_uin, (type == GG_SESSION_DCC_SEND) ? "SEND" : "GET");

	if (!ip || ip == INADDR_NONE || !port || !my_uin || !peer_uin) {
		gg_debug(GG_DEBUG_MISC, "// gg_dcc_transfer() invalid arguments\n");
		errno = EINVAL;
		return NULL;
	}

	if (!(d = (gg_dcc*)calloc(1, sizeof(*d)))) {
		gg_debug(GG_DEBUG_MISC, "// gg_dcc_transfer() not enough memory\n");
		return NULL;
	}

	d->check = GG_CHECK_WRITE;
	d->state = GG_STATE_CONNECTING;
	d->type = type;
	d->timeout = GG_DEFAULT_TIMEOUT;
	d->file_fd = -1;
	d->active = 1;
	d->fd = -1;
	d->uin = my_uin;
	d->peer_uin = peer_uin;

	if ((d->fd = gg_connect(&addr, port, 1)) == -1) {
		gg_debug(GG_DEBUG_MISC, "// gg_dcc_transfer() connection failed\n");
		free(d);
		return NULL;
	}

	return d;
}

/**
 * Rozpoczyna odbieranie pliku przez zwrotne połączenie bezpośrednie.
 *
 * \param ip Adres IP nadawcy
 * \param port Port nadawcy
 * \param my_uin Własny numer
 * \param peer_uin Numer nadawcy
 *
 * \return Struktura \c gg_dcc lub \c NULL w przypadku błędu
 *
 * \ingroup dcc6
 */
struct gg_dcc *gg_dcc_get_file(uint32_t ip, uint16_t port, uin_t my_uin, uin_t peer_uin)
{
	gg_debug(GG_DEBUG_MISC, "// gg_dcc_get_file() handing over to gg_dcc_transfer()\n");

	return gg_dcc_transfer(ip, port, my_uin, peer_uin, GG_SESSION_DCC_GET);
}

/**
 * Rozpoczyna wysyłanie pliku.
 *
 * \param ip Adres IP odbiorcy
 * \param port Port odbiorcy
 * \param my_uin Własny numer
 * \param peer_uin Numer odbiorcy
 *
 * \return Struktura \c gg_dcc lub \c NULL w przypadku błędu
 *
 * \ingroup dcc6
 */
struct gg_dcc *gg_dcc_send_file(uint32_t ip, uint16_t port, uin_t my_uin, uin_t peer_uin)
{
	gg_debug(GG_DEBUG_MISC, "// gg_dcc_send_file() handing over to gg_dcc_transfer()\n");

	return gg_dcc_transfer(ip, port, my_uin, peer_uin, GG_SESSION_DCC_SEND);
}

/**
 * Rozpoczyna połączenie głosowe.
 *
 * \param ip Adres IP odbiorcy
 * \param port Port odbiorcy
 * \param my_uin Własny numer
 * \param peer_uin Numer odbiorcy
 *
 * \return Struktura \c gg_dcc lub \c NULL w przypadku błędu
 *
 * \ingroup dcc6
 */
struct gg_dcc *gg_dcc_voice_chat(uint32_t ip, uint16_t port, uin_t my_uin, uin_t peer_uin)
{
	gg_debug(GG_DEBUG_MISC, "// gg_dcc_voice_chat() handing over to gg_dcc_transfer()\n");

	return gg_dcc_transfer(ip, port, my_uin, peer_uin, GG_SESSION_DCC_VOICE);
}

/**
 * Ustawia typ przychodzącego połączenia bezpośredniego.
 *
 * Funkcję należy wywołać po otrzymaniu zdarzenia \c GG_EVENT_DCC_CALLBACK.
 *
 * \param d Struktura połączenia
 * \param type Rodzaj połączenia (\c GG_SESSION_DCC_SEND lub
 *             \c GG_SESSION_DCC_VOICE)
 *
 * \ingroup dcc6
 */
void gg_dcc_set_type(struct gg_dcc *d, int type)
{
	d->type = type;
	d->state = (type == GG_SESSION_DCC_SEND) ? GG_STATE_SENDING_FILE_INFO : GG_STATE_SENDING_VOICE_REQUEST;
}

/**
 * \internal Funkcja zwrotna połączenia bezpośredniego.
 *
 * Pole \c callback struktury \c gg_dcc zawiera wskaźnik do tej funkcji.
 * Wywołuje ona \c gg_watch_fd() i zachowuje wynik w polu \c event.
 *
 * \note Funkcjonalność funkcjo zwrotnej nie jest już wspierana.
 *
 * \param d Struktura połączenia
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 */
static int gg_dcc_callback(struct gg_dcc *d)
{
	struct gg_event *e = gg_dcc_watch_fd(d);

	d->event = e;

	return (e != NULL) ? 0 : -1;
}

/**
 * Tworzy gniazdo nasłuchujące dla połączeń bezpośrednich.
 *
 * Funkcja przywiązuje gniazdo do pierwszego wolnego portu TCP.
 *
 * \param uin Własny numer
 * \param port Preferowany port (jeśli równy 0 lub -1, próbuje się domyślnego)
 *
 * \return Struktura \c gg_dcc lub \c NULL w przypadku błędu
 *
 * \ingroup dcc6
 */
struct gg_dcc *gg_dcc_socket_create(uin_t uin, uint16_t port)
{
	struct gg_dcc *c;
	struct sockaddr_in sin;
	SOCKET sock;
	int bound = 0, errno2;

	gg_debug(GG_DEBUG_FUNCTION, "** gg_create_dcc_socket(%d, %d);\n", uin, port);

	if (!uin) {
		gg_debug(GG_DEBUG_MISC, "// gg_create_dcc_socket() invalid arguments\n");
		errno = EINVAL;
		return NULL;
	}

	if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		gg_debug(GG_DEBUG_MISC, "// gg_create_dcc_socket() can't create socket (%s)\n", strerror(errno));
		return NULL;
	}

	if (port == 0 || port == -1)
		port = GG_DEFAULT_DCC_PORT;

	while (!bound) {
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = INADDR_ANY;
		sin.sin_port = htons(port);

		gg_debug(GG_DEBUG_MISC, "// gg_create_dcc_socket() trying port %d\n", port);
		if (!bind(sock, (struct sockaddr*) &sin, sizeof(sin)))
			bound = 1;
		else {
			if (++port == 65535) {
				gg_debug(GG_DEBUG_MISC, "// gg_create_dcc_socket() no free port found\n");
				gg_sock_close(sock);
				return NULL;
			}
		}
	}

	if (listen(sock, 10)) {
		gg_debug(GG_DEBUG_MISC, "// gg_create_dcc_socket() unable to listen (%s)\n", strerror(errno));
		errno2 = errno;
		gg_sock_close(sock);
		errno = errno2;
		return NULL;
	}

	gg_debug(GG_DEBUG_MISC, "// gg_create_dcc_socket() bound to port %d\n", port);

	if (!(c = (gg_dcc*)malloc(sizeof(*c)))) {
		gg_debug(GG_DEBUG_MISC, "// gg_create_dcc_socket() not enough memory for struct\n");
		gg_sock_close(sock);
		return NULL;
	}
	memset(c, 0, sizeof(*c));

	c->port = c->id = port;
	c->fd = sock;
	c->type = GG_SESSION_DCC_SOCKET;
	c->uin = uin;
	c->timeout = -1;
	c->state = GG_STATE_LISTENING;
	c->check = GG_CHECK_READ;
	c->callback = gg_dcc_callback;
	c->destroy = gg_dcc_free;

	return c;
}

/**
 * Wysyła ramkę danych połączenia głosowego.
 *
 * \param d Struktura połączenia
 * \param buf Bufor z danymi
 * \param length Długość bufora z danymi
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 *
 * \ingroup dcc6
 */
int gg_dcc_voice_send(struct gg_dcc *d, char *buf, int length)
{
	struct packet_s {
		uint8_t type;
		uint32_t length;
	} GG_PACKED;
	struct packet_s packet;

	gg_debug(GG_DEBUG_FUNCTION, "++ gg_dcc_voice_send(%p, %p, %d);\n", d, buf, length);
	if (!d || !buf || length < 0 || d->type != GG_SESSION_DCC_VOICE) {
		gg_debug(GG_DEBUG_MISC, "// gg_dcc_voice_send() invalid argument\n");
		errno = EINVAL;
		return -1;
	}

	packet.type = 0x03; /* XXX */
	packet.length = gg_fix32(length);

	if (gg_sock_write(d->fd, &packet, sizeof(packet)) < (signed)sizeof(packet)) {
		gg_debug(GG_DEBUG_MISC, "// gg_dcc_voice_send() write() failed\n");
		return -1;
	}
	gg_dcc_debug_data("write", d->fd, &packet, sizeof(packet));

	if (gg_sock_write(d->fd, buf, length) < length) {
		gg_debug(GG_DEBUG_MISC, "// gg_dcc_voice_send() write() failed\n");
		return -1;
	}
	gg_dcc_debug_data("write", d->fd, buf, length);

	return 0;
}

/**
 * \internal Odbiera dane z połączenia bezpośredniego z obsługą błędów.
 *
 * \param fd Deskryptor gniazda
 * \param buf Bufor na dane
 * \param size Rozmiar bufora na dane
 */
#define gg_dcc_read(fd, buf, size) \
{ \
	int tmp = gg_sock_read(fd, buf, size); \
	\
	if (tmp < (int) size) { \
		if (tmp == -1) { \
			gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() read() failed (errno=%d, %s)\n", errno, strerror(errno)); \
		} else if (tmp == 0) { \
			gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() read() failed, connection broken\n"); \
		} else { \
			gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() read() failed (%d bytes, %d needed)\n", tmp, size); \
		} \
		e->type = GG_EVENT_DCC_ERROR; \
		e->event.dcc_error = GG_ERROR_DCC_HANDSHAKE; \
		return e; \
	} \
	gg_dcc_debug_data("read", fd, buf, size); \
}

/**
 * \internal Wysyła dane do połączenia bezpośredniego z obsługą błędów.
 *
 * \param fd Deskryptor gniazda
 * \param buf Bufor z danymi
 * \param size Rozmiar bufora z danymi
 */
#define gg_dcc_write(fd, buf, size) \
{ \
	int tmp; \
	gg_dcc_debug_data("write", fd, buf, size); \
	tmp = gg_sock_write(fd, buf, size); \
	if (tmp < (int) size) { \
		if (tmp == -1) { \
			gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() write() failed (errno=%d, %s)\n", errno, strerror(errno)); \
		} else { \
			gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() write() failed (%d needed, %d done)\n", size, tmp); \
		} \
		e->type = GG_EVENT_DCC_ERROR; \
		e->event.dcc_error = GG_ERROR_DCC_HANDSHAKE; \
		return e; \
	} \
}

/**
 * Funkcja wywoływana po zaobserwowaniu zmian na deskryptorze połączenia.
 *
 * Funkcja zwraca strukturę zdarzenia \c gg_event. Jeśli rodzaj zdarzenia
 * to \c GG_EVENT_NONE, nie wydarzyło się jeszcze nic wartego odnotowania.
 * Strukturę zdarzenia należy zwolnić funkcja \c gg_event_free.
 *
 * \param h Struktura połączenia
 *
 * \return Struktura zdarzenia lub \c NULL jeśli wystąpił błąd
 *
 * \ingroup dcc6
 */
struct gg_event *gg_dcc_watch_fd(struct gg_dcc *h)
{
	struct gg_event *e;
	int foo;

	gg_debug(GG_DEBUG_FUNCTION, "** gg_dcc_watch_fd(%p);\n", h);

	if (!h || (h->type != GG_SESSION_DCC && h->type != GG_SESSION_DCC_SOCKET && h->type != GG_SESSION_DCC_SEND && h->type != GG_SESSION_DCC_GET && h->type != GG_SESSION_DCC_VOICE)) {
		gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() invalid argument\n");
		errno = EINVAL;
		return NULL;
	}

	if (!(e = (gg_event*)calloc(1, sizeof(*e)))) {
		gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() not enough memory\n");
		return NULL;
	}

	e->type = GG_EVENT_NONE;

	if (h->type == GG_SESSION_DCC_SOCKET) {
		struct sockaddr_in sin;
		struct gg_dcc *c;
		SOCKET fd;
		int one = 1;
		int sin_len = sizeof(sin);

		if ((fd = accept(h->fd, (struct sockaddr*) &sin, &sin_len)) == -1) {
			gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() can't accept() new connection (errno=%d, %s)\n", errno, strerror(errno));
			return e;
		}

		gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() new direct connection from %s:%d\n", inet_ntoa(sin.sin_addr), htons(sin.sin_port));

#ifdef FIONBIO
		if (ioctl(fd, FIONBIO, &one) == -1) {
#else
		if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1) {
#endif
			gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() can't set nonblocking (errno=%d, %s)\n", errno, strerror(errno));
			gg_sock_close(fd);
			e->type = GG_EVENT_DCC_ERROR;
			e->event.dcc_error = GG_ERROR_DCC_HANDSHAKE;
			return e;
		}

		if (!(c = (gg_dcc*)calloc(1, sizeof(*c)))) {
			gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() not enough memory for client data\n");

			free(e);
			gg_sock_close(fd);
			return NULL;
		}

		c->fd = fd;
		c->check = GG_CHECK_READ;
		c->state = GG_STATE_READING_UIN_1;
		c->type = GG_SESSION_DCC;
		c->timeout = GG_DEFAULT_TIMEOUT;
		c->file_fd = -1;
		c->remote_addr = sin.sin_addr.s_addr;
		c->remote_port = ntohs(sin.sin_port);

		e->type = GG_EVENT_DCC_NEW;
		e->event.dcc_new = c;

		return e;
	} else {
		struct gg_dcc_tiny_packet tiny;
		struct gg_dcc_small_packet small;
		struct gg_dcc_big_packet big;
		int size, tmp, res;
		unsigned int utmp;
		int res_size = sizeof(res);
		char buf[1024], ack[] = "UDAG";

		struct gg_dcc_file_info_packet {
			struct gg_dcc_big_packet big;
			struct gg_file_info file_info;
		} GG_PACKED;
		struct gg_dcc_file_info_packet file_info_packet;

		switch (h->state) {
			case GG_STATE_READING_UIN_1:
			case GG_STATE_READING_UIN_2:
			{
				uin_t uin;

				gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() GG_READING_UIN_%d\n", (h->state == GG_STATE_READING_UIN_1) ? 1 : 2);

				gg_dcc_read(h->fd, &uin, sizeof(uin));

				if (h->state == GG_STATE_READING_UIN_1) {
					h->state = GG_STATE_READING_UIN_2;
					h->check = GG_CHECK_READ;
					h->timeout = GG_DEFAULT_TIMEOUT;
					h->peer_uin = gg_fix32(uin);
				} else {
					h->state = GG_STATE_SENDING_ACK;
					h->check = GG_CHECK_WRITE;
					h->timeout = GG_DEFAULT_TIMEOUT;
					h->uin = gg_fix32(uin);
					e->type = GG_EVENT_DCC_CLIENT_ACCEPT;
				}

				return e;
			}

			case GG_STATE_SENDING_ACK:
				gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() GG_SENDING_ACK\n");

				gg_dcc_write(h->fd, ack, 4);

				h->state = GG_STATE_READING_TYPE;
				h->check = GG_CHECK_READ;
				h->timeout = GG_DEFAULT_TIMEOUT;

				return e;

			case GG_STATE_READING_TYPE:
				gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() GG_STATE_READING_TYPE\n");

				gg_dcc_read(h->fd, &small, sizeof(small));

				small.type = gg_fix32(small.type);

				switch (small.type) {
					case 0x0003:	/* XXX */
						gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() callback\n");
						h->type = GG_SESSION_DCC_SEND;
						h->state = GG_STATE_SENDING_FILE_INFO;
						h->check = GG_CHECK_WRITE;
						h->timeout = GG_DEFAULT_TIMEOUT;

						e->type = GG_EVENT_DCC_CALLBACK;

						break;

					case 0x0002:	/* XXX */
						gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() dialin\n");
						h->type = GG_SESSION_DCC_GET;
						h->state = GG_STATE_READING_REQUEST;
						h->check = GG_CHECK_READ;
						h->timeout = GG_DEFAULT_TIMEOUT;
						h->incoming = 1;

						break;

					default:
						gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() unknown dcc type (%.4x) from %ld\n", small.type, h->peer_uin);
						e->type = GG_EVENT_DCC_ERROR;
						e->event.dcc_error = GG_ERROR_DCC_HANDSHAKE;
				}

				return e;

			case GG_STATE_READING_REQUEST:
				gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() GG_STATE_READING_REQUEST\n");

				gg_dcc_read(h->fd, &small, sizeof(small));

				small.type = gg_fix32(small.type);

				switch (small.type) {
					case 0x0001:	/* XXX */
						gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() file transfer request\n");
						h->state = GG_STATE_READING_FILE_INFO;
						h->check = GG_CHECK_READ;
						h->timeout = GG_DEFAULT_TIMEOUT;
						break;

					case 0x0003:	/* XXX */
						gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() voice chat request\n");
						h->state = GG_STATE_SENDING_VOICE_ACK;
						h->check = GG_CHECK_WRITE;
						h->timeout = GG_DCC_TIMEOUT_VOICE_ACK;
						h->type = GG_SESSION_DCC_VOICE;
						e->type = GG_EVENT_DCC_NEED_VOICE_ACK;

						break;

					default:
						gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() unknown dcc request (%.4x) from %ld\n", small.type, h->peer_uin);
						e->type = GG_EVENT_DCC_ERROR;
						e->event.dcc_error = GG_ERROR_DCC_HANDSHAKE;
				}

				return e;

			case GG_STATE_READING_FILE_INFO:
				gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() GG_STATE_READING_FILE_INFO\n");

				gg_dcc_read(h->fd, &file_info_packet, sizeof(file_info_packet));

				memcpy(&h->file_info, &file_info_packet.file_info, sizeof(h->file_info));

				h->file_info.mode = gg_fix32(h->file_info.mode);
				h->file_info.size = gg_fix32(h->file_info.size);

				h->state = GG_STATE_SENDING_FILE_ACK;
				h->check = GG_CHECK_WRITE;
				h->timeout = GG_DCC_TIMEOUT_FILE_ACK;

				e->type = GG_EVENT_DCC_NEED_FILE_ACK;

				return e;

			case GG_STATE_SENDING_FILE_ACK:
				gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() GG_STATE_SENDING_FILE_ACK\n");

				big.type = gg_fix32(0x0006);	/* XXX */
				big.dunno1 = gg_fix32(h->offset);
				big.dunno2 = 0;

				gg_dcc_write(h->fd, &big, sizeof(big));

				h->state = GG_STATE_READING_FILE_HEADER;
				h->chunk_size = sizeof(big);
				h->chunk_offset = 0;
				if (!(h->chunk_buf = (char*)malloc(sizeof(big)))) {
					gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() out of memory\n");
					free(e);
					return NULL;
				}
				h->check = GG_CHECK_READ;
				h->timeout = GG_DEFAULT_TIMEOUT;

				return e;

			case GG_STATE_SENDING_VOICE_ACK:
				gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() GG_STATE_SENDING_VOICE_ACK\n");

				tiny.type = 0x01;	/* XXX */

				gg_dcc_write(h->fd, &tiny, sizeof(tiny));

				h->state = GG_STATE_READING_VOICE_HEADER;
				h->check = GG_CHECK_READ;
				h->timeout = GG_DEFAULT_TIMEOUT;

				h->offset = 0;

				return e;

			case GG_STATE_READING_FILE_HEADER:
				gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() GG_STATE_READING_FILE_HEADER\n");

				tmp = gg_sock_read(h->fd, h->chunk_buf + h->chunk_offset, h->chunk_size - h->chunk_offset);

				if (tmp == -1) {
					gg_debug(GG_DEBUG_MISC, "// gg_watch_fd() read() failed (errno=%d, %s)\n", errno, strerror(errno));
					e->type = GG_EVENT_DCC_ERROR;
					e->event.dcc_error = GG_ERROR_DCC_NET;
					return e;
				}

				gg_dcc_debug_data("read", h->fd, h->chunk_buf + h->chunk_offset, h->chunk_size - h->chunk_offset);

				h->chunk_offset += tmp;

				if (h->chunk_offset < h->chunk_size)
					return e;

				memcpy(&big, h->chunk_buf, sizeof(big));
				free(h->chunk_buf);
				h->chunk_buf = NULL;

				big.type = gg_fix32(big.type);
				h->chunk_size = gg_fix32(big.dunno1);
				h->chunk_offset = 0;

				if (big.type == 0x0005)	{ /* XXX */
					gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() transfer refused\n");
					e->type = GG_EVENT_DCC_ERROR;
					e->event.dcc_error = GG_ERROR_DCC_REFUSED;
					return e;
				}

				if (h->chunk_size == 0) {
					gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() empty chunk, EOF\n");
					e->type = GG_EVENT_DCC_DONE;
					return e;
				}

				h->state = GG_STATE_GETTING_FILE;
				h->check = GG_CHECK_READ;
				h->timeout = GG_DEFAULT_TIMEOUT;
				h->established = 1;

				return e;

			case GG_STATE_READING_VOICE_HEADER:
				gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() GG_STATE_READING_VOICE_HEADER\n");

				gg_dcc_read(h->fd, &tiny, sizeof(tiny));

				switch (tiny.type) {
					case 0x03:	/* XXX */
						h->state = GG_STATE_READING_VOICE_SIZE;
						h->check = GG_CHECK_READ;
						h->timeout = GG_DEFAULT_TIMEOUT;
						h->established = 1;
						break;
					case 0x04:	/* XXX */
						gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() peer breaking connection\n");
						/* XXX zwracać odpowiedni event */
					default:
						gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() unknown request (%.2x)\n", tiny.type);
						e->type = GG_EVENT_DCC_ERROR;
						e->event.dcc_error = GG_ERROR_DCC_HANDSHAKE;
				}

				return e;

			case GG_STATE_READING_VOICE_SIZE:
				gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() GG_STATE_READING_VOICE_SIZE\n");

				gg_dcc_read(h->fd, &small, sizeof(small));

				small.type = gg_fix32(small.type);

				if (small.type < 16 || small.type > sizeof(buf)) {
					gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() invalid voice frame size (%d)\n", small.type);
					e->type = GG_EVENT_DCC_ERROR;
					e->event.dcc_error = GG_ERROR_DCC_NET;

					return e;
				}

				h->chunk_size = small.type;
				h->chunk_offset = 0;

				if (!(h->voice_buf = (char*)malloc(h->chunk_size))) {
					gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() out of memory for voice frame\n");
					free(e);
					return NULL;
				}

				h->state = GG_STATE_READING_VOICE_DATA;
				h->check = GG_CHECK_READ;
				h->timeout = GG_DEFAULT_TIMEOUT;

				return e;

			case GG_STATE_READING_VOICE_DATA:
				gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() GG_STATE_READING_VOICE_DATA\n");

				tmp = gg_sock_read(h->fd, h->voice_buf + h->chunk_offset, h->chunk_size - h->chunk_offset);
				if (tmp < 1) {
					if (tmp == -1) {
						gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() read() failed (errno=%d, %s)\n", errno, strerror(errno));
					} else {
						gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() read() failed, connection broken\n");
					}
					e->type = GG_EVENT_DCC_ERROR;
					e->event.dcc_error = GG_ERROR_DCC_NET;
					return e;
				}

				gg_dcc_debug_data("read", h->fd, h->voice_buf + h->chunk_offset, tmp);

				h->chunk_offset += tmp;

				if (h->chunk_offset >= h->chunk_size) {
					e->type = GG_EVENT_DCC_VOICE_DATA;
					e->event.dcc_voice_data.data = (unsigned char*) h->voice_buf;
					e->event.dcc_voice_data.length = h->chunk_size;
					h->state = GG_STATE_READING_VOICE_HEADER;
					h->voice_buf = NULL;
				}

				h->check = GG_CHECK_READ;
				h->timeout = GG_DEFAULT_TIMEOUT;

				return e;

			case GG_STATE_CONNECTING:
			{
				uin_t uins[2];

				gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() GG_STATE_CONNECTING\n");

				res = 0;
				if ((foo = gg_getsockopt(h->fd, SOL_SOCKET, SO_ERROR, &res, &res_size)) || res) {
					gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() connection failed (fd=%d,errno=%d(%s),foo=%d,res=%d(%s))\n", h->fd, errno, strerror(errno), foo, res, strerror(res));
					e->type = GG_EVENT_DCC_ERROR;
					e->event.dcc_error = GG_ERROR_DCC_HANDSHAKE;
					return e;
				}

				gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() connected, sending uins\n");

				uins[0] = gg_fix32(h->uin);
				uins[1] = gg_fix32(h->peer_uin);

				gg_dcc_write(h->fd, uins, sizeof(uins));

				h->state = GG_STATE_READING_ACK;
				h->check = GG_CHECK_READ;
				h->timeout = GG_DEFAULT_TIMEOUT;

				return e;
			}

			case GG_STATE_READING_ACK:
				gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() GG_STATE_READING_ACK\n");

				gg_dcc_read(h->fd, buf, 4);

				if (strncmp(buf, ack, 4)) {
					gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() did't get ack\n");

					e->type = GG_EVENT_DCC_ERROR;
					e->event.dcc_error = GG_ERROR_DCC_HANDSHAKE;
					return e;
				}

				h->check = GG_CHECK_WRITE;
				h->timeout = GG_DEFAULT_TIMEOUT;
				h->state = GG_STATE_SENDING_REQUEST;

				return e;

			case GG_STATE_SENDING_VOICE_REQUEST:
				gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() GG_STATE_SENDING_VOICE_REQUEST\n");

				small.type = gg_fix32(0x0003);

				gg_dcc_write(h->fd, &small, sizeof(small));

				h->state = GG_STATE_READING_VOICE_ACK;
				h->check = GG_CHECK_READ;
				h->timeout = GG_DEFAULT_TIMEOUT;

				return e;

			case GG_STATE_SENDING_REQUEST:
				gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() GG_STATE_SENDING_REQUEST\n");

				small.type = (h->type == GG_SESSION_DCC_GET) ? gg_fix32(0x0003) : gg_fix32(0x0002);	/* XXX */

				gg_dcc_write(h->fd, &small, sizeof(small));

				switch (h->type) {
					case GG_SESSION_DCC_GET:
						h->state = GG_STATE_READING_REQUEST;
						h->check = GG_CHECK_READ;
						h->timeout = GG_DEFAULT_TIMEOUT;
						break;

					case GG_SESSION_DCC_SEND:
						h->state = GG_STATE_SENDING_FILE_INFO;
						h->check = GG_CHECK_WRITE;
						h->timeout = GG_DEFAULT_TIMEOUT;

						if (h->file_fd == -1)
							e->type = GG_EVENT_DCC_NEED_FILE_INFO;
						break;

					case GG_SESSION_DCC_VOICE:
						h->state = GG_STATE_SENDING_VOICE_REQUEST;
						h->check = GG_CHECK_WRITE;
						h->timeout = GG_DEFAULT_TIMEOUT;
						break;
				}

				return e;

			case GG_STATE_SENDING_FILE_INFO:
				gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() GG_STATE_SENDING_FILE_INFO\n");

				if (h->file_fd == -1) {
					e->type = GG_EVENT_DCC_NEED_FILE_INFO;
					return e;
				}

				small.type = gg_fix32(0x0001);	/* XXX */

				gg_dcc_write(h->fd, &small, sizeof(small));

				file_info_packet.big.type = gg_fix32(0x0003);	/* XXX */
				file_info_packet.big.dunno1 = 0;
				file_info_packet.big.dunno2 = 0;

				memcpy(&file_info_packet.file_info, &h->file_info, sizeof(h->file_info));

				/* zostają teraz u nas, więc odwracamy z powrotem */
				h->file_info.size = gg_fix32(h->file_info.size);
				h->file_info.mode = gg_fix32(h->file_info.mode);

				gg_dcc_write(h->fd, &file_info_packet, sizeof(file_info_packet));

				h->state = GG_STATE_READING_FILE_ACK;
				h->check = GG_CHECK_READ;
				h->timeout = GG_DCC_TIMEOUT_FILE_ACK;

				return e;

			case GG_STATE_READING_FILE_ACK:
				gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() GG_STATE_READING_FILE_ACK\n");

				gg_dcc_read(h->fd, &big, sizeof(big));

				/* XXX sprawdzać wynik */
				h->offset = gg_fix32(big.dunno1);

				h->state = GG_STATE_SENDING_FILE_HEADER;
				h->check = GG_CHECK_WRITE;
				h->timeout = GG_DEFAULT_TIMEOUT;

				e->type = GG_EVENT_DCC_ACK;

				return e;

			case GG_STATE_READING_VOICE_ACK:
				gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() GG_STATE_READING_VOICE_ACK\n");

				gg_dcc_read(h->fd, &tiny, sizeof(tiny));

				if (tiny.type != 0x01) {
					gg_debug(GG_DEBUG_MISC, "// invalid reply (%.2x), connection refused\n", tiny.type);
					e->type = GG_EVENT_DCC_ERROR;
					e->event.dcc_error = GG_ERROR_DCC_REFUSED;
					return e;
				}

				h->state = GG_STATE_READING_VOICE_HEADER;
				h->check = GG_CHECK_READ;
				h->timeout = GG_DEFAULT_TIMEOUT;

				e->type = GG_EVENT_DCC_ACK;

				return e;

			case GG_STATE_SENDING_FILE_HEADER:
				gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() GG_STATE_SENDING_FILE_HEADER\n");

				h->chunk_offset = 0;

				if ((h->chunk_size = h->file_info.size - h->offset) > 4096) {
					h->chunk_size = 4096;
					big.type = gg_fix32(0x0003);  /* XXX */
				} else
					big.type = gg_fix32(0x0002);  /* XXX */

				big.dunno1 = gg_fix32(h->chunk_size);
				big.dunno2 = 0;

				gg_dcc_write(h->fd, &big, sizeof(big));

				h->state = GG_STATE_SENDING_FILE;
				h->check = GG_CHECK_WRITE;
				h->timeout = GG_DEFAULT_TIMEOUT;
				h->established = 1;

				return e;

			case GG_STATE_SENDING_FILE:
				gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() GG_STATE_SENDING_FILE\n");

				if ((utmp = h->chunk_size - h->chunk_offset) > sizeof(buf))
					utmp = sizeof(buf);

				gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() offset=%d, size=%d\n", h->offset, h->file_info.size);

				/* koniec pliku? */
				if (h->file_info.size == 0) {
					gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() read() reached eof on empty file\n");
					e->type = GG_EVENT_DCC_DONE;

					return e;
				}

				if (h->offset >= h->file_info.size) {
					gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() offset >= size, finished\n");
					e->type = GG_EVENT_DCC_DONE;
					return e;
				}

				lseek(h->file_fd, h->offset, SEEK_SET);

				size = read(h->file_fd, buf, utmp);

				/* błąd */
				if (size == -1) {
					gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() read() failed. (errno=%d, %s)\n", errno, strerror(errno));

					e->type = GG_EVENT_DCC_ERROR;
					e->event.dcc_error = GG_ERROR_DCC_FILE;

					return e;
				}

				/* koniec pliku? */
				if (size == 0) {
					gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() read() reached eof\n");
					e->type = GG_EVENT_DCC_ERROR;
					e->event.dcc_error = GG_ERROR_DCC_EOF;

					return e;
				}

				/* jeśli wczytaliśmy więcej, utnijmy. */
				if (h->offset + size > h->file_info.size) {
					gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() read() too much (read=%d, ofs=%d, size=%d)\n", size, h->offset, h->file_info.size);
					size = h->file_info.size - h->offset;

					if (size < 1) {
						gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() reached EOF after cutting\n");
						e->type = GG_EVENT_DCC_DONE;
						return e;
					}
				}

				tmp = gg_sock_write(h->fd, buf, size);

				if (tmp == -1) {
					gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() write() failed (%s)\n", strerror(errno));
					e->type = GG_EVENT_DCC_ERROR;
					e->event.dcc_error = GG_ERROR_DCC_NET;
					return e;
				}

				if (tmp == 0) {
					gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() write() failed (connection reset)\n");
					e->type = GG_EVENT_DCC_ERROR;
					e->event.dcc_error = GG_ERROR_DCC_NET;
					return e;
				}

				h->offset += tmp;

				if (h->offset >= h->file_info.size) {
					e->type = GG_EVENT_DCC_DONE;
					return e;
				}

				h->chunk_offset += tmp;

				if (h->chunk_offset >= h->chunk_size) {
					gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() chunk finished\n");
					h->state = GG_STATE_SENDING_FILE_HEADER;
					h->timeout = GG_DEFAULT_TIMEOUT;
				} else {
					h->state = GG_STATE_SENDING_FILE;
					h->timeout = GG_DCC_TIMEOUT_SEND;
				}

				h->check = GG_CHECK_WRITE;

				return e;

			case GG_STATE_GETTING_FILE:
				gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() GG_STATE_GETTING_FILE\n");

				if ((utmp = h->chunk_size - h->chunk_offset) > sizeof(buf))
					utmp = sizeof(buf);

				if (h->offset >= h->file_info.size) {
					gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() offset >= size, finished\n");
					e->type = GG_EVENT_DCC_DONE;
					return e;
				}

				size = gg_sock_read(h->fd, buf, utmp);

				gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() ofs=%d, size=%d, read()=%d\n", h->offset, h->file_info.size, size);

				/* błąd */
				if (size == -1) {
					gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() read() failed. (errno=%d, %s)\n", errno, strerror(errno));

					e->type = GG_EVENT_DCC_ERROR;
					e->event.dcc_error = GG_ERROR_DCC_NET;

					return e;
				}

				/* koniec? */
				if (size == 0) {
					gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() read() reached eof\n");
					e->type = GG_EVENT_DCC_ERROR;
					e->event.dcc_error = GG_ERROR_DCC_EOF;

					return e;
				}

				tmp = write(h->file_fd, buf, size);

				if (tmp == -1 || tmp < size) {
					gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() write() failed (%d:fd=%d:res=%d:%s)\n", tmp, h->file_fd, size, strerror(errno));
					e->type = GG_EVENT_DCC_ERROR;
					e->event.dcc_error = GG_ERROR_DCC_NET;
					return e;
				}

				h->offset += size;

				if (h->offset >= h->file_info.size) {
					e->type = GG_EVENT_DCC_DONE;
					return e;
				}

				h->chunk_offset += size;

				if (h->chunk_offset >= h->chunk_size) {
					gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() chunk finished\n");
					h->state = GG_STATE_READING_FILE_HEADER;
					h->timeout = GG_DEFAULT_TIMEOUT;
					h->chunk_offset = 0;
					h->chunk_size = sizeof(big);
					if (!(h->chunk_buf = (char*)malloc(sizeof(big)))) {
						gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() out of memory\n");
						free(e);
						return NULL;
					}
				} else {
					h->state = GG_STATE_GETTING_FILE;
					h->timeout = GG_DCC_TIMEOUT_GET;
				}

				h->check = GG_CHECK_READ;

				return e;

			default:
				gg_debug(GG_DEBUG_MISC, "// gg_dcc_watch_fd() GG_STATE_???\n");
				e->type = GG_EVENT_DCC_ERROR;
				e->event.dcc_error = GG_ERROR_DCC_HANDSHAKE;

				return e;
		}
	}

	return e;
}

/**
 * Zwalnia zasoby używane przez połączenie bezpośrednie.
 *
 * \param d Struktura połączenia
 *
 * \ingroup dcc6
 */
void gg_dcc_free(struct gg_dcc *d)
{
	gg_debug(GG_DEBUG_FUNCTION, "** gg_dcc_free(%p);\n", d);

	if (!d)
		return;

	if (d->fd != -1)
		gg_sock_close(d->fd);

	free(d->chunk_buf);
	free(d);
}

/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: notnil
 * End:
 *
 * vim: shiftwidth=8:
 */
