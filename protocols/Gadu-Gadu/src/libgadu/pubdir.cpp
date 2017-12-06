/* coding: UTF-8 */
/* $Id: pubdir.c 11370 2010-03-13 16:17:54Z dezred $ */

/*
 *  (C) Copyright 2001-2006 Wojtek Kaniewski <wojtekka@irc.pl>
 *                          Dawid Jarosz <dawjar@poczta.onet.pl>
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
 * \file pubdir.c
 *
 * \brief Obsługa katalogu publicznego
 */

#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include "win32.h"
#define random() rand()
#else
#include <unistd.h>
#endif

#include "libgadu.h"

/**
 * Rejestruje nowego użytkownika.
 *
 * Wymaga wcześniejszego pobrania tokenu za pomocą \c gg_token().
 *
 * \param email Adres e-mail
 * \param password Hasło
 * \param tokenid Identyfikator tokenu
 * \param tokenval Zawartość tokenu
 * \param async Flaga połączenia asynchronicznego
 *
 * \return Struktura \c gg_http lub \c NULL w przypadku błędu
 *
 * \ingroup register
 */
struct gg_http *gg_register3(const char *email, const char *password, const char *tokenid, const char *tokenval, int async)
{
	struct gg_http *h;
	char *__pwd, *__email, *__tokenid, *__tokenval, *form, *query;

	if (!email || !password || !tokenid || !tokenval) {
		gg_debug(GG_DEBUG_MISC, "=> register, NULL parameter\n");
		errno = EFAULT;
		return NULL;
	}

	__pwd = gg_urlencode(password);
	__email = gg_urlencode(email);
	__tokenid = gg_urlencode(tokenid);
	__tokenval = gg_urlencode(tokenval);

	if (!__pwd || !__email || !__tokenid || !__tokenval) {
		gg_debug(GG_DEBUG_MISC, "=> register, not enough memory for form fields\n");
		free(__pwd);
		free(__email);
		free(__tokenid);
		free(__tokenval);
		return NULL;
	}

	form = gg_saprintf("pwd=%s&email=%s&tokenid=%s&tokenval=%s&code=%u",
			__pwd, __email, __tokenid, __tokenval,
			gg_http_hash("ss", email, password));

	free(__pwd);
	free(__email);
	free(__tokenid);
	free(__tokenval);

	if (!form) {
		gg_debug(GG_DEBUG_MISC, "=> register, not enough memory for form query\n");
		return NULL;
	}

	gg_debug(GG_DEBUG_MISC, "=> register, %s\n", form);

	query = gg_saprintf(
		"Host: " GG_REGISTER_HOST "\r\n"
		"Content-Type: application/x-www-form-urlencoded\r\n"
		"User-Agent: " GG_HTTP_USERAGENT "\r\n"
		"Content-Length: %d\r\n"
		"Pragma: no-cache\r\n"
		"\r\n"
		"%s",
		(int) strlen(form), form);

	free(form);

	if (!query) {
		gg_debug(GG_DEBUG_MISC, "=> register, not enough memory for query\n");
		return NULL;
	}

	if (!(h = gg_http_connect(GG_REGISTER_HOST, GG_REGISTER_PORT, async, "POST", "/appsvc/fmregister3.asp", query))) {
		gg_debug(GG_DEBUG_MISC, "=> register, gg_http_connect() failed mysteriously\n");
		free(query);
		return NULL;
	}

	h->type = GG_SESSION_REGISTER;

	free(query);

	h->callback = gg_pubdir_watch_fd;
	h->destroy = gg_pubdir_free;
	
	if (!async)
		gg_pubdir_watch_fd(h);
	
	return h;
}

#ifdef DOXYGEN

/**
 * Funkcja wywoływana po zaobserwowaniu zmian na deskryptorze połączenia.
 *
 * Operacja będzie zakończona, gdy pole \c state będzie równe \c GG_STATE_DONE.
 * Jeśli wystąpi błąd, \c state będzie równe \c GG_STATE_ERROR, a kod błędu
 * znajdzie się w polu \c error.
 *
 * \note W rzeczywistości funkcja jest makrem rozwijanym do
 * \c gg_pubdir_watch_fd().
 *
 * \param h Struktura połączenia
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 *
 * \ingroup register
 */
int gg_register_watch_fd(struct gg_httpd *h)
{
	return gg_pubdir_watch_fd(h);
}

/**
 * Zwalnia zasoby po operacji.
 *
 * \note W rzeczywistości funkcja jest makrem rozwijanym do \c gg_pubdir_free().
 *
 * \param h Struktura połączenia
 *
 * \ingroup register
 */
void gg_register_free(struct gg_http *h)
{
	return gg_pubdir_free(h);
}

#endif /* DOXYGEN */

/**
 * Usuwa użytkownika.
 *
 * Wymaga wcześniejszego pobrania tokenu za pomocą \c gg_token().
 *
 * \param uin Numer Gadu-Gadu
 * \param password Hasło
 * \param tokenid Identyfikator tokenu
 * \param tokenval Zawartość tokenu
 * \param async Flaga połączenia asynchronicznego
 *
 * \return Struktura \c gg_http lub \c NULL w przypadku błędu
 *
 * \ingroup unregister
 */
struct gg_http *gg_unregister3(uin_t uin, const char *password, const char *tokenid, const char *tokenval, int async)
{
	struct gg_http *h;
	char *__fmpwd, *__pwd, *__tokenid, *__tokenval, *form, *query;

	if (!password || !tokenid || !tokenval) {
		gg_debug(GG_DEBUG_MISC, "=> unregister, NULL parameter\n");
		errno = EFAULT;
		return NULL;
	}
    
	__pwd = gg_saprintf("%ld", random());
	__fmpwd = gg_urlencode(password);
	__tokenid = gg_urlencode(tokenid);
	__tokenval = gg_urlencode(tokenval);

	if (!__fmpwd || !__pwd || !__tokenid || !__tokenval) {
		gg_debug(GG_DEBUG_MISC, "=> unregister, not enough memory for form fields\n");
		free(__pwd);
		free(__fmpwd);
		free(__tokenid);
		free(__tokenval);
		return NULL;
	}

	form = gg_saprintf("fmnumber=%d&fmpwd=%s&delete=1&pwd=%s&email=deletedaccount@gadu-gadu.pl&tokenid=%s&tokenval=%s&code=%u", uin, __fmpwd, __pwd, __tokenid, __tokenval, gg_http_hash("ss", "deletedaccount@gadu-gadu.pl", __pwd));

	free(__fmpwd);
	free(__pwd);
	free(__tokenid);
	free(__tokenval);

	if (!form) {
		gg_debug(GG_DEBUG_MISC, "=> unregister, not enough memory for form query\n");
		return NULL;
	}

	gg_debug(GG_DEBUG_MISC, "=> unregister, %s\n", form);

	query = gg_saprintf(
		"Host: " GG_REGISTER_HOST "\r\n"
		"Content-Type: application/x-www-form-urlencoded\r\n"
		"User-Agent: " GG_HTTP_USERAGENT "\r\n"
		"Content-Length: %d\r\n"
		"Pragma: no-cache\r\n"
		"\r\n"
		"%s",
		(int) strlen(form), form);

	free(form);

	if (!query) {
		gg_debug(GG_DEBUG_MISC, "=> unregister, not enough memory for query\n");
		return NULL;
	}

	if (!(h = gg_http_connect(GG_REGISTER_HOST, GG_REGISTER_PORT, async, "POST", "/appsvc/fmregister3.asp", query))) {
		gg_debug(GG_DEBUG_MISC, "=> unregister, gg_http_connect() failed mysteriously\n");
		free(query);
		return NULL;
	}

	h->type = GG_SESSION_UNREGISTER;

	free(query);

	h->callback = gg_pubdir_watch_fd;
	h->destroy = gg_pubdir_free;
	
	if (!async)
		gg_pubdir_watch_fd(h);
	
	return h;
}

#ifdef DOXYGEN

/**
 * Funkcja wywoływana po zaobserwowaniu zmian na deskryptorze połączenia.
 *
 * Operacja będzie zakończona, gdy pole \c state będzie równe \c GG_STATE_DONE.
 * Jeśli wystąpi błąd, \c state będzie równe \c GG_STATE_ERROR, a kod błędu
 * znajdzie się w polu \c error.
 *
 * \note W rzeczywistości funkcja jest makrem rozwijanym do
 * \c gg_pubdir_watch_fd().
 *
 * \param h Struktura połączenia
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 *
 * \ingroup unregister
 */
int gg_unregister_watch_fd(struct gg_httpd *h)
{
	return gg_pubdir_watch_fd(h);
}

/**
 * Zwalnia zasoby po operacji.
 *
 * \note W rzeczywistości funkcja jest makrem rozwijanym do \c gg_pubdir_free().
 *
 * \param h Struktura połączenia
 *
 * \ingroup unregister
 */
void gg_unregister_free(struct gg_http *h)
{
	return gg_pubdir_free(h);
}

#endif /* DOXYGEN */

/**
 * Zmienia hasło użytkownika.
 *
 * Wymaga wcześniejszego pobrania tokenu za pomocą \c gg_token().
 *
 * \param uin Numer Gadu-Gadu
 * \param email Adres e-mail
 * \param passwd Obecne hasło
 * \param newpasswd Nowe hasło
 * \param tokenid Identyfikator tokenu
 * \param tokenval Zawartość tokenu
 * \param async Flaga połączenia asynchronicznego
 *
 * \return Struktura \c gg_http lub \c NULL w przypadku błędu
 *
 * \ingroup passwd
 */
struct gg_http *gg_change_passwd4(uin_t uin, const char *email, const char *passwd, const char *newpasswd, const char *tokenid, const char *tokenval, int async)
{
	struct gg_http *h;
	char *form, *query, *__email, *__fmpwd, *__pwd, *__tokenid, *__tokenval;

	if (!uin || !email || !passwd || !newpasswd || !tokenid || !tokenval) {
		gg_debug(GG_DEBUG_MISC, "=> change, NULL parameter\n");
		errno = EFAULT;
		return NULL;
	}
	
	__fmpwd = gg_urlencode(passwd);
	__pwd = gg_urlencode(newpasswd);
	__email = gg_urlencode(email);
	__tokenid = gg_urlencode(tokenid);
	__tokenval = gg_urlencode(tokenval);

	if (!__fmpwd || !__pwd || !__email || !__tokenid || !__tokenval) {
		gg_debug(GG_DEBUG_MISC, "=> change, not enough memory for form fields\n");
		free(__fmpwd);
		free(__pwd);
		free(__email);
		free(__tokenid);
		free(__tokenval);
		return NULL;
	}
	
	if (!(form = gg_saprintf("fmnumber=%d&fmpwd=%s&pwd=%s&email=%s&tokenid=%s&tokenval=%s&code=%u", uin, __fmpwd, __pwd, __email, __tokenid, __tokenval, gg_http_hash("ss", email, newpasswd)))) {
		gg_debug(GG_DEBUG_MISC, "=> change, not enough memory for form fields\n");
		free(__fmpwd);
		free(__pwd);
		free(__email);
		free(__tokenid);
		free(__tokenval);

		return NULL;
	}
	
	free(__fmpwd);
	free(__pwd);
	free(__email);
	free(__tokenid);
	free(__tokenval);
	
	gg_debug(GG_DEBUG_MISC, "=> change, %s\n", form);

	query = gg_saprintf(
		"Host: " GG_REGISTER_HOST "\r\n"
		"Content-Type: application/x-www-form-urlencoded\r\n"
		"User-Agent: " GG_HTTP_USERAGENT "\r\n"
		"Content-Length: %d\r\n"
		"Pragma: no-cache\r\n"
		"\r\n"
		"%s",
		(int) strlen(form), form);

	free(form);

	if (!query) {
		gg_debug(GG_DEBUG_MISC, "=> change, not enough memory for query\n");
		return NULL;
	}

	if (!(h = gg_http_connect(GG_REGISTER_HOST, GG_REGISTER_PORT, async, "POST", "/appsvc/fmregister3.asp", query))) {
		gg_debug(GG_DEBUG_MISC, "=> change, gg_http_connect() failed mysteriously\n");
		free(query);
		return NULL;
	}

	h->type = GG_SESSION_PASSWD;

	free(query);

	h->callback = gg_pubdir_watch_fd;
	h->destroy = gg_pubdir_free;

	if (!async)
		gg_pubdir_watch_fd(h);

	return h;
}

#ifdef DOXYGEN

/**
 * Funkcja wywoływana po zaobserwowaniu zmian na deskryptorze połączenia.
 *
 * Operacja będzie zakończona, gdy pole \c state będzie równe \c GG_STATE_DONE.
 * Jeśli wystąpi błąd, \c state będzie równe \c GG_STATE_ERROR, a kod błędu
 * znajdzie się w polu \c error.
 *
 * \note W rzeczywistości funkcja jest makrem rozwijanym do
 * \c gg_pubdir_watch_fd().
 *
 * \param h Struktura połączenia
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 *
 * \ingroup passwd
 */
int gg_change_passwd_watch_fd(struct gg_httpd *h)
{
	return gg_pubdir_watch_fd(h);
}

/**
 * Zwalnia zasoby po operacji.
 *
 * \note W rzeczywistości funkcja jest makrem rozwijanym do \c gg_pubdir_free().
 *
 * \param h Struktura połączenia
 *
 * \ingroup passwd
 */
void gg_change_passwd_free(struct gg_http *h)
{
	return gg_pubdir_free(h);
}

#endif /* DOXYGEN */

/**
 * Wysyła hasło użytkownika na e-mail.
 *
 * Wymaga wcześniejszego pobrania tokenu za pomocą \c gg_token().
 *
 * \param uin Numer Gadu-Gadu
 * \param email Adres e-mail (podany przy rejestracji)
 * \param tokenid Identyfikator tokenu
 * \param tokenval Zawartość tokenu
 * \param async Flaga połączenia asynchronicznego
 *
 * \return Struktura \c gg_http lub \c NULL w przypadku błędu
 *
 * \ingroup remind
 */
struct gg_http *gg_remind_passwd3(uin_t uin, const char *email, const char *tokenid, const char *tokenval, int async)
{
	struct gg_http *h;
	char *form, *query, *__tokenid, *__tokenval, *__email;

	if (!tokenid || !tokenval || !email) {
		gg_debug(GG_DEBUG_MISC, "=> remind, NULL parameter\n");
		errno = EFAULT;
		return NULL;
	}
	
	__tokenid = gg_urlencode(tokenid);
	__tokenval = gg_urlencode(tokenval);
	__email = gg_urlencode(email);

	if (!__tokenid || !__tokenval || !__email) {
		gg_debug(GG_DEBUG_MISC, "=> remind, not enough memory for form fields\n");
		free(__tokenid);
		free(__tokenval);
		free(__email);
		return NULL;
	}

	if (!(form = gg_saprintf("userid=%d&code=%u&tokenid=%s&tokenval=%s&email=%s", uin, gg_http_hash("u", uin), __tokenid, __tokenval, __email))) {
		gg_debug(GG_DEBUG_MISC, "=> remind, not enough memory for form fields\n");
		free(__tokenid);
		free(__tokenval);
		free(__email);
		return NULL;
	}

	free(__tokenid);
	free(__tokenval);
	free(__email);
	
	gg_debug(GG_DEBUG_MISC, "=> remind, %s\n", form);

	query = gg_saprintf(
		"Host: " GG_REMIND_HOST "\r\n"
		"Content-Type: application/x-www-form-urlencoded\r\n"
		"User-Agent: " GG_HTTP_USERAGENT "\r\n"
		"Content-Length: %d\r\n"
		"Pragma: no-cache\r\n"
		"\r\n"
		"%s",
		(int) strlen(form), form);

	free(form);

	if (!query) {
		gg_debug(GG_DEBUG_MISC, "=> remind, not enough memory for query\n");
		return NULL;
	}

	if (!(h = gg_http_connect(GG_REMIND_HOST, GG_REMIND_PORT, async, "POST", "/appsvc/fmsendpwd3.asp", query))) {
		gg_debug(GG_DEBUG_MISC, "=> remind, gg_http_connect() failed mysteriously\n");
		free(query);
		return NULL;
	}

	h->type = GG_SESSION_REMIND;

	free(query);

	h->callback = gg_pubdir_watch_fd;
	h->destroy = gg_pubdir_free;

	if (!async)
		gg_pubdir_watch_fd(h);

	return h;
}

#ifdef DOXYGEN

/**
 * Funkcja wywoływana po zaobserwowaniu zmian na deskryptorze połączenia.
 *
 * Operacja będzie zakończona, gdy pole \c state będzie równe \c GG_STATE_DONE.
 * Jeśli wystąpi błąd, \c state będzie równe \c GG_STATE_ERROR, a kod błędu
 * znajdzie się w polu \c error.
 *
 * \note W rzeczywistości funkcja jest makrem rozwijanym do
 * \c gg_pubdir_watch_fd().
 *
 * \param h Struktura połączenia
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 *
 * \ingroup remind
 */
int gg_remind_watch_fd(struct gg_httpd *h)
{
	return gg_pubdir_watch_fd(h);
}

/**
 * Zwalnia zasoby po operacji.
 *
 * \note W rzeczywistości funkcja jest makrem rozwijanym do \c gg_pubdir_free().
 *
 * \param h Struktura połączenia
 *
 * \ingroup remind
 */
void gg_remind_free(struct gg_http *h)
{
	return gg_pubdir_free(h);
}

#endif /* DOXYGEN */

/**
 * Funkcja wywoływana po zaobserwowaniu zmian na deskryptorze połączenia.
 *
 * Operacja będzie zakończona, gdy pole \c state będzie równe \c GG_STATE_DONE.
 * Jeśli wystąpi błąd, \c state będzie równe \c GG_STATE_ERROR, a kod błędu
 * znajdzie się w polu \c error.
 *
 * \param h Struktura połączenia
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 */
int gg_pubdir_watch_fd(struct gg_http *h)
{
	struct gg_pubdir *p;
	char *tmp;

	if (!h) {
		errno = EFAULT;
		return -1;
	}

	if (h->state == GG_STATE_ERROR) {
		gg_debug(GG_DEBUG_MISC, "=> pubdir, watch_fd issued on failed session\n");
		errno = EINVAL;
		return -1;
	}
	
	if (h->state != GG_STATE_PARSING) {
		if (gg_http_watch_fd(h) == -1) {
			gg_debug(GG_DEBUG_MISC, "=> pubdir, http failure\n");
			errno = EINVAL;
			return -1;
		}
	}

	if (h->state != GG_STATE_PARSING)
		return 0;
	
	h->state = GG_STATE_DONE;
	
	if (!(h->data = p = (gg_pubdir*)malloc(sizeof(struct gg_pubdir)))) {
		gg_debug(GG_DEBUG_MISC, "=> pubdir, not enough memory for results\n");
		return -1;
	}

	p->success = 0;
	p->uin = 0;
	
	gg_debug(GG_DEBUG_MISC, "=> pubdir, let's parse \"%s\"\n", h->body);

	if ((tmp = strstr(h->body, "Tokens okregisterreply_packet.reg.dwUserId="))) {
		p->success = 1;
		p->uin = strtol(tmp + sizeof("Tokens okregisterreply_packet.reg.dwUserId=") - 1, NULL, 0);
		gg_debug(GG_DEBUG_MISC, "=> pubdir, success (okregisterreply, uin=%d)\n", p->uin);
	} else if ((tmp = strstr(h->body, "success")) || (tmp = strstr(h->body, "results"))) {
		p->success = 1;
		if (tmp[7] == ':')
			p->uin = strtol(tmp + 8, NULL, 0);
		gg_debug(GG_DEBUG_MISC, "=> pubdir, success (uin=%d)\n", p->uin);
	} else
		gg_debug(GG_DEBUG_MISC, "=> pubdir, error.\n");

	return 0;
}

/**
 * Zwalnia zasoby po operacji na katalogu publicznym.
 *
 * \param h Struktura połączenia
 */
void gg_pubdir_free(struct gg_http *h)
{
	if (!h)
		return;
	
	free(h->data);
	gg_http_free(h);
}

/**
 * Pobiera token do autoryzacji operacji na katalogu publicznym.
 *
 * Token jest niezbędny do tworzenia nowego i usuwania użytkownika,
 * zmiany hasła itd.
 *
 * \param async Flaga połączenia asynchronicznego
 *
 * \return Struktura \c gg_http lub \c NULL w przypadku błędu
 *
 * \ingroup token
 */
struct gg_http *gg_token(int async)
{
	struct gg_http *h;
	const char *query;

	query = "Host: " GG_REGISTER_HOST "\r\n"
		"Content-Type: application/x-www-form-urlencoded\r\n"
		"User-Agent: " GG_HTTP_USERAGENT "\r\n"
		"Content-Length: 0\r\n"
		"Pragma: no-cache\r\n"
		"\r\n";

	if (!(h = gg_http_connect(GG_REGISTER_HOST, GG_REGISTER_PORT, async, "POST", "/appsvc/regtoken.asp", query))) {
		gg_debug(GG_DEBUG_MISC, "=> token, gg_http_connect() failed mysteriously\n");
		return NULL;
	}

	h->type = GG_SESSION_TOKEN;

	h->callback = gg_token_watch_fd;
	h->destroy = gg_token_free;
	
	if (!async)
		gg_token_watch_fd(h);
	
	return h;
}

/**
 * Funkcja wywoływana po zaobserwowaniu zmian na deskryptorze połączenia.
 *
 * Operacja będzie zakończona, gdy pole \c state będzie równe \c GG_STATE_DONE.
 * Jeśli wystąpi błąd, \c state będzie równe \c GG_STATE_ERROR, a kod błędu
 * znajdzie się w polu \c error.
 *
 * \param h Struktura połączenia
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 *
 * \ingroup token
 */
int gg_token_watch_fd(struct gg_http *h)
{
	if (!h) {
		errno = EFAULT;
		return -1;
	}

	if (h->state == GG_STATE_ERROR) {
		gg_debug(GG_DEBUG_MISC, "=> token, watch_fd issued on failed session\n");
		errno = EINVAL;
		return -1;
	}
	
	if (h->state != GG_STATE_PARSING) {
		if (gg_http_watch_fd(h) == -1) {
			gg_debug(GG_DEBUG_MISC, "=> token, http failure\n");
			errno = EINVAL;
			return -1;
		}
	}

	if (h->state != GG_STATE_PARSING)
		return 0;
	
	/* jeśli h->data jest puste, to ściągaliśmy tokenid i url do niego,
	 * ale jeśli coś tam jest, to znaczy, że mamy drugi etap polegający
	 * na pobieraniu tokenu. */
	if (!h->data) {
		int width, height, length;
		char *url = NULL, *tokenid = NULL, *path, *headers;
		const char *host;
		struct gg_http *h2;
		struct gg_token *t;

		gg_debug(GG_DEBUG_MISC, "=> token body \"%s\"\n", h->body);

		if (h->body && (!(url = (char*)malloc(strlen(h->body) + 1)) || !(tokenid = (char*)malloc(strlen(h->body) + 1)))) {
			gg_debug(GG_DEBUG_MISC, "=> token, not enough memory for results\n");
			free(url);
			return -1;
		}
		
		if (!h->body || sscanf(h->body, "%d %d %d\r\n%s\r\n%s", &width, &height, &length, tokenid, url) != 5) {
			gg_debug(GG_DEBUG_MISC, "=> token, parsing failed\n");
			free(url);
			free(tokenid);
			errno = EINVAL;
			return -1;
		}
		
		/* dostaliśmy tokenid i wszystkie niezbędne informacje,
		 * więc pobierzmy obrazek z tokenem */

		if (strncmp(url, "http://", 7)) {
			path = gg_saprintf("%s?tokenid=%s", url, tokenid);
			host = GG_REGISTER_HOST;
		} else {
			char *slash = strchr(url + 7, '/');

			if (slash) {
				path = gg_saprintf("%s?tokenid=%s", slash, tokenid);
				*slash = 0;
				host = url + 7;
			} else {
				gg_debug(GG_DEBUG_MISC, "=> token, url parsing failed\n");
				free(url);
				free(tokenid);
				errno = EINVAL;
				return -1;
			}
		}

		if (!path) {
			gg_debug(GG_DEBUG_MISC, "=> token, not enough memory for token url\n");
			free(url);
			free(tokenid);
			return -1;
		}

		if (!(headers = gg_saprintf("Host: %s\r\nUser-Agent: " GG_HTTP_USERAGENT "\r\n\r\n", host))) {
			gg_debug(GG_DEBUG_MISC, "=> token, not enough memory for token url\n");
			free(path);
			free(url);
			free(tokenid);
			return -1;
		}			

		if (!(h2 = gg_http_connect(host, GG_REGISTER_PORT, h->async, "GET", path, headers))) {
			gg_debug(GG_DEBUG_MISC, "=> token, gg_http_connect() failed mysteriously\n");
			free(headers);
			free(url);
			free(path);
			free(tokenid);
			return -1;
		}

		free(headers);
		free(path);
		free(url);

		gg_http_free_fields(h);

		memcpy(h, h2, sizeof(struct gg_http));
		free(h2);

		h->type = GG_SESSION_TOKEN;

		h->callback = gg_token_watch_fd;
		h->destroy = gg_token_free;
	
		if (!h->async)
			gg_token_watch_fd(h);

		if (!(h->data = t = (struct gg_token*)malloc(sizeof(struct gg_token)))) {
			gg_debug(GG_DEBUG_MISC, "=> token, not enough memory for token data\n");
			free(tokenid);
			return -1;
		}

		t->width = width;
		t->height = height;
		t->length = length;
		t->tokenid = tokenid;
	} else {
		/* obrazek mamy w h->body */
		h->state = GG_STATE_DONE;
	}
	
	return 0;
}

/**
 * Zwalnia zasoby po operacji pobierania tokenu.
 *
 * \param h Struktura połączenia
 *
 * \ingroup token
 */
void gg_token_free(struct gg_http *h)
{
	struct gg_token *t;

	if (!h)
		return;

	if (t = (struct gg_token*)h->data)
		free(t->tokenid);
	
	free(h->data);
	gg_http_free(h);
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
