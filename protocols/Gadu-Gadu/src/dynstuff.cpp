/* $Id: dynstuff.c 11259 2010-02-17 04:47:22Z borkra $ */

/*
 *	(C) Copyright 2001-2003 Wojtek Kaniewski <wojtekka@irc.pl>
 *							Dawid Jarosz <dawjar@poczta.onet.pl>
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License Version 2 as
 *	published by the Free Software Foundation.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "gg.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * list_add_sorted()
 *
 * dodaje do listy dany element. przy okazji mo�e te� skopiowa� zawarto��.
 * je�li poda si� jako ostatni parametr funkcj� por�wnuj�c� zawarto��
 * element�w, mo�e posortowa� od razu.
 *
 *	- list - wska�nik do listy,
 *	- data - wska�nik do elementu,
 *	- alloc_size - rozmiar elementu, je�li chcemy go skopiowa�.
 *
 * zwraca wska�nik zaalokowanego elementu lub NULL w przpadku b��du.
 */
void *list_add_sorted(list_t *list, void *data, int alloc_size, int (*comparision)(void *, void *))
{
	if (!list) {
		errno = EFAULT;
		return NULL;
	}

	list_t newlist = (list_t)malloc(sizeof(struct list));

	newlist->data = data;
	newlist->next = NULL;

	if (alloc_size) {
		newlist->data = malloc(alloc_size);
		memcpy(newlist->data, data, alloc_size);
	}

	list_t tmp;
	if (!(tmp = *list)) {
		*list = newlist;
	} else {
		if (!comparision) {
			while (tmp->next)
				tmp = tmp->next;
			tmp->next = newlist;
		} else {
			list_t prev = NULL;

			while (comparision(newlist->data, tmp->data) > 0) {
				prev = tmp;
				tmp = tmp->next;
				if (!tmp)
					break;
			}

			if (!prev) {
				tmp = *list;
				*list = newlist;
				newlist->next = tmp;
			} else {
				prev->next = newlist;
				newlist->next = tmp;
			}
		}
	}

	return newlist->data;
}

/*
 * list_add()
 *
 * wrapper do list_add_sorted(), kt�ry zachowuje poprzedni� sk�adni�.
 */
void *list_add(list_t *list, void *data, int alloc_size)
{
	return list_add_sorted(list, data, alloc_size, NULL);
}

/*
 * list_remove()
 *
 * usuwa z listy wpis z podanym elementem.
 *
 *	- list - wska�nik do listy,
 *	- data - element,
 *	- free_data - zwolni� pami�� po elemencie.
 */
int list_remove(list_t *list, void *data, int free_data)
{
	list_t tmp, last = NULL;

	if (!list || !*list) {
		errno = EFAULT;
		return -1;
	}

	tmp = *list;
	if (tmp->data == data) {
		*list = tmp->next;
	} else {
		for (; tmp && tmp->data != data; tmp = tmp->next)
			last = tmp;
		if (!tmp) {
			errno = ENOENT;
			return -1;
		}
		last->next = tmp->next;
	}

	if (free_data)
		free(tmp->data);
	free(tmp);

	return 0;
}

/*
 * list_count()
 *
 * zwraca ilo�� element�w w danej li�cie.
 *
 *	- list - lista.
 */
int list_count(list_t list)
{
	int count = 0;

	for (; list; list = list->next)
		count++;

	return count;
}

/*
 * list_destroy()
 *
 * niszczy wszystkie elementy listy.
 *
 *	- list - lista,
 *	- free_data - czy zwalnia� bufor danych?
 */
int list_destroy(list_t list, int free_data)
{
	list_t tmp;

	while (list) {
		if (free_data)
			free(list->data);

		tmp = list->next;

		free(list);

		list = tmp;
	}

	return 0;
}

/*
 * string_realloc()
 *
 * upewnia si�, �e w stringu b�dzie wystarczaj�co du�o miejsca.
 *
 *	- s - ci�g znak�w,
 *	- count - wymagana ilo�� znak�w (bez ko�cowego '\0').
 */
static void string_realloc(string_t s, int count)
{
	char *tmp;

	if (s->str && count + 1 <= s->size)
		return;

	tmp = (char*)realloc(s->str, count + 81);
	if (!s->str)
		*tmp = 0;
	tmp[count + 80] = 0;
	s->size = count + 81;
	s->str = tmp;
}

/*
 * string_append_c()
 *
 * dodaje do danego ci�gu jeden znak, alokuj�c przy tym odpowiedni� ilo��
 * pami�ci.
 *
 *	- s - ci�g znak�w.
 *	- c - znaczek do dopisania.
 */
int string_append_c(string_t s, char c)
{
	if (!s) {
		errno = EFAULT;
		return -1;
	}

	string_realloc(s, s->len + 1);

	s->str[s->len + 1] = 0;
	s->str[s->len++] = c;

	return 0;
}

/*
 * string_append_n()
 *
 * dodaje tekst do bufora alokuj�c odpowiedni� ilo�� pami�ci.
 *
 *	- s - ci�g znak�w,
 *	- str - tekst do dopisania,
 *	- count - ile znak�w tego tekstu dopisa�? (-1 znaczy, �e ca�y).
 */
int string_append_n(string_t s, const char *str, int count)
{
	if (!s || !str) {
		errno = EFAULT;
		return -1;
	}

	if (count == -1)
		count = (int)mir_strlen(str);

	string_realloc(s, s->len + count);

	s->str[s->len + count] = 0;
	strncpy(s->str + s->len, str, count);

	s->len += count;

	return 0;
}

int string_append(string_t s, const char *str)
{
	return string_append_n(s, str, -1);
}

/*
 * string_insert_n()
 *
 * wstawia tekst w podane miejsce bufora.
 *
 *	- s - ci�g znak�w,
 *	- index - miejsce, gdzie mamy wpisa� (liczone od 0),
 *	- str - tekst do dopisania,
 *	- count - ilo�� znak�w do dopisania (-1 znaczy, �e wszystkie).
 */
void string_insert_n(string_t s, int index, const char *str, int count)
{
	if (!s || !str)
		return;

	if (count == -1)
		count = (int)mir_strlen(str);

	if (index > s->len)
		index = s->len;

	string_realloc(s, s->len + count);

	memmove(s->str + index + count, s->str + index, s->len + 1 - index);
	memmove(s->str + index, str, count);

	s->len += count;
}

void string_insert(string_t s, int index, const char *str)
{
	string_insert_n(s, index, str, -1);
}

/*
 * string_init()
 *
 * inicjuje struktur� string. alokuje pami�� i przypisuje pierwsz� warto��.
 *
 *	- value - je�li NULL, ci�g jest pusty, inaczej kopiuje tam.
 *
 * zwraca zaalokowan� struktur� `string'.
 */
string_t string_init(const char *value)
{
	string_t tmp = (string_t)malloc(sizeof(struct string));

	if (!value)
		value = "";

	tmp->str = _strdup(value);
	tmp->len = (int)mir_strlen(value);
	tmp->size = (int)mir_strlen(value) + 1;

	return tmp;
}

/*
 * string_clear()
 *
 * czy�ci zawarto�� struktury `string'.
 *
 *	- s - ci�g znak�w.
 */
void string_clear(string_t s)
{
	if (!s)
		return;

	if (s->size > 160) {
		s->str = (char*)realloc(s->str, 80);
		s->size = 80;
	}

	s->str[0] = 0;
	s->len = 0;
}

/*
 * string_free()
 *
 * zwalnia pami�� po strukturze string i mo�e te� zwolni� pami�� po samym
 * ci�gu znak�w.
 *
 *	- s - struktura, kt�r� wycinamy,
 *	- free_string - zwolni� pami�� po ci�gu znak�w?
 *
 * je�li free_string=0 zwraca wska�nik do ci�gu, inaczej NULL.
 */
char *string_free(string_t s, int free_string)
{
	char *tmp = NULL;

	if (!s)
		return NULL;

	if (free_string)
		free(s->str);
	else
		tmp = s->str;

	free(s);

	return tmp;
}

/*
 * _itoa()
 *
 * prosta funkcja, kt�ra zwraca tekstow� reprezentacj� liczby. w obr�bie
 * danego wywo�ania jakiej� funkcji lub wyra�enia mo�e by� wywo�ania 10
 * razy, poniewa� tyle mamy statycznych bufor�w. lepsze to ni� ci�g�e
 * tworzenie tymczasowych bufor�w na stosie i sprintf()owanie.
 *
 *	- i - liczba do zamiany.
 *
 * zwraca adres do bufora, kt�rego _NIE_NALE�Y_ zwalnia�.
 */

const char *ditoa(long int i)
{
	static char bufs[10][16];
	static int index = 0;
	char *tmp = bufs[index++];

	if (index > 9)
		index = 0;

	mir_snprintf(tmp, 16, "%ld", i);
	return tmp;
}

/*
 * array_make()
 *
 * tworzy tablic� tekst�w z jednego, rozdzielonego podanymi znakami.
 *
 *	- string - tekst wej�ciowy,
 *	- sep - lista element�w oddzielaj�cych,
 *	- max - maksymalna ilo�� element�w tablicy. je�li r�wne 0, nie ma
 *			ogranicze� rozmiaru tablicy.
 *	- trim - czy wi�ksz� ilo�� element�w oddzielaj�cych traktowa� jako
 *			 jeden (na przyk�ad spacje, tabulacja itp.)
 *	- quotes - czy pola mog� by� zapisywane w cudzys�owiach lub
 *			   apostrofach z escapowanymi znakami.
 *
 * zaalokowan� tablic� z zaalokowanymi ci�gami znak�w, kt�r� nale�y
 * zwolni� funkcj� array_free()
 */
char **array_make(const char *string, const char *sep, int max, int trim, int quotes)
{
	const char *p, *q;
	char **result = NULL;
	int items = 0, last = 0;

	if (!string || !sep)
		goto failure;

	for (p = string; ; ) {
		int len = 0;
		char *token = NULL;

		if (max && items >= max - 1)
			last = 1;

		if (trim) {
			while (*p && strchr(sep, *p))
				p++;
			if (!*p)
				break;
		}

		if (!last && quotes && (*p == '\'' || *p == '\"')) {
			char sep = *p;

			for (q = p + 1, len = 0; *q; q++, len++) {
				if (*q == '\\') {
					q++;
					if (!*q)
						break;
				} else if (*q == sep)
					break;
			}

			if ((token = (char*)calloc(1, len + 1))) {
				char *r = token;

				for (q = p + 1; *q; q++, r++) {
					if (*q == '\\') {
						q++;

						if (!*q)
							break;

						switch (*q) {
							case 'n':
								*r = '\n';
								break;
							case 'r':
								*r = '\r';
								break;
							case 't':
								*r = '\t';
								break;
							default:
								*r = *q;
						}
					} else if (*q == sep) {
						break;
					} else
						*r = *q;
				}

				*r = 0;
			}

			p = (*q) ? q + 1 : q;

		} else {
			for (q = p, len = 0; *q && (last || !strchr(sep, *q)); q++, len++);
			token = (char*)calloc(1, len + 1);
			strncpy(token, p, len);
			token[len] = 0;
			p = q;
		}

		result = (char**)realloc(result, (items + 2) * sizeof(char*));
		result[items] = token;
		result[++items] = NULL;

		if (!*p)
			break;

		p++;
	}

failure:
	if (!items)
		result = (char**)calloc(1, sizeof(char*));

	return result;
}

/*
 * array_count()
 *
 * zwraca ilo�� element�w tablicy.
 */
int array_count(char **array)
{
	int result = 0;

	if (!array)
		return 0;

	while (*array) {
		result++;
		array++;
	}

	return result;
}

/*
 * array_add()
 *
 * dodaje element do tablicy.
 */
void array_add(char ***array, char *string)
{
	int count = array_count(*array);

	*array = (char**)realloc(*array, (count + 2) * sizeof(char*));
	(*array)[count + 1] = NULL;
	(*array)[count] = string;
}

/*
 * array_join()
 *
 * ��czy elementy tablicy w jeden string oddzielaj�c elementy odpowiednim
 * separatorem.
 *
 *	- array - wska�nik do tablicy,
 *	- sep - seperator.
 *
 * zwr�cony ci�g znak�w nale�y zwolni�.
 */
char *array_join(char **array, const char *sep)
{
	if (!array)
		return _strdup("");

	string_t s = string_init(NULL);
	for (int i = 0; array[i]; i++) {
		if (i)
			string_append(s, sep);

		string_append(s, array[i]);
	}

	return string_free(s, 0);
}

/*
 * array_contains()
 *
 * stwierdza, czy tablica zawiera podany element.
 *
 *	- array - tablica,
 *	- string - szukany ci�g znak�w,
 *	- casesensitive - czy mamy zwraca� uwag� na wielko�� znak�w?
 *
 * 0/1
 */
int array_contains(char **array, const char *string, int casesensitive)
{
	int i;

	if (!array || !string)
		return 0;

	for (i = 0; array[i]; i++) {
		if (casesensitive && !mir_strcmp(array[i], string))
			return 1;
		if (!casesensitive && !strcasecmp(array[i], string))
			return 1;
	}

	return 0;
}

/*
 * array_free()
 *
 * zwalnia pamie� zajmowan� przez tablic�.
 */
void array_free(char **array)
{
	char **tmp;

	if (!array)
		return;

	for (tmp = array; *tmp; tmp++)
		free(*tmp);

	free(array);
}
