/* coding: UTF-8 */
/* $Id: common.c 13762 2011-08-09 12:35:16Z dezred $ */

/*
 *  (C) Copyright 2001-2002 Wojtek Kaniewski <wojtekka@irc.pl>
 *                          Robert J. Woźny <speedy@ziew.org>
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

/*
 * Funkcje konwersji między UTF-8 i CP1250 są oparte o kod biblioteki iconv.
 * Informacje o prawach autorskich oryginalnego kodu zamieszczono poniżej:
 *
 * Copyright (C) 1999-2001, 2004 Free Software Foundation, Inc.
 * This file is part of the GNU LIBICONV Library.
 *
 * The GNU LIBICONV Library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * The GNU LIBICONV Library is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the GNU LIBICONV Library; see the file COPYING.LIB.
 * If not, write to the Free Software Foundation, Inc., 51 Franklin Street,
 * Fifth Floor, Boston, MA 02110-1301, USA.
 */

/**
 * \file common.c
 *
 * \brief Funkcje wykorzystywane przez różne moduły biblioteki
 */
#include <sys/types.h>
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

#include <errno.h>
#include <fcntl.h>
#ifndef _WIN32
#include <netdb.h>
#endif /* _WIN32 */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#endif /* _WIN32 */

#include "libgadu.h"
#include "internal.h"

/**
 * Plik, do którego będą przekazywane informacje odpluskwiania.
 *
 * Funkcja \c gg_debug() i pochodne mogą być przechwytywane przez aplikację
 * korzystającą z biblioteki, by wyświetlić je na żądanie użytkownika lub
 * zapisać do późniejszej analizy. Jeśli nie określono pliku, wybrane
 * informacje będą wysyłane do standardowego wyjścia błędu (\c stderr).
 *
 * \ingroup debug
 */
FILE *gg_debug_file = NULL;

#ifndef GG_DEBUG_DISABLE

/**
 * \internal Przekazuje informacje odpluskwiania do odpowiedniej funkcji.
 *
 * Jeśli aplikacja ustawiła odpowiednią funkcję obsługi w
 * \c gg_debug_handler_session lub \c gg_debug_handler, jest ona wywoływana.
 * W przeciwnym wypadku wynik jest wysyłany do standardowego wyjścia błędu.
 *
 * \param sess Struktura sesji (może być \c NULL)
 * \param level Poziom informacji
 * \param format Format wiadomości (zgodny z \c printf)
 * \param ap Lista argumentów (zgodna z \c printf)
 */
static void gg_debug_common(struct gg_session *sess, int level, const char *format, va_list ap)
{
	if (gg_debug_handler_session)
		(*gg_debug_handler_session)(sess, level, format, ap);
	else if (gg_debug_handler)
		(*gg_debug_handler)(level, format, ap);
	else if (gg_debug_level & level)
		vfprintf(gg_debug_file ? gg_debug_file : stderr, format, ap);
}


/**
 * \internal Przekazuje informację odpluskawiania.
 *
 * \param level Poziom wiadomości
 * \param format Format wiadomości (zgodny z \c printf)
 *
 * \ingroup debug
 */
void gg_debug(int level, const char *format, ...)
{
	va_list ap;
	int old_errno = errno;
	va_start(ap, format);
	gg_debug_common(NULL, level, format, ap);
	va_end(ap);
	errno = old_errno;
}

/**
 * \internal Przekazuje informację odpluskwiania związaną z sesją.
 *
 * \param sess Struktura sesji
 * \param level Poziom wiadomości
 * \param format Format wiadomości (zgodny z \c printf)
 *
 * \ingroup debug
 */
void gg_debug_session(struct gg_session *sess, int level, const char *format, ...)
{
	va_list ap;
	int old_errno = errno;
	va_start(ap, format);
	gg_debug_common(sess, level, format, ap);
	va_end(ap);
	errno = old_errno;
}

/**
 * \internal Przekazuje informację odpluskwiania związane z zawartością pamięci.
 *
 * \param sess Struktura sesji
 * \param buf Adres w pamięci
 * \param buf_length Ilość danych do wyświetlenia
 * \param format Format wiadomości (zgodny z \c printf)
 *
 * \ingroup debug
 */
void gg_debug_dump_session(struct gg_session *sess, const void *buf, unsigned int buf_length, const char *format, ...)
{
	va_list ap;

	if ((gg_debug_level & GG_DEBUG_DUMP)) {
		  unsigned int i;

		  va_start(ap, format);
		  gg_debug_common(sess, GG_DEBUG_DUMP, format, ap);
		  for (i = 0; i < buf_length; ++i)
			  gg_debug_session(sess, GG_DEBUG_DUMP, " %.2x", ((unsigned char*) buf)[i]);
		  gg_debug_session(sess, GG_DEBUG_DUMP, "\n");
		  va_end(ap);
	  }
}

#endif

/**
 * \internal Odpowiednik funkcji \c vsprintf alokujący miejsce na wynik.
 *
 * Funkcja korzysta z funkcji \c vsnprintf, sprawdzając czy dostępna funkcja
 * systemowa jest zgodna ze standardem C99 czy wcześniejszymi.
 *
 * \param format Format wiadomości (zgodny z \c printf)
 * \param ap Lista argumentów (zgodna z \c printf)
 *
 * \return Zaalokowany bufor lub NULL, jeśli zabrakło pamięci.
 *
 * \ingroup helper
 */
char *gg_vsaprintf(const char *format, va_list ap)
{
	int size = 0;
	char *buf = NULL;

#ifdef GG_CONFIG_HAVE_VA_COPY
	va_list aq;

	va_copy(aq, ap);
#else
#  ifdef GG_CONFIG_HAVE___VA_COPY
	va_list aq;

	__va_copy(aq, ap);
#  endif
#endif

#ifndef GG_CONFIG_HAVE_C99_VSNPRINTF
	{
		int res;
		char *tmp;

		size = 128;
		do {
			size *= 2;
			if (!(tmp = (char*)realloc(buf, size))) {
				free(buf);
				return NULL;
			}
			buf = tmp;
			res = vsnprintf(buf, size, format, ap);
		} while (res == size - 1 || res == -1);
	}
#else
	{
		char tmp[2];

		/* libce Solarisa przy buforze NULL zawsze zwracają -1, więc
		 * musimy podać coś istniejącego jako cel printf()owania. */
		size = vsnprintf(tmp, sizeof(tmp), format, ap);
		if (!(buf = malloc(size + 1)))
			return NULL;
	}
#endif

#ifdef GG_CONFIG_HAVE_VA_COPY
	vsnprintf(buf, size + 1, format, aq);
	va_end(aq);
#else
#  ifdef GG_CONFIG_HAVE___VA_COPY
	vsnprintf(buf, size + 1, format, aq);
	va_end(aq);
#  else
	vsnprintf(buf, size + 1, format, ap);
#  endif
#endif

	return buf;
}

/**
 * \internal Odpowiednik funkcji \c sprintf alokujący miejsce na wynik.
 *
 * Funkcja korzysta z funkcji \c vsnprintf, sprawdzając czy dostępna funkcja
 * systemowa jest zgodna ze standardem C99 czy wcześniejszymi.
 *
 * \param format Format wiadomości (zgodny z \c printf)
 *
 * \return Zaalokowany bufor lub NULL, jeśli zabrakło pamięci.
 *
 * \ingroup helper
 */
char *gg_saprintf(const char *format, ...)
{
	va_list ap;
	char *res;

	va_start(ap, format);
	res = gg_vsaprintf(format, ap);
	va_end(ap);

	return res;
}

/**
 * \internal Pobiera linię tekstu z bufora.
 *
 * Funkcja niszczy bufor źródłowy bezpowrotnie, dzieląc go na kolejne ciągi
 * znaków i obcina znaki końca linii.
 *
 * \param ptr Wskaźnik do zmiennej, która przechowuje aktualne położenie
 *            w analizowanym buforze
 *
 * \return Wskaźnik do kolejnej linii tekstu lub NULL, jeśli to już koniec
 *         bufora.
 */
char *gg_get_line(char **ptr)
{
	char *foo, *res;

	if (!ptr || !*ptr || !strcmp(*ptr, ""))
		return NULL;

	res = *ptr;

	if (!(foo = strchr(*ptr, '\n')))
		*ptr += strlen(*ptr);
	else {
		size_t len;
		*ptr = foo + 1;
		*foo = 0;

		len = strlen(res);

		if (len > 1 && res[len - 1] == '\r')
			res[len - 1] = 0;
	}

	return res;
}

/**
 * \internal Czyta linię tekstu z gniazda.
 *
 * Funkcja czyta tekst znak po znaku, więc nie jest efektywna, ale dzięki
 * brakowi buforowania, nie koliduje z innymi funkcjami odczytu.
 *
 * \param sock Deskryptor gniazda
 * \param buf Wskaźnik do bufora
 * \param length Długość bufora
 *
 * \return Zwraca \c buf jeśli się powiodło, lub \c NULL w przypadku błędu.
 */
char *gg_read_line(SOCKET sock, char *buf, int length)
{
	int ret;

	if (!buf || length < 0)
		return NULL;

	for (; length > 1; buf++, length--) {
		do {
			if ((ret = gg_sock_read(sock, buf, 1)) == -1 && errno != EINTR && errno != EAGAIN) {
				gg_debug(GG_DEBUG_MISC, "// gg_read_line() error on read (errno=%d, %s)\n", errno, strerror(errno));
				*buf = 0;
				return NULL;
			} else if (ret == 0) {
				gg_debug(GG_DEBUG_MISC, "// gg_read_line() eof reached\n");
				*buf = 0;
				return NULL;
			}
		} while (ret == -1 && (errno == EINTR || errno == EAGAIN));

		if (*buf == '\n') {
			buf++;
			break;
		}
	}

	*buf = 0;
	return buf;
}

/**
 * \internal Nawiązuje połączenie TCP.
 *
 * \param addr Wskaźnik na strukturę \c in_addr z adresem serwera
 * \param port Port serwera
 * \param async Flaga asynchronicznego połączenia
 *
 * \return Deskryptor gniazda lub -1 w przypadku błędu
 *
 * \ingroup helper
 */
#ifdef GG_CONFIG_MIRANDA
SOCKET gg_connect_internal(void *addr, int port, int async, SOCKET *gg_sock)
#else
SOCKET gg_connect(void *addr, int port, int async)
#endif
{
	SOCKET sock;
	int one = 1, errno2;
	struct sockaddr_in sin;
	struct in_addr *a = (in_addr*)addr;
	struct sockaddr_in myaddr;

	gg_debug(GG_DEBUG_FUNCTION, "** gg_connect(%s, %d, %d);\n", inet_ntoa(*a), port, async);

	if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		gg_debug(GG_DEBUG_MISC, "// gg_connect() socket() failed (errno=%d, %s)\n", errno, strerror(errno));
		return -1;
	}

	memset(&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;

	myaddr.sin_addr.s_addr = gg_local_ip;

	if (bind(sock, (struct sockaddr *) &myaddr, sizeof(myaddr)) == -1) {
		gg_debug(GG_DEBUG_MISC, "// gg_connect() bind() failed (errno=%d, %s)\n", errno, strerror(errno));
		errno2 = errno;
		gg_sock_close(sock);
		errno = errno2;
		return -1;
	}

#ifdef GG_CONFIG_MIRANDA
	if (gg_sock) *gg_sock = sock;
#endif

	if (async) {
#ifdef FIONBIO
		if (ioctl(sock, FIONBIO, &one) == -1) {
#else
		if (fcntl(sock, F_SETFL, O_NONBLOCK) == -1) {
#endif
			gg_debug(GG_DEBUG_MISC, "// gg_connect() ioctl() failed (errno=%d, %s)\n", errno, strerror(errno));
			errno2 = errno;
			gg_sock_close(sock);
			errno = errno2;
			return -1;
		}
	}

	sin.sin_port = htons((uint16_t)port);
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = a->s_addr;

	errno = 0;
	if (connect(sock, (struct sockaddr*) &sin, sizeof(sin)) == -1) {
		if (errno && (!async || errno != EINPROGRESS)) {
			gg_debug(GG_DEBUG_MISC, "// gg_connect() connect() failed (errno=%d, %s)\n", errno, strerror(errno));
			errno2 = errno;
			gg_sock_close(sock);
			errno = errno2;
			return -1;
		}
		gg_debug(GG_DEBUG_MISC, "// gg_connect() connect() in progress\n");
	}

	return sock;
}

#ifdef GG_CONFIG_MIRANDA
SOCKET gg_connect(void *addr, int port, int async)
{
	return gg_connect_internal(addr, port, async, 0);
}
#endif

/**
 * \internal Usuwa znaki końca linii.
 *
 * Funkcja działa bezpośrednio na buforze.
 *
 * \param line Bufor z tekstem
 *
 * \ingroup helper
 */
void gg_chomp(char *line)
{
	size_t len;

	if (!line)
		return;

	len = strlen(line);

	if (len > 0 && line[len - 1] == '\n')
		line[--len] = 0;
	if (len > 0 && line[len - 1] == '\r')
		line[--len] = 0;
}

/**
 * \internal Koduje ciąg znaków do postacji adresu HTTP.
 *
 * Zamienia znaki niedrukowalne, spoza ASCII i mające specjalne znaczenie
 * dla protokołu HTTP na encje postaci \c %XX, gdzie \c XX jest szesnastkową
 * wartością znaku.
 * 
 * \param str Ciąg znaków do zakodowania
 *
 * \return Zaalokowany bufor lub \c NULL w przypadku błędu.
 *
 * \ingroup helper
 */
char *gg_urlencode(const char *str)
{
	char *q, *buf, hex[] = "0123456789abcdef";
	const char *p;
	unsigned int size = 0;

	if (!str)
		str = "";

	for (p = str; *p; p++, size++) {
		if (!((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9') || *p == ' ') || (*p == '@') || (*p == '.') || (*p == '-'))
			size += 2;
	}

	if (!(buf = (char*)malloc(size + 1)))
		return NULL;

	for (p = str, q = buf; *p; p++, q++) {
		if ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9') || (*p == '@') || (*p == '.') || (*p == '-'))
			*q = *p;
		else {
			if (*p == ' ')
				*q = '+';
			else {
				*q++ = '%';
				*q++ = hex[*p >> 4 & 15];
				*q = hex[*p & 15];
			}
		}
	}

	*q = 0;

	return buf;
}

/**
 * \internal Wyznacza skrót dla usług HTTP.
 *
 * Funkcja jest wykorzystywana do wyznaczania skrótu adresu e-mail, hasła
 * i innych wartości przekazywanych jako parametry usług HTTP.
 *
 * W parametrze \c format należy umieścić znaki określające postać kolejnych
 * parametrów: \c 's' jeśli parametr jest ciągiem znaków, \c 'u' jeśli jest
 * liczbą.
 *
 * \param format Format kolejnych parametrów (niezgodny z \c printf)
 *
 * \return Wartość skrótu
 */
int gg_http_hash(const char *format, ...)
{
	unsigned int a, c, i, j;
	va_list ap;
	int b = -1;

	va_start(ap, format);

	for (j = 0; j < strlen(format); j++) {
		char *arg, buf[16];

		if (format[j] == 'u') {
			mir_snprintf(buf, sizeof(buf), "%d", va_arg(ap, uin_t));
			arg = buf;
		} else {
			if (!(arg = va_arg(ap, char*)))
				arg = "";
		}

		i = 0;
		while ((c = (unsigned char) arg[i++]) != 0) {
			a = (c ^ b) + (c << 8);
			b = (a >> 24) | (a << 8);
		}
	}

	va_end(ap);

	return (b < 0 ? -b : b);
}

/**
 * \internal Zestaw znaków kodowania base64.
 */
static char gg_base64_charset[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/**
 * \internal Koduje ciąg znaków do base64.
 *
 * Wynik funkcji należy zwolnić za pomocą \c free.
 *
 * \param buf Bufor z danami do zakodowania
 *
 * \return Zaalokowany bufor z zakodowanymi danymi
 *
 * \ingroup helper
 */
char *gg_base64_encode(const char *buf)
{
	char *out, *res;
	unsigned int i = 0, j = 0, k = 0, len = (unsigned int)strlen(buf);

	res = out = (char*)malloc((len / 3 + 1) * 4 + 2);

	if (!res)
		return NULL;

	while (j <= len) {
		switch (i % 4) {
			case 0:
				k = (buf[j] & 252) >> 2;
				break;
			case 1:
				if (j < len)
					k = ((buf[j] & 3) << 4) | ((buf[j + 1] & 240) >> 4);
				else
					k = (buf[j] & 3) << 4;

				j++;
				break;
			case 2:
				if (j < len)
					k = ((buf[j] & 15) << 2) | ((buf[j + 1] & 192) >> 6);
				else
					k = (buf[j] & 15) << 2;

				j++;
				break;
			case 3:
				k = buf[j++] & 63;
				break;
		}
		*out++ = gg_base64_charset[k];
		i++;
	}

	if (i % 4)
		for (j = 0; j < 4 - (i % 4); j++, out++)
			*out = '=';

	*out = 0;

	return res;
}

/**
 * \internal Dekoduje ciąg znaków zapisany w base64.
 *
 * Wynik funkcji należy zwolnić za pomocą \c free.
 *
 * \param buf Bufor źródłowy z danymi do zdekodowania
 *
 * \return Zaalokowany bufor ze zdekodowanymi danymi
 *
 * \ingroup helper
 */
char *gg_base64_decode(const char *buf)
{
	char *res, *save, *foo, val;
	const char *end;
	unsigned int index = 0;

	if (!buf)
		return NULL;

	save = res = (char*)calloc(1, (strlen(buf) / 4 + 1) * 3 + 2);

	if (!save)
		return NULL;

	end = buf + strlen(buf);

	while (*buf && buf < end) {
		if (*buf == '\r' || *buf == '\n') {
			buf++;
			continue;
		}
		if (!(foo = strchr(gg_base64_charset, *buf)))
			foo = gg_base64_charset;
		val = (int)(foo - gg_base64_charset);
		buf++;
		switch (index) {
			case 0:
				*res |= val << 2;
				break;
			case 1:
				*res++ |= val >> 4;
				*res |= val << 4;
				break;
			case 2:
				*res++ |= val >> 2;
				*res |= val << 6;
				break;
			case 3:
				*res++ |= val;
				break;
		}
		index++;
		index %= 4;
	}
	*res = 0;

	return save;
}

/**
 * \internal Tworzy nagłówek autoryzacji serwera pośredniczącego.
 *
 * Dane pobiera ze zmiennych globalnych \c gg_proxy_username i
 * \c gg_proxy_password.
 *
 * \return Zaalokowany bufor z tekstem lub NULL, jeśli serwer pośredniczący
 *         nie jest używany lub nie wymaga autoryzacji.
 */
char *gg_proxy_auth()
{
	char *tmp, *enc, *out;
	size_t tmp_size;

	if (!gg_proxy_enabled || !gg_proxy_username || !gg_proxy_password)
		return NULL;

	if (!(tmp = (char*)malloc((tmp_size = strlen(gg_proxy_username) + strlen(gg_proxy_password) + 2))))
		return NULL;

	snprintf(tmp, tmp_size, "%s:%s", gg_proxy_username, gg_proxy_password);

	if (!(enc = gg_base64_encode(tmp))) {
		free(tmp);
		return NULL;
	}

	free(tmp);

	if (!(out = (char*)malloc(strlen(enc) + 40))) {
		free(enc);
		return NULL;
	}

	snprintf(out, strlen(enc) + 40,  "Proxy-Authorization: Basic %s\r\n", enc);

	free(enc);

	return out;
}

/**
 * \internal Tablica pomocnicza do wyznaczania sumy kontrolnej.
 */
static uint32_t gg_crc32_table[256];

/**
 * \internal Flaga wypełnienia tablicy pomocniczej do wyznaczania sumy
 * kontrolnej.
 */
static int gg_crc32_initialized = 0;

/**
 * \internal Tworzy tablicę pomocniczą do wyznaczania sumy kontrolnej.
 */
static void gg_crc32_make_table(void)
{
	uint32_t h = 1;
	unsigned int i, j;

	memset(gg_crc32_table, 0, sizeof(gg_crc32_table));

	for (i = 128; i; i >>= 1) {
		h = (h >> 1) ^ ((h & 1) ? 0xedb88320L : 0);

		for (j = 0; j < 256; j += 2 * i)
			gg_crc32_table[i + j] = gg_crc32_table[j] ^ h;
	}

	gg_crc32_initialized = 1;
}

/**
 * Wyznacza sumę kontrolną CRC32.
 *
 * \param crc Suma kontrola poprzedniego bloku danych lub 0 jeśli liczona
 *            jest suma kontrolna pierwszego bloku
 * \param buf Bufor danych
 * \param len Długość bufora danych
 *
 * \return Suma kontrolna.
 */
uint32_t gg_crc32(uint32_t crc, const unsigned char *buf, int len)
{
	if (!gg_crc32_initialized)
		gg_crc32_make_table();

	if (!buf || len < 0)
		return crc;

	crc ^= 0xffffffffL;

	while (len--)
		crc = (crc >> 8) ^ gg_crc32_table[(crc ^ *buf++) & 0xff];

	return crc ^ 0xffffffffL;
}

/**
 * \internal Tablica konwersji między CP1250 a UTF-8.
 */
static const uint16_t table_cp1250[] = {
	0x20ac, '?',    0x201a,    '?', 0x201e, 0x2026, 0x2020, 0x2021, 
	   '?', 0x2030, 0x0160, 0x2039, 0x015a, 0x0164, 0x017d, 0x0179, 
	   '?', 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014, 
	   '?', 0x2122, 0x0161, 0x203a, 0x015b, 0x0165, 0x017e, 0x017a, 
	0x00a0, 0x02c7, 0x02d8, 0x0141, 0x00a4, 0x0104, 0x00a6, 0x00a7, 
	0x00a8, 0x00a9, 0x015e, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x017b, 
	0x00b0, 0x00b1, 0x02db, 0x0142, 0x00b4, 0x00b5, 0x00b6, 0x00b7, 
	0x00b8, 0x0105, 0x015f, 0x00bb, 0x013d, 0x02dd, 0x013e, 0x017c, 
	0x0154, 0x00c1, 0x00c2, 0x0102, 0x00c4, 0x0139, 0x0106, 0x00c7, 
	0x010c, 0x00c9, 0x0118, 0x00cb, 0x011a, 0x00cd, 0x00ce, 0x010e, 
	0x0110, 0x0143, 0x0147, 0x00d3, 0x00d4, 0x0150, 0x00d6, 0x00d7, 
	0x0158, 0x016e, 0x00da, 0x0170, 0x00dc, 0x00dd, 0x0162, 0x00df, 
	0x0155, 0x00e1, 0x00e2, 0x0103, 0x00e4, 0x013a, 0x0107, 0x00e7, 
	0x010d, 0x00e9, 0x0119, 0x00eb, 0x011b, 0x00ed, 0x00ee, 0x010f, 
	0x0111, 0x0144, 0x0148, 0x00f3, 0x00f4, 0x0151, 0x00f6, 0x00f7, 
	0x0159, 0x016f, 0x00fa, 0x0171, 0x00fc, 0x00fd, 0x0163, 0x02d9, 
};

/**
 * \internal Zamienia tekst kodowany CP1250 na UTF-8.
 *
 * \param b Tekst źródłowy w CP1250.
 *
 * \return Zaalokowany bufor z tekstem w UTF-8.
 */
char *gg_cp_to_utf8(const char *b)
{
	unsigned char *buf = (unsigned char *) b;
	char *newbuf;
	int newlen = 0;
	int i, j;

	for (i = 0; buf[i]; i++) {
		uint16_t znak = (buf[i] < 0x80) ? buf[i] : table_cp1250[buf[i]-0x80];

		if (znak < 0x80)	newlen += 1;
		else if (znak < 0x800)	newlen += 2;
		else			newlen += 3;
	}

	if (!(newbuf = (char*)malloc(newlen + 1))) {
		gg_debug(GG_DEBUG_MISC, "// gg_cp_to_utf8() not enough memory\n");
		return NULL;
	}

	for (i = 0, j = 0; buf[i]; i++) {
		uint16_t znak = (buf[i] < 0x80) ? buf[i] : table_cp1250[buf[i]-0x80];
		int count;

		if (znak < 0x80)	count = 1;
		else if (znak < 0x800)	count = 2;
		else			count = 3;

		switch (count) {
			case 3: newbuf[j+2] = 0x80 | (znak & 0x3f); znak = znak >> 6; znak |= 0x800;
			case 2: newbuf[j+1] = 0x80 | (znak & 0x3f); znak = znak >> 6; znak |= 0xc0;
			case 1: newbuf[j] = (char)znak;
		}
		j += count;
	}
	newbuf[j] = '\0';

	return newbuf;
}

/**
 * \internal Dekoduje jeden znak UTF-8.
 *
 * \note Funkcja nie jest kompletną implementacją UTF-8, a wersją uproszczoną
 * do potrzeb kodowania CP1250.
 *
 * \param s Tekst źródłowy.
 * \param n Długość tekstu źródłowego.
 * \param ch Wskaźnik na wynik dekodowania.
 *
 * \return Długość zdekodowanej sekwencji w bajtach lub wartość mniejsza
 * od zera w przypadku błędu.
 */
static int gg_utf8_helper(unsigned char *s, int n, uint16_t *ch)
{
	unsigned char c = s[0];

	if (c < 0x80) {
		*ch = c;
		return 1;
	}

	if (c < 0xc2) 
		return -1;

	if (c < 0xe0) {
		if (n < 2)
			return -2;
		if (!((s[1] ^ 0x80) < 0x40))
			return -1;
		*ch = ((uint16_t) (c & 0x1f) << 6) | (uint16_t) (s[1] ^ 0x80);
		return 2;
	} 
	
	if (c < 0xf0) {
		if (n < 3)
			return -2;
		if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40 && (c >= 0xe1 || s[1] >= 0xa0)))
			return -1;
		*ch = ((uint16_t) (c & 0x0f) << 12) | ((uint16_t) (s[1] ^ 0x80) << 6) | (uint16_t) (s[2] ^ 0x80);
		return 3;
	}

	return -1;
}

/**
 * \internal Zamienia tekst kodowany UTF-8 na CP1250.
 *
 * \param b Tekst źródłowy w UTF-8.
 *
 * \return Zaalokowany bufor z tekstem w CP1250.
 */
char *gg_utf8_to_cp(const char *b)
{
	unsigned char *buf = (unsigned char *) b;
	char *newbuf;
	int newlen = 0;
	int len;
	int i, j;

	len = (int)strlen(b);

	for (i = 0; i < len; newlen++) {
		uint16_t discard;
		int ret;
		
		ret = gg_utf8_helper(&buf[i], len - i, &discard);

		if (ret > 0)
			i += ret;
		else
			i++;
	}

	if (!(newbuf = (char*)malloc(newlen + 1))) {
		gg_debug(GG_DEBUG_MISC, "// gg_utf8_to_cp() not enough memory\n");
		return NULL;
	}

	for (i = 0, j = 0; buf[i]; j++) {
		uint16_t znak;
		int ret, k;

		ret = gg_utf8_helper(&buf[i], len - i, &znak);

		if (ret > 0) {
			i += ret;
		} else {
			znak = '?';
			i++;
		}

		if (znak < 0x80) {
			newbuf[j] = (char)znak;
			continue;
		}

		newbuf[j] = '?';

		for (k = 0; k < (sizeof(table_cp1250)/sizeof(table_cp1250[0])); k++) {
			if (table_cp1250[k] == znak) {
				newbuf[j] = (0x80 | k);
				break;
			}
		}
	}
	newbuf[j] = '\0';

	return newbuf;
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
