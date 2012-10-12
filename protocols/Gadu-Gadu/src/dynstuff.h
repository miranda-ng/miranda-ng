/* $Id: dynstuff.h 4366 2006-12-20 22:40:37Z ono $ */

/*
 *	(C) Copyright 2001-2002 Wojtek Kaniewski <wojtekka@irc.pl>
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

#ifndef __DYNSTUFF_H
#define __DYNSTUFF_H

/* listy */

struct list {
	void *data;
	struct list *next;
};

typedef struct list * list_t;

void *list_add(list_t *list, void *data, int alloc_size);
void *list_add_sorted(list_t *list, void *data, int alloc_size, int (*comparision)(void *, void *));
int list_remove(list_t *list, void *data, int free_data);
int list_count(list_t list);
int list_destroy(list_t list, int free_data);

/* stringi */

struct string {
	char *str;
	int len, size;
};

typedef struct string * string_t;

string_t string_init(const char *str);
int string_append(string_t s, const char *str);
int string_append_n(string_t s, const char *str, int count);
int string_append_c(string_t s, char ch);
void string_insert(string_t s, int index, const char *str);
void string_insert_n(string_t s, int index, const char *str, int count);
void string_clear(string_t s);
char *string_free(string_t s, int free_string);

/* tablice stringów */

char **array_make(const char *string, const char *sep, int max, int trim, int quotes);
char *array_join(char **array, const char *sep);
void array_add(char ***array, char *string);
int array_count(char **array);
int array_contains(char **array, const char *string, int casesensitive);
void array_free(char **array);

/* rozszerzenia libców */

const char *ditoa(long int i);

#endif /* __DYNSTUFF_H */
