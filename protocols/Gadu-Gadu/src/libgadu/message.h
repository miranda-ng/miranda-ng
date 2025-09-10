/*
 *  (C) Copyright 2009 Wojtek Kaniewski <wojtekka@irc.pl>
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

#ifndef LIBGADU_MESSAGE_H
#define LIBGADU_MESSAGE_H

#include <sys/types.h>
#include "libgadu.h"

#if 0

struct gg_message {
	uin_t *recipients;
	size_t recipient_count;
	char *text;
	char *html;
	char *attributes;
	size_t attributes_length;
	uint32_t msgclass;
	uint32_t seq;

	int auto_convert;
	char *text_converted;
	char *html_converted;
};

#define GG_MESSAGE_CHECK(gm, result) \
	if ((gm) == NULL) { \
		errno = EINVAL; \
		return (result); \
	}

int gg_message_init(gg_message_t *gm, int msgclass, int seq, uin_t *recipients,
	size_t recipient_count, char *text, char *xhtml, char *attributes,
	size_t attributes_length, int auto_convert);

#endif

size_t gg_message_html_to_text(char *dst, unsigned char *format,
	size_t *format_len, const char *html, gg_encoding_t encoding);
size_t gg_message_text_to_html(char *dst, const char *src,
	gg_encoding_t encoding, const unsigned char *format, size_t format_len);

char * gg_message_html_to_text_110(const char *html);
char * gg_message_text_to_html_110(const char *text, ssize_t text_len);

#endif /* LIBGADU_MESSAGE_H */
