/*
 *  (C) Copyright 2001-2009 Wojtek Kaniewski <wojtekka@irc.pl>
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
 * \file resolver.c
 *
 * \brief Funkcje rozwiązywania nazw
 */

#include "internal.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "strman.h"
#include "network.h"
#include "resolver.h"
#include "session.h"

#ifdef GG_CONFIG_HAVE_FORK
#include <sys/wait.h>
#include <signal.h>
#endif

/** Sposób rozwiązywania nazw serwerów */
static gg_resolver_t gg_global_resolver_type = GG_RESOLVER_DEFAULT;

/** Funkcja rozpoczynająca rozwiązywanie nazwy */
static int (*gg_global_resolver_start)(int *fd, void **private_data, const char *hostname);

/** Funkcja zwalniająca zasoby po rozwiązaniu nazwy */
static void (*gg_global_resolver_cleanup)(void **private_data, int force);

/**
 * \internal Odpowiednik \c gethostbyname zapewniający współbieżność.
 *
 * Jeśli dany system dostarcza \c gethostbyname_r, używa się tej wersji, jeśli
 * nie, to zwykłej \c gethostbyname. Wynikiem jest tablica adresów zakończona
 * wartością INADDR_NONE, którą należy zwolnić po użyciu.
 *
 * \param hostname Nazwa serwera
 * \param result Wskaźnik na wskaźnik z tablicą adresów zakończoną INADDR_NONE
 * \param count Wskaźnik na zmienną, do ktorej zapisze się liczbę wyników
 * \param pthread Flaga blokowania unicestwiania wątku podczas alokacji pamięci
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 */
int gg_gethostbyname_real(const char *hostname, struct in_addr **result, unsigned int *count, int pthread)
{
#ifdef GG_CONFIG_HAVE_GETHOSTBYNAME_R
	char *buf = NULL;
	char *new_buf = NULL;
	struct hostent he;
	struct hostent *he_ptr = NULL;
	size_t buf_len;
	int res;
	int h_errnop;
	int ret = 0;

	if (result == NULL) {
		errno = EINVAL;
		return -1;
	}

	buf_len = 1024;
	res = -1;
	buf = malloc(buf_len);

	if (buf != NULL) {
		while (1) {
#ifndef sun
			ret = gethostbyname_r(hostname, &he, buf, buf_len, &he_ptr, &h_errnop);
			if (ret != ERANGE)
				break;
#else
			he_ptr = gethostbyname_r(hostname, &he, buf, buf_len, &h_errnop);
			if (he_ptr != NULL || errno != ERANGE)
				break;
#endif

			buf_len *= 2;

			new_buf = realloc(buf, buf_len);

			if (new_buf != NULL)
				buf = new_buf;

			if (new_buf == NULL) {
				ret = ENOMEM;
				break;
			}
		}

		if (ret == 0 && he_ptr != NULL && he_ptr->h_addr_list[0] != NULL) {
			int i;

			/* Policz liczbę adresów */

			for (i = 0; he_ptr->h_addr_list[i] != NULL; i++);

			/* Zaalokuj */
			*result = malloc((i + 1) * sizeof(struct in_addr));
			if (*result != NULL) {
				/* Kopiuj */

				for (i = 0; he_ptr->h_addr_list[i] != NULL; i++)
					memcpy(&((*result)[i]), he_ptr->h_addr_list[i], sizeof(struct in_addr));

				(*result)[i].s_addr = INADDR_NONE;

				*count = i;

				res = 0;
			} else
				res = -1;
		}

		free(buf);
		buf = NULL;
	}

	return res;
#else /* GG_CONFIG_HAVE_GETHOSTBYNAME_R */
	struct hostent *he;
	int i;

	if (result == NULL || count == NULL) {
		errno = EINVAL;
		return -1;
	}

	he = gethostbyname(hostname);

	if (he == NULL || he->h_addr_list[0] == NULL)
		return -1;

	/* Policz liczbę adresów */

	for (i = 0; he->h_addr_list[i] != NULL; i++);

	/* Zaalokuj */

	*result = malloc((i + 1) * sizeof(struct in_addr));
	if (*result == NULL)
		return -1;

	/* Kopiuj */

	for (i = 0; he->h_addr_list[i] != NULL; i++)
		memcpy(&((*result)[i]), he->h_addr_list[i], sizeof(struct in_addr));

	(*result)[i].s_addr = INADDR_NONE;

	*count = i;

	return 0;
#endif /* GG_CONFIG_HAVE_GETHOSTBYNAME_R */
}

/**
 * \internal Rozwiązuje nazwę i zapisuje wynik do podanego gniazda.
 *
 * \note Użycie logowania w tej funkcji może mieć negatywny wpływ na
 * aplikacje jednowątkowe korzystające.
 *
 * \param fd Deskryptor gniazda
 * \param hostname Nazwa serwera
 * \param pthread Flaga blokowania unicestwiania wątku podczas alokacji pamięci
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 */
static int gg_resolver_run(int fd, const char *hostname, int pthread)
{
	struct in_addr addr_ip[2], *addr_list = NULL;
	unsigned int addr_count;
	int res = 0;

	if ((addr_ip[0].s_addr = inet_addr(hostname)) == INADDR_NONE) {
		if (gg_gethostbyname_real(hostname, &addr_list, &addr_count, pthread) == -1) {
			free(addr_list);
			addr_list = NULL;
			addr_count = 0;
			/* addr_ip[0] już zawiera INADDR_NONE */
		}
	} else {
		addr_ip[1].s_addr = INADDR_NONE;
		addr_count = 1;
	}

	if (send(fd, addr_list != NULL ? addr_list : addr_ip,
		(addr_count + 1) * sizeof(struct in_addr), 0) !=
		(int)((addr_count + 1) * sizeof(struct in_addr)))
	{
		res = -1;
	}

	free(addr_list);
	addr_list = NULL;
	return res;
}

/**
 * \internal Odpowiednik \c gethostbyname zapewniający współbieżność.
 *
 * Jeśli dany system dostarcza \c gethostbyname_r, używa się tej wersji, jeśli
 * nie, to zwykłej \c gethostbyname. Funkcja służy do zachowania zgodności
 * ABI i służy do pobierania tylko pierwszego adresu -- pozostałe mogą
 * zostać zignorowane przez aplikację.
 *
 * \param hostname Nazwa serwera
 *
 * \return Zaalokowana struktura \c in_addr lub NULL w przypadku błędu.
 */
struct in_addr *gg_gethostbyname(const char *hostname)
{
	struct in_addr *result;
	unsigned int count;

	if (gg_gethostbyname_real(hostname, &result, &count, 0) == -1)
		return NULL;

	return result;
}

#ifdef GG_CONFIG_HAVE_FORK

/**
 * \internal Struktura przekazywana do wątku rozwiązującego nazwę.
 */
struct gg_resolver_fork_data {
	int pid;		/*< Identyfikator procesu */
};

/**
 * \internal Rozwiązuje nazwę serwera w osobnym procesie.
 *
 * Połączenia asynchroniczne nie mogą blokować procesu w trakcie rozwiązywania
 * nazwy serwera. W tym celu tworzona jest para gniazd, nowy proces i dopiero
 * w nim przeprowadzane jest rozwiązywanie nazwy. Deskryptor gniazda do odczytu
 * zapisuje się w strukturze sieci i czeka na dane w postaci struktury
 * \c in_addr. Jeśli nie znaleziono nazwy, zwracana jest \c INADDR_NONE.
 *
 * \param fd Wskaźnik na zmienną, gdzie zostanie umieszczony deskryptor gniazda
 * \param priv_data Wskaźnik na zmienną, gdzie zostanie umieszczony wskaźnik
 *                  do numeru procesu potomnego rozwiązującego nazwę
 * \param hostname Nazwa serwera do rozwiązania
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 */
static int gg_resolver_fork_start(int *fd, void **priv_data, const char *hostname)
{
	struct gg_resolver_fork_data *data = NULL;
	int pipes[2], new_errno;

	gg_debug(GG_DEBUG_FUNCTION, "** gg_resolver_fork_start(%p, %p, \"%s\");\n", fd, priv_data, hostname);

	if (fd == NULL || priv_data == NULL || hostname == NULL) {
		gg_debug(GG_DEBUG_MISC, "// gg_resolver_fork_start() invalid arguments\n");
		errno = EFAULT;
		return -1;
	}

	data = malloc(sizeof(struct gg_resolver_fork_data));

	if (data == NULL) {
		gg_debug(GG_DEBUG_MISC, "// gg_resolver_fork_start() out of memory for resolver data\n");
		return -1;
	}

	if (socketpair(AF_LOCAL, SOCK_STREAM, 0, pipes) == -1) {
		gg_debug(GG_DEBUG_MISC, "// gg_resolver_fork_start() unable "
			"to create pipes (errno=%d, %s)\n",
			errno, strerror(errno));
		free(data);
		return -1;
	}

	data->pid = fork();

	if (data->pid == -1) {
		new_errno = errno;
		goto cleanup;
	}

	if (data->pid == 0) {
		int status;

		close(pipes[0]);

		status = (gg_resolver_run(pipes[1], hostname, 0) == -1) ? 1 : 0;

#ifdef HAVE__EXIT
		_exit(status);
#else
		exit(status);
#endif
	}

	close(pipes[1]);

	gg_debug(GG_DEBUG_MISC, "// gg_resolver_fork_start() %p\n", data);

	*fd = pipes[0];
	*priv_data = data;

	return 0;

cleanup:
	free(data);
	close(pipes[0]);
	close(pipes[1]);

	errno = new_errno;

	return -1;
}

/**
 * \internal Usuwanie zasobów po procesie rozwiązywaniu nazwy.
 *
 * Funkcja wywoływana po zakończeniu rozwiązanywania nazwy lub przy zwalnianiu
 * zasobów sesji podczas rozwiązywania nazwy.
 *
 * \param priv_data Wskaźnik na zmienną przechowującą wskaźnik do prywatnych
 *                  danych
 * \param force Flaga usuwania zasobów przed zakończeniem działania
 */
static void gg_resolver_fork_cleanup(void **priv_data, int force)
{
	struct gg_resolver_fork_data *data;

	if (priv_data == NULL || *priv_data == NULL)
		return;

	data = (struct gg_resolver_fork_data*) *priv_data;
	*priv_data = NULL;

	if (force)
		kill(data->pid, SIGKILL);

	/* we don't care about child's exit status, just want to clean it up */
	(void)waitpid(data->pid, NULL, WNOHANG);

	free(data);
}

#endif /* GG_CONFIG_HAVE_FORK */

#ifdef _WIN32

/**
 * \internal Struktura przekazywana do wątku rozwiązującego nazwę.
 */
struct gg_resolver_win32_data {
	HANDLE thread;		/*< Uchwyt wątku */
	CRITICAL_SECTION mutex;	/*< Semafor wątku */
	char *hostname;		/*< Nazwa serwera */
	int wfd;		/*< Deskryptor do zapisu */
	int orphan;		/*< Wątek powinien sam po sobie posprzątać */
	int finished;		/*< Wątek już skończył pracę */
};

/**
 * \internal Wątek rozwiązujący nazwę.
 *
 * \param arg Wskaźnik na strukturę \c gg_resolver_win32_data
 */
static DWORD WINAPI gg_resolver_win32_thread(void *arg)
{
	struct gg_resolver_win32_data *data = arg;
	int result, is_orphan;

	result = gg_resolver_run(data->wfd, data->hostname, 0);

	EnterCriticalSection(&data->mutex);
	is_orphan = data->orphan;
	data->finished = 1;
	LeaveCriticalSection(&data->mutex);

	if (is_orphan) {
		CloseHandle(data->thread);
		DeleteCriticalSection(&data->mutex);
		close(data->wfd);
		free(data->hostname);
		free(data);
	}

	ExitThread(result);
	return 0; /* żeby kompilator nie marudził */
}

/**
 * \internal Rozwiązuje nazwę serwera w osobnym wątku.
 *
 * Funkcja działa analogicznie do \c gg_resolver_pthread_start(), z tą różnicą,
 * że działa na wątkach Win32. Jest dostępna wyłącznie przy kompilacji dla
 * systemu Windows.
 *
 * \param fd Wskaźnik na zmienną, gdzie zostanie umieszczony deskryptor gniazda
 * \param priv_data Wskaźnik na zmienną, gdzie zostanie umieszczony wskaźnik
 *                  do prywatnych danych wątku rozwiązującego nazwę
 * \param hostname Nazwa serwera do rozwiązania
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 */
static int gg_resolver_win32_start(int *fd, void **priv_data, const char *hostname)
{
	struct gg_resolver_win32_data *data = NULL;
	int pipes[2], new_errno;
	CRITICAL_SECTION *mutex = NULL;

	gg_debug(GG_DEBUG_FUNCTION, "** gg_resolver_win32_start(%p, %p, \"%s\");\n", fd, priv_data, hostname);

	if (fd == NULL || priv_data == NULL || hostname == NULL) {
		gg_debug(GG_DEBUG_MISC, "// gg_resolver_win32_start() invalid arguments\n");
		errno = EFAULT;
		return -1;
	}

	data = malloc(sizeof(struct gg_resolver_win32_data));

	if (data == NULL) {
		gg_debug(GG_DEBUG_MISC, "// gg_resolver_win32_start() out of memory for resolver data\n");
		return -1;
	}

	data->orphan = 0;
	data->finished = 0;

	if (socketpair(AF_LOCAL, SOCK_STREAM, 0, pipes) == -1) {
		gg_debug(GG_DEBUG_MISC, "// gg_resolver_win32_start() unable to "
			"create pipes (errno=%d, %s)\n",
			errno, strerror(errno));
		free(data);
		return -1;
	}

	data->hostname = strdup(hostname);

	if (data->hostname == NULL) {
		gg_debug(GG_DEBUG_MISC, "// gg_resolver_win32_start() out of memory\n");
		new_errno = errno;
		goto cleanup;
	}

	data->wfd = pipes[1];

	mutex = &data->mutex;
	InitializeCriticalSection(mutex);

	data->thread = CreateThread(NULL, 0, gg_resolver_win32_thread, data, 0, NULL);
	if (!data->thread) {
		gg_debug(GG_DEBUG_MISC, "// gg_resolver_win32_start() unable to create thread\n");
		new_errno = errno;
		goto cleanup;
	}

	gg_debug(GG_DEBUG_MISC, "// gg_resolver_win32_start() %p\n", data);

	*fd = pipes[0];
	*priv_data = data;

	return 0;

cleanup:
	if (data) {
		free(data->hostname);
		free(data);
	}

	close(pipes[0]);
	close(pipes[1]);

	if (mutex)
		DeleteCriticalSection(mutex);

	errno = new_errno;

	return -1;
}

/**
 * \internal Usuwanie zasobów po wątku rozwiązywaniu nazwy.
 *
 * Funkcja wywoływana po zakończeniu rozwiązanywania nazwy lub przy zwalnianiu
 * zasobów sesji podczas rozwiązywania nazwy.
 *
 * \param priv_data Wskaźnik na zmienną przechowującą wskaźnik do prywatnych
 *                  danych
 * \param force Flaga usuwania zasobów przed zakończeniem działania
 */
static void gg_resolver_win32_cleanup(void **priv_data, int force)
{
	struct gg_resolver_win32_data *data;

	if (priv_data == NULL || *priv_data == NULL)
		return;

	data = (struct gg_resolver_win32_data *) *priv_data;
	*priv_data = NULL;

	if (WaitForSingleObject(data->thread, 0) == WAIT_TIMEOUT) {
		int finished;
		/* We cannot call TerminateThread here - it doesn't
		 * release critical section locks (see MSDN docs).
		 * if (force) TerminateThread(data->thread, 0);
		 */
		EnterCriticalSection(&data->mutex);
		finished = data->finished;
		if (!finished)
			data->orphan = 1;
		LeaveCriticalSection(&data->mutex);
		if (!finished)
			return;
	}

	CloseHandle(data->thread);
	DeleteCriticalSection(&data->mutex);
	close(data->wfd);
	free(data->hostname);
	free(data);
}

#endif /* _WIN32 */

/**
 * Ustawia sposób rozwiązywania nazw w sesji.
 *
 * \param gs Struktura sesji
 * \param type Sposób rozwiązywania nazw (patrz \ref build-resolver)
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 */
int gg_session_set_resolver(struct gg_session *gs, gg_resolver_t type)
{
	GG_SESSION_CHECK(gs, -1);

	if (type == GG_RESOLVER_DEFAULT) {
		if (gg_global_resolver_type != GG_RESOLVER_DEFAULT) {
			gs->resolver_type = gg_global_resolver_type;
			gs->resolver_start = gg_global_resolver_start;
			gs->resolver_cleanup = gg_global_resolver_cleanup;
			return 0;
		}

#ifdef _WIN32
		type = GG_RESOLVER_WIN32;
#elif defined(GG_CONFIG_HAVE_PTHREAD)
		type = GG_RESOLVER_PTHREAD;
#elif defined(GG_CONFIG_HAVE_FORK)
		type = GG_RESOLVER_FORK;
#endif
	}

	switch (type) {
#ifdef GG_CONFIG_HAVE_FORK
		case GG_RESOLVER_FORK:
			gs->resolver_type = type;
			gs->resolver_start = gg_resolver_fork_start;
			gs->resolver_cleanup = gg_resolver_fork_cleanup;
			return 0;
#endif

#ifdef _WIN32
		case GG_RESOLVER_WIN32:
			gs->resolver_type = type;
			gs->resolver_start = gg_resolver_win32_start;
			gs->resolver_cleanup = gg_resolver_win32_cleanup;
			return 0;
#endif

		default:
			errno = EINVAL;
			return -1;
	}
}

/**
 * Zwraca sposób rozwiązywania nazw w sesji.
 *
 * \param gs Struktura sesji
 *
 * \return Sposób rozwiązywania nazw
 */
gg_resolver_t gg_session_get_resolver(struct gg_session *gs)
{
	GG_SESSION_CHECK(gs, (gg_resolver_t) -1);

	return gs->resolver_type;
}

/**
 * Ustawia własny sposób rozwiązywania nazw w sesji.
 *
 * \param gs Struktura sesji
 * \param resolver_start Funkcja rozpoczynająca rozwiązywanie nazwy
 * \param resolver_cleanup Funkcja zwalniająca zasoby
 *
 * Parametry funkcji rozpoczynającej rozwiązywanie nazwy wyglądają następująco:
 *  - \c "int *fd" &mdash; wskaźnik na zmienną, gdzie zostanie umieszczony deskryptor gniazda
 *  - \c "void **priv_data" &mdash; wskaźnik na zmienną, gdzie można umieścić
 *    wskaźnik do prywatnych danych na potrzeby rozwiązywania nazwy
 *  - \c "const char *name" &mdash; nazwa serwera do rozwiązania
 *
 * Parametry funkcji zwalniającej zasoby wyglądają następująco:
 *  - \c "void **priv_data" &mdash; wskaźnik na zmienną przechowującą wskaźnik
 *    do prywatnych danych, należy go ustawić na \c NULL po zakończeniu
 *  - \c "int force" &mdash; flaga mówiąca o tym, że zasoby są zwalniane przed
 *    zakończeniem rozwiązywania nazwy, np. z powodu zamknięcia sesji.
 *
 * Własny kod rozwiązywania nazwy powinien stworzyć potok, parę gniazd lub
 * inny deskryptor pozwalający na co najmniej odbiór danych i przekazać go
 * w parametrze \c fd. Na platformie Windows możliwe jest przekazanie jedynie
 * deskryptora gniazda. Po zakończeniu rozwiązywania nazwy powinien wysłać
 * otrzymany adres IP w postaci sieciowej (big-endian) do deskryptora. Jeśli
 * rozwiązywanie nazwy się nie powiedzie, należy wysłać \c INADDR_NONE.
 * Następnie zostanie wywołana funkcja zwalniająca zasoby z parametrem
 * \c force równym \c 0. Gdyby sesja została zakończona przed rozwiązaniem
 * nazwy, np. za pomocą funkcji \c gg_logoff(), funkcja zwalniająca zasoby
 * zostanie wywołana z parametrem \c force równym \c 1.
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 */
int gg_session_set_custom_resolver(struct gg_session *gs,
	int (*resolver_start)(int*, void**, const char*),
	void (*resolver_cleanup)(void**, int))
{
	GG_SESSION_CHECK(gs, -1);

	if (resolver_start == NULL || resolver_cleanup == NULL) {
		errno = EINVAL;
		return -1;
	}

	gs->resolver_type = GG_RESOLVER_CUSTOM;
	gs->resolver_start = resolver_start;
	gs->resolver_cleanup = resolver_cleanup;

	return 0;
}

/**
 * Ustawia sposób rozwiązywania nazw połączenia HTTP.
 *
 * \param gh Struktura połączenia
 * \param type Sposób rozwiązywania nazw (patrz \ref build-resolver)
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 */
int gg_http_set_resolver(struct gg_http *gh, gg_resolver_t type)
{
	if (gh == NULL) {
		errno = EINVAL;
		return -1;
	}

	if (type == GG_RESOLVER_DEFAULT) {
		if (gg_global_resolver_type != GG_RESOLVER_DEFAULT) {
			gh->resolver_type = gg_global_resolver_type;
			gh->resolver_start = gg_global_resolver_start;
			gh->resolver_cleanup = gg_global_resolver_cleanup;
			return 0;
		}

#ifdef _WIN32
		type = GG_RESOLVER_WIN32;
#elif defined(GG_CONFIG_HAVE_PTHREAD)
		type = GG_RESOLVER_PTHREAD;
#elif defined(GG_CONFIG_HAVE_FORK)
		type = GG_RESOLVER_FORK;
#endif
	}

	switch (type) {
#ifdef GG_CONFIG_HAVE_FORK
		case GG_RESOLVER_FORK:
			gh->resolver_type = type;
			gh->resolver_start = gg_resolver_fork_start;
			gh->resolver_cleanup = gg_resolver_fork_cleanup;
			return 0;
#endif

#ifdef _WIN32
		case GG_RESOLVER_WIN32:
			gh->resolver_type = type;
			gh->resolver_start = gg_resolver_win32_start;
			gh->resolver_cleanup = gg_resolver_win32_cleanup;
			return 0;
#endif

		default:
			errno = EINVAL;
			return -1;
	}
}

/**
 * Zwraca sposób rozwiązywania nazw połączenia HTTP.
 *
 * \param gh Struktura połączenia
 *
 * \return Sposób rozwiązywania nazw
 */
gg_resolver_t gg_http_get_resolver(struct gg_http *gh)
{
	if (gh == NULL) {
		errno = EINVAL;
		return GG_RESOLVER_INVALID;
	}

	return gh->resolver_type;
}

/**
 * Ustawia własny sposób rozwiązywania nazw połączenia HTTP.
 *
 * \param gh Struktura sesji
 * \param resolver_start Funkcja rozpoczynająca rozwiązywanie nazwy
 * \param resolver_cleanup Funkcja zwalniająca zasoby
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 */
int gg_http_set_custom_resolver(struct gg_http *gh,
	int (*resolver_start)(int*, void**, const char*),
	void (*resolver_cleanup)(void**, int))
{
	if (gh == NULL || resolver_start == NULL || resolver_cleanup == NULL) {
		errno = EINVAL;
		return -1;
	}

	gh->resolver_type = GG_RESOLVER_CUSTOM;
	gh->resolver_start = resolver_start;
	gh->resolver_cleanup = resolver_cleanup;

	return 0;
}

/**
 * Ustawia sposób rozwiązywania nazw globalnie dla biblioteki.
 *
 * \param type Sposób rozwiązywania nazw (patrz \ref build-resolver)
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 */
int gg_global_set_resolver(gg_resolver_t type)
{
	switch (type) {
		case GG_RESOLVER_DEFAULT:
			gg_global_resolver_type = type;
			gg_global_resolver_start = NULL;
			gg_global_resolver_cleanup = NULL;
			return 0;

#ifdef GG_CONFIG_HAVE_FORK
		case GG_RESOLVER_FORK:
			gg_global_resolver_type = type;
			gg_global_resolver_start = gg_resolver_fork_start;
			gg_global_resolver_cleanup = gg_resolver_fork_cleanup;
			return 0;
#endif

#ifdef _WIN32
		case GG_RESOLVER_WIN32:
			gg_global_resolver_type = type;
			gg_global_resolver_start = gg_resolver_win32_start;
			gg_global_resolver_cleanup = gg_resolver_win32_cleanup;
			return 0;
#endif

		default:
			errno = EINVAL;
			return -1;
	}
}

/**
 * Zwraca sposób rozwiązywania nazw globalnie dla biblioteki.
 *
 * \return Sposób rozwiązywania nazw
 */
gg_resolver_t gg_global_get_resolver(void)
{
	return gg_global_resolver_type;
}

/**
 * Ustawia własny sposób rozwiązywania nazw globalnie dla biblioteki.
 *
 * \param resolver_start Funkcja rozpoczynająca rozwiązywanie nazwy
 * \param resolver_cleanup Funkcja zwalniająca zasoby
 *
 * Patrz \ref gg_session_set_custom_resolver.
 *
 * \return 0 jeśli się powiodło, -1 w przypadku błędu
 */
int gg_global_set_custom_resolver(
	int (*resolver_start)(int*, void**, const char*),
	void (*resolver_cleanup)(void**, int))
{
	if (resolver_start == NULL || resolver_cleanup == NULL) {
		errno = EINVAL;
		return -1;
	}

	gg_global_resolver_type = GG_RESOLVER_CUSTOM;
	gg_global_resolver_start = resolver_start;
	gg_global_resolver_cleanup = resolver_cleanup;

	return 0;
}

/**
 * Odczytuje dane z procesu/wątku rozwiązywania nazw.
 *
 * \param fd Deskryptor
 * \param buf Wskaźnik na bufor
 * \param len Długość bufora
 *
 * \return Patrz recv() i read().
 */
int gg_resolver_recv(int fd, void *buf, size_t len)
{
#ifndef _WIN32
	return read(fd, buf, len);
#else
	return recv(fd, buf, len, 0);
#endif
}
