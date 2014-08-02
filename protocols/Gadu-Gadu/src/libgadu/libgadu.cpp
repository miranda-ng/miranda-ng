/* coding: UTF-8 */
/* $Id: libgadu.c 13762 2011-08-09 12:35:16Z dezred $ */

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
 * \file libgadu.c
 *
 * \brief Główny moduł biblioteki
 */

#ifndef _WIN64
#define _USE_32BIT_TIME_T
#endif

#include <sys/types.h>
#ifdef _WIN32
#include "win32.h"
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#ifdef sun
#  include <sys/filio.h>
#endif
#endif /* _WIN32 */

#include "compat.h"
#include "libgadu.h"
#include "protocol.h"
#include "resolver.h"
#include "internal.h"

#include <errno.h>
#ifndef _WIN32
#include <netdb.h>
#endif /* _WIN32 */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#ifndef _WIN32
#include <unistd.h>
#endif /* _WIN32 */
#if !defined(GG_CONFIG_MIRANDA) && defined(GG_CONFIG_HAVE_OPENSSL)
#  include <openssl/err.h>
#  include <openssl/rand.h>
#endif

/**
 * Poziom rejestracji informacji odpluskwiających. Zmienna jest maską bitową
 * składającą się ze stałych \c GG_DEBUG_...
 *
 * \ingroup debug
 */
int gg_debug_level = 0;

/**
 * Funkcja, do której są przekazywane informacje odpluskwiające. Jeśli zarówno
 * ten \c gg_debug_handler, jak i \c gg_debug_handler_session, są równe
 * \c NULL, informacje są wysyłane do standardowego wyjścia błędu (\c stderr).
 *
 * \param level Poziom rejestracji
 * \param format Format wiadomości (zgodny z \c printf)
 * \param ap Lista argumentów (zgodna z \c printf)
 *
 * \note Funkcja jest przesłaniana przez \c gg_debug_handler_session.
 *
 * \ingroup debug
 */
void (*gg_debug_handler)(int level, const char *format, va_list ap) = NULL;

/**
 * Funkcja, do której są przekazywane informacje odpluskwiające. Jeśli zarówno
 * ten \c gg_debug_handler, jak i \c gg_debug_handler_session, są równe
 * \c NULL, informacje są wysyłane do standardowego wyjścia błędu.
 *
 * \param sess Sesja której dotyczy informacja lub \c NULL
 * \param level Poziom rejestracji
 * \param format Format wiadomości (zgodny z \c printf)
 * \param ap Lista argumentów (zgodna z \c printf)
 *
 * \note Funkcja przesłania przez \c gg_debug_handler_session.
 *
 * \ingroup debug
 */
void (*gg_debug_handler_session)(struct gg_session *sess, int level, const char *format, va_list ap) = NULL;

/**
 * Port gniazda nasłuchującego dla połączeń bezpośrednich.
 * 
 * \ingroup ip
 */
int gg_dcc_port = 0;

/**
 * Adres IP gniazda nasłuchującego dla połączeń bezpośrednich.
 *
 * \ingroup ip
 */
unsigned long gg_dcc_ip = 0;

/**
 * Adres lokalnego interfejsu IP, z którego wywoływane są wszystkie połączenia.
 *
 * \ingroup ip
 */
unsigned long gg_local_ip = 0;

/**
 * Flaga włączenia połączeń przez serwer pośredniczący.
 *
 * \ingroup proxy
 */
int gg_proxy_enabled = 0;

/**
 * Adres serwera pośredniczącego.
 *
 * \ingroup proxy
 */
char *gg_proxy_host = NULL;

/**
 * Port serwera pośredniczącego.
 *
 * \ingroup proxy
 */
int gg_proxy_port = 0;

/**
 * Flaga używania serwera pośredniczącego jedynie dla usług HTTP.
 *
 * \ingroup proxy
 */
int gg_proxy_http_only = 0;

/**
 * Nazwa użytkownika do autoryzacji serwera pośredniczącego.
 *
 * \ingroup proxy
 */
char *gg_proxy_username = NULL;

/**
 * Hasło użytkownika do autoryzacji serwera pośredniczącego.
 *
 * \ingroup proxy
 */
char *gg_proxy_password = NULL;

#ifndef DOXYGEN

#ifndef lint
static char rcsid[]
#ifdef __GNUC__
__attribute__ ((unused))
#endif
= "$Id: libgadu.c 13762 2011-08-09 12:35:16Z dezred $";
#endif

#endif /* DOXYGEN */

/**
 * Zwraca wersję biblioteki.
 *
 * \return Wskaźnik na statyczny bufor z wersją biblioteki.
 *
 * \ingroup version
 */
const char *gg_libgadu_version()
{
	return GG_LIBGADU_VERSION;
}

#ifdef GG_CONFIG_HAVE_UINT64_T
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
#endif /* GG_CONFIG_HAVE_UINT64_T */

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

/**
 * \internal Liczy skrót z hasła i ziarna.
 *
 * \param password Hasło
 * \param seed Ziarno podane przez serwer
 *
 * \return Wartość skrótu
 */
unsigned int gg_login_hash(const unsigned char *password, unsigned int seed)
{
	unsigned int x, y, z;

	y = seed;

	for (x = 0; *password; password++) {
		x = (x & 0xffffff00) | *password;
		y ^= x;
		y += x;
		x <<= 8;
		y ^= x;
		x <<= 8;
		y -= x;
		x <<= 8;
		y ^= x;

		z = y & 0x1F;
		y = (y << z) | (y >> (32 - z));
	}

	return y;
}

/**
 * \internal Odbiera od serwera dane binarne.
 *
 * Funkcja odbiera dane od serwera zajmując się SSL/TLS w razie konieczności.
 * Obsługuje EINTR, więc użytkownik nie musi się przejmować przerwanymi
 * wywołaniami systemowymi.
 *
 * \param sess Struktura sesji
 * \param buf Bufor na danymi
 * \param length Długość bufora
 *
 * \return To samo co funkcja systemowa \c read
 */
int gg_read(struct gg_session *sess, char *buf, int length)
{
#ifdef GG_CONFIG_MIRANDA
	if (sess->ssl != NULL)
		return si.read(sess->ssl, buf, length, 0);
#elif GG_CONFIG_HAVE_OPENSSL
	if (sess->ssl != NULL) {
		for (;;) {
			int res, err;

			res = SSL_read(sess->ssl, buf, length);

			if (res < 0) {
				err = SSL_get_error(sess->ssl, res);

				if (err == SSL_ERROR_SYSCALL && errno == EINTR)
					continue;

				if (err == SSL_ERROR_WANT_READ)
					errno = EAGAIN;
				else if (err != SSL_ERROR_SYSCALL)
					errno = EINVAL;

				return -1;
			}

			return res;
		}
	}
#endif

	return gg_sock_read(sess->fd, buf, length);
}

/**
 * \internal Wysyła do serwera dane binarne.
 *
 * Funkcja wysyła dane do serwera zajmując się SSL/TLS w razie konieczności.
 * Obsługuje EINTR, więc użytkownik nie musi się przejmować przerwanymi
 * wywołaniami systemowymi.
 *
 * \note Funkcja nie zajmuje się buforowaniem wysyłanych danych (patrz
 * gg_write()).
 *
 * \param sess Struktura sesji
 * \param buf Bufor z danymi
 * \param length Długość bufora
 *
 * \return To samo co funkcja systemowa \c write
 */
static int gg_write_common(struct gg_session *sess, const char *buf, int length)
{
#ifdef GG_CONFIG_MIRANDA
	if (sess->ssl != NULL)
		return si.write(sess->ssl, buf, length);
#elif GG_CONFIG_HAVE_OPENSSL
	if (sess->ssl != NULL) {
		for (;;) {
			int res, err;

			res = SSL_write(sess->ssl, (void *)buf, length);

			if (res < 0) {
				err = SSL_get_error(sess->ssl, res);

				if (err == SSL_ERROR_SYSCALL && errno == EINTR)
					continue;

				if (err == SSL_ERROR_WANT_WRITE)
					errno = EAGAIN;
				else if (err != SSL_ERROR_SYSCALL)
					errno = EINVAL;

				return -1;
			}

			return res;
		}
	}
#endif

	return gg_sock_write(sess->fd, buf, length);
}



/**
 * \internal Wysyła do serwera dane binarne.
 *
 * Funkcja wysyła dane do serwera zajmując się TLS w razie konieczności.
 *
 * \param sess Struktura sesji
 * \param buf Bufor z danymi
 * \param length Długość bufora
 *
 * \return To samo co funkcja systemowa \c write
 */
int gg_write(struct gg_session *sess, const char *buf, int length)
{
	int res = 0;

	if (!sess->async) {
		int written = 0;

		while (written < length) {
			res = gg_write_common(sess, buf + written, length - written);

			if (res == -1)
				return -1;

			written += res;
			res = written;
		}
	} else {
		res = 0;

		if (sess->send_buf == NULL) {
			res = gg_write_common(sess, buf, length);

			if (res == -1)
				return -1;
		}

		if (res < length) {
			char *tmp;

			if (!(tmp = (char*)realloc(sess->send_buf, sess->send_left + length - res))) {
				errno = ENOMEM;
				return -1;
			}

			sess->send_buf = tmp;

			memcpy(sess->send_buf + sess->send_left, buf + res, length - res);

			sess->send_left += length - res;
		}
	}

	return res;
}

/**
 * \internal Odbiera pakiet od serwera.
 *
 * Funkcja odczytuje nagłówek pakietu, a następnie jego zawartość i zwraca
 * w zaalokowanym buforze.
 *
 * Przy połączeniach asynchronicznych, funkcja może nie być w stanie
 * skompletować całego pakietu -- w takim przypadku zwróci -1, a kodem błędu
 * będzie \c EAGAIN.
 *
 * \param sess Struktura sesji
 *
 * \return Wskaźnik do zaalokowanego bufora
 */
void *gg_recv_packet(struct gg_session *sess)
{
	struct gg_header h;
	char *buf = NULL;
	int ret = 0;
	unsigned int offset, size = 0;

	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_recv_packet(%p);\n", sess);

	if (!sess) {
		errno = EFAULT;
		return NULL;
	}

	if (sess->recv_left < 1) {
		if (sess->header_buf) {
			memcpy(&h, sess->header_buf, sess->header_done);
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_recv_packet() header recv: resuming last read (%d bytes left)\n", sizeof(h) - sess->header_done);
			free(sess->header_buf);
			sess->header_buf = NULL;
		} else
			sess->header_done = 0;

		while (sess->header_done < sizeof(h)) {
			ret = gg_read(sess, (char*) &h + sess->header_done, sizeof(h) - sess->header_done);

			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_recv_packet() header recv(%d,%p,%d) = %d\n", sess->fd, &h + sess->header_done, sizeof(h) - sess->header_done, ret);

			if (!ret) {
				errno = ECONNRESET;
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_recv_packet() header recv() failed: connection broken\n");
				return NULL;
			}

			if (ret == -1) {
				if (errno == EAGAIN) {
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_recv_packet() header recv() incomplete header received\n");

					if (!(sess->header_buf = (char*)malloc(sess->header_done))) {
						gg_debug_session(sess, GG_DEBUG_MISC, "// gg_recv_packet() header recv() not enough memory\n");
						return NULL;
					}

					memcpy(sess->header_buf, &h, sess->header_done);

					errno = EAGAIN;

					return NULL;
				}

				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_recv_packet() header recv() failed: errno=%d, %s\n", errno, strerror(errno));

				return NULL;
			}

			sess->header_done += ret;

		}

		h.type = gg_fix32(h.type);
		h.length = gg_fix32(h.length);
	} else
		memcpy(&h, sess->recv_buf, sizeof(h));

	/* jakieś sensowne limity na rozmiar pakietu */
	if (h.length > 65535) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_recv_packet() invalid packet length (%d)\n", h.length);
		errno = ERANGE;
		return NULL;
	}

	if (sess->recv_left > 0) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_recv_packet() resuming last gg_recv_packet()\n");
		size = sess->recv_left;
		offset = sess->recv_done;
		buf = sess->recv_buf;
	} else {
		if (!(buf = (char*)malloc(sizeof(h) + h.length + 1))) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_recv_packet() not enough memory for packet data\n");
			return NULL;
		}

		memcpy(buf, &h, sizeof(h));

		offset = 0;
		size = h.length;
	}

	while (size > 0) {
		ret = gg_read(sess, buf + sizeof(h) + offset, size);
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_recv_packet() body recv(%d,%p,%d) = %d\n", sess->fd, buf + sizeof(h) + offset, size, ret);
		if (!ret) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_recv_packet() body recv() failed: connection broken\n");
			errno = ECONNRESET;
			return NULL;
		}
		if (ret > -1 && ret <= (int)size) {
			offset += ret;
			size -= ret;
		} else if (ret == -1) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_recv_packet() body recv() failed (errno=%d, %s)\n", errno, strerror(errno));

			if (errno == EAGAIN) {
				gg_debug_session(sess, GG_DEBUG_MISC, "// gg_recv_packet() %d bytes received, %d left\n", offset, size);
				sess->recv_buf = buf;
				sess->recv_left = size;
				sess->recv_done = offset;
				return NULL;
			}

			free(buf);
			return NULL;
		}
	}

	sess->recv_left = 0;

	gg_debug_dump_session(sess, buf, sizeof(h) + h.length, "// gg_recv_packet(0x%.2x)", h.type);

	return buf;
}

/**
 * \internal Wysyła pakiet do serwera.
 *
 * Funkcja konstruuje pakiet do wysłania z dowolnej liczby fragmentów. Jeśli
 * rozmiar pakietu jest za duży, by móc go wysłać za jednym razem, pozostała
 * część zostanie zakolejkowana i wysłana, gdy będzie to możliwe.
 *
 * \param sess Struktura sesji
 * \param type Rodzaj pakietu
 * \param ... Lista kolejnych części pakietu (wskaźnik na bufor i długość
 *            typu \c int) zakończona \c NULL
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 */
int gg_send_packet(struct gg_session *sess, int type, ...)
{
	struct gg_header *h;
	char *tmp;
	unsigned int tmp_length;
	void *payload;
	unsigned int payload_length;
	va_list ap;
	int res;

	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_send_packet(%p, 0x%.2x, ...);\n", sess, type);

	tmp_length = sizeof(struct gg_header);

	if (!(tmp = (char*)malloc(tmp_length))) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_send_packet() not enough memory for packet header\n");
		return -1;
	}

	va_start(ap, type);

	payload = va_arg(ap, void *);

	while (payload) {
		char *tmp2;

		payload_length = va_arg(ap, unsigned int);

		if (!(tmp2 = (char*)realloc(tmp, tmp_length + payload_length))) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_send_packet() not enough memory for payload\n");
			free(tmp);
			va_end(ap);
			return -1;
		}

		tmp = tmp2;

		memcpy(tmp + tmp_length, payload, payload_length);
		tmp_length += payload_length;

		payload = va_arg(ap, void *);
	}

	va_end(ap);

	h = (struct gg_header*) tmp;
	h->type = gg_fix32(type);
	h->length = gg_fix32(tmp_length - sizeof(struct gg_header));

	gg_debug_dump_session(sess, tmp, tmp_length, "// gg_send_packet(0x%.2x)", gg_fix32(h->type));

	res = gg_write(sess, tmp, tmp_length);

	free(tmp);

	if (res == -1) {
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_send_packet() write() failed. res = %d, errno = %d (%s)\n", res, errno, strerror(errno));
		return -1;
	}

	if (sess->async)
		gg_debug_session(sess, GG_DEBUG_MISC, "// gg_send_packet() partial write(), %d sent, %d left, %d total left\n", res, tmp_length - res, sess->send_left);

	if (sess->send_buf)
		sess->check |= GG_CHECK_WRITE;

	return 0;
}

/**
 * \internal Funkcja zwrotna sesji.
 *
 * Pole \c callback struktury \c gg_session zawiera wskaźnik do tej funkcji.
 * Wywołuje ona \c gg_watch_fd i zachowuje wynik w polu \c event.
 *
 * \note Korzystanie z tej funkcjonalności nie jest już zalecane.
 *
 * \param sess Struktura sesji
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 */
static int gg_session_callback(struct gg_session *sess)
{
	if (!sess) {
		errno = EFAULT;
		return -1;
	}

	return ((sess->event = gg_watch_fd(sess)) != NULL) ? 0 : -1;
}

/**
 * Łączy się z serwerem Gadu-Gadu.
 *
 * Przy połączeniu synchronicznym funkcja zakończy działanie po nawiązaniu
 * połączenia lub gdy wystąpi błąd. Po udanym połączeniu należy wywoływać
 * funkcję \c gg_watch_fd(), która odbiera informacje od serwera i zwraca
 * informacje o zdarzeniach.
 *
 * Przy połączeniu asynchronicznym funkcja rozpocznie procedurę połączenia
 * i zwróci zaalokowaną strukturę. Pole \c fd struktury \c gg_session zawiera
 * deskryptor, który należy obserwować funkcją \c select, \c poll lub za
 * pomocą mechanizmów użytej pętli zdarzeń (Glib, Qt itp.). Pole \c check
 * jest maską bitową mówiącą, czy biblioteka chce być informowana o możliwości
 * odczytu danych (\c GG_CHECK_READ) czy zapisu danych (\c GG_CHECK_WRITE).
 * Po zaobserwowaniu zmian na deskryptorze należy wywołać funkcję
 * \c gg_watch_fd(). Podczas korzystania z połączeń asynchronicznych, w trakcie
 * połączenia może zostać stworzony dodatkowy proces rozwiązujący nazwę
 * serwera -- z tego powodu program musi poprawnie obsłużyć sygnał SIGCHLD.
 *
 * \note Po nawiązaniu połączenia z serwerem należy wysłać listę kontaktów
 * za pomocą funkcji \c gg_notify() lub \c gg_notify_ex().
 *
 * \param p Struktura opisująca parametry połączenia. Wymagane pola: uin,
 *          password, async.
 *
 * \return Wskaźnik do zaalokowanej struktury sesji \c gg_session lub NULL
 *         w przypadku błędu.
 *
 * \ingroup login
 */
#ifdef GG_CONFIG_MIRANDA
struct gg_session *gg_login(const struct gg_login_params *p, SOCKET *gg_sock, int *gg_failno)
#else
struct gg_session *gg_login(const struct gg_login_params *p)
#endif
{
	struct gg_session *sess = NULL;
	char *hostname;
	int port;

	if (!p) {
		gg_debug(GG_DEBUG_FUNCTION, "** gg_login(%p);\n", p);
		errno = EFAULT;
		return NULL;
	}

	gg_debug(GG_DEBUG_FUNCTION, "** gg_login(%p: [uin=%u, async=%d, ...]);\n", p, p->uin, p->async);

	if (!(sess = (gg_session*)malloc(sizeof(struct gg_session)))) {
		gg_debug(GG_DEBUG_MISC, "// gg_login() not enough memory for session data\n");
		goto fail;
	}

	memset(sess, 0, sizeof(struct gg_session));

	if (!p->password || !p->uin) {
		gg_debug(GG_DEBUG_MISC, "// gg_login() invalid arguments. uin and password needed\n");
		errno = EFAULT;
		goto fail;
	}

	if (!(sess->password = strdup(p->password))) {
		gg_debug(GG_DEBUG_MISC, "// gg_login() not enough memory for password\n");
		goto fail;
	}

	if (p->hash_type < 0 || p->hash_type > GG_LOGIN_HASH_SHA1) {
		gg_debug(GG_DEBUG_MISC, "// gg_login() invalid arguments. unknown hash type (%d)\n", p->hash_type);
		errno = EFAULT;
		goto fail;
	}

	sess->uin = p->uin;
	sess->state = GG_STATE_RESOLVING;
	sess->check = GG_CHECK_READ;
	sess->timeout = GG_DEFAULT_TIMEOUT;
	sess->async = p->async;
	sess->type = GG_SESSION_GG;
	sess->initial_status = p->status;
	sess->callback = gg_session_callback;
	sess->destroy = gg_free_session;
	sess->port = (p->server_port) ? p->server_port : ((gg_proxy_enabled) ? GG_HTTPS_PORT : GG_DEFAULT_PORT);
	sess->server_addr = p->server_addr;
	sess->external_port = p->external_port;
	sess->external_addr = p->external_addr;
	sess->client_port = p->client_port;

	if (p->protocol_features == 0) {
		sess->protocol_features = GG_FEATURE_MSG80 | GG_FEATURE_STATUS80 | GG_FEATURE_DND_FFC | GG_FEATURE_IMAGE_DESCR | GG_FEATURE_UNKNOWN_100 | GG_FEATURE_USER_DATA | GG_FEATURE_MSG_ACK | GG_FEATURE_TYPING_NOTIFICATION;
	} else {
		sess->protocol_features = (p->protocol_features & ~(GG_FEATURE_STATUS77 | GG_FEATURE_MSG77));

		if (!(p->protocol_features & GG_FEATURE_STATUS77))
			sess->protocol_features |= GG_FEATURE_STATUS80;

		if (!(p->protocol_features & GG_FEATURE_MSG77))
			sess->protocol_features |= GG_FEATURE_MSG80;
	}

	if (!(sess->status_flags = p->status_flags))
		sess->status_flags = GG_STATUS_FLAG_UNKNOWN | GG_STATUS_FLAG_SPAM;

	sess->protocol_version = (p->protocol_version) ? p->protocol_version : GG_DEFAULT_PROTOCOL_VERSION;

	if (p->era_omnix)
		sess->protocol_flags |= GG_ERA_OMNIX_MASK;
	if (p->has_audio)
		sess->protocol_flags |= GG_HAS_AUDIO_MASK;
	sess->client_version = (p->client_version) ? strdup(p->client_version) : NULL;
	sess->last_sysmsg = p->last_sysmsg;
	sess->image_size = p->image_size;
	sess->pid = -1;
	sess->encoding = p->encoding;

	if (gg_session_set_resolver(sess, p->resolver) == -1) {
		gg_debug(GG_DEBUG_MISC, "// gg_login() invalid arguments. unsupported resolver type (%d)\n", p->resolver);
		errno = EFAULT;
		goto fail;
	}

	if (p->status_descr) {
		int max_length;

		if (sess->protocol_version >= 0x2d)
			max_length = GG_STATUS_DESCR_MAXSIZE;
		else
			max_length = GG_STATUS_DESCR_MAXSIZE_PRE_8_0;

		if (sess->protocol_version >= 0x2d && p->encoding != GG_ENCODING_UTF8)
			sess->initial_descr = gg_cp_to_utf8(p->status_descr);
		else
			sess->initial_descr = strdup(p->status_descr);

		if (!sess->initial_descr) {
			gg_debug(GG_DEBUG_MISC, "// gg_login() not enough memory for status\n");
			goto fail;
		}
		
		// XXX pamiętać, żeby nie ciąć w środku znaku utf-8
		
		if ((signed)strlen(sess->initial_descr) > max_length)
			sess->initial_descr[max_length] = 0;
	}

#ifdef GG_CONFIG_MIRANDA
	sess->tls = p->tls;
#endif
	if (p->tls == 1) {
#ifdef GG_CONFIG_HAVE_OPENSSL
		char buf[1024];

		OpenSSL_add_ssl_algorithms();

		if (!RAND_status()) {
			char rdata[1024];
			struct {
				time_t time;
				void *ptr;
			} rstruct;

			time(&rstruct.time);
			rstruct.ptr = (void *) &rstruct;

			RAND_seed((void *) rdata, sizeof(rdata));
			RAND_seed((void *) &rstruct, sizeof(rstruct));
		}

		sess->ssl_ctx = SSL_CTX_new(SSLv3_client_method());

		if (!sess->ssl_ctx) {
			ERR_error_string_n(ERR_get_error(), buf, sizeof(buf));
			gg_debug(GG_DEBUG_MISC, "// gg_login() SSL_CTX_new() failed: %s\n", buf);
			goto fail;
		}

		SSL_CTX_set_verify(sess->ssl_ctx, SSL_VERIFY_NONE, NULL);

		sess->ssl = SSL_new(sess->ssl_ctx);

		if (!sess->ssl) {
			ERR_error_string_n(ERR_get_error(), buf, sizeof(buf));
			gg_debug(GG_DEBUG_MISC, "// gg_login() SSL_new() failed: %s\n", buf);
			goto fail;
		}
#elif !defined(GG_CONFIG_MIRANDA)
		gg_debug(GG_DEBUG_MISC, "// gg_login() client requested TLS but no support compiled in\n");
#endif
	}

	if (gg_proxy_enabled) {
		hostname = gg_proxy_host;
		sess->proxy_port = port = gg_proxy_port;
	} else {
		hostname = GG_APPMSG_HOST;
		port = GG_APPMSG_PORT;
	}

	if (p->hash_type)
		sess->hash_type = p->hash_type;
	else
		sess->hash_type = GG_LOGIN_HASH_SHA1;

	if (!p->async) {
		struct in_addr addr;

		if (!sess->server_addr) {
			if ((addr.s_addr = inet_addr(hostname)) == INADDR_NONE) {
				if (gg_gethostbyname_real(hostname, &addr, 0) == -1) {
					gg_debug(GG_DEBUG_MISC, "// gg_login() host \"%s\" not found\n", hostname);
#ifdef GG_CONFIG_MIRANDA
					errno = EACCES;
					*gg_failno = GG_FAILURE_RESOLVING;
#endif
					goto fail;
				}
			}
		} else {
			addr.s_addr = sess->server_addr;
			port = sess->port;
		}

		sess->hub_addr = addr.s_addr;

		if (gg_proxy_enabled)
			sess->proxy_addr = addr.s_addr;

#ifdef GG_CONFIG_MIRANDA
		if ((sess->fd = gg_connect_internal(&addr, port, 0, gg_sock)) == -1) {
#else
		if ((sess->fd = gg_connect(&addr, port, 0)) == -1) {
#endif
			gg_debug(GG_DEBUG_MISC, "// gg_login() connection failed (errno=%d, %s)\n", errno, strerror(errno));

			/* nie wyszło? próbujemy portu 443. */
			if (sess->server_addr) {
				sess->port = GG_HTTPS_PORT;

#ifdef GG_CONFIG_MIRANDA
				if ((sess->fd = gg_connect_internal(&addr, GG_HTTPS_PORT, 0, gg_sock)) == -1) {
#else
				if ((sess->fd = gg_connect(&addr, GG_HTTPS_PORT, 0)) == -1) {
#endif
					/* ostatnia deska ratunku zawiodła?
					 * w takim razie zwijamy manatki. */
					gg_debug_session(sess, GG_DEBUG_MISC, "// gg_login() connection failed (errno=%d, %s)\n", errno, strerror(errno));
					goto fail;
				}
			} else {
				goto fail;
			}
		}

		if (sess->server_addr)
			sess->state = GG_STATE_CONNECTING_GG;
		else
			sess->state = GG_STATE_CONNECTING_HUB;

		while (sess->state != GG_STATE_CONNECTED) {
			struct gg_event *e;

			if (!(e = gg_watch_fd(sess))) {
				gg_debug(GG_DEBUG_MISC, "// gg_login() critical error in gg_watch_fd()\n");
				goto fail;
			}

			if (e->type == GG_EVENT_CONN_FAILED) {
				errno = EACCES;
#ifdef GG_CONFIG_MIRANDA
				*gg_failno = e->event.failure;
#endif
				gg_debug(GG_DEBUG_MISC, "// gg_login() could not login\n");
				gg_event_free(e);
				goto fail;
			}

			gg_event_free(e);
		}

		return sess;
	}

	if (!sess->server_addr || gg_proxy_enabled) {
		if (sess->resolver_start(&sess->fd, &sess->resolver, hostname) == -1) {
			gg_debug(GG_DEBUG_MISC, "// gg_login() resolving failed (errno=%d, %s)\n", errno, strerror(errno));
			goto fail;
		}
	} else {
		if ((sess->fd = gg_connect(&sess->server_addr, sess->port, sess->async)) == -1) {
			gg_debug(GG_DEBUG_MISC, "// gg_login() direct connection failed (errno=%d, %s)\n", errno, strerror(errno));
			goto fail;
		}
		sess->state = GG_STATE_CONNECTING_GG;
		sess->check = GG_CHECK_WRITE;
		sess->soft_timeout = 1;
	}

	return sess;

fail:
	gg_free_session(sess);

	return NULL;
}

/**
 * Wysyła do serwera pakiet utrzymania połączenia.
 *
 * Klient powinien regularnie co minutę wysyłać pakiet utrzymania połączenia,
 * inaczej serwer uzna, że klient stracił łączność z siecią i zerwie
 * połączenie.
 *
 * \param sess Struktura sesji
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 *
 * \ingroup login
 */
int gg_ping(struct gg_session *sess)
{
	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_ping(%p);\n", sess);

	if (!sess) {
		errno = EFAULT;
		return -1;
	}

	if (sess->state != GG_STATE_CONNECTED) {
		errno = ENOTCONN;
		return -1;
	}

	return gg_send_packet(sess, GG_PING, NULL);
}

/**
 * Kończy połączenie z serwerem.
 *
 * Funkcja nie zwalnia zasobów, więc po jej wywołaniu należy użyć
 * \c gg_free_session(). Jeśli chce się ustawić opis niedostępności, należy
 * wcześniej wywołać funkcję \c gg_change_status_descr() lub
 * \c gg_change_status_descr_time().
 *
 * \note Jeśli w buforze nadawczym połączenia z serwerem znajdują się jeszcze
 * dane (np. z powodu strat pakietów na łączu), prawdopodobnie zostaną one
 * utracone przy zrywaniu połączenia. Aby mieć pewność, że opis statusu
 * zostanie zachowany, należy ustawić stan \c GG_STATUS_NOT_AVAIL_DESCR
 * za pomocą funkcji \c gg_change_status_descr() i poczekać na zdarzenie
 * \c GG_EVENT_DISCONNECT_ACK.
 *
 * \param sess Struktura sesji
 *
 * \ingroup login
 */
void gg_logoff(struct gg_session *sess)
{
	if (!sess)
		return;

	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_logoff(%p);\n", sess);

#ifdef GG_CONFIG_MIRANDA
	if (sess->ssl != NULL)
		si.shutdown(sess->ssl);
#elif GG_CONFIG_HAVE_OPENSSL
	if (sess->ssl != NULL)
		SSL_shutdown(sess->ssl);
#endif

	sess->resolver_cleanup(&sess->resolver, 1);

	if (sess->fd != -1) {
		shutdown(sess->fd, SHUT_RDWR);
		gg_sock_close(sess->fd);
		sess->fd = -1;
	}

	if (sess->send_buf) {
		free(sess->send_buf);
		sess->send_buf = NULL;
		sess->send_left = 0;
	}
}

/**
 * Zwalnia zasoby używane przez połączenie z serwerem. Funkcję należy wywołać
 * po zamknięciu połączenia z serwerem, by nie doprowadzić do wycieku zasobów
 * systemowych.
 *
 * \param sess Struktura sesji
 *
 * \ingroup login
 */
void gg_free_session(struct gg_session *sess)
{
	struct gg_dcc7 *dcc;

	if (!sess)
		return;

	/* XXX dopisać zwalnianie i zamykanie wszystkiego, co mogło zostać */

	free(sess->password);
	free(sess->initial_descr);
	free(sess->client_version);
	free(sess->header_buf);

#ifdef GG_CONFIG_MIRANDA
	if (sess->ssl != NULL)
		si.sfree(sess->ssl);
#elif GG_CONFIG_HAVE_OPENSSL
	if (sess->ssl != NULL)
		SSL_free(sess->ssl);

	if (sess->ssl_ctx)
		SSL_CTX_free(sess->ssl_ctx);
#endif

	sess->resolver_cleanup(&sess->resolver, 1);

	if (sess->fd != -1)
		gg_sock_close(sess->fd);

	while (sess->images)
		gg_image_queue_remove(sess, sess->images, 1);

	free(sess->send_buf);

	for (dcc = sess->dcc7_list; dcc; dcc = dcc->next)
		dcc->sess = NULL;

	free(sess);
}

#ifndef DOXYGEN

/**
 * \internal Funkcja wysyłająca pakiet zmiany statusu użytkownika.
 *
 * \param sess Struktura sesji
 * \param status Nowy status użytkownika
 * \param descr Opis statusu użytkownika (lub \c NULL)
 * \param time Czas powrotu w postaci uniksowego znacznika czasu (lub 0)
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 *
 * \ingroup status
 */
static int gg_change_status_common(struct gg_session *sess, int status, const char *descr, int time)
{
	char *new_descr = NULL;
	uint32_t new_time;
	int descr_len = 0;
	int descr_len_max;
	int packet_type;
	int append_null = 0;
	int res;

	if (!sess) {
		errno = EFAULT;
		return -1;
	}

	if (sess->state != GG_STATE_CONNECTED) {
		errno = ENOTCONN;
		return -1;
	}

	/* XXX, obcinać stany których stary protokół niezna (czyt. dnd->aw; ffc->av) */

	/* dodaj flagę obsługi połączeń głosowych zgodną z GG 7.x */
	if ((sess->protocol_version >= 0x2a) && (sess->protocol_version < 0x2d /* ? */ ) && (sess->protocol_flags & GG_HAS_AUDIO_MASK) && !GG_S_I(status))
		status |= GG_STATUS_VOICE_MASK;

	sess->status = status;

	if (sess->protocol_version >= 0x2d) {
		if (descr != NULL && sess->encoding != GG_ENCODING_UTF8) {
			new_descr = gg_cp_to_utf8(descr);

			if (!new_descr)
				return -1;
		}

		if (sess->protocol_version >= 0x2e)
			packet_type = GG_NEW_STATUS80;
		else /* sess->protocol_version == 0x2d */
			packet_type = GG_NEW_STATUS80BETA;
		descr_len_max = GG_STATUS_DESCR_MAXSIZE;
		append_null = 1;

	} else {
		packet_type = GG_NEW_STATUS;
		descr_len_max = GG_STATUS_DESCR_MAXSIZE_PRE_8_0;

		if (time != 0)
			append_null = 1;
	}

	if (descr) {
		descr_len = (int)strlen((new_descr) ? new_descr : descr);

		if (descr_len > descr_len_max)
			descr_len = descr_len_max;

		// XXX pamiętać o tym, żeby nie ucinać w środku znaku utf-8
	}

	if (time)
		new_time = gg_fix32(time);

	if (packet_type == GG_NEW_STATUS80) {
		struct gg_new_status80 p;

		p.status		= gg_fix32(status);
		p.flags			= gg_fix32(sess->status_flags);
		p.description_size	= gg_fix32(descr_len);
		res = gg_send_packet(sess,
				packet_type,
				&p,
				sizeof(p),
				(new_descr) ? new_descr : descr,
				descr_len,
				NULL);

	} else {
		struct gg_new_status p;

		p.status = gg_fix32(status);
		res = gg_send_packet(sess,
				packet_type,
				&p,
				sizeof(p),
				(new_descr) ? new_descr : descr,
				descr_len,
				(append_null) ? "\0" : NULL,
				(append_null) ? 1 : 0,
				(time) ? &new_time : NULL,
				(time) ? sizeof(new_time) : 0,
				NULL);
	}

	free(new_descr);
	return res;
}

#endif /* DOXYGEN */

/**
 * Zmienia status użytkownika.
 *
 * \param sess Struktura sesji
 * \param status Nowy status użytkownika
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 *
 * \ingroup status
 */
int gg_change_status(struct gg_session *sess, int status)
{
	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_change_status(%p, %d);\n", sess, status);

	return gg_change_status_common(sess, status, NULL, 0);
}

/**
 * Zmienia status użytkownika na status opisowy.
 *
 * \param sess Struktura sesji
 * \param status Nowy status użytkownika
 * \param descr Opis statusu użytkownika
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 *
 * \ingroup status
 */
int gg_change_status_descr(struct gg_session *sess, int status, const char *descr)
{
	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_change_status_descr(%p, %d, \"%s\");\n", sess, status, descr);

	return gg_change_status_common(sess, status, descr, 0);
}

/**
 * Zmienia status użytkownika na status opisowy z podanym czasem powrotu.
 *
 * \param sess Struktura sesji
 * \param status Nowy status użytkownika
 * \param descr Opis statusu użytkownika
 * \param time Czas powrotu w postaci uniksowego znacznika czasu
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 *
 * \ingroup status
 */
int gg_change_status_descr_time(struct gg_session *sess, int status, const char *descr, int time)
{
	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_change_status_descr_time(%p, %d, \"%s\", %d);\n", sess, status, descr, time);

	return gg_change_status_common(sess, status, descr, time);
}

/**
 * Funkcja zmieniająca flagi statusu.
 *
 * \param sess Struktura sesji
 * \param flags Nowe flagi statusu
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 *
 * \note Aby zmiany weszły w życie, należy ponownie ustawić status za pomocą
 * funkcji z rodziny \c gg_change_status().
 *
 * \ingroup status
 */
int gg_change_status_flags(struct gg_session *sess, int flags)
{
	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_change_status_flags(%p, 0x%08x);\n", sess, flags);

	if (sess == NULL) {
		errno = EFAULT;
		return -1;
	}

	sess->status_flags = flags;

	return 0;
}

/**
 * Wysyła wiadomość do użytkownika.
 *
 * Zwraca losowy numer sekwencyjny, który można zignorować albo wykorzystać
 * do potwierdzenia.
 *
 * \param sess Struktura sesji
 * \param msgclass Klasa wiadomości
 * \param recipient Numer adresata
 * \param message Treść wiadomości
 *
 * \return Numer sekwencyjny wiadomości lub -1 w przypadku błędu.
 *
 * \ingroup messages
 */
int gg_send_message(struct gg_session *sess, int msgclass, uin_t recipient, const unsigned char *message)
{
	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_send_message(%p, %d, %u, %p)\n", sess, msgclass, recipient, message);

	return gg_send_message_confer_richtext(sess, msgclass, 1, &recipient, message, NULL, 0);
}

/**
 * Wysyła wiadomość formatowaną.
 *
 * Zwraca losowy numer sekwencyjny, który można zignorować albo wykorzystać
 * do potwierdzenia.
 *
 * \param sess Struktura sesji
 * \param msgclass Klasa wiadomości
 * \param recipient Numer adresata
 * \param message Treść wiadomości
 * \param format Informacje o formatowaniu
 * \param formatlen Długość informacji o formatowaniu
 *
 * \return Numer sekwencyjny wiadomości lub -1 w przypadku błędu.
 *
 * \ingroup messages
 */
int gg_send_message_richtext(struct gg_session *sess, int msgclass, uin_t recipient, const unsigned char *message, const unsigned char *format, int formatlen)
{
	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_send_message_richtext(%p, %d, %u, %p, %p, %d);\n", sess, msgclass, recipient, message, format, formatlen);

	return gg_send_message_confer_richtext(sess, msgclass, 1, &recipient, message, format, formatlen);
}

/**
 * Wysyła wiadomość w ramach konferencji.
 *
 * Zwraca losowy numer sekwencyjny, który można zignorować albo wykorzystać
 * do potwierdzenia.
 *
 * \param sess Struktura sesji
 * \param msgclass Klasa wiadomości
 * \param recipients_count Liczba adresatów
 * \param recipients Wskaźnik do tablicy z numerami adresatów
 * \param message Treść wiadomości
 *
 * \return Numer sekwencyjny wiadomości lub -1 w przypadku błędu.
 *
 * \ingroup messages
 */
int gg_send_message_confer(struct gg_session *sess, int msgclass, int recipients_count, uin_t *recipients, const unsigned char *message)
{
	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_send_message_confer(%p, %d, %d, %p, %p);\n", sess, msgclass, recipients_count, recipients, message);

	return gg_send_message_confer_richtext(sess, msgclass, recipients_count, recipients, message, NULL, 0);
}

/**
 * \internal Dodaje tekst na koniec bufora.
 * 
 * \param dst Wskaźnik na bufor roboczy
 * \param pos Wskaźnik na aktualne położenie w buforze roboczym
 * \param src Dodawany tekst
 * \param len Długość dodawanego tekstu
 */
static void gg_append(char *dst, int *pos, const void *src, int len)
{
	if (dst != NULL)
		memcpy(&dst[*pos], src, len);

	*pos += len;
}

/**
 * \internal Zamienia tekst z formatowaniem Gadu-Gadu na HTML.
 *
 * \param dst Bufor wynikowy (może być \c NULL)
 * \param src Tekst źródłowy w UTF-8
 * \param format Atrybuty tekstu źródłowego
 * \param format_len Długość bloku atrybutów tekstu źródłowego
 *
 * \note Wynikowy tekst nie jest idealnym kodem HTML, ponieważ ma jak
 * dokładniej odzwierciedlać to, co wygenerowałby oryginalny klient.
 *
 * \note Dokleja \c \\0 na końcu bufora wynikowego.
 *
 * \return Długość tekstu wynikowego bez \c \\0 (nawet jeśli \c dst to \c NULL).
 */
static int gg_convert_to_html(char *dst, const char *src, const unsigned char *format, int format_len)
{
	const char span_fmt[] = "<span style=\"color:#%02x%02x%02x; font-family:'MS Shell Dlg 2'; font-size:9pt; \">";
	const int span_len = 75;
	const char img_fmt[] = "<img name=\"%02x%02x%02x%02x%02x%02x%02x%02x\">";
	const int img_len = 29;
	int char_pos = 0;
	int format_idx = 0;
	unsigned char old_attr = 0;
	const unsigned char *color = (const unsigned char*) "\x00\x00\x00";
	int len, i;
	const unsigned char *format_ = (const unsigned char*) format;

	len = 0;

	/* Nie mamy atrybutów dla pierwsze znaku, a tekst nie jest pusty, więc
	 * tak czy inaczej trzeba otworzyć <span>. */

	if (src[0] != 0 && (format_idx + 3 > format_len || (format_[format_idx] | (format_[format_idx + 1] << 8)) != 0)) {
		if (dst != NULL)
			sprintf(&dst[len], span_fmt, 0, 0, 0);

		len += span_len;
	}

	/* Pętla przechodzi też przez kończące \0, żeby móc dokleić obrazek
	 * na końcu tekstu. */

	for (i = 0; ; i++) {
		/* Analizuj atrybuty tak długo jak dotyczą aktualnego znaku. */
		for (;;) {
			unsigned char attr;
			int attr_pos;

			if (format_idx + 3 > format_len)
				break;

			attr_pos = format_[format_idx] | (format_[format_idx + 1] << 8);

			if (attr_pos != char_pos)
				break;

			attr = format_[format_idx + 2];

			/* Nie doklejaj atrybutów na końcu, co najwyżej obrazki. */

			if (src[i] == 0)
				attr &= ~(GG_FONT_BOLD | GG_FONT_ITALIC | GG_FONT_UNDERLINE | GG_FONT_COLOR);

			format_idx += 3;

			if ((attr & (GG_FONT_BOLD | GG_FONT_ITALIC | GG_FONT_UNDERLINE | GG_FONT_COLOR)) != 0 || (attr == 0 && old_attr != 0)) {
				if (char_pos != 0) {
					if ((old_attr & GG_FONT_UNDERLINE) != 0)
						gg_append(dst, &len, "</u>", 4);

					if ((old_attr & GG_FONT_ITALIC) != 0)
						gg_append(dst, &len, "</i>", 4);

					if ((old_attr & GG_FONT_BOLD) != 0)
						gg_append(dst, &len, "</b>", 4);

					if (src[i] != 0)
						gg_append(dst, &len, "</span>", 7);
				}

				if (((attr & GG_FONT_COLOR) != 0) && (format_idx + 3 <= format_len)) {
					color = &format_[format_idx];
					format_idx += 3;
				} else {
					color = (unsigned char*) "\x00\x00\x00";
				}

				if (src[i] != 0) {
					if (dst != NULL)
						sprintf(&dst[len], span_fmt, color[0], color[1], color[2]);
					len += span_len;
				}
			} else if (char_pos == 0 && src[0] != 0) {
				if (dst != NULL)
					sprintf(&dst[len], span_fmt, 0, 0, 0);
				len += span_len;
			}

			if ((attr & GG_FONT_BOLD) != 0)
				gg_append(dst, &len, "<b>", 3);

			if ((attr & GG_FONT_ITALIC) != 0)
				gg_append(dst, &len, "<i>", 3);

			if ((attr & GG_FONT_UNDERLINE) != 0)
				gg_append(dst, &len, "<u>", 3);

			if (((attr & GG_FONT_IMAGE) != 0) && (format_idx + 10 <= format_len)) {
				if (dst != NULL) {
					sprintf(&dst[len], img_fmt,
						format_[format_idx + 9],
						format_[format_idx + 8], 
						format_[format_idx + 7],
						format_[format_idx + 6], 
						format_[format_idx + 5],
						format_[format_idx + 4],
						format_[format_idx + 3],
						format_[format_idx + 2]);
				}

				len += img_len;
				format_idx += 10;
			}

			old_attr = attr;
		}

		/* Doklej znak zachowując htmlowe escapowanie. */

		switch (src[i]) {
			case '&':
				gg_append(dst, &len, "&amp;", 5);
				break;
			case '<':
				gg_append(dst, &len, "&lt;", 4);
				break;
			case '>':
				gg_append(dst, &len, "&gt;", 4);
				break;
			case '\'':
				gg_append(dst, &len, "&apos;", 6);
				break;
			case '\"':
				gg_append(dst, &len, "&quot;", 6);
				break;
			case '\n':
				gg_append(dst, &len, "<br>", 4);
				break;
			case '\r':
			case 0:
				break;
			default:
				if (dst != NULL)
					dst[len] = src[i];
				len++;
		}

		/* Sprawdź, czy bajt nie jest kontynuacją znaku unikodowego. */

		if ((src[i] & 0xc0) != 0xc0)
			char_pos++;

		if (src[i] == 0)
			break;
	}

	/* Zamknij tagi. */

	if ((old_attr & GG_FONT_UNDERLINE) != 0)
		gg_append(dst, &len, "</u>", 4);

	if ((old_attr & GG_FONT_ITALIC) != 0)
		gg_append(dst, &len, "</i>", 4);

	if ((old_attr & GG_FONT_BOLD) != 0)
		gg_append(dst, &len, "</b>", 4);

	if (src[0] != 0)
		gg_append(dst, &len, "</span>", 7);

	if (dst != NULL)
		dst[len] = 0;

	return len;
}

/**
 * Wysyła wiadomość formatowaną w ramach konferencji.
 *
 * Zwraca losowy numer sekwencyjny, który można zignorować albo wykorzystać
 * do potwierdzenia.
 *
 * \param sess Struktura sesji
 * \param msgclass Klasa wiadomości
 * \param recipients_count Liczba adresatów
 * \param recipients Wskaźnik do tablicy z numerami adresatów
 * \param message Treść wiadomości
 * \param format Informacje o formatowaniu
 * \param formatlen Długość informacji o formatowaniu
 *
 * \return Numer sekwencyjny wiadomości lub -1 w przypadku błędu.
 * 
 * \ingroup messages
 */
int gg_send_message_confer_richtext(struct gg_session *sess, int msgclass, int recipients_count, uin_t *recipients, const unsigned char *message, const unsigned char *format, int formatlen)
{
	struct gg_send_msg s;
	struct gg_send_msg80 s80;
	struct gg_msg_recipients r;
	char *cp_msg = NULL;
	char *utf_msg = NULL;
	char *html_msg = NULL;
	int seq_no;
	int i, j, k;
	uin_t *recps;

	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_send_message_confer_richtext(%p, %d, %d, %p, %p, %p, %d);\n", sess, msgclass, recipients_count, recipients, message, format, formatlen);

	if (!sess) {
		errno = EFAULT;
		return -1;
	}

	if (sess->state != GG_STATE_CONNECTED) {
		errno = ENOTCONN;
		return -1;
	}

	if (message == NULL || recipients_count <= 0 || recipients_count > 0xffff || (recipients_count != 1 && recipients == NULL)) {
		errno = EINVAL;
		return -1;
	}

	if (sess->encoding == GG_ENCODING_UTF8) {
		if (!(cp_msg = gg_utf8_to_cp((const char *) message)))
			return -1;

		utf_msg = (char*) message;
	} else {
		if (sess->protocol_version >= 0x2d) {
			if (!(utf_msg = gg_cp_to_utf8((const char *) message)))
				return -1;
		}

		cp_msg = (char*) message;
	}

	if (sess->protocol_version < 0x2d) {
		if (!sess->seq)
			sess->seq = 0x01740000 | (rand() & 0xffff);
		seq_no = sess->seq;
		sess->seq += (rand() % 0x300) + 0x300;

		s.msgclass = gg_fix32(msgclass);
		s.seq = gg_fix32(seq_no);
	} else {
		int len;
		
		// Drobne odchylenie od protokołu. Jeśli wysyłamy kilka
		// wiadomości w ciągu jednej sekundy, zwiększamy poprzednią
		// wartość, żeby każda wiadomość miała unikalny numer.

		seq_no = (int)time(NULL);

		if (seq_no <= sess->seq)
			seq_no = sess->seq + 1;

		sess->seq = seq_no;

		if (format == NULL || formatlen < 3) {
			format = (unsigned char*) "\x02\x06\x00\x00\x00\x08\x00\x00\x00";
			formatlen = 9;
		}

		len = gg_convert_to_html(NULL, utf_msg, format + 3, formatlen - 3);

		html_msg = (char*)malloc(len + 1);

		if (html_msg == NULL) {
			seq_no = -1;
			goto cleanup;
		}

		gg_convert_to_html(html_msg, utf_msg, format + 3, formatlen - 3);

		s80.seq = gg_fix32(seq_no);
		s80.msgclass = gg_fix32(msgclass);
		s80.offset_plain = gg_fix32(sizeof(s80) + (uint32_t)strlen(html_msg) + 1);
		s80.offset_attr = gg_fix32(sizeof(s80) + (uint32_t)strlen(html_msg) + 1 + (uint32_t)strlen(cp_msg) + 1);
	}

	if (recipients_count > 1) {
		r.flag = 0x01;
		r.count = gg_fix32(recipients_count - 1);

		recps = (uin_t*)malloc(sizeof(uin_t) * recipients_count);

		if (!recps) {
			seq_no = -1;
			goto cleanup;
		}

		for (i = 0; i < recipients_count; i++) {
			for (j = 0, k = 0; j < recipients_count; j++) {
				if (recipients[j] != recipients[i]) {
					recps[k] = gg_fix32(recipients[j]);
					k++;
				}
			}

			if (sess->protocol_version < 0x2d) {
				s.recipient = gg_fix32(recipients[i]);

				if (gg_send_packet(sess, GG_SEND_MSG, &s, sizeof(s), cp_msg, strlen(cp_msg) + 1, &r, sizeof(r), recps, (recipients_count - 1) * sizeof(uin_t), format, formatlen, NULL) == -1)
					seq_no = -1;
			} else {
				s80.recipient = gg_fix32(recipients[i]);

				if (gg_send_packet(sess, GG_SEND_MSG80, &s80, sizeof(s80), html_msg, strlen(html_msg) + 1, cp_msg, strlen(cp_msg) + 1, &r, sizeof(r), recps, (recipients_count - 1) * sizeof(uin_t), format, formatlen, NULL) == -1)
					seq_no = -1;
			}
		}

		free(recps);
	} else {
		if (sess->protocol_version < 0x2d) {
			s.recipient = gg_fix32(recipients[0]);

			if (gg_send_packet(sess, GG_SEND_MSG, &s, sizeof(s), cp_msg, strlen(cp_msg) + 1, format, formatlen, NULL) == -1)
				seq_no = -1;
		} else {
			s80.recipient = gg_fix32(recipients[0]);

			if (gg_send_packet(sess, GG_SEND_MSG80, &s80, sizeof(s80), html_msg, strlen(html_msg) + 1, cp_msg, strlen(cp_msg) + 1, format, formatlen, NULL) == -1)
				seq_no = -1;
		}
	}

cleanup:
	if (cp_msg != (char*) message)
		free(cp_msg);

	if (utf_msg != (char*) message)
		free(utf_msg);

	free(html_msg);

	return seq_no;
}

/**
 * Wysyła wiadomość binarną przeznaczoną dla klienta.
 *
 * Wiadomości między klientami przesyła się np. w celu wywołania zwrotnego
 * połączenia bezpośredniego. Funkcja zwraca losowy numer sekwencyjny,
 * który można zignorować albo wykorzystać do potwierdzenia.
 *
 * \param sess Struktura sesji
 * \param msgclass Klasa wiadomości
 * \param recipient Numer adresata
 * \param message Treść wiadomości
 * \param message_len Długość wiadomości
 *
 * \return Numer sekwencyjny wiadomości lub -1 w przypadku błędu.
 *
 * \ingroup messages
 */
int gg_send_message_ctcp(struct gg_session *sess, int msgclass, uin_t recipient, const unsigned char *message, int message_len)
{
	struct gg_send_msg s;

	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_send_message_ctcp(%p, %d, %u, ...);\n", sess, msgclass, recipient);

	if (!sess) {
		errno = EFAULT;
		return -1;
	}

	if (sess->state != GG_STATE_CONNECTED) {
		errno = ENOTCONN;
		return -1;
	}

	s.recipient = gg_fix32(recipient);
	s.seq = gg_fix32(0);
	s.msgclass = gg_fix32(msgclass);

	return gg_send_packet(sess, GG_SEND_MSG, &s, sizeof(s), message, message_len, NULL);
}

/**
 * Wysyła żądanie obrazka o podanych parametrach.
 *
 * Wiadomości obrazkowe nie zawierają samych obrazków, a tylko ich rozmiary
 * i sumy kontrolne. Odbiorca najpierw szuka obrazków w swojej pamięci
 * podręcznej i dopiero gdy ich nie znajdzie, wysyła żądanie do nadawcy.
 * Wynik zostanie przekazany zdarzeniem \c GG_EVENT_IMAGE_REPLY.
 *
 * \param sess Struktura sesji
 * \param recipient Numer adresata
 * \param size Rozmiar obrazka w bajtach
 * \param crc32 Suma kontrola obrazka
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 *
 * \ingroup messages
 */
int gg_image_request(struct gg_session *sess, uin_t recipient, int size, uint32_t crc32)
{
	struct gg_send_msg s;
	struct gg_msg_image_request r;
	char dummy = 0;
	int res;

	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_image_request(%p, %d, %u, 0x%.4x);\n", sess, recipient, size, crc32);

	if (!sess) {
		errno = EFAULT;
		return -1;
	}

	if (sess->state != GG_STATE_CONNECTED) {
		errno = ENOTCONN;
		return -1;
	}

	if (size < 0) {
		errno = EINVAL;
		return -1;
	}

	s.recipient = gg_fix32(recipient);
	s.seq = gg_fix32(0);
	s.msgclass = gg_fix32(GG_CLASS_MSG);

	r.flag = 0x04;
	r.size = gg_fix32(size);
	r.crc32 = gg_fix32(crc32);

	res = gg_send_packet(sess, GG_SEND_MSG, &s, sizeof(s), &dummy, 1, &r, sizeof(r), NULL);

	if (!res) {
		struct gg_image_queue *q = (gg_image_queue*)malloc(sizeof(*q));
		char *buf;

		if (!q) {
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_image_request() not enough memory for image queue\n");
			return -1;
		}

		buf = (char*)malloc(size);
		if (size && !buf)
		{
			gg_debug_session(sess, GG_DEBUG_MISC, "// gg_image_request() not enough memory for image\n");
			free(q);
			return -1;
		}

		memset(q, 0, sizeof(*q));

		q->sender = recipient;
		q->size = size;
		q->crc32 = crc32;
		q->image = buf;

		if (!sess->images)
			sess->images = q;
		else {
			struct gg_image_queue *qq;

			for (qq = sess->images; qq->next; qq = qq->next)
				;

			qq->next = q;
		}
	}

	return res;
}

/**
 * Wysyła żądany obrazek.
 *
 * \param sess Struktura sesji
 * \param recipient Numer adresata
 * \param filename Nazwa pliku
 * \param image Bufor z obrazkiem
 * \param size Rozmiar obrazka
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 *
 * \ingroup messages
 */
int gg_image_reply(struct gg_session *sess, uin_t recipient, const char *filename, const char *image, int size)
{
	struct gg_msg_image_reply *r;
	struct gg_send_msg s;
	const char *tmp;
	char buf[1910];
	int res = -1;

	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_image_reply(%p, %d, \"%s\", %p, %d);\n", sess, recipient, filename, image, size);

	if (!sess || !filename || !image) {
		errno = EFAULT;
		return -1;
	}

	if (sess->state != GG_STATE_CONNECTED) {
		errno = ENOTCONN;
		return -1;
	}

	if (size < 0) {
		errno = EINVAL;
		return -1;
	}

	/* wytnij ścieżki, zostaw tylko nazwę pliku */
	while ((tmp = strrchr(filename, '/')) || (tmp = strrchr(filename, '\\')))
		filename = tmp + 1;

	if (strlen(filename) < 1 || strlen(filename) > 1024) {
		errno = EINVAL;
		return -1;
	}

	s.recipient = gg_fix32(recipient);
	s.seq = gg_fix32(0);
	s.msgclass = gg_fix32(GG_CLASS_MSG);

	buf[0] = 0;
	r = (gg_msg_image_reply*)&buf[1];

	r->flag = 0x05;
	r->size = gg_fix32(size);
	r->crc32 = gg_fix32(gg_crc32(0, (unsigned char*) image, size));

	while (size > 0) {
		int buflen, chunklen;

		/* \0 + struct gg_msg_image_reply */
		buflen = sizeof(struct gg_msg_image_reply) + 1;

		/* w pierwszym kawałku jest nazwa pliku */
		if (r->flag == 0x05) {
			strcpy(buf + buflen, filename);
			buflen += (int)strlen(filename) + 1;
		}

		chunklen = (size >= (int)sizeof(buf) - buflen) ? ((int)sizeof(buf) - buflen) : size;

		memcpy(buf + buflen, image, chunklen);
		size -= chunklen;
		image += chunklen;

		res = gg_send_packet(sess, GG_SEND_MSG, &s, sizeof(s), buf, buflen + chunklen, NULL);

		if (res == -1)
			break;

		r->flag = 0x06;
	}

	return res;
}

/**
 * Wysyła do serwera listę kontaktów.
 *
 * Funkcja informuje serwer o liście kontaktów, których statusy będą
 * obserwowane lub kontaktów, które bedą blokowane. Dla każdego z \c count
 * kontaktów tablica \c userlist zawiera numer, a tablica \c types rodzaj
 * kontaktu (\c GG_USER_NORMAL, \c GG_USER_OFFLINE, \c GG_USER_BLOCKED).
 *
 * Listę kontaktów należy \b zawsze wysyłać po połączeniu, nawet jeśli
 * jest pusta.
 *
 * \param sess Struktura sesji
 * \param userlist Wskaźnik do tablicy numerów kontaktów
 * \param types Wskaźnik do tablicy rodzajów kontaktów
 * \param count Liczba kontaktów
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 *
 * \ingroup contacts
 */
int gg_notify_ex(struct gg_session *sess, uin_t *userlist, char *types, int count)
{
	struct gg_notify *n;
	uin_t *u;
	char *t;
	int i, res = 0;

	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_notify_ex(%p, %p, %p, %d);\n", sess, userlist, types, count);

	if (!sess) {
		errno = EFAULT;
		return -1;
	}

	if (sess->state != GG_STATE_CONNECTED) {
		errno = ENOTCONN;
		return -1;
	}

	if (!userlist || !count)
		return gg_send_packet(sess, GG_LIST_EMPTY, NULL);

	while (count > 0) {
		int part_count, packet_type;

		if (count > 400) {
			part_count = 400;
			packet_type = GG_NOTIFY_FIRST;
		} else {
			part_count = count;
			packet_type = GG_NOTIFY_LAST;
		}

		if (!(n = (struct gg_notify*) malloc(sizeof(*n) * part_count)))
			return -1;

		for (u = userlist, t = types, i = 0; i < part_count; u++, t++, i++) {
			n[i].uin = gg_fix32(*u);
			n[i].dunno1 = *t;
		}

		if (gg_send_packet(sess, packet_type, n, sizeof(*n) * part_count, NULL) == -1) {
			free(n);
			res = -1;
			break;
		}

		count -= part_count;
		userlist += part_count;
		types += part_count;

		free(n);
	}

	return res;
}

/**
 * Wysyła do serwera listę kontaktów.
 *
 * Funkcja jest odpowiednikiem \c gg_notify_ex(), gdzie wszystkie kontakty
 * są rodzaju \c GG_USER_NORMAL.
 *
 * \param sess Struktura sesji
 * \param userlist Wskaźnik do tablicy numerów kontaktów
 * \param count Liczba kontaktów
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 *
 * \ingroup contacts
 */
int gg_notify(struct gg_session *sess, uin_t *userlist, int count)
{
	struct gg_notify *n;
	uin_t *u;
	int i, res = 0;

	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_notify(%p, %p, %d);\n", sess, userlist, count);

	if (!sess) {
		errno = EFAULT;
		return -1;
	}

	if (sess->state != GG_STATE_CONNECTED) {
		errno = ENOTCONN;
		return -1;
	}

	if (!userlist || !count)
		return gg_send_packet(sess, GG_LIST_EMPTY, NULL);

	while (count > 0) {
		int part_count, packet_type;

		if (count > 400) {
			part_count = 400;
			packet_type = GG_NOTIFY_FIRST;
		} else {
			part_count = count;
			packet_type = GG_NOTIFY_LAST;
		}

		if (!(n = (struct gg_notify*) malloc(sizeof(*n) * part_count)))
			return -1;

		for (u = userlist, i = 0; i < part_count; u++, i++) {
			n[i].uin = gg_fix32(*u);
			n[i].dunno1 = GG_USER_NORMAL;
		}

		if (gg_send_packet(sess, packet_type, n, sizeof(*n) * part_count, NULL) == -1) {
			res = -1;
			free(n);
			break;
		}

		free(n);

		userlist += part_count;
		count -= part_count;
	}

	return res;
}

/**
 * Dodaje kontakt.
 *
 * Dodaje do listy kontaktów dany numer w trakcie połączenia. Aby zmienić
 * rodzaj kontaktu (np. z normalnego na zablokowany), należy najpierw usunąć
 * poprzedni rodzaj, ponieważ serwer operuje na maskach bitowych.
 *
 * \param sess Struktura sesji
 * \param uin Numer kontaktu
 * \param type Rodzaj kontaktu
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 *
 * \ingroup contacts
 */
int gg_add_notify_ex(struct gg_session *sess, uin_t uin, char type)
{
	struct gg_add_remove a;

	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_add_notify_ex(%p, %u, %d);\n", sess, uin, type);

	if (!sess) {
		errno = EFAULT;
		return -1;
	}

	if (sess->state != GG_STATE_CONNECTED) {
		errno = ENOTCONN;
		return -1;
	}

	a.uin = gg_fix32(uin);
	a.dunno1 = type;

	return gg_send_packet(sess, GG_ADD_NOTIFY, &a, sizeof(a), NULL);
}

/**
 * Dodaje kontakt.
 *
 * Funkcja jest odpowiednikiem \c gg_add_notify_ex(), gdzie rodzaj wszystkich
 * kontaktów to \c GG_USER_NORMAL.
 *
 * \param sess Struktura sesji
 * \param uin Numer kontaktu
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 *
 * \ingroup contacts
 */
int gg_add_notify(struct gg_session *sess, uin_t uin)
{
	return gg_add_notify_ex(sess, uin, GG_USER_NORMAL);
}

/**
 * Usuwa kontakt.
 *
 * Usuwa z listy kontaktów dany numer w trakcie połączenia.
 *
 * \param sess Struktura sesji
 * \param uin Numer kontaktu
 * \param type Rodzaj kontaktu
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 *
 * \ingroup contacts
 */
int gg_remove_notify_ex(struct gg_session *sess, uin_t uin, char type)
{
	struct gg_add_remove a;

	gg_debug_session(sess, GG_DEBUG_FUNCTION, "** gg_remove_notify_ex(%p, %u, %d);\n", sess, uin, type);

	if (!sess) {
		errno = EFAULT;
		return -1;
	}

	if (sess->state != GG_STATE_CONNECTED) {
		errno = ENOTCONN;
		return -1;
	}

	a.uin = gg_fix32(uin);
	a.dunno1 = type;

	return gg_send_packet(sess, GG_REMOVE_NOTIFY, &a, sizeof(a), NULL);
}

/**
 * Usuwa kontakt.
 *
 * Funkcja jest odpowiednikiem \c gg_add_notify_ex(), gdzie rodzaj wszystkich
 * kontaktów to \c GG_USER_NORMAL.
 *
 * \param sess Struktura sesji
 * \param uin Numer kontaktu
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 *
 * \ingroup contacts
 */
int gg_remove_notify(struct gg_session *sess, uin_t uin)
{
	return gg_remove_notify_ex(sess, uin, GG_USER_NORMAL);
}

/**
 * Wysyła do serwera zapytanie dotyczące listy kontaktów.
 *
 * Funkcja służy do importu lub eksportu listy kontaktów do serwera.
 * W odróżnieniu od funkcji \c gg_notify(), ta lista kontaktów jest przez
 * serwer jedynie przechowywana i nie ma wpływu na połączenie. Format
 * listy kontaktów jest ignorowany przez serwer, ale ze względu na
 * kompatybilność z innymi klientami, należy przechowywać dane w tym samym
 * formacie co oryginalny klient Gadu-Gadu.
 *
 * Program nie musi się przejmować fragmentacją listy kontaktów wynikającą
 * z protokołu -- wysyła i odbiera kompletną listę.
 *
 * \param sess Struktura sesji
 * \param type Rodzaj zapytania
 * \param request Treść zapytania (może być równe NULL)
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 *
 * \ingroup importexport
 */
int gg_userlist_request(struct gg_session *sess, char type, const char *request)
{
	int len;

	if (!sess) {
		errno = EFAULT;
		return -1;
	}

	if (sess->state != GG_STATE_CONNECTED) {
		errno = ENOTCONN;
		return -1;
	}

	if (!request) {
		sess->userlist_blocks = 1;
		return gg_send_packet(sess, GG_USERLIST_REQUEST, &type, sizeof(type), NULL);
	}

	len = (int)strlen(request);

	sess->userlist_blocks = 0;

	while (len > 2047) {
		sess->userlist_blocks++;

		if (gg_send_packet(sess, GG_USERLIST_REQUEST, &type, sizeof(type), request, 2047, NULL) == -1)
			return -1;

		if (type == GG_USERLIST_PUT)
			type = GG_USERLIST_PUT_MORE;

		request += 2047;
		len -= 2047;
	}

	sess->userlist_blocks++;

	return gg_send_packet(sess, GG_USERLIST_REQUEST, &type, sizeof(type), request, len, NULL);
}

/**
 * Informuje rozmówcę o pisaniu wiadomości.
 *
 * \param sess Struktura sesji
 * \param recipient Numer adresata
 * \param length Długość wiadomości lub 0 jeśli jest pusta
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 *
 * \ingroup messages
 */
int gg_typing_notification(struct gg_session *sess, uin_t recipient, int length){
	struct gg_typing_notification pkt;
	uin_t uin;

	pkt.length = gg_fix16((uint16_t)length);
	uin = gg_fix32(recipient);
	memcpy(&pkt.uin, &uin, sizeof(uin_t));

	return gg_send_packet(sess, GG_TYPING_NOTIFICATION, &pkt, sizeof(pkt), NULL);
}

/**
 * Rozłącza inną sesję multilogowania.
 *
 * \param gs Struktura sesji
 * \param conn_id Sesja do rozłączenia
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 *
 * \ingroup login
 */
int gg_multilogon_disconnect(struct gg_session *gs, gg_multilogon_id_t conn_id)
{
	struct gg_multilogon_disconnect pkt;

	pkt.conn_id = conn_id;

	return gg_send_packet(gs, GG_MULTILOGON_DISCONNECT, &pkt, sizeof(pkt), NULL);
}

/* @} */

/*
 * Local variables:
 * c-indentation-style: k&r
 * c-basic-offset: 8
 * indent-tabs-mode: notnil
 * End:
 *
 * vim: shiftwidth=8:
 */
