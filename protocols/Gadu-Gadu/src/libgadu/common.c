/* $Id: common.c 13762 2011-08-09 12:35:16Z dezred $ */

/*
 *  (C) Copyright 2001-2006 Wojtek Kaniewski <wojtekka@irc.pl>
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

	memset(&sin, 0, sizeof(sin));
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
	char *q, *buf;
	const char hex[] = "0123456789abcdef";
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
char *gg_proxy_auth(void)
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
static const uint32_t gg_crc32_table[256] =
{
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba,
	0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
	0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,
	0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
	0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940,
	0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116,
	0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
	0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
	0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a,
	0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818,
	0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
	0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
	0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c,
	0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
	0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
	0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
	0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086,
	0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4,
	0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
	0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
	0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
	0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe,
	0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
	0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
	0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252,
	0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60,
	0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
	0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
	0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04,
	0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,
	0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
	0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
	0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e,
	0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
	0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
	0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
	0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0,
	0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6,
	0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
	0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

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
	if (buf == NULL || len < 0)
		return crc;

	crc ^= 0xffffffffL;

	while (len--)
		crc = (crc >> 8) ^ gg_crc32_table[(crc ^ *buf++) & 0xff];

	return crc ^ 0xffffffffL;
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
